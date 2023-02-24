# ITNOA
# CMakeList.txt : CMake project for SipCmd, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

project ("SipCmd" VERSION 1.0 LANGUAGES CXX)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

set(PTLib_USE_STATIC_LIBS 1)
find_package(PTLib REQUIRED)

set(OPAL_USE_STATIC_LIBS 1)
find_package(OPAL REQUIRED)

# Add source to this project's library.
add_library(SipCmdLib "src/commands.cpp" "src/channels.cpp")

target_include_directories(SipCmdLib PUBLIC ${PTLIB_INCLUDE_DIRS})
target_link_libraries(SipCmdLib PRIVATE ${PTLIB_LIBRARIES})

target_include_directories(SipCmdLib PUBLIC ${OPAL_INCLUDE_DIRS})
target_link_libraries(SipCmdLib PRIVATE ${OPAL_LIBRARIES})

# Add source to this project's executable.
add_executable (SipCmd "src/main.cpp")

target_link_libraries(SipCmd PRIVATE SipCmdLib)
message(${PTLIB_INCLUDE_DIRS})

if (CMAKE_VERSION VERSION_GREATER 3.10)
  set_property(TARGET SipCmdLib PROPERTY CXX_STANDARD 14)
  set_property(TARGET SipCmd PROPERTY CXX_STANDARD 14)
endif()

# TODO: Add tests and install targets if needed.
