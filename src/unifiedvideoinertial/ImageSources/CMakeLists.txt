###
# Build a separate static library for image acquisition
###
set(HEADER_LOCATION ${INCLUDE_SOURCE_DIR}/unifiedvideoinertial/ImageSources)
set(API
    "${HEADER_LOCATION}/ImageSource.h"
    "${HEADER_LOCATION}/ImageSourceFactories.h")
set(SOURCES
    CVImageSource.cpp
    # DK2ImageSource.cpp
    ImageSource.cpp
    FakeImageSource.cpp
    # Oculus_DK2.cpp
    # Oculus_DK2.h
    ${API}
    )

if(WIN32)
    list(APPEND SOURCES
        CheckFirmwareVersion.h
        DirectShowHDKCameraFactory.h
        DirectShowImageSource.cpp
        DirectShowToCV.h)
else()
    list(APPEND SOURCES
        UVCImageSource.cpp)
endif()
add_library(uvbi-image-sources STATIC ${SOURCES})
target_compile_features(uvbi-image-sources
    PUBLIC
    cxx_std_11)


target_link_libraries(uvbi-image-sources PUBLIC
    opencv_core
    opencv_highgui
    opencv_videoio
    opencv_imgproc
    uvbi-base)
if(WIN32)
    target_link_libraries(uvbi-image-sources PUBLIC
        directshow-camera
        osvr_codecvt
        wbemuuid)
    target_include_directories(uvbi-image-sources PRIVATE ${BOOST_INCLUDE_DIRS})
else()
    target_link_libraries(uvbi-image-sources PRIVATE ${libuvc_LIBRARIES} ${LIBUSB1_LIBRARIES})
    target_include_directories(uvbi-image-sources PRIVATE ${libuvc_INCLUDE_DIRS} ${LIBUSB1_INCLUDE_DIRS})
endif()
