# TODO: Test Mac and Windows.

# some options, help text says all
AddOption('--with-32bits', dest='32bits', action='store_true',
          help='Force 32 bit compilation ("-m32" GCC option) on Unix.')

AddOption('--with-msvs', dest='msvs', action='store_true',
          help='Create a MSVS solution file under Windows.')

AddOption('--with-debug', dest='debug', action='store_true',
          help='Add debugging symbols')

matlab_is_32_bits = GetOption('32bits')
make_msvs         = GetOption('msvs')

# the mex_builder tool automatically sets various environment variables
sndfile      = Environment(tools = ['default', 'packaging', ('mex_builder', {'mex': True})])

# print sndfile['CC']
# sndfile.Replace(CC="clang")

platform     = sndfile['PLATFORM']
msvs_variant = "Release"

# OS dependent stuff, we assume GCC on Unix like platforms
if platform == "posix":
    sndfile.Append(LIBPATH="Linux",
                   CCFLAGS="-std=c99 -O2 -pedantic -Wall -Wextra -fdump-rtl-expand",
                   LIBS=["m"])
    if sndfile['CC'] == 'gcc':
        sndfile.Append(LINKFLAGS="-Wl,--as-needed")
    if matlab_is_32_bits:
        sndfile.Append(CCFLAGS="-m32", LINKFLAGS="-m32",
                       CPPDEFINES="_FILE_OFFSET_BITS=64")
    sndfile_lib = "sndfile"
elif platform == "win32":
    sndfile.Append(LIBPATH="Win", CPPPATH="Win")
    sndfile_lib = "libsndfile-1"
elif platform == "darwin":
    sndfile.Append(LIBPATH="Mac",
                   CCFLAGS="-std=c99 -O2 -pedantic -Wall -Wextra",
                   LIBS=["m"])
    sndfile_lib = "sndfile"
else:
    exit("Oops, not a supported platform.")

# define operating system independent options and dependencies
sndfile.Append(CPPPATH = "include")

if not (GetOption('clean') or GetOption('help')):
    # look for libsndfile plus header and exit if either one isn't found
    conf = sndfile.Configure()
    if not conf.CheckLibWithHeader(sndfile_lib, 'sndfile.h', 'c'):
        exit("You need to install libsndfile(-dev)!")
    sndfile = conf.Finish()

if GetOption('debug'):
    sndfile.MergeFlags(["-g", "-O0"])
    msvs_variant = "Debug"

# add compile targets
if platform != 'win32':
    msndfile = sndfile.MexExtension("msndfile", ["msndfile.c"])
else:
    # optionally create MS VS project, otherwise just compile
    sndfile.MexExtension("msndfile", ["msndfile.c", "msndfile.def"],
                         only_deps=make_msvs)
    if make_msvs:
        sndfile_vs = sndfile.MSVSProject("msndfile"+sndfile['MSVSPROJECTSUFFIX'],
                                         ["msndfile.c", "msndfile.def"])
        MSVSSolution("msndfile", [sndfile_vs], msvs_variant)

# package the software
sndfile.Package(
    source      = [msndfile, "msndfile.m"],
    NAME        = "msndfile",
    VERSION     = "0.1",
    PACKAGETYPE = "zip"
)
