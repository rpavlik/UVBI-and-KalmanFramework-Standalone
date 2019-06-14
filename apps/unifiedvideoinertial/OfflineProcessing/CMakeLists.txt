if(Boost_FOUND)
    add_executable(uvbi-offline-processing
        OfflineProcessing.cpp
        # CSVCellGroup.h
        QuatToEuler.h
        # ../MakeHDKTrackingSystem.h
        )

    set_target_properties(uvbi-offline-processing PROPERTIES
        FOLDER "${PROJ_FOLDER}")
    target_link_libraries(uvbi-offline-processing
        PRIVATE
        uvbi-core
        videotrackershared_hdkdata
        JsonCpp::JsonCpp)
    target_include_directories(uvbi-offline-processing PRIVATE ${BOOST_INCLUDE_DIRS})
endif()
