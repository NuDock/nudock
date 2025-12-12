# Get nlohmann_json
find_package(nlohmann_json)
if(nlohmann_json_FOUND)
    message("Found nlohmann_json")
    if(NOT TARGET nlohmann_json::nlohmann_json)
        add_library(nlohmann_json::nlohmann_json ALIAS nlohmann_json)
    endif()
elseif(EXISTS "${PROJECT_SOURCE_DIR}/externals/json")
    message("Did not find nlohmann_json. We will compile submodule instead.")
    set(JSON_Install ON CACHE INTERNAL "")
    set(JSON_BuildTests OFF CACHE INTERNAL "")
    add_subdirectory(externals/json)
    install(TARGETS nlohmann_json 
            INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )
else()
    message("Did not find nlohmann_json locally. We will fetch it directly from github instead")

    # Adds json directorj
    include(FetchContent)
    FetchContent_Declare(nlohmann_json URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz)
    set(JSON_Install ON CACHE INTERNAL "")
    set(JSON_BuildTests OFF CACHE INTERNAL "")
    FetchContent_MakeAvailable(nlohmann_json)
    install(TARGETS nlohmann_json 
            INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )
endif(nlohmann_json_FOUND)

# Get cpp-httplib 
find_package(httplib)
if(httplib_FOUND)
    message("Found cpp-httplib: ${httplib_VERSION}")
#elseif(EXISTS "${PROJECT_SOURCE_DIR}/externals/cpp-httplib")
#    message("Did not find cpp-httplib. We will compile submodule instead.")
#
#    # Adds cpp-httplib directory
#    set(HTTPLIB_INSTALL OFF)
#    add_subdirectory(externals/cpp-httplib EXCLUDE_FROM_ALL)
#    set_property(TARGET httplib APPEND PROPERTY
#        INTERFACE_INCLUDE_DIRECTORIES $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
#    )
#    install(TARGETS httplib 
#        EXPORT nudockTargets
#        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/
#        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}/
#        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}/
#    )
else()
    message("Did not find cpp-httplib locally. We will fetch it directly from github instead")

    # Adds cpp-httplib directory
    include(FetchContent)
    FetchContent_Declare(httplib URL https://api.github.com/repos/yhirose/cpp-httplib/tarball/v0.20.0)
    # Disable features we don't need
    set(HTTPLIB_USE_BROTLI_IF_AVAILABLE OFF CACHE INTERNAL "")
    set(HTTPLIB_USE_ZLIB_IF_AVAILABLE OFF CACHE INTERNAL "")
    set(HTTPLIB_USE_OPENSSL_IF_AVAILABLE OFF CACHE INTERNAL "")
    set(HTTPLIB_USE_ZSTD_IF_AVAILABLE OFF CACHE INTERNAL "")
    set(HTTPLIB_REQUIRE_BROTLI OFF CACHE INTERNAL "")
    set(HTTPLIB_REQUIRE_OPENSSL OFF CACHE INTERNAL "")
    set(HTTPLIB_REQUIRE_ZLIB OFF CACHE INTERNAL "")
    set(HTTPLIB_REQUIRE_ZSTD OFF CACHE INTERNAL "")
    FetchContent_MakeAvailable(httplib)
    #add_subdirectory(${cpp-httplib_SOURCE_DIR} ${cpp-httplib_BINARY_DIR} EXCLUDE_FROM_ALL)
    set_property(TARGET httplib APPEND PROPERTY
        INTERFACE_INCLUDE_DIRECTORIES $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    )
    install(TARGETS httplib
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}/
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}/
    )

    # Generate httplibConfig.cmake file
    include(CMakePackageConfigHelpers)
    configure_package_config_file(
        ${CMAKE_CURRENT_LIST_DIR}/../cmake/httplibConfig.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/httplibConfig.cmake
        INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/httplib
    )
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/httplibConfig.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/httplib
    )
    install(EXPORT httplibTargets
        FILE httplibTargets.cmake
        NAMESPACE httplib::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/httplib
    )
endif(httplib_FOUND)

# Get nlohmann_json_schema_validator
find_package(nlohmann_json_schema_validator)
if(nlohmann_json_schema_validator_FOUND)
    message("Found nlohmann_json_schema_validator")
elseif(EXISTS "${PROJECT_SOURCE_DIR}/externals/json-schema-validator")
    message("Did not find nlohmann_json_schema_validator. We will compile submodule instead.")  

    # Not actually installing it, we only need the headers
    set(JSON_VALIDATOR_INSTALL ON CACHE INTERNAL "")
    set(BUILD_TESTS OFF CACHE INTERNAL "")
    set(BUILD_EXAMPLES OFF CACHE INTERNAL "")

    add_subdirectory(externals/json-schema-validator)
else()
    message("Did not find nlohmann_json_schema_validator locally. Use git clone --recurse-submodules to get it or we will fetch it directly from github instead")
endif(nlohmann_json_schema_validator_FOUND)