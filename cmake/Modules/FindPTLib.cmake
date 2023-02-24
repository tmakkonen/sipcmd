#-------------------------------------------------------------------------------------
# ITNOA
#
# This file is based on https://github.com/snikulov/cmake-modules
#
# Locate PTLib library (http://www.opalvoip.org/)
# This module defines
#    PTLIB_FOUND, if false, do not try to link to PTLib
#    PTLIB_LIBRARIES
#    PTLIB_INCLUDE_DIRS, where to find ptlib headers
#
# 2013/03/19 - aagenosov
#  Module created
# 2013/03/21 - aagenosov
#  Added macro to define version of PTLib
## 2015/08/13 - snikulov
#  Added linux support
#-------------------------------------------------------------------------------------

# get version macro
# first param - path to include
macro(ptlib_get_version _include_PATH version)
    if(EXISTS "${_include_PATH}/ptbuildopts.h")
        file(STRINGS "${_include_PATH}/ptbuildopts.h" _VER_STRING_AUX REGEX ".*#define[ ]+PTLIB_VERSION[ ]+")
    else()
        file(STRINGS "${_include_PATH}/ptlib_config.h" _VER_STRING_AUX REGEX ".*#define[ ]+PTLIB_VERSION[ ]+")
    endif()
    string(REGEX MATCHALL "[0-9]+[.][0-9]+[.][0-9]+" ${version} "${_VER_STRING_AUX}")
endmacro()

include(FindPkgConfig)
PKG_CHECK_MODULES(PC_PTLIB "ptlib")

find_path(PTLIB_INCLUDE_DIRS ptlib.h
    PATHS
    ${PC_PTLIB_INCLUDE_DIRS}
    /usr/local/include
    /usr/include
    /opt/local/include
    /opt/csw/include
    /opt/include
    $ENV{PTLIB_DIR}/include
    ${PTLIB_DIR}/include
    )

if(PC_PTLIB_FOUND)

    set(PTLIB_VERSION      ${PC_PTLIB_VERSION})
    set(PTLIB_INCULDE_DIRS ${PC_PTLIB_INCLUDE_DIRS})

    find_library(PTLIB_LIBRARIES
        NAMES ${PC_PTLIB_LIBRARIES}
        PATH ${PC_PTLIB_LIBRARY_DIRS})

else()

    if(PTLib_USE_STATIC_LIBS)
        set(ptlib_postfix "${ptlib_postfix}S")
    endif()

    set(PTLIB_NAME_RELEASE "ptlib${ptlib_postfix}")
    set(PTLIB_NAME_DEBUG "ptlib${ptlib_postfix}D")
    set(PTLIB64_NAME_RELEASE "ptlib64${ptlib_postfix}")
    set(PTLIB64_NAME_DEBUG "ptlib64${ptlib_postfix}D")


    find_library(PTLIB_LIBRARY_RELEASE
        NAMES
        ${PTLIB_NAME_RELEASE}
        ${PTLIB64_NAME_RELEASE}
        PATHS
        /usr/local
        /usr
        /sw
        /opt/local
        /opt/csw
        /opt
        $ENV{PTLIB_DIR}/lib
        ${PTLIB_DIR}/lib
        NO_DEFAULT_PATH
        )

    find_library(PTLIB_LIBRARY_DEBUG
        NAMES
        ${PTLIB_NAME_DEBUG}
        ${PTLIB64_NAME_DEBUG}
        PATHS
        /usr/local
        /usr
        /sw
        /opt/local
        /opt/csw
        /opt
        $ENV{PTLIB_DIR}/lib
        ${PTLIB_DIR}/lib
        NO_DEFAULT_PATH
        )

    if(PTLIB_INCLUDE_DIRS)
        ptlib_get_version(${PTLIB_INCLUDE_DIRS} PTLIB_VERSION)
    endif()

    if(PTLIB_LIBRARY_DEBUG AND PTLIB_LIBRARY_RELEASE)
        set(PTLIB_LIBRARIES
            debug ${PTLIB_LIBRARY_DEBUG}
            optimized ${PTLIB_LIBRARY_RELEASE}
            CACHE STRING "PTLib Libraries")
    endif()
endif()

include(FindPackageHandleStandardArgs)

# handle the QUIETLY and REQUIRED arguments and set PTLIB_FOUND to TRUE if
# all listed variables are TRUE
FIND_PACKAGE_HANDLE_STANDARD_ARGS(PTLib DEFAULT_MSG PTLIB_LIBRARIES PTLIB_INCLUDE_DIRS)

MARK_AS_ADVANCED(PTLIB_INCLUDE_DIRS PTLIB_LIBRARIES
    PTLIB_LIBRARY_DEBUG PTLIB_LIBRARY_RELEASE PTLIB_VERSION)

if(PTLIB_FOUND)
    message("-- PTLib version is: ${PTLIB_VERSION}")

    # if we found the ptlib - and using dll's
    # short hack for install and copy
    if(NOT PTLib_USE_STATIC_LIBS AND CMAKE_SYSTEM_NAME STREQUAL "Windows")
        find_file(PTLIB_DLL_RELEASE
            NAMES
            ${PTLIB_NAME_RELEASE}.dll ${PTLIB64_NAME_RELEASE}.dll
            PATHS
            $ENV{PTLIB_DIR}/bin
            ${PTLIB_DIR}/bin
            NO_DEFAULT_PATH
            )

        find_file(PTLIB_DLL_DEBUG
            NAMES
            ${PTLIB_NAME_DEBUG}.dll ${PTLIB64_NAME_DEBUG}.dll
            PATHS
            $ENV{PTLIB_DIR}/bin
            ${PTLIB_DIR}/bin
            NO_DEFAULT_PATH
            )
        get_filename_component(PTLIB_RUNTIME_DIR ${PTLIB_DLL_DEBUG} PATH)
        MARK_AS_ADVANCED(PTLIB_DLL_DEBUG PTLIB_DLL_RELEASE)
    endif()

endif()
