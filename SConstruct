# vim:ft=python
# TODO: Test Mac.

# some of the third party tools require at least Python 2.5
EnsurePythonVersion(2,5)

import os
import platform

Help(
"""This build system compiles the msndfile Mex files.  To compile, use one of
the following build targets:
    msndfile     -> compile msndfile (default)
    msndfile-dbg -> compile msndfile with debugging information
    makezip      -> create a zip file (contains msndfile + libsndfile)
    install      -> install msndfile to directory "msndfile"
    all          -> runs both msndfile and makezip
"""
)

###################################
# environment variables and options
###################################

# modifiable environment variables
env_vars = Variables()
env_vars.AddVariables(
    ('CC', 'The C compiler'),
    PathVariable('DESTDIR', 'The install destination', os.curdir,
                 PathVariable.PathIsDir),
    BoolVariable('verbose', 'Set more verbose build options', False),
    ('AD_BACKEND', 'The backend used by asciidoc', 'html5'),
    ('AD_DOCTYPE', 'The doctype set by asciidoc'),
    ('AD_FLAGS', 'Extra flags passed to asciidoc'),
    ('A2X_FORMAT', 'The format output by a2x'),
    ('A2X_DOCTYPE', 'The doctype set by a2x'),
    ('A2X_FLAGS', 'Extra flags passed to a2x'),
)

#####################
# set the environment
#####################

# initialise the environment
#
# NOTE: this uses a custom "default" tool that removes the Microsoft tools from
# the default tool lists, since msndfile requires C99 support
#
# TODO: find a better way to remove MSVC support
env = Environment(tools = ['default', 'packaging', 'matlab'],
                  variables = env_vars)

# if AsciiDoc is installed, add the asciidoc tool to the environment
ad_tool = Tool('asciidoc')
ad_exists = ad_tool.exists(env)
if ad_exists:
    ad_tool(env)
else:
    print "info: asciidoc not available, cannot build documentation."

# The Matlab package directory
env['pkg_dir'] = "+msndfile"

##############################################
# platform dependent environment configuration
##############################################

# assume a GCC-compatible compiler
env.Append(
    CCFLAGS    = "-std=c99 -O2 -pedantic -Wall -Wextra",
    LINKFLAGS  = "-Wl,-O1 -Wl,--no-copy-dt-needed-entries -Wl,--as-needed",
    CPPDEFINES = "NDEBUG"
)

# activate optimizations in GCC 4.5
if env['CC'] == 'gcc' and env['CCVERSION'] >= '4.5':
    env.Append(CCFLAGS=[
        "-ftree-vectorize",
        "-fno-reorder-blocks", # Matlab crashes without this!
    ])

    if env['PLATFORM'] != "win32":
        env.Append(CCFLAGS=[
            "-floop-interchange",
            "-floop-strip-mine",
            "-floop-block",
        ])

    if env['verbose']:
        env.Append(CCFLAGS="-ftree-vectorizer-verbose=2")

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

if env['PLATFORM'] == "win32":

    # enforce searching in the top-level Win directory
    win_path = os.sep.join([os.path.abspath(os.path.curdir), 'Win'])
    env.Append(LIBPATH = win_path, CPPPATH = win_path)

    env.Replace(WINDOWS_INSERT_DEF = True)

    sndfile_lib = "libsndfile-1"

########################################
# check the system for required features
########################################

if not (GetOption('clean') or GetOption('help')):
    conf = env.Configure()

    if not conf.CheckLibWithHeader(sndfile_lib, 'sndfile.h', 'c'):
        exit("error: you need to install libsndfile(-dev)")

    if conf.CheckCHeader('stdint.h'):
        env.Append(CPPDEFINES="HAVE_STDINT_H")
    elif conf.CheckCHeader('sys/types.h'):
        env.Append(CPPDEFINES="HAVE_SYS_TYPES_H")
        if env['PLATFORM'] == 'win32':
            print "info: using compatibility header in place of stdint.h"
        else:
            print "info: using sys/types.h in place of stdint.h"
    else:
        print "info: using compatibility header in place of stdint.h"

    env = conf.Finish()

#####################
# compilation targets
#
# These are: msndfile (build and debug variants), the corresponding m-Files, and
# a visual studio project (if the current platform is Windows and MinGW is not
# used).
#####################

env['do_debug'] = False
msndfile, mfiles = env.SConscript(
    dirs        = 'src',
    variant_dir = "build",
    exports     = "env",
    duplicate   = False
)

env['do_debug'] = True
msndfile_dbg = env.SConscript(
    dirs        = 'src',
    variant_dir = "debug",
    exports     = "env",
    duplicate   = False
)

if env['PLATFORM'] == 'win32' and 'msvs' in env['TOOLS']:
    msndfile_vs = env.MSVSProject(
        target      = "msndfile${MSVSPROJECTSUFFIX}",
        buildtarget = ["msndfile", "msndfile-dbg"],
        runfile     = os.sep.join(["${MATLAB['ROOT']}", "bin", "matlab.exe"]),
        srcs        = env.Glob(os.sep.join(["src", '$pkg_dir', "*.c"]), strings=True),
        localincs   = env.Glob(os.sep.join(["src", '$pkg_dir', "*.h"]), strings=True),
        incs        = os.sep.join(["Win", "sndfile.h"]),
        variant     = ["Release", "Debug"]
    )

    Alias("vsproj", msndfile_vs + msndfile + msndfile_dbg)

    Help("    vsproj       -> create a visual studio project file")

#####################
# build documentation
#
# By default, the documentation is compiled to HTML and PDF.  This can be
# changed by modifying the AD_BACKEND and A2X_FORMAT variables, respectively.
#####################

# set document builder flags
dblatex_opts = (
    "-s doc/db2latex_mod.sty",
    "-P xref.with.number.and.title",
)
env.Append(A2X_FLAGS = '-L')
env.Append(A2X_FLAGS = '--dblatex-opts "' + ' '.join(dblatex_opts) + '"')

if ad_exists:
    # build web and PDF documentation
    docs = env.AsciiDoc(['doc/index.txt'])
    pdf  = env.A2X(['doc/index.txt'])

    Alias('doc', docs)
    Alias('pdf', pdf)

    Help("    doc          -> compiles documentation to HTML\n")
    Help("    pdf          -> compiles documentation to PDF")

######################
# package the software
######################

# define the package sources and corresponding install targets
pkg_src = msndfile + mfiles
if env['PLATFORM'] == 'win32':
    pkg_src.append(
        env.File('Win' + os.sep +
                 '${SHLIBPREFIX}' + sndfile_lib + '${SHLIBSUFFIX}')
    )

install_path  = os.sep.join(['$DESTDIR', '$pkg_dir'])
msndfile_inst = env.Install(install_path, pkg_src)

sndfile_pkg = env.Package(
    NAME        = "msndfile",
    VERSION     = "1.0",
    PACKAGETYPE = "zip",
    source = msndfile_inst + ['README.md', 'LICENSE', 'LGPL-2.1']
)

#############
# miscellanea
#############

# define some useful aliases
Alias("makezip", sndfile_pkg)
Alias("install", msndfile_inst)
Alias("msndfile", msndfile)
Alias("msndfile-dbg", msndfile_dbg)
Alias("all", msndfile + sndfile_pkg)

# set the default target
Default(msndfile)

Help(
"""\n
The following environment variables can be overridden by passing them *after*
the call to scons, e.g., "scons CC=gcc":
"""
+ env_vars.GenerateHelpText(env)
)
