# vim:ft=python
# TODO: Test Mac.

# some options, help text says all
AddOption('--with-32bits', dest='32bits', action='store_true',
          help='Force 32 bit compilation ("-m32" GCC option) on Unix.')

AddOption('--with-debug', dest='debug', action='store_true',
          help='Add debugging symbols')

# general help
Help(
"""This build system compiles the msndfile Mex file.  To compile, use one of
the following build targets:
    msndfile  -> compile msndfile (default)
    makezip   -> create a zip file (contains msndfile + libsndfile)
    all       -> runs both msndfile and makezip
"""
)

matlab_is_32_bits = GetOption('32bits')

env_vars = Variables()
env_vars.Add('CC', 'The C compiler')

# the mex_builder tool automatically sets various environment variables
sndfile = Environment(tools = ['default', 'packaging', 'matlab'],
                      variables = env_vars)

# help on environment overrides
Help("""
The following environment variables can be overridden by passing them *after*
the call to scons, i.e. "scons CC=gcc":""")
Help(env_vars.GenerateHelpText(sndfile))

platform     = sndfile['PLATFORM']

# OS dependent stuff, we assume GCC on Unix like platforms
if platform == "posix":

    sndfile.Append(
        LIBPATH = "Linux",
        CCFLAGS = "-std=c99 -O2 -pedantic -Wall -Wextra -fdump-rtl-expand",
        LIBS    = ["m"]
    )

    if sndfile['CC'] == 'gcc':
        sndfile.Append(LINKFLAGS="-Wl,--as-needed")

    if matlab_is_32_bits:
        sndfile.Append(
            CCFLAGS    = "-m32",
            LINKFLAGS  = "-m32",
            CPPDEFINES = "_FILE_OFFSET_BITS=64"
        )

    sndfile_lib = "sndfile"

elif platform == "win32":

    sndfile.Append(LIBPATH="Win", CPPPATH="Win")

    sndfile_lib = "libsndfile-1"

elif platform == "darwin":

    sndfile.Append(
        LIBPATH = "Mac",
        CCFLAGS = "-std=c99 -O2 -pedantic -Wall -Wextra",
        LIBS    = ["m"]
    )

    sndfile_lib = "sndfile"
else:
    exit("Oops, not a supported platform.")

if not (GetOption('clean') or GetOption('help')):
    # look for libsndfile plus header and exit if either one isn't found
    conf = sndfile.Configure()
    if not conf.CheckLibWithHeader(sndfile_lib, 'sndfile.h', 'c'):
        exit("You need to install libsndfile(-dev)!")
    sndfile = conf.Finish()

if GetOption('debug'):
    sndfile.MergeFlags(["-g", "-O0"])

msndfile = sndfile.Mex("msndfile", ["msndfile.c"])

if platform == 'win32':
    # TODO: test debugging!
    sndfile_debug = sndfile.Clone().MergeFlags(["-g", "-O0"])
    msndfile_dbg  = sndfile_debug.Mex("msndfile", ["msndfile.c"])

    sndfile_vs = sndfile.MSVSProject(
        target      = "msndfile" + sndfile['MSVSPROJECTSUFFIX'],
        buildtarget = [msndfile, msndfile_dbg],
        runfile     = "matlab",
        srcs        = ["msndfile.c"],
        localincs   = ["msndfile.h"],
        incs        = ["sndfile.h"],
        variant     = ["Release", "Debug"]
    )
    Alias("vsproj", sndfile_vs)
    Help(
"""    vsproj    -> create a visual studio project file
"""
    )

# package the software

pkg_src = [msndfile, "msndfile.m"]
if platform == 'win32':
    pkg_src += [sndfile['SHLIBPREFIX'] + sndfile_lib + sndfile['SHLIBSUFFIX']]

sndfile_pkg = sndfile.Package(
    source      = pkg_src,
    NAME        = "msndfile",
    VERSION     = "0.1",
    PACKAGETYPE = "zip"
)

# some useful aliases
Alias("makezip", sndfile_pkg)
Alias("msndfile", msndfile)
Alias("all", [msndfile, sndfile_pkg])

# options help
Help(
"""
The following options are supported:
    --with-32bits   -> Force 32 bit compilation ("-m32" GCC option) on Unix.
    --with-debug    -> Add debugging symbols.
"""
)

Default(msndfile)
