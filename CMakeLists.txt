cmake_minimum_required (VERSION 2.6)
project (ircd)

# hide default options from ccmake
mark_as_advanced (FORCE CMAKE_BUILD_TYPE CMAKE_INSTALL_PREFIX)

# custom options
option (BUILD_DEBUG "Compile with debug options" OFF)
option (USE_GETOPT "Use getopt to parse command line arguments" ON)
option (USE_BACKTRACE "Print backtrace on error" ON)
option (USE_EPOLL "Use epoll instead of select" OFF)

# setup compiler/linker flags
if (BUILD_DEBUG)
	message(STATUS "Debug build")
	set(CMAKE_BUILD_TYPE "Debug")
else ()
	message(STATUS "Release build")
	set(CMAKE_BUILD_TYPE "Release")
endif ()

set (CMAKE_C_FLAGS "-std=c99 -Wall")
set (CMAKE_C_FLAGS_RELEASE "-O3")
set (CMAKE_C_FLAGS_DEBUG "-g -pg")

# setup custom definitions
if (USE_GETOPT)
	message(STATUS "Enabling getopt support")
	add_definitions ("-D_BSD_SOURCE -D_XOPEN_SOURCE")
endif ()
if (USE_BACKTRACE)
	message(STATUS "Enabling backtrace support")
	add_definitions ("-DDEBUG_PRINT_BACKTRACE")
	set (LINK_FLAGS "-rdynamic")
endif ()
if (USE_EPOLL)
	message(STATUS "Enabling epoll")
	add_definitions ("-DUSE_EPOLL")
endif ()
file (GLOB ircd_SRCS src/*.c src/commands/*.c)

add_executable (ircd ${ircd_SRCS})

