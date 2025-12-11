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
    add_subdirectory(externals/json EXCLUDE_FROM_ALL)
    install(TARGETS nlohmann_json 
            EXPORT nudockTargets
            INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )
else()
    message("Did not find nlohmann_json locally. We will fetch it directly from github instead")

    # Adds json directory
    include(FetchContent)
    FetchContent_Declare(nlohmann_json URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz)
    set(JSON_Install ON CACHE INTERNAL "")
    set(JSON_BuildTests OFF CACHE INTERNAL "")
    FetchContent_MakeAvailable(nlohmann_json)
    install(TARGETS nlohmann_json 
            EXPORT nudockTargets
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
    FetchContent_Declare(httplib URL https://github.com/yhirose/cpp-httplib/archive/refs/tags/v0.20.0.tar.gz)
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
        EXPORT nudockTargets
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
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
    set_property(TARGET nlohmann_json_schema_validator APPEND PROPERTY
        INTERFACE_INCLUDE_DIRECTORIES $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/>
    )
    install(TARGETS nlohmann_json_schema_validator
        EXPORT nudockTargets
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}/
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}/
    )
    install(FILES externals/json-schema-validator/src/nlohmann/json-schema.hpp
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/nlohmann
    )
else()
    message("Did not find nlohmann_json_schema_validator locally. Use git clone --recurse-submodules to get it or we will fetch it directly from github instead")
endif(nlohmann_json_schema_validator_FOUND)