# TODO: Test Mac and Windows.
# TODO: Add an option to do
#           env = Environment(platform = 'win32')
#       to create a MSVC project for Windows from Linux.

import os

# Add an option to enforce 32 bit compilation for students using 32 bit Matlab
# on 64 bit platforms.
AddOption('--linux32', dest='linux32', action='store_true',
          help='Force 32 bit compilation ("-m32" GCC option) on Linux.')
AddOption('--make-msvc', dest='msvc', action='store_true',
          help='Create a MSVC project file.')

# the mex tool automatically sets various environment variables
sndfile = Environment(tools=['default', ('matlab', {'mex': True})])
platform = sndfile['PLATFORM']

# this tells SCons where to find mexversion.c
Repository(sndfile["MATLAB"]["SRC"])

# define operating system independent options and dependencies
sndfile.Append(
    CPPPATH = "include",
    LIBS    = (["libmex", "libmx"] if platform == "win32" else ["mex", "mx"]),
    WINDOWS_INSERT_MANIFEST = True,
)
if os.name != 'nt':
    sndfile.Append(LIBS="m")

# OS dependent stuff, we assume GCC on Unix like platforms
if os.name == "posix":
    # add "exceptions" option, without which any mex function that raises an
    # exception (e.g., mexErrMsgTxt()) causes Matlab to crash; _FILE_OFFSET_BITS
    # fixes libsndfile errors
    sndfile.Append(LIBPATH="Linux",
                   CCFLAGS = "-fexceptions -pthread -std=c99 -pedantic -Wall -Wextra -Wpadded -dr")
    if GetOption('linux32'):
        sndfile.Append(CCFLAGS="-m32", LINKFLAGS="-m32",
                       CPPDEFINES="_FILE_OFFSET_BITS=64")
    sndfile_lib = "sndfile"
elif os.name == "nt":
    sndfile.Append(LIBPATH="Win", CPPPATH="Win")
    sndfile_lib = "libsndfile-1"
elif os.name == "mac":
    sndfile.Append(LIBPATH="Mac",
                   CCFLAGS="-fexceptions -std=c99 -pedantic")
    sndfile_lib = "sndfile"
else:
    exit("Oops, not a supported platform.")

# clone environment from msndfile to mexversion
mexversion = sndfile.Clone()

# do env dependent stuff
sndfile.Append(LIBS = sndfile_lib)

# add targets
if os.name != 'nt':
    mexversion_obj = mexversion.SharedObject("mexversion.c")
    sndfile.SharedLibrary("msndfile", ["msndfile.c", mexversion_obj])
else:
    sndfile.SharedLibrary("msndfile", ["msndfile.c", "msndfile.def"])
