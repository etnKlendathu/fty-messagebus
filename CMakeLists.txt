cmake_minimum_required(VERSION 3.13)
cmake_policy(VERSION 3.13)

############################################################################################################################################

project(fty-messagebus
    VERSION 1.0.0
    DESCRIPTION "fty messagebus"
)

############################################################################################################################################
find_package(fty-cmake PATHS ${CMAKE_BINARY_DIR}/fty-cmake REQUIRED)
############################################################################################################################################

option(BUILD_AMQP     "Build AMQP addon"      OFF)
option(BUILD_MALAMUTE "Build Malamute addon"  ON)
option(BUILD_MQTT     "Build MQTT addon"      OFF)

############################################################################################################################################

etn_target(static ${PROJECT_NAME}-common
    SOURCES
        common/plugin.h
        common/helper.h
        common/helper.cpp
    USES
        uuid
    PRIVATE
)

etn_target(shared ${PROJECT_NAME}
    PUBLIC_HEADERS
        fty/messagebus/message.h
        fty/messagebus/message-bus.h
    SOURCES
        src/message.cpp
        src/message-bus.cpp
        src/libloader.h
        src/libloader.cpp
        src/plugin.h
    INCLUDE_DIRS
        plugins
    USES
        dl
        fty-pack
        fty-utils
        fty_common_logging
        ${PROJECT_NAME}-common
)

############################################################################################################################################
add_subdirectory(plugins)
############################################################################################################################################
if (BUILD_TESTING)
    etn_test_target(${PROJECT_NAME}
        SOURCES
            main.cpp
        USES
            mlm
            czmq
    )
endif()
############################################################################################################################################
