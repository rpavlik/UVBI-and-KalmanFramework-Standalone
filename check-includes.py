#!/usr/bin/env python3

import json
import re
import subprocess
import sys
from itertools import chain
from pathlib import Path
from pprint import pprint

import networkx as nx
from networkx.readwrite import json_graph


class FileMigrationBase:
    def __init__(self, parse_dirs, extra_search_dirs, include_dirs):
        self.graph = nx.DiGraph()
        self.include_re = re.compile(
            r'^ *# *include *(?P<quoted_name>(?P<open>["<])(?P<name>[^">]*)[">])')
        self.parse_dirs = set(parse_dirs)
        self.all_dirs = self.parse_dirs.union(extra_search_dirs)
        self.include_dirs = include_dirs
        for d in self.parse_dirs:
            self.parse_dir(d)

    def find_in_dir_list(self, rel_path, dirs=None, try_drop_leading_path=False):
        if dirs is None:
            dirs = self.all_dirs
        candidates = (d/rel_path for d in dirs)
        found = [p for p in candidates if p.is_file()]
        if not found:
            # Check to see if there's a leading directory we can strip
            rel_path = Path(rel_path)
            if try_drop_leading_path and rel_path != rel_path.name:
                return self.find_in_dir_list(rel_path.name, dirs=dirs)
            return rel_path, False

        if len(found) > 1:
            raise RuntimeError("Got more than one resolution:", found)

        return found[0], True

    def file_known(self, resolved_path):
        return self.graph.nodes[str(resolved_path)].get('known', False)

    def process_file(self, fn):
        """Add a file to the graph, and read and parse its includes into the graph."""
        if not fn.is_file():
            return
        G = self.graph
        current_file_node = str(fn)
        G.add_node(current_file_node)
        G.nodes[current_file_node]['known'] = True
        with open(str(fn), 'r', encoding='utf') as fp:
            for line in fp:
                line = line.rstrip()
                match = self.include_re.match(line)
                if not match:
                    continue

                include_name = match.group('name')
                path, resolved = self.find_in_dir_list(
                    include_name, try_drop_leading_path=True)
                if not resolved:
                    continue

                angle_include = (match.group('open') == "<")
                node_name = str(path)
                G.add_edge(current_file_node, node_name,
                           include_path=include_name,
                           angle_include=angle_include)

    def parse_dir(self, d, **kwargs):
        """Process all source and header files in a directory."""
        for fn in d.glob('*.h'):
            self.process_file(fn, **kwargs)
        for fn in d.glob('*.cpp'):
            self.process_file(fn, **kwargs)

    def make_fn_absolute(self, fn):
        """Make a file absolute by searching by name."""
        fn = Path(fn)
        if not fn.is_absolute:
            new_fn, resolved = self.find_in_dir_list(fn)
            if not resolved:
                raise RuntimeError("Could not find file " + str(new_fn))
            fn = new_fn
        return str(fn)

    def get_transitive_includers(self, fn):
        """Get a list of all files that (transitively) include fn."""
        fn = self.make_fn_absolute(fn)
        if fn not in self.graph:
            raise RuntimeError(str(fn) + " not in graph")
        return list(nx.dfs_preorder_nodes(nx.reversed(self.graph), fn))

    def get_transitive_includes(self, fn):
        """Get a generator of all files transitively included by fn."""
        fn = self.make_fn_absolute(fn)
        if fn not in self.graph:
            raise RuntimeError(str(fn) + " not in graph")
        return nx.dfs_preorder_nodes(self.graph, fn)

    def get_transitive_includes_for_all(self, collection):
        """Return a set including all members of collection
        and all files transitively included by them."""
        ret = set(collection)
        for fn in collection:
            includes = set(self.get_transitive_includes(fn))
            ret = ret.union(includes)
        return ret

    def get_include_search_path_for(self, includer, angle_include=False):
        search_path = []
        if not angle_include:
            includer = Path(includer)
            assert(includer.is_absolute())
            includer_dir = includer.parent
            search_path = [includer_dir]
        search_path.extend(self.include_dirs)
        return search_path

    def resolve_include(self, includer, path_included, angle_include=False):
        search_path = self.get_include_search_path_for(
            includer, angle_include=angle_include)
        full_include, resolved = self.find_in_dir_list(
            path_included, search_path)
        if not resolved:
            return None
        return full_include

    def make_most_concise_include(self, includer, resolved_include, angle_include=False):
        resolved_include = Path(resolved_include)
        assert(resolved_include.is_absolute())
        search_path = self.get_include_search_path_for(
            includer, angle_include=angle_include)
        for d in search_path:
            try:
                relative = resolved_include.relative_to(d)
                return str(relative)
            except ValueError:
                continue
        return None


