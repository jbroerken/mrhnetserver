#########################################################################
#
#  CMAKE
#
#########################################################################

###
#  Minimum Version
#  ---------------
#  The CMake version required.
###
cmake_minimum_required(VERSION 3.1)

###
#  CMake Configuration
#  -------------------
#  Configuration settings for CMake.
#
#  NOTE:
#  These settings have to be applied before the project() setting!
###
set(CMAKE_CXX_COMPILER "g++")
set(CMAKE_CXX_STANDARD 14)

###
#  Project Info
#  ------------
#  General simple information about our project.
###
project(mrhnetserver VERSION 1.0.0
                     DESCRIPTION "MRH net server executable"
                     LANGUAGES CXX)

#########################################################################
#
#  PATHS
#
#########################################################################

###
#  Install Paths
#  -------------
#  The paths for our created binary file(s).
###
set(BIN_INSTALL_PATH "/usr/local/bin/")

###
#  Build Paths
#  -----------
#  The paths for the cmake build.
###
set(BUILD_DIR_PATH "${CMAKE_SOURCE_DIR}/build/")
file(MAKE_DIRECTORY ${BUILD_DIR_PATH})

###
#  Source Paths
#  ------------
#  The paths to the source files to use.
#  Add OS specific source files in their own list.
###
set(SRC_DIR_PATH "${CMAKE_SOURCE_DIR}/src/")

set(SRC_LIST_NET_MESSAGE_V1 "${SRC_DIR_PATH}/NetMessage/Ver/NetMessageV1.cpp"
                            "${SRC_DIR_PATH}/NetMessage/Ver/NetMessageV1.h")
       
set(SRC_LIST_NET_MESSAGE "${SRC_DIR_PATH}/NetMessage/MsQuic.cpp"
                         "${SRC_DIR_PATH}/NetMessage/MsQuic.h"
                         "${SRC_DIR_PATH}/NetMessage/NetServer.cpp"
                         "${SRC_DIR_PATH}/NetMessage/NetServer.h"
                         "${SRC_DIR_PATH}/NetMessage/NetConnection.cpp"
                         "${SRC_DIR_PATH}/NetMessage/NetConnection.h"
                         "${SRC_DIR_PATH}/NetMessage/NetMessage.cpp"
                         "${SRC_DIR_PATH}/NetMessage/NetMessage.h"
                         "${SRC_DIR_PATH}/NetMessage/NetException.h")
                         
set(SRC_LIST_DATABASE "${SRC_DIR_PATH}/Database/Database.cpp"
                      "${SRC_DIR_PATH}/Database/Database.h"
                      "${SRC_DIR_PATH}/Database/DatabaseTable.h")
                      
set(SRC_LIST_WORKER_POOL "${SRC_DIR_PATH}/WorkerPool/WorkerPool.cpp"
                         "${SRC_DIR_PATH}/WorkerPool/WorkerPool.h"
                         "${SRC_DIR_PATH}/WorkerPool/WorkerShared.h"
                         "${SRC_DIR_PATH}/WorkerPool/WorkerTask.h")
                         
set(SRC_LIST_COMMUNICATION "${SRC_DIR_PATH}/Communication/ExchangeContainer.cpp"
                           "${SRC_DIR_PATH}/Communication/ExchangeContainer.h"
                           "${SRC_DIR_PATH}/Communication/MessageExchange.h"
                           "${SRC_DIR_PATH}/Communication/CommunicationTask.cpp"
                           "${SRC_DIR_PATH}/Communication/CommunicationTask.h"
                           "${SRC_DIR_PATH}/Communication/CommunicationMain.cpp"
                           "${SRC_DIR_PATH}/Communication/CommunicationMain.h")
                           
set(SRC_LIST_CONNECTION "${SRC_DIR_PATH}/Connection/ConnectionTask.cpp"
                        "${SRC_DIR_PATH}/Connection/ConnectionTask.h"
                        "${SRC_DIR_PATH}/Connection/ConnectionMain.cpp"
                        "${SRC_DIR_PATH}/Connection/ConnectionMain.h")
                 
