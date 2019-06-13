
if(WIN32)
    add_subdirectory(DirectShowCameraLib)
endif()

# ls *.cpp *.h | sort | sed  -e 's/^/"${CMAKE_CURRENT_SOURCE_DIR}\//' -e 's/$/"/'
set(KALMANFRAMEWORK_VIDEOTRACKERSHARED_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}"
    CACHE INTERNAL "" FORCE)
set(HEADER_LOCATION ${INCLUDE_SOURCE_DIR}/videotrackershared)
set(CORE_API
    "${HEADER_LOCATION}/BasicTypes.h"
    "${HEADER_LOCATION}/BlobExtractor.h"
    "${HEADER_LOCATION}/BlobParams.h"
    "${HEADER_LOCATION}/CameraDistortionModel.h"
    "${HEADER_LOCATION}/CameraParameters.h"
    "${HEADER_LOCATION}/cvToEigen.h"
    "${HEADER_LOCATION}/cvUtils.h"
    "${HEADER_LOCATION}/EdgeHoleBasedLedExtractor.h"
    "${HEADER_LOCATION}/EdgeHoleBlobExtractor.h"
    "${HEADER_LOCATION}/GenericBlobExtractor.h"
    "${HEADER_LOCATION}/IdentifierHelpers.h"
    "${HEADER_LOCATION}/LedMeasurement.h"
    "${HEADER_LOCATION}/ProjectPoint.h"
    "${HEADER_LOCATION}/RealtimeLaplacian.h"
    "${HEADER_LOCATION}/SBDBlobExtractor.h"
    "${HEADER_LOCATION}/UndistortMeasurements.h"
)
add_library(videotrackershared_core SHARED
    BlobExtractor.cpp
    EdgeHoleBasedLedExtractor.cpp
    EdgeHoleBlobExtractor.cpp
    GenericBlobExtractor.cpp
    SBDBlobExtractor.cpp
    ${CORE_API})

target_compile_features(videotrackershared_core PUBLIC cxx_std_11)
target_include_directories(videotrackershared_core
    PUBLIC
    $<BUILD_INTERFACE:${INCLUDE_SOURCE_DIR}>
    $<BUILD_INTERFACE:${HEADER_LOCATION}>
    $<BUILD_INTERFACE:${OpenCV_INCLUDE_DIRS}>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
target_link_libraries(videotrackershared_core PUBLIC
    opencv_core
    opencv_imgproc)

set(IO_API
    "${HEADER_LOCATION}/GetOptionalParameter.h"
    "${HEADER_LOCATION}/OptionalStream.h"
    "${HEADER_LOCATION}/ParseBlobParams.h"
)

add_library(videotrackershared_io INTERFACE)
target_sources(videotrackershared_io INTERFACE ${IO_API})
target_compile_features(videotrackershared_io INTERFACE cxx_std_11)
target_link_libraries(videotrackershared_io INTERFACE JsonCpp::JsonCpp)
target_include_directories(videotrackershared_io INTERFACE
    $<BUILD_INTERFACE:${INCLUDE_SOURCE_DIR}>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

set(HDKDATA_API
    "${HEADER_LOCATION}/HDKData.h"
    "${HEADER_LOCATION}/LoadCalibration.h")
add_library(videotrackershared_hdkdata STATIC
    HDKData.cpp
    ${HDKDATA_API})
target_link_libraries(videotrackershared_hdkdata PUBLIC videotrackershared_core)

if(BUILD_ADVANCED_DEV_TOOLS)
    add_executable(blob_extraction_demo
        BlobExtractionDemo.cpp)
    target_link_libraries(blob_extraction_demo
        PUBLIC
        ${VIDEOTRACKER_EXTRA_LIBS}
        videotrackershared_io
        JsonCpp::JsonCpp)
    target_compile_definitions(blob_extraction_demo
        PRIVATE
        UVBI_USING_EDGE_HOLE_EXTRACTOR)
endif()