class UVBIFileMigration(FileMigrationBase):
    def __init__(self, root):
        self.root = Path(root).resolve()
        self.inc = self.root / 'inc'
        self.inc_dir_name = str(self.inc)
        self.src = self.root / 'src'
        modules = (
            'FlexKalman',
            'videotrackershared',
            Path('videotrackershared')/'ImageSources',
            'unifiedvideoinertial'
        )
        dirs_to_process = [self.inc / m for m in modules]
        dirs_to_process.extend((self.src / m for m in modules))
        super().__init__(parse_dirs=dirs_to_process, extra_search_dirs=(
            self.inc, self.src), include_dirs=(self.inc,))

    def is_public_header(self, fn):
        return str(fn).startswith(self.inc_dir_name)

    def get_public_headers(self):
        return (f for f in self.graph if self.is_public_header(f))

    def make_headers_public(self, headers):
        """Move these headers from the source directory to the public include directory."""
        for fn in headers:
            fn = Path(fn)
            relative = fn.relative_to(self.src)
            dest = self.inc / relative
            print("Moving", self.shorten(fn), "to", self.shorten(dest))
            fn.rename(dest)

    def fix_public_transitive_headers(self):
        """Make sure all transitive includes of public header are also public."""
        public_headers = set(self.get_public_headers())
        print("Public headers:", len(public_headers))
        transitive = self.get_transitive_includes_for_all(public_headers)
        print("Transitive includes of public headers:", len(transitive))
        must_move = transitive.difference(public_headers)
        if not must_move:
            # Don't need to move anything
            return False
        self.make_headers_public(must_move)
        return True

    def shorten(self, fn):
        try:
            return str(Path(fn).relative_to(self.root))
        except ValueError:
            return str(fn)

    def modify_include(self, fn, old_include, new_include):
        pattern = 's:{}:{}:'.format(
            # Escape periods
            old_include.replace(".", r"\."),
            new_include)
        cmd = ['sed', '-i', pattern, str(fn)]
        print("Fixing include in", self.shorten(fn),
              "by applying substitution:", pattern)
        subprocess.check_call(cmd)

    def fix_includes(self):
        """Correct and simplify include lines, and find files that should be public headers."""
        G = self.graph
        possible_public = set()
        include_fixes = []
        for includer, included, path_used in G.edges(data='include_path'):
            if not self.file_known(included):
                # Skip system/external dep includes
                continue

            angle_include = G.edges[includer, included]['angle_include']

            # Make sure includes can be found,
            # and shorten up quote includes
            best_include = self.make_most_concise_include(
                includer,
                included,
                angle_include=angle_include
            )
            if not best_include:
                print("Couldn't figure out how to refer to",
                      self.shorten(included), "when including from", self.shorten(includer))
                possible_public.add(included)
                continue

            # Can only check validity of angle includes, not shorten them.
            if angle_include:
                continue
            if best_include != path_used:
                include_fixes.append((includer, path_used, best_include))

        # If there are headers to make public, do that first
        # and skip include fixes, since they'll be different after.
        if possible_public:
            print("These headers might need to be made public:")
            for fn in possible_public:
                print("  -", self.shorten(fn))
            return True, possible_public

        # Only include fixes - do them.
        if include_fixes:
            for includer, path_used, correct_include in include_fixes:
                self.modify_include(includer, path_used, correct_include)
            return True, None

        # No changes
        return False, None


ROOT = Path(__file__).resolve().parent


# G = migration.graph

# with open('includes.json', 'w', encoding='utf-8') as fp:
#     json.dump(json_graph.node_link_data(migration.graph), fp, indent=4)

while True:
    migration = UVBIFileMigration(ROOT)

    print("Ensuring includes of public headers are public...")
    if migration.fix_public_transitive_headers():
        print("Some headers moved!")
        continue
        # migration = UVBIFileMigration(ROOT)

    print("Correcting include lines")
    changes, possible_public = migration.fix_includes()
    if not changes:
        print("Processing done!")
        break

    if possible_public:
        print("Moving those headers to become public")
        migration.make_headers_public(possible_public)
