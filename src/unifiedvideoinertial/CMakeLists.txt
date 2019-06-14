
# osvr_convert_json(org_osvr_unifiedvideoinertial_json
#     org_osvr_unifiedvideoinertial.json
#     "${CMAKE_CURRENT_BINARY_DIR}/org_osvr_unifiedvideoinertial_json.h")

# Make sure we can link these upcoming static libraries into a shared lib
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_definitions(-DUVBI_USING_EDGE_HOLE_EXTRACTOR)


add_subdirectory(ImageSources)

set(HEADER_LOCATION ${INCLUDE_SOURCE_DIR}/unifiedvideoinertial)
###
# Static library with core of the tracker
###

set(API
    "${HEADER_LOCATION}/Assumptions.h"
    "${HEADER_LOCATION}/BeaconIdTypes.h"
    "${HEADER_LOCATION}/BeaconSetupData.h"
    "${HEADER_LOCATION}/BodyIdTypes.h"
    "${HEADER_LOCATION}/CannedIMUMeasurement.h"
    "${HEADER_LOCATION}/ConfigParams.h"
    "${HEADER_LOCATION}/ImageProcessing.h"
    "${HEADER_LOCATION}/ModelTypes.h"
    "${HEADER_LOCATION}/RangeTransform.h"
    "${HEADER_LOCATION}/TrackedBody.h"
    "${HEADER_LOCATION}/TrackedBodyTarget.h"
    "${HEADER_LOCATION}/TrackingSystem.h"
    "${HEADER_LOCATION}/Types.h"
)

add_library(uvbi-core STATIC
    AngVelTools.h
    ApplyIMUToState.cpp
    ApplyIMUToState.h
    AssignMeasurementsToLeds.h
    BeaconSetupData.cpp
    BodyTargetInterface.h
    Clamp.h
    ConfigParams.cpp
    CrossProductMatrix.h
    ForEachTracked.h
    HDKLedIdentifier.cpp
    HDKLedIdentifier.h
    HDKLedIdentifierFactory.cpp
    HDKLedIdentifierFactory.h
    HistoryContainer.h
    ImagePointMeasurement.h
    IMUStateMeasurements.h
    LED.cpp
    LED.h
    LedIdentifier.cpp
    LedIdentifier.h
    PinholeCameraFlip.h
    PoseEstimator_RANSAC.cpp
    PoseEstimator_RANSAC.h
    PoseEstimator_RANSACKalman.cpp
    PoseEstimator_RANSACKalman.h
    PoseEstimator_SCAATKalman.cpp
    PoseEstimator_SCAATKalman.h
    PoseEstimatorTypes.h
    RoomCalibration.cpp
    RoomCalibration.h
    SpaceTransformations.h
    StateHistory.h
    TrackedBody.cpp
    TrackedBodyIMU.cpp
    TrackedBodyIMU.h
    TrackedBodyTarget.cpp
    TrackingDebugDisplay.cpp
    TrackingDebugDisplay.h
    TrackingSystem_Impl.cpp
    TrackingSystem_Impl.h
    TrackingSystem.cpp
    UsefulQuaternions.h)
target_compile_features(uvbi-core
    PUBLIC
    cxx_std_11)
target_link_libraries(uvbi-core
    PUBLIC
    ${VIDEOTRACKER_EXTRA_LIBS}
    opencv_core
    opencv_imgproc
    opencv_videoio
    opencv_calib3d
    opencv_highgui
    KalmanFramework
    optional-lite
    variant-lite
    $<BUILD_INTERFACE:eigen-headers>
    PRIVATE
    videotrackershared_core
    videotrackershared_io
    util-headers)
# target_include_directories(uvbi-core
#     PUBLIC
#     ${KALMANFRAMEWORK_VIDEOTRACKERSHARED_INCLUDE_DIR})
target_compile_definitions(uvbi-core PRIVATE UVBI_CORE)



if(BUILD_TESTING)
    ###
    # Synthetic-data verification of usage of room calibration in IMU code and IMU filtering
    ###
    add_executable(uvbi-test-imu
        TestIMU.h
        TestIMU.cpp
        TestIMU_Common.h
        TestIMU_EKF.cpp
        TestIMU_UKF.cpp)
    target_link_libraries(uvbi-test-imu PRIVATE uvbi-core kf-catch2-interface)
    set_target_properties(uvbi-test-imu PROPERTIES
        FOLDER "${PROJ_FOLDER}")
    add_test(NAME TestIMU COMMAND uvbi-test-imu)
endif()

# Main plugin, disabled because you can't build an OSVR plugin without OSVR
# osvr_add_plugin(NAME org_osvr_unifiedvideoinertial
#     CPP # indicates we'd like to use the C++ wrapper
#     SOURCES
#     org_osvr_unifiedvideoinertial.cpp
#     "${CMAKE_CURRENT_BINARY_DIR}/org_osvr_unifiedvideoinertial_json.h"
#     AdditionalReports.h
#     ConfigurationParser.h
#     MakeHDKTrackingSystem.h
#     ImageProcessingThread.cpp
#     ImageProcessingThread.h
#     IMUMessage.h
#     ProcessIMUMessage.h
#     ThreadsafeBodyReporting.cpp
#     ThreadsafeBodyReporting.h
#     TrackerThread.cpp
#     TrackerThread.h
#     $<TARGET_OBJECTS:uvbi-hdkdata>
#     ${KALMANFRAMEWORK_VIDEOTRACKERSHARED_SOURCES_IO})

# target_link_libraries(org_osvr_unifiedvideoinertial
#     osvr::osvrAnalysisPluginKit
#     uvbi-core
#     uvbi-image-sources
#     JsonCpp::JsonCpp
#     util-headers
#     folly-headers)

# These are the non-OSVR-dependent files from the plugin:
# everything except the org_osvr_unifiedvideoinertial.cpp
# and the JSON literal
set(API
    "${HEADER_LOCATION}/ConfigurationParser.h"
    "${HEADER_LOCATION}/MakeHDKTrackingSystem.h")
add_library(uvbi_plugin_parts SHARED
    AdditionalReports.h
    ImageProcessingThread.cpp
    ImageProcessingThread.h
    IMUMessage.h
    ProcessIMUMessage.h
    ThreadsafeBodyReporting.cpp
    ThreadsafeBodyReporting.h
    TrackerThread.cpp
    TrackerThread.h
    ${API})

target_link_libraries(uvbi_plugin_parts
    PUBLIC
    uvbi-core
    uvbi-image-sources
    JsonCpp::JsonCpp
    PRIVATE
    util-headers
    folly-headers
    videotrackershared_io
    videotrackershared_hdkdata
    )
