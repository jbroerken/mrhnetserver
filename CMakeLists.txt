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

set(SRC_LIST_ALL "${SRC_DIR_PATH}/Logger.cpp"
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
add_executable(mrhnetserver ${SRC_LIST_ALL})

###
#  Required Libraries
#  ------------------
#  Libraries required by this application.
###
set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)

find_package(Threads REQUIRED)

target_link_libraries(mrhnetserver PUBLIC Threads::Threads)

###
#  Source Definitions
#  ------------------
#  Preprocessor source definitions.
###
target_compile_definitions(mrhnetserver PRIVATE MRH_NET_SERVER_LOG_FILE_PATH="/var/log/mrhnetserver.log")
target_compile_definitions(mrhnetserver PRIVATE MRH_NET_SERVER_BACKTRACE_FILE_PATH="/var/log/mrh/bt_mrhnetserver.log")
target_compile_definitions(mrhnetserver PRIVATE MRH_LOGGER_PRINT_CLI=0)
target_compile_definitions(mrhnetserver PRIVATE CONNECTION_TASK_MAX_AUTH_RETRY=3)
target_compile_definitions(mrhnetserver PRIVATE CONNECTION_TASK_MAX_MESSAGE_PER_LOOP=10)

###
#  Install
#  -------
#  Application installation.
###
install(TARGETS mrhnetserver
        DESTINATION ${BIN_INSTALL_PATH})