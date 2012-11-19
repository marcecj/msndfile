# vim:ft=python
# TODO: Test Mac.

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
    ('ASCIIDOCBACKEND', 'The backend used by asciidoc', 'html5'),
    ('A2XFORMAT', 'The format output by a2x'),
)

AddOption('--force-mingw',
          dest='forcemingw',
          default=False,
          action='store_true',
          help='Force the use of mingw on Windows platforms.'
         )

#####################
# set the environment
#####################

# the Matlab tool automatically sets various environment variables
if os.name == 'nt' and GetOption('forcemingw'):
    env = Environment(tools = ['mingw', 'filesystem', 'zip', 'packaging', 'matlab', 'asciidoc'],
                      variables = env_vars)
else:
    env = Environment(tools = ['default', 'packaging', 'matlab', 'asciidoc'],
                      variables = env_vars)

# The matlab package directory
env['pkg_dir'] = "+msndfile"

##############################################
# platform dependent environment configuration
##############################################

# assume a GCC-compatible compiler on Unix like platforms
if env['PLATFORM'] in ("posix", "darwin"):

    env.Append(CCFLAGS   = "-DNDEBUG -ansi -O2 -pedantic -Wall -Wextra",
               LINKFLAGS = "-Wl,-O1 -Wl,--no-copy-dt-needed-entries -Wl,--as-needed")

    # activate optimizations in GCC 4.5
    if env['CC'] == 'gcc' and env['CCVERSION'] >= '4.5':
        env.Append(CCFLAGS=[
            "-ftree-vectorize",
            "-ftree-vectorizer-verbose=2",
            "-floop-interchange",
            "-floop-strip-mine",
            "-floop-block",
            "-fno-reorder-blocks", # Matlab crashes without this!
        ])

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

elif env['PLATFORM'] == "win32":

    # enforce searching in the top-level Win directory
    win_path = os.sep.join([os.path.abspath(os.path.curdir), 'Win'])
    env.Append(LIBPATH = win_path, CPPPATH = win_path)

    env.Replace(WINDOWS_INSERT_DEF = True)

    sndfile_lib = "libsndfile-1"

else:

    exit("Oops, not a supported platform.")

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
msndfile, mfiles = env.SConscript(dirs='src',
                          variant_dir = "build",
                          exports     = "env",
                          duplicate   = False)

env['do_debug'] = True
msndfile_dbg = env.SConscript(dirs='src',
                              variant_dir = "debug",
                              exports     = "env",
                              duplicate   = False)

if env['PLATFORM'] == 'win32' and 'msvs' in env['TOOLS']:
    msndfile_vs = env.MSVSProject(
        target      = "msndfile" + env['MSVSPROJECTSUFFIX'],
        buildtarget = ["msndfile", "msndfile-dbg"],
        runfile     = os.sep.join([env['MATLAB']['ROOT'], "bin", "matlab.exe"]),
        srcs        = Glob(os.sep.join(["src", env['pkg_dir'], "*.c"]), strings=True),
        localincs   = Glob(os.sep.join(["src", env['pkg_dir'], "*.h"]), strings=True),
        incs        = os.sep.join(["Win", "sndfile.h"]),
        variant     = ["Release", "Debug"]
    )

    Alias("vsproj", msndfile_vs + msndfile + msndfile_dbg)

    Help("    vsproj       -> create a visual studio project file")

#####################
# build documentation
#
# By default, the documentation is compiled to HTML and PDF.  This can be
# changed by modifying the ASCIIDOCBACKEND and A2XFORMAT variables,
# respectively.
#####################

# set document builder flags
env.Append(A2XFLAGS = '-L -k')

# create an alias for building the documentation, but only if the asciidoc
# binary could be found
if env.WhereIs('asciidoc'):
    docs = env.AsciiDoc(['doc/index.txt'])
    Alias('doc', docs)
    Help("    doc          -> compiles documentation to HTML")
else:
    print "info: asciidoc not found, cannot build documentation"

# create an alias for building the documentation to PDF, but only if the a2x
# binary could be found
if env.WhereIs('a2x'):
    pdf = env.A2X(['doc/index.txt'])
    Alias('pdf', pdf)
    Help("\n    pdf          -> compiles documentation to PDF")
else:
    print "info: a2x not found, cannot build documentation"

######################
# package the software
######################

# define the package sources and corresponding install targets
pkg_src = msndfile + mfiles
if env['PLATFORM'] == 'win32':
    pkg_src.append(env.File('Win' + os.sep +
                        env['SHLIBPREFIX'] + sndfile_lib + env['SHLIBSUFFIX']))

msndfile_inst = env.Install(os.sep.join([env['DESTDIR'], env['pkg_dir']]),
                            pkg_src)

doc_inst = env.InstallAs(
    os.sep.join([env['DESTDIR'], 'manual.pdf']), pdf
)

sndfile_pkg = env.Package(
    NAME        = "msndfile",
    VERSION     = "1.0",
    PACKAGETYPE = "zip",
    source = doc_inst + msndfile_inst + ['README.md', 'LICENSE', 'LGPL-2.1']
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
The following options may be set:
    --force-mingw   ->  Force the use of mingw (Windows only).

The following environment variables can be overridden by passing them *after*
the call to scons, e.g., "scons CC=gcc":
"""
+ env_vars.GenerateHelpText(env)
)