set(SRC_LIST_BASE "${SRC_DIR_PATH}/ServerAuth.cpp"
                  "${SRC_DIR_PATH}/ServerAuth.h"
                  "${SRC_DIR_PATH}/ServerException.h"
                  "${SRC_DIR_PATH}/ActorType.h"
                  "${SRC_DIR_PATH}/CLI.cpp"
                  "${SRC_DIR_PATH}/CLI.h"
                  "${SRC_DIR_PATH}/Configuration.cpp"
                  "${SRC_DIR_PATH}/Configuration.h"
                  "${SRC_DIR_PATH}/Timer.cpp"
                  "${SRC_DIR_PATH}/Timer.h"
                  "${SRC_DIR_PATH}/Logger.cpp"
                  "${SRC_DIR_PATH}/Logger.h"
                  "${SRC_DIR_PATH}/Revision.h"
                  "${SRC_DIR_PATH}/Main.cpp")

#########################################################################
#
#  TARGET
#
#########################################################################

###
#  Target
#  ------
#  The target(s) to build.
###
add_executable(mrhnetserver ${SRC_LIST_NET_MESSAGE_V1}
                            ${SRC_LIST_NET_MESSAGE}
                            ${SRC_LIST_DATABASE}
                            ${SRC_LIST_WORKER_POOL}
                            ${SRC_LIST_COMMUNICATION}
                            ${SRC_LIST_CONNECTION}
                            ${SRC_LIST_BASE})

###
#  Required Libraries
#  ------------------
#  Libraries required by this application.
###
set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)

find_package(Threads REQUIRED)
find_library(libmsquic NAMES msquic REQUIRED)
find_library(libsodium NAMES sodium REQUIRED)
find_library(libmysqlcppcon NAMES mysqlcppconn mysqlcppconn8.2 libmysqlcppconn libmysqlcppconn8.2 REQUIRED)

target_link_libraries(mrhnetserver PUBLIC Threads::Threads)
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    target_link_libraries(mrhnetserver PUBLIC
                          "-framework CoreFoundation"
                          "-framework Security")
endif()
target_link_libraries(mrhnetserver PUBLIC msquic)
target_link_libraries(mrhnetserver PUBLIC sodium)

###
#  Source Definitions
#  ------------------
#  Preprocessor source definitions.
###
target_compile_definitions(mrhnetserver PRIVATE MRH_NET_SERVER_LOG_FILE_PATH="/var/log/mrhnetserver.log")
target_compile_definitions(mrhnetserver PRIVATE MRH_NET_SERVER_BACKTRACE_FILE_PATH="/var/log/bt_mrhnetserver.log")
target_compile_definitions(mrhnetserver PRIVATE MRH_LOGGER_PRINT_CLI=0)

target_compile_definitions(mrhnetserver PRIVATE MRH_SRV_REGISTRATION_NAME="mrh_srv_reg")
target_compile_definitions(mrhnetserver PRIVATE MRH_SRV_ALPN_NAME="mrh_srv_alpn")
target_compile_definitions(mrhnetserver PRIVATE CLIENT_STREAMS_PER_DIRECTION=32)

target_compile_definitions(mrhnetserver PRIVATE CONNECTION_TASK_MAX_AUTH_RETRY=3)
target_compile_definitions(mrhnetserver PRIVATE CONNECTION_TASK_MAX_MESSAGE_PER_LOOP=10)
target_compile_definitions(mrhnetserver PRIVATE CONNECTION_TASK_MAX_UPDATE_DIFF_S=300)
target_compile_definitions(mrhnetserver PRIVATE CONNECTION_TASK_EXTENDED_LOGGING=0)

target_compile_definitions(mrhnetserver PRIVATE COMMUNICATION_SERVER_SET_LAST_UPDATE_MS=240000)
target_compile_definitions(mrhnetserver PRIVATE COMMUNICATION_TASK_MAX_AUTH_RETRY=3)
target_compile_definitions(mrhnetserver PRIVATE COMMUNICATION_TASK_MAX_MESSAGE_PER_LOOP=10)
target_compile_definitions(mrhnetserver PRIVATE COMMUNICATION_TASK_MAX_UPDATE_DIFF_S=300)
target_compile_definitions(mrhnetserver PRIVATE COMMUNICATION_TASK_EXTENDED_LOGGING=0)

###
#  Install
#  -------
#  Application installation.
###
install(TARGETS mrhnetserver
        DESTINATION ${BIN_INSTALL_PATH})