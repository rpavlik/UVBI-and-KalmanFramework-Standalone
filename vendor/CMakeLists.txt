
if(BUILD_TESTING)
    # Will search for a system install first.
    add_subdirectory(catch-single-header-2.8.0)
endif()

# optional-lite 3.2.0 (std::optional wrapper/backport) and
# variant-lite 1.2.2 (std::variant wrapper/backport)
# included directly in inc/unifiedvideoinertial

# Interface target for util headers
add_library(util-headers INTERFACE)
target_include_directories(util-headers INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/util-headers")

# Interface target for the header-only functionality in Folly
add_library(folly-headers INTERFACE)
target_include_directories(folly-headers INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/folly")
target_compile_definitions(folly-headers INTERFACE FOLLY_NO_CONFIG) # we don't build a folly/folly-config.h

# Interface target for TypePack
add_library(typepack-headers INTERFACE)
target_include_directories(typepack-headers INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/typepack")

# Interface target for intrusive_ptr support for COM
add_library(com_smart_pointer INTERFACE)
target_include_directories(com_smart_pointer INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/com_smart_pointer" ${Boost_INCLUDE_DIRS})

if(WIN32)
    add_library(comutils-interface INTERFACE)
    target_link_libraries(comutils-interface INTERFACE com_smart_pointer)
    target_include_directories(comutils-interface INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}")
    target_compile_options(comutils-interface INTERFACE "${OSVR_CXX11_FLAGS}")

    # Add an "OBJECT" library type, to include directly in
    # static libraries. Note that if you use $<TARGET_OBJECTS:comutils-obj>
    # in your source list, you'll also want to link to comutils-interface to
    # get the includes and flags right.
    add_library(comutils-obj OBJECT
        comutils/ComInit.cpp
        comutils/ComInit.h
        comutils/ComPtr.h
        comutils/ComVariant.h)
    target_include_directories(comutils-obj PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
    target_compile_options(comutils-obj PUBLIC "${OSVR_CXX11_FLAGS}")

    # Make a static library, for those who can just link it in privately.
    # Simpler usage for the immediate consumer as long as they're a DLL or EXE
    add_library(comutils STATIC
        $<TARGET_OBJECTS:comutils-obj>)
    target_link_libraries(comutils PUBLIC comutils-interface)

endif()

if(WIN32)
    # Only known to work on Windows, should only be used for development.
    add_library(FloatExceptions INTERFACE)
    target_include_directories(FloatExceptions INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/FloatExceptions")
endif()

# Interface target for Eigen
add_library(eigen-headers INTERFACE)
target_compile_definitions(eigen-headers INTERFACE EIGEN_MPL2_ONLY)
find_package(Eigen3 QUIET)
if(EIGEN3_FOUND)
    target_include_directories(eigen-headers INTERFACE "${EIGEN3_INCLUDE_DIR}")
else()
    # Bundled 3.3.7
    target_include_directories(eigen-headers INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/eigen")
endif()
