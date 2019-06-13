
set(INCLUDE_SOURCE_DIR ${PROJECT_SOURCE_DIR}/inc)

add_subdirectory(FlexKalman)

# if(WIN32)
#     add_subdirectory(DirectShowCameraLib)
# endif()

add_subdirectory(videotrackershared)

# add_subdirectory(unifiedvideoinertialtracker)
