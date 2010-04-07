# TODO: add some options (mostly for Linux, like whether to enforce 32 bits or not)
cmake_minimum_required(VERSION 2.6)

# NOTE: various variables start with the project name, such as *_SOURCE_DIR
project(MSNDFILE C)

# set(CMAKE_C_COMPILER "x86_64-pc-linux-gnu-gcc-4.1.2")
set(CMAKE_MODULE_PATH "${MSNDFILE_SOURCE_DIR}")

# find matlab related stuff (thank god somebody already wrote this)
# NOTE: I had to modify FindMatlab.cmake, it's nowhere near all-encompassing.
# It'll probably have to be adapted further. I hope I'll find the time to send a
# patch upstream (though maybe CMake 2.8 has an updated version).
include(FindMatlab)

# NOTE: this doesn't really do what I want, which is bail on configure with a
# nice message when a header isn't found instead of at compile time or with a
# stupid error message.
#
# include(CheckIncludeFile)
# check_include_file(sndfile.h SNDFILE_H)

# TODO: why must "lib" be prepended to library names?
if(WIN32)

    # FIXME: dlls are not found at runtime because they have to be in the same
    # folder as the executable, but they are not copied there
    message("Configuring for Windows")

    set(LIB_DIR Win)

    set(SNDFILE_LIB libsndfile-1)
    add_library(libsndfile SHARED IMPORTED)

    # Deal with stupid non-standard library/header locations under Windows.
    # Maybe more possibilities need to be added here.
    # (alternative: copy relevant files to project directory)
    # set(SNDFILE_DIRS
    #     "C:/Programme/Mega-Nerd/libsndfile")
    # find_path(SNDFILE_H_DIR sndfile.h ${SNDFILE_DIRS})
    # find_path(SNDFILE_LIB_DIR libsndfile-1 ${SNDFILE_DIRS})
    find_file(SNDFILE_H_WIN sndfile.h ${LIB_DIR})

    set(MEX_VERSION_C "${MATLAB_INCLUDE_DIR}/mexversion.rc")
    set(MEX_EXT "mexw32")

    # bundle libsndfile under Windows
    set(SNDFILE_H_DIR ${LIB_DIR})

elseif(APPLE)

    # TODO: test under OS X!
    message("Configuring for OS X")

    # add exceptions option, see "UNIX" section for explanation
    set(CMAKE_C_FLAGS
        "${CMAKE_C_FLAGS} -fexceptions")
    message("Compiler flags:\t${CMAKE_C_FLAGS}")

    set(LIB_DIR Mac)

    set(SNDFILE_LIB sndfile)
    add_library(libsndfile SHARED IMPORTED)

    set(MEX_VERSION_C "${MATLAB_INCLUDE_DIR}/../src/mexversion.c")
    set(MEX_EXT "mexmaci")

    # here we need to link to libm
    set(ADDITIONAL_LIBS "m")

elseif(UNIX)

    message("Configuring for Unix (Linux)")

    # enforce 32 bit compilation; add "exceptions" option, without which any mex
    # function that raises an exception (e.g. mexErrMsgTxt()) causes matlab to
    # crash
    set(CMAKE_C_FLAGS
        "${CMAKE_C_FLAGS} -m32 -fexceptions")
    message("Compiler flags:\t${CMAKE_C_FLAGS}")

    set(LIB_DIR Linux)

    set(SNDFILE_LIB sndfile)
    add_library(libsndfile SHARED IMPORTED)

    # abuse MATLAB_INCLUDE_DIR to get to the src directory
    set(MEX_VERSION_C "${MATLAB_INCLUDE_DIR}/../src/mexversion.c")
    set(MEX_EXT "mexglx")

    # additional CFLAGS; "-dr" dumps GCCs internal code representation (for a
    # call graph utility)
    set(EXTRA_CFLAGS "-std=c99 -Wall -Wextra -Wpadded -dr")
    # here we need to link to libm
    set(ADDITIONAL_LIBS "m")

else()

    message("Unsupported platform, exiting.")
    return()

endif()

# NOTE: link directories only apply to targets defined afterwards
include_directories(${MSNDFILE_SOURCE_DIR} ${MATLAB_INCLUDE_DIR} ${SNDFILE_H_DIR})
link_directories(${MSNDFILE_SOURCE_DIR}/${LIB_DIR} ${SNDFILE_LIB_DIR})

add_library(msndfile MODULE
    msndfile.c msndfile.def
    ${SNDFILE_H_WIN}
    ${MEX_VERSION_C})

set_target_properties(msndfile PROPERTIES
    COMPILE_FLAGS "${EXTRA_CFLAGS}"
    LINK_FLAGS ""
    DEFINE_SYMBOL "MATLAB_MEX_FILE"
    PREFIX ""
    SUFFIX ".${MEX_EXT}")

target_link_libraries(msndfile
    ${ADDITIONAL_LIBS} ${SNDFILE_LIB} ${MATLAB_MX_LIBRARY} ${MATLAB_MEX_LIBRARY})
