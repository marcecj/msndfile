# vim:ft=python
# TODO: Test Mac.

import os

# some options, help text says all
AddOption('--with-32bits', dest='32bits', action='store_true',
          help='Force 32 bit compilation ("-m32" GCC option) on Unix.')

# general help
Help(
"""This build system compiles the msndfile Mex file.  To compile, use one of
the following build targets:
    msndfile     -> compile msndfile (default)
    msndfile-dbg -> compile msndfile with debugging information
    makezip      -> create a zip file (contains msndfile + libsndfile)
    all          -> runs both msndfile and makezip
"""
)

matlab_is_32_bits = GetOption('32bits')

env_vars = Variables()
env_vars.Add('CC', 'The C compiler')

# the mex_builder tool automatically sets various environment variables
sndfile = Environment(tools = ['default', 'packaging', 'matlab'],
                      variables = env_vars)

# help on environment overrides
Help(
"""
The following environment variables can be overridden by passing them *after*
the call to scons, i.e. "scons CC=gcc":"""
)
Help(env_vars.GenerateHelpText(sndfile))

platform = sndfile['PLATFORM']

# OS dependent stuff, we assume GCC on Unix like platforms
if platform == "posix":

    sndfile.Append(
        LIBPATH = "Linux",
        CCFLAGS = "-std=c99 -O2 -pedantic -Wall -Wextra",
        LIBS    = ["m"]
    )

    if sndfile['CC'] == 'gcc':
        sndfile.Append(CCFLAGS="-fdump-rtl-expand")
        sndfile.Append(LINKFLAGS="-Wl,--as-needed")

    if matlab_is_32_bits:
        sndfile.Append(
            CCFLAGS    = "-m32",
            LINKFLAGS  = "-m32",
            CPPDEFINES = "_FILE_OFFSET_BITS=64"
        )

    sndfile_lib = "sndfile"

elif platform == "win32":

    # enforce searching in the top-level Win directory
    win_path = os.sep.join([os.path.abspath(os.path.curdir), 'Win'])

    sndfile.Append(LIBPATH=win_path, CPPPATH=win_path)
    sndfile.Replace(WINDOWS_INSERT_DEF = True)

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

do_debug = False
msndfile = sndfile.SConscript(os.sep.join(['src', 'SConstruct']),
                              variant_dir = "build",
                              exports     = ["sndfile", "do_debug"],
                              duplicate   = False)

do_debug = True
msndfile_dbg = sndfile.SConscript(os.sep.join(['src', 'SConstruct']),
                                  variant_dir = "debug",
                                  exports     = ["sndfile", "do_debug"],
                                  duplicate   = False)

if platform == 'win32':
    build_targets = [os.sep.join([d, "msndfile"]) + sndfile['MATLAB']['MEX_EXT']
                     for d in ["build", "debug"]]

    sndfile_vs = MSVSProject(
        target      = "msndfile" + sndfile['MSVSPROJECTSUFFIX'],
        buildtarget = build_targets,
        runfile     = os.sep.join([sndfile['MATLAB']['ROOT'], "bin", "matlab.exe"]),
        srcs        = os.sep.join(["src", "msndfile.c"]),
        localincs   = os.sep.join(["src", "msndfile.h"]),
        incs        = os.sep.join(["Win", "sndfile.h"]),
        variant     = ["Release", "Debug"]
    )
    Alias("vsproj", sndfile_vs)
    Help(
"""    vsproj    -> create a visual studio project file
"""
    )

# package the software

pkg_src = [msndfile, os.sep.join(["src", "msndfile.m"])]
if platform == 'win32':
    pkg_src += [os.sep.join(['Win', sndfile['SHLIBPREFIX'] + sndfile_lib + sndfile['SHLIBSUFFIX']])]

sndfile.Install(".", pkg_src)
sndfile_pkg = sndfile.Package(
    NAME        = "msndfile",
    VERSION     = "0.1",
    PACKAGETYPE = "zip"
)

# some useful aliases
Alias("makezip", sndfile_pkg)
Alias("msndfile", msndfile)
Alias("msndfile-dbg", msndfile_dbg)
Alias("all", [msndfile, sndfile_pkg])

# options help
Help(
"""
The following options are supported:
    --with-32bits   -> Force 32 bit compilation ("-m32" GCC option) on Unix.
"""
)

Default(msndfile)
