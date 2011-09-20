# vim:ft=python
# TODO: Test Mac.

import os
import platform

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

cur_platform = env['PLATFORM']

# OS dependent stuff, we assume GCC on Unix like platforms
if cur_platform == "posix":

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

    # if the system is 64 bit and Matlab is 32 bit, compile for 32 bit; since
    # Matlab currently only runs on x86 architectures, checking for x86_64
    # should suffice
    if platform.machine() == "x86_64" \
       and not env['MATLAB']['ARCH'].endswith('64'):
        env.Append(
            CCFLAGS    = "-m32",
            LINKFLAGS  = "-m32",
            CPPDEFINES = "_FILE_OFFSET_BITS=64"
        )

    sndfile_lib = "sndfile"

elif cur_platform == "win32":

    # enforce searching in the top-level Win directory
    win_path = os.sep.join([os.path.abspath(os.path.curdir), 'Win'])
    env.Append(LIBPATH=win_path, CPPPATH=win_path)

    env.Replace(WINDOWS_INSERT_DEF = True)

    sndfile_lib = "libsndfile-1"

elif cur_platform == "darwin":

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
if cur_platform == 'win32':
    msndfile_vs = env.MSVSProject(
        target      = "msndfile" + env['MSVSPROJECTSUFFIX'],
        buildtarget = ["msndfile", "msndfile-dbg"],
        runfile     = os.sep.join([env['MATLAB']['ROOT'], "bin", "matlab.exe"]),
        srcs        = Glob(os.sep.join(["src", "*.c"]), strings=True),
        localincs   = Glob(os.sep.join(["src", "*.h"]), strings=True),
        incs        = os.sep.join(["Win", "sndfile.h"]),
        variant     = ["Release", "Debug"]
    )

    Alias("vsproj", [msndfile_vs, msndfile, msndfile_dbg])

    win_help_text = """    vsproj    -> create a visual studio project file"""

# package the software

pkg_src = [msndfile, Glob(os.sep.join(['src', '*.m']))]
if cur_platform == 'win32':
    pkg_src += ['Win' + os.sep + env['SHLIBPREFIX'] + sndfile_lib + env['SHLIBSUFFIX']]

msndfile_inst = env.Install("msndfile", pkg_src)
sndfile_pkg = env.Package(
    NAME        = "msndfile",
    VERSION     = "0.1",
    PACKAGETYPE = "zip"
)

# create an alias for building the documentation
if env.WhereIs('asciidoc') is not None:
    docs = env.AsciiDoc(['README', 'INSTALL', 'LICENSE'])
    Alias('doc', docs)

# some useful aliases
Alias("makezip", sndfile_pkg)
Alias("install", msndfile_inst)
Alias("msndfile", msndfile)
Alias("msndfile-dbg", msndfile_dbg)
Alias("all", [msndfile, sndfile_pkg])

Default(msndfile)

# generate the help text
Help(
"""This build system compiles the msndfile Mex files.  To compile, use one of
the following build targets:
    msndfile     -> compile msndfile (default)
    msndfile-dbg -> compile msndfile with debugging information
    makezip      -> create a zip file (contains msndfile + libsndfile)
    install      -> install msndfile to directory "msndfile"
    doc          -> compiles documentation to HTML
    all          -> runs both msndfile and makezip
"""
+ win_help_text +
"""
The following environment variables can be overridden by passing them *after*
the call to scons, i.e. "scons CC=gcc":"""
+ env_vars.GenerateHelpText(env)
)
