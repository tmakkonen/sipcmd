#------------------------------------------------------------------------------------
# ITNOA
#
# This file is based on https://github.com/snikulov/cmake-modules
#
# Locate OPAL library (http://www.opalvoip.org/)
# This module defines
#    OPAL_FOUND, if false, do not try to link to OPAL
#    OPAL_LIBRARIES
#    OPAL_INCLUDE_DIRS, where to find opal headers
#
# 2013/03/19 - aagenosov
#  Module created
# 2013/03/21 - aagenosov
#  Added macro to define version of OPAL
# 2015/08/13 - snikulov
#  Added linux support
#-------------------------------------------------------------------------------------

# get version macro
# first param - path to include
macro(opal_get_version _include_PATH version)
    if (EXISTS "${_include_PATH}/opal/buildopts.h")
        file(STRINGS "${_include_PATH}/opal/buildopts.h" _VER_STRING_AUX REGEX ".*#define[ ]+OPAL_VERSION[ ]+")
    else()
        file(STRINGS "${_include_PATH}/opal_config.h" _VER_STRING_AUX REGEX ".*#define[ ]+OPAL_VERSION[ ]+")
    endif()
    string(REGEX MATCHALL "[0-9]+[.][0-9]+[.][0-9]+" ${version} "${_VER_STRING_AUX}")
endmacro()

find_package(PTLib REQUIRED)

include(FindPkgConfig)
PKG_CHECK_MODULES(PC_OPAL "opal")

find_path(OPAL_INCLUDE_DIRS opal.h
    PATHS
    ${PC_OPAL_INCLUDE_DIRS}
    /usr/local/include
    /usr/include
    /opt/local/include
    /opt/csw/include
    /opt/include
    $ENV{OPAL_DIR}/include
    ${OPAL_DIR}/include
    )

if(PC_OPAL_FOUND)

    set(OPAL_VERSION      ${PC_OPAL_VERSION})
    set(OPAL_INCLUDE_DIRS ${PC_OPAL_INCLUDE_DIRS})

    find_library(OPAL_LIBRARIES
        NAMES ${PC_OPAL_LIBRARIES}
        PATH ${PC_OPAL_LIBRARY_DIRS})

else()

    if(OPAL_USE_STATIC_LIBS)
        set(opal_postfix "${opal_postfix}S")
    endif()

    set(OPAL_NAME_RELEASE "opal${opal_postfix}")
    set(OPAL_NAME_DEBUG "opal${opal_postfix}D")
    set(OPAL64_NAME_RELEASE "opal64${opal_postfix}")
    set(OPAL64_NAME_DEBUG "opal64${opal_postfix}D")


    find_library(OPAL_LIBRARY_RELEASE
        NAMES
        ${OPAL_NAME_RELEASE}
        ${OPAL64_NAME_RELEASE}
        PATHS
        /usr/local
        /usr
        /sw
        /opt/local
        /opt/csw
        /opt
        $ENV{OPAL_DIR}/lib
        ${OPAL_DIR}/lib
        NO_DEFAULT_PATH
        )

    find_library(OPAL_LIBRARY_DEBUG
        NAMES
        ${OPAL_NAME_DEBUG}
        ${OPAL64_NAME_DEBUG}
        PATHS
        /usr/local
        /usr
        /sw
        /opt/local
        /opt/csw
        /opt
        $ENV{OPAL_DIR}/lib
        ${OPAL_DIR}/lib
        NO_DEFAULT_PATH
        )

    if(OPAL_INCLUDE_DIRS)
        opal_get_version(${OPAL_INCLUDE_DIRS} OPAL_VERSION)
    endif()

    if(OPAL_LIBRARY_DEBUG AND OPAL_LIBRARY_RELEASE)
        set(OPAL_LIBRARIES
            debug ${OPAL_LIBRARY_DEBUG}
            optimized ${OPAL_LIBRARY_RELEASE}
            CACHE STRING "OPAL Libraries")
    endif()
endif()

include(FindPackageHandleStandardArgs)

# handle the QUIETLY and REQUIRED arguments and set OPAL_FOUND to TRUE if
# all listed variables are TRUE
FIND_PACKAGE_HANDLE_STANDARD_ARGS(OPAL DEFAULT_MSG OPAL_LIBRARIES OPAL_INCLUDE_DIRS)

MARK_AS_ADVANCED(OPAL_INCLUDE_DIRS OPAL_LIBRARIES
    OPAL_LIBRARY_DEBUG OPAL_LIBRARY_RELEASE)

if(OPAL_FOUND)
    message("-- OPAL version is: ${OPAL_VERSION}")
    # short hack for install and copy
    if(NOT OPAL_USE_STATIC_LIBS AND CMAKE_SYSTEM_NAME STREQUAL "Windows")
        find_file(OPAL_DLL_RELEASE
            NAMES
            ${OPAL_NAME_RELEASE}.dll ${OPAL64_NAME_RELEASE}.dll
            PATHS
            $ENV{OPAL_DIR}/bin
            ${OPAL_DIR}/bin
            NO_DEFAULT_PATH
            )

        find_file(OPAL_DLL_DEBUG
            NAMES
            ${OPAL_NAME_DEBUG}.dll ${OPAL64_NAME_DEBUG}.dll
            PATHS
            $ENV{OPAL_DIR}/bin
            ${OPAL_DIR}/bin
            NO_DEFAULT_PATH
            )
        get_filename_component(OPAL_RUNTIME_DIR ${OPAL_DLL_DEBUG} PATH)
        MARK_AS_ADVANCED(OPAL_DLL_DEBUG OPAL_DLL_RELEASE OPAL_RUNTIME_DIR)
    endif()

    if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
        set(PATH_TO_OPAL_PLUGINS ${OPAL_RUNTIME_DIR}/plugins)

        if (EXISTS ${PATH_TO_OPAL_PLUGINS} AND IS_DIRECTORY ${PATH_TO_OPAL_PLUGINS})
            file(GLOB _opal_plugins_ "${PATH_TO_OPAL_PLUGINS}/*.dll")
        endif()

        set(OPAL_PLUGINS)
        foreach(_plugin ${_opal_plugins_})
            list(APPEND OPAL_PLUGINS ${_plugin})
        endforeach()
        message("-- OPAL plugins: ${OPAL_PLUGINS}")
        MARK_AS_ADVANCED(PATH_TO_OPAL_PLUGINS OPAL_PLUGINS)
    endif()
endif()
