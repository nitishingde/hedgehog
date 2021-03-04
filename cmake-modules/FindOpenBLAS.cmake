
# - Try to find openblas
# Once done, this will define
#
#  OpenBLAS_FOUND - system has openblas
#  OpenBLAS_INCLUDE_DIRS - the openblas include directories
#  OpenBLAS_LIBRARIES - link these to use libuv
#
# Set the OpenBLAS_USE_STATIC variable to specify if static libraries should
# be preferred to shared ones.


include(FindPackageHandleStandardArgs)

if(DEFINED OpenBLAS_ROOT_DIR)
    find_path(
            OpenBLAS_INCLUDE_DIR
            NAMES cblas.h
            PATHS ${OpenBLAS_ROOT_DIR}
            PATH_SUFFIXES include
            NO_DEFAULT_PATH
            NO_PACKAGE_ROOT_PATH
            NO_CMAKE_PATH
            NO_CMAKE_ENVIRONMENT_PATH
            NO_SYSTEM_ENVIRONMENT_PATH
            NO_CMAKE_SYSTEM_PATH
    )
    find_library(
            OpenBLAS_LIBRARY
            NAMES openblas
            PATHS ${OpenBLAS_ROOT_DIR}
            PATH_SUFFIXES lib lib64
            NO_DEFAULT_PATH
            NO_PACKAGE_ROOT_PATH
            NO_CMAKE_PATH
            NO_CMAKE_ENVIRONMENT_PATH
            NO_SYSTEM_ENVIRONMENT_PATH
            NO_CMAKE_SYSTEM_PATH
    )
else(DEFINED OpenBLAS_ROOT_DIR)
    set(OpenBLAS_ROOT_DIR "" CACHE PATH "Folder contains OpenBLAS")
    find_path(OpenBLAS_INCLUDE_DIR cblas.h
            PATHS ${OpenBLAS_ROOT_DIR}
            PATH_SUFFIXES include)
    find_library(OpenBLAS_LIBRARY openblas
            PATHS ${OpenBLAS_ROOT_DIR}
            PATH_SUFFIXES lib lib64)
endif(DEFINED OpenBLAS_ROOT_DIR)

message(OpenBLAS_LIBRARY: ${OpenBLAS_LIBRARY})

find_package_handle_standard_args(OpenBLAS DEFAULT_MSG OpenBLAS_INCLUDE_DIR OpenBLAS_LIBRARY)

if(OpenBLAS_FOUND)
    set(OpenBLAS_INCLUDE_DIRS ${OpenBLAS_INCLUDE_DIR})
    set(OpenBLAS_LIBRARIES ${OpenBLAS_LIBRARY})
    message(STATUS "Found OpenBLAS    (include: ${OpenBLAS_INCLUDE_DIR}, library: ${OpenBLAS_LIBRARY})")
endif()

mark_as_advanced(OpenBLAS_ROOT_DIR OpenBLAS_LIBRARY OpenBLAS_INCLUDE_DIR)