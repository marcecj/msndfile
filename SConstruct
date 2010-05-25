# TODO: Test Mac and Windows.

import os

# some options, help text says all
AddOption('--linux32', dest='linux32', action='store_true',
          help='Force 32 bit compilation ("-m32" GCC option) on Linux.')
AddOption('--make-msvc', dest='msvc', action='store_true',
          help='Create a MSVS solution file on Windows.')
AddOption('--debug-syms', dest='debug', action='store_true',
          help='Add debugging symbols')

matlab_is_32_bits = GetOption('linux32')
make_msvc         = GetOption('msvc')


# the mex tool automatically sets various environment variables
sndfile  = Environment(tools = ['default', ('matlab', {'mex': True})])
platform = sndfile['PLATFORM']

if platform == "win32":
    # Matlab doesn't follow the Windows standard and adds a 'lib' prefix anyway
    common_libs = ["libmex", "libmx"]
else:
    common_libs = ["mex", "mx", "m"]

# this tells SCons where to find mexversion.c
sndfile.Repository(sndfile["MATLAB"]["SRC"])

# OS dependent stuff, we assume GCC on Unix like platforms
if platform == "posix":
    # add "exceptions" option, without which any mex function that raises an
    # exception (e.g., mexErrMsgTxt()) causes Matlab to crash
    sndfile.Append(LIBPATH="Linux",
                   CCFLAGS = "-O2 -fexceptions -std=c99 -pedantic -pthread -Wall -Wextra -Wpadded -dr",
                   LINKFLAGS="--as-needed")
    if matlab_is_32_bits:
        sndfile.Append(CCFLAGS="-m32", LINKFLAGS="-m32",
                       CPPDEFINES="_FILE_OFFSET_BITS=64")
    sndfile_lib = "sndfile"
elif platform == "win32":
    sndfile.Append(LIBPATH="Win", CPPPATH="Win")
    sndfile_lib = "libsndfile-1"
elif platform == "darwin":
    sndfile.Append(LIBPATH="Mac",
                   CCFLAGS="-O2 -fexceptions -std=c99 -pedantic -pthread -Wall -Wextra -Wpadded",
                   LINKFLAGS="--as-needed")
    sndfile_lib = "sndfile"
else:
    exit("Oops, not a supported platform.")

# define operating system independent options and dependencies
sndfile.Append(CPPPATH = "include",
               WINDOWS_INSERT_MANIFEST = True)

# clone environment from msndfile to mexversion
mexversion = sndfile.Clone()

# look for libraries and corresponding headers and exit if they aren't found
# (autoconf-like behaviour)
if not GetOption('clean'):
    conf = sndfile.Configure()
    if not conf.CheckLibWithHeader(sndfile_lib, 'sndfile.h', 'c'):
        exit("You need to install libsndfile(-dev)!")
    sndfile = conf.Finish()

sndfile.Append(LIBS = common_libs)

if GetOption('debug'):
    sndfile.MergeFlags(["-g", "-O0"])

# add compile targets
if platform != 'win32':
    mexversion_obj = mexversion.SharedObject("mexversion.c")
    sndfile.SharedLibrary("msndfile", ["msndfile.c", mexversion_obj])
else:
    # optionally create MS VS project, otherwise just compile
    if make_msvc:
        sndfile_vs = sndfile.MSVSProject("msndfile"+sndfile['MSVSPROJECTSUFFIX'],
                                         ["msndfile.c", "msndfile.def"])
        MSVSSolution(target="msndfile", projects=[sndfile_vs])
    else:
        sndfile.SharedLibrary("msndfile", ["msndfile.c", "msndfile.def"])
