#!/usr/bin/env python3

from itertools import chain
from pprint import pprint
from pathlib import Path
import subprocess
import networkx as nx
from networkx.readwrite import json_graph
import json
import re

INCLUDE_RE = re.compile(r'#include "(?P<file>[^/]*)"')

ROOT = Path(__file__).resolve().parent

INC = ROOT / 'inc'
SRC = ROOT / 'src'

ALL_DIRS = (
    INC,
    INC/'FlexKalman',
    INC/'videotrackershared',
    INC/'videotrackershared'/'ImageSources',
    INC/'unifiedvideoinertial',
    SRC/'videotrackershared',
    SRC/'videotrackershared'/'ImageSources',
    SRC/'unifiedvideoinertial',
)


def find_in_dir_list(rel_path, dirs):
    candidates = (d/rel_path for d in dirs)
    found = [p for p in candidates if p.is_file()]
    if not found:
        return rel_path, False
    if len(found) > 1:
        raise RuntimeError("Got more than one resolution:", found)
    return found[0], True


class FileMigration:
    def __init__(self):
        self.graph = nx.DiGraph()
        self.include_re = re.compile(
            r'^ *# *include *(?P<quoted_name>(?P<open>["<])(?P<name>[^">]*)[">])')

    def file_known(self, resolved_path):
        return self.graph.nodes[str(resolved_path)].get('known', False)

    def process_file(self, fn, public=False):
        if not fn.is_file():
            return
        G = self.graph
        name = fn.name
        current_file_node = str(fn)
        G.add_node(current_file_node,
                   path=str(fn),
                   public=public,
                   known=True)
        with open(str(fn), 'r', encoding='utf') as fp:
            for line in fp:
                line = line.rstrip()
                match = self.include_re.match(line)
                if not match:
                    continue

                include_name = match.group('name')
                path, resolved = find_in_dir_list(include_name, ALL_DIRS)
                if not resolved:
                    continue
                angle_include = (match.group('open') == "<")
                sys_include = (angle_include and include_name not in G)

                # if sys_include:
                #     continue
                #     node_name = include_name
                #     G.add_node(node_name,
                #                system_header=True)
                # else:
                #     node_name = Path(include_name).name
                node_name = str(path)
                G.add_edge(current_file_node, node_name,
                           include_path=include_name,
                           angle_include=angle_include)

    def parse_dir(self, d, **kwargs):
        for fn in d.glob('*.h'):
            self.process_file(fn, **kwargs)
        for fn in d.glob('*.cpp'):
            self.process_file(fn, **kwargs)


migration = FileMigration()
migration.parse_dir(INC/'FlexKalman', public=True)
migration.parse_dir(INC/'videotrackershared', public=True)
migration.parse_dir(INC/'videotrackershared'/'ImageSources', public=True)
migration.parse_dir(INC/'unifiedvideoinertial', public=True)
migration.parse_dir(SRC/'videotrackershared')
migration.parse_dir(SRC/'videotrackershared'/'ImageSources')
migration.parse_dir(SRC/'unifiedvideoinertial')

INCLUDE_DIRS = (Path(INC))
G = migration.graph

moves = []

# for filename in G:
#     if G.nodes[filename]['system_header']:
#         continue
#     print(filename, list(G.successors(filename)))

# pprint(json_graph.node_link_data(G))

with open('includes.json', 'w', encoding='utf-8') as fp:
    json.dump(json_graph.node_link_data(G), fp, indent=4)

public_headers = set((f for f in G if G.node[f].get('public')))
# print(public_headers)
print(len(public_headers))


def find_transitive_included(headers):
    ret = set(headers)
    for h in headers:
        ret = ret.union(set(nx.dfs_successors(G, h)))
    return ret


transitive = find_transitive_included(public_headers)
print(len(transitive))
must_move = set(x for x in transitive if x not in public_headers)
# for h in transitive:
#     print(h, G.node[h].get('public'))
# # print(list(transitive))
# must_move = (h for h in transitive if not G.node[h].get('public'))
print(list(must_move))

for includer, included, path_used in G.edges(data='include_path'):
    if not migration.file_known(included):
        continue
    includer_attrs = G.nodes[includer]
    included_attrs = G.nodes[included]
#     if included_attrs.get('system_header'):
#         continue
#     if 'path' not in included_attrs:
#         print("Include issue:",
#               G.nodes[includer]['path'], "tries to include", path_used,
#               "but not resolved")

    includer_dir = Path(includer_attrs['path']).parent
    _, resolved = find_in_dir_list(path_used, (includer_dir, INC))
    if not resolved:
        print("sed -i {} 's/{}/{}/'".format(includer, path_used, Path(included).relative_to(INC)))
        # print(includer, "includes", path_used,
        #       "but should include", Path(included).relative_to(INC))
#     found = [(d/path_used).is_file() for d in (includer_dir, INC)]
#     local_include = includer_dir / path_used
#     common_include = INC / path_used
#     if local_include.is_file():
#         # it's fine
#         continue
#     if common_include.is_file():
#         # it's fine
#         continue

#     if 'path' not in included_attrs:

#         print("Include issue:",
#               includer_attrs['path'],
#               "tries to include", included)
#         continue
#     included_path = included_attrs['path']
#     if includer_attrs['public'] and not included_attrs['public']:
#         # Must make public
#         move = (
#             included_path,
#             INC / (included_path.relative_to(SRC))
#         )
#         print("move:", move)
#         # moves.append()
