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

# modifiable environment variables
env_vars = Variables()
env_vars.Add('CC', 'The C compiler')
env_vars.Add('DESTDIR', 'The install destination', os.curdir)

AddOption('--force-mingw',
          dest='forcemingw',
          default=False,
          action='store_true',
          help='Force the use of mingw on Windows platforms.'
         )


# the Matlab tool automatically sets various environment variables
if os.name == 'nt' and GetOption('forcemingw'):
    env = Environment(tools = ['mingw', 'filesystem', 'zip', 'packaging', 'matlab'],
                      variables = env_vars)
else:
    env = Environment(tools = ['default', 'packaging', 'matlab'],
                      variables = env_vars)

# define an AsciiDoc builder
asciidoc = env.Builder(action = ['asciidoc -o ${TARGET} ${SOURCE}'],
                       suffix = '.html',
                       single_source = True)
env['BUILDERS']['AsciiDoc'] = asciidoc

# The matlab package directory
env['pkg_dir'] = "+msndfile"

cur_platform = env['PLATFORM']

# OS dependent stuff, we assume a GCC-compatible compiler on Unix like platforms
if cur_platform in ("posix", "darwin"):

    env.Append(CCFLAGS   = "-DNDEBUG -ansi -O2 -pedantic -Wall -Wextra",
               LINKFLAGS = "-Wl,-O1 -Wl,--no-copy-dt-needed-entries -Wl,--as-needed")

    # Activate optimizations in GCC 4.5
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

elif cur_platform == "win32":

    # enforce searching in the top-level Win directory
    win_path = os.sep.join([os.path.abspath(os.path.curdir), 'Win'])
    env.Append(LIBPATH = win_path, CPPPATH = win_path)

    env.Replace(WINDOWS_INSERT_DEF = True)

    sndfile_lib = "libsndfile-1"

else:

    exit("Oops, not a supported platform.")

if not (GetOption('clean') or GetOption('help')):
    conf = env.Configure()

    # look for libsndfile plus header and exit if either one isn't found
    if not conf.CheckLibWithHeader(sndfile_lib, 'sndfile.h', 'c'):
        exit("You need to install libsndfile(-dev)!")

    # we use the types defined in stdint.h, which not all versions of Visual
    # Studio have
    if not conf.CheckHeader('stdint.h', language='c'):
        exit("You need the stdint header!")

    env = conf.Finish()

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

if cur_platform == 'win32' and 'msvs' in env['TOOLS']:
    msndfile_vs = env.MSVSProject(
        target      = "msndfile" + env['MSVSPROJECTSUFFIX'],
        buildtarget = ["msndfile", "msndfile-dbg"],
        runfile     = os.sep.join([env['MATLAB']['ROOT'], "bin", "matlab.exe"]),
        srcs        = Glob(os.sep.join(["src", "*.c"]), strings=True),
        localincs   = Glob(os.sep.join(["src", "*.h"]), strings=True),
        incs        = os.sep.join(["Win", "sndfile.h"]),
        variant     = ["Release", "Debug"]
    )

    Alias("vsproj", msndfile_vs + msndfile + msndfile_dbg)

    Help("    vsproj       -> create a visual studio project file")

# package the software

# define the package sources and corresponding install targets
pkg_src = msndfile + mfiles
if cur_platform == 'win32':
    pkg_src.append(env.File('Win' + os.sep +
                        env['SHLIBPREFIX'] + sndfile_lib + env['SHLIBSUFFIX']))

msndfile_inst = env.Install(os.sep.join([env['DESTDIR'], env['pkg_dir']]),
                            pkg_src)
sndfile_pkg = env.Package(
    NAME        = "msndfile",
    VERSION     = "0.1",
    PACKAGETYPE = "zip"
)

# create an alias for building the documentation, but only if the asciidoc
# binary could be found
if env.WhereIs('asciidoc') is not None:
    docs = env.AsciiDoc(['README', 'INSTALL', 'LICENSE'])

    Alias('doc', docs)

    Help("    doc          -> compiles documentation to HTML")
else:
    print "asciidoc not found! Cannot build documentation."

# define some useful aliases
Alias("makezip", sndfile_pkg)
Alias("install", msndfile_inst)
Alias("msndfile", msndfile)
Alias("msndfile-dbg", msndfile_dbg)
Alias("all", msndfile + sndfile_pkg)

Default(msndfile)

Help(
"""\n
The following options may be set:
    --force-mingw   ->  Force the use of mingw (Windows only).

The following environment variables can be overridden by passing them *after*
the call to scons, i.e. "scons CC=gcc":"""
+ env_vars.GenerateHelpText(env)
)
