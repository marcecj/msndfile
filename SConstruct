# vim:ft=python
# TODO: Test Mac.

import os

# some options, help text says all
AddOption('--with-32bits', dest='32bits', action='store_true',
          help='Force 32 bit compilation ("-m32" GCC option) on Unix.')

# modifiable environment variables
env_vars = Variables()
env_vars.Add('CC', 'The C compiler')

# the matlab tool automatically sets various environment variables
env = Environment(tools = ['default', 'packaging', 'matlab'],
                  variables = env_vars)

# define an asciidoc builder
asciidoc = env.Builder(action = ['asciidoc -o $TARGET ${SOURCE}'],
                       suffix = '.html',
                       single_source = True)
env['BUILDERS']['AsciiDoc'] = asciidoc

platform = env['PLATFORM']

# OS dependent stuff, we assume GCC on Unix like platforms
if platform == "posix":

    env.Append(
        LIBPATH = "Linux",
        CCFLAGS = "-ansi -O2 -pedantic -Wall -Wextra",
        LIBS    = ["m"]
    )

    if env['CC'] == 'gcc':
        env.Append(CCFLAGS="-fdump-rtl-expand")
        # TODO: Currently these options don't do anything.  Maybe newer GCC
        # versions (with graphite) can vectorize the transposition for-loops?
        # env.Append(CCFLAGS=" -ftree-vectorize -ftree-vectorizer-verbose=2")
        env.Append(LINKFLAGS="-Wl,--as-needed")

    if GetOption('32bits'):
        env.Append(
            CCFLAGS    = "-m32",
            LINKFLAGS  = "-m32",
            CPPDEFINES = "_FILE_OFFSET_BITS=64"
        )

    sndfile_lib = "sndfile"

elif platform == "win32":

    # enforce searching in the top-level Win directory
    win_path = os.sep.join([os.path.abspath(os.path.curdir), 'Win'])
    env.Append(LIBPATH=win_path, CPPPATH=win_path)

    env.Replace(WINDOWS_INSERT_DEF = True)

    sndfile_lib = "libsndfile-1"

elif platform == "darwin":

    env.Append(
        CCFLAGS = "-ansi -O2 -pedantic -Wall -Wextra",
        LIBS    = ["m"]
    )

    sndfile_lib = "sndfile"

else:

    exit("Oops, not a supported platform.")

if not (GetOption('clean') or GetOption('help')):
    # look for libsndfile plus header and exit if either one isn't found
    conf = env.Configure()
    if not conf.CheckLibWithHeader(sndfile_lib, 'sndfile.h', 'c'):
        exit("You need to install libsndfile(-dev)!")
    env = conf.Finish()

do_debug = False
msndfile = env.SConscript(os.sep.join(['src', 'SConstruct']),
                          variant_dir = "build",
                          exports     = ["env", "do_debug"],
                          duplicate   = False)

do_debug = True
msndfile_dbg = env.SConscript(os.sep.join(['src', 'SConstruct']),
                              variant_dir = "debug",
                              exports     = ["env", "do_debug"],
                              duplicate   = False)

win_help_text = ""
if platform == 'win32':
    build_targets = [d + os.sep + t + env['MATLAB']['MEX_EXT']
                     for d in ("build", "debug")
                     for t in ("msndread", "msndblockread")]

    sndfile_vs = MSVSProject(
        target      = "msndfile" + env['MSVSPROJECTSUFFIX'],
        buildtarget = build_targets,
        runfile     = os.sep.join([env['MATLAB']['ROOT'], "bin", "matlab.exe"]),
        srcs        = Glob(os.sep.join(["src", "*.c"]), strings=True),
        localincs   = Glob(os.sep.join(["src", "*.h"]), strings=True),
        incs        = os.sep.join(["Win", "sndfile.h"]),
        variant     = ["Release", "Debug"]
    )
    Alias("vsproj", sndfile_vs)

    win_help_text = """    vsproj    -> create a visual studio project file"""

# package the software

pkg_src = [msndfile, Glob(os.sep.join(['src', '*.m']))]
if platform == 'win32':
    pkg_src += ['Win' + os.sep + env['SHLIBPREFIX'] + sndfile_lib + env['SHLIBSUFFIX']]

env.Install(".", pkg_src)
sndfile_pkg = env.Package(
    NAME        = "msndfile",
    VERSION     = "0.1",
    PACKAGETYPE = "zip"
)

# create an alias for building the documentation
docs = env.AsciiDoc(['README', 'INSTALL', 'LICENSE'])

# some useful aliases
Alias("makezip", sndfile_pkg)
Alias("msndfile", msndfile)
Alias("msndfile-dbg", msndfile_dbg)
Alias('doc', docs)
Alias("all", [msndfile, sndfile_pkg])

Default(msndfile)

# generate the help text
Help(
"""This build system compiles the msndfile Mex files.  To compile, use one of
the following build targets:
    msndfile     -> compile msndfile (default)
    msndfile-dbg -> compile msndfile with debugging information
    makezip      -> create a zip file (contains msndfile + libsndfile)
    doc          -> compiles documentation to HTML
    all          -> runs both msndfile and makezip
"""
+ win_help_text +
"""
The following environment variables can be overridden by passing them *after*
the call to scons, i.e. "scons CC=gcc":"""
+ env_vars.GenerateHelpText(env) +
"""
The following options are supported:
    --with-32bits   -> Force 32 bit compilation ("-m32" GCC option) on Unix.
"""
)
