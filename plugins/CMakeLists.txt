############################################################################################################################################

etn_target(shared plugin-mlm
    SOURCES
        mlm/mlm.h
        mlm/mlm.cpp
        mlm/mlm-message.h
        mlm/mlm-message.cpp
        mlm/mlm-listener.h
        mlm/mlm-listener.cpp
    INCLUDE_DIRS
        ${CMAKE_CURRENT_SOURCE_DIR}/../
    USES
        ${PROJECT_NAME}-common
        fty_common_logging
        fty-utils
        fty-pack
        mlm
        czmq
        pthread
    TARGET_DESTINATION
        ${CMAKE_INSTALL_PREFIX}/messagebus
)

############################################################################################################################################
