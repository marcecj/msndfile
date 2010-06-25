# TODO: Test Mac and Windows.

# some options, help text says all
AddOption('--linux32', dest='linux32', action='store_true',
          help='Force 32 bit compilation ("-m32" GCC option) on Linux.')

AddOption('--make-msvs', dest='msvs', action='store_true',
          help='Create a MSVS solution file under Windows.')

AddOption('--debug-syms', dest='debug', action='store_true',
          help='Add debugging symbols')

matlab_is_32_bits = GetOption('linux32')
make_msvs         = GetOption('msvs')

# the mex tool automatically sets various environment variables
sndfile      = Environment(tools = ['default', ('matlab', {'mex': True})])
platform     = sndfile['PLATFORM']
msvs_variant = "Release"

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
                   CCFLAGS="-std=c99 -O2 -fexceptions -pedantic -pthread -Wall -Wextra -Wpadded -fdump-rtl-expand",
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
                   CCFLAGS="-std=c99 -O2 -fexceptions -pedantic -pthread -Wall -Wextra -Wpadded",
                   LINKFLAGS="--as-needed")
    sndfile_lib = "sndfile"
else:
    exit("Oops, not a supported platform.")

# define operating system independent options and dependencies
sndfile.Append(CPPPATH = "include",
               WINDOWS_INSERT_MANIFEST = True)

# clone environment from msndfile to mexversion
mexversion = sndfile.Clone()

if not (GetOption('clean') or GetOption('help')):
    # look for libsndfile plus header and exit if either one isn't found
    conf = sndfile.Configure()
    if not conf.CheckLibWithHeader(sndfile_lib, 'sndfile.h', 'c'):
        exit("You need to install libsndfile(-dev)!")
    sndfile = conf.Finish()

sndfile.Append(LIBS = common_libs)

if GetOption('debug'):
    sndfile.MergeFlags(["-g", "-O0"])
    msvs_variant = "Debug"

# add compile targets
if platform != 'win32':
    mexversion_obj = mexversion.SharedObject("mexversion.c")
    sndfile.SharedLibrary("msndfile", ["msndfile.c", mexversion_obj])
else:
    # optionally create MS VS project, otherwise just compile
    if make_msvs:
        sndfile_vs = sndfile.MSVSProject("msndfile"+sndfile['MSVSPROJECTSUFFIX'],
                                         ["msndfile.c", "msndfile.def"])
        MSVSSolution("msndfile", [sndfile_vs], msvs_variant)
    else:
        sndfile.SharedLibrary("msndfile", ["msndfile.c", "msndfile.def"])
