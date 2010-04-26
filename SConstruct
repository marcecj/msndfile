# TODO: Test Mac and Windows.
# TODO: Add an option to do
#           env = Environment(platform = 'win32')
#       to create a MSVC project for Windows from Linux.

import os

# Add an option to enforce 32 bit compilation for students using 32 bit Matlab
# on 64 bit platforms.
AddOption('--linux32', dest='linux32', action='store_true',
        help='Force 32 bit compilation ("-m32" GCC option) on Linux.')

# the mex tool automatically sets various environment variables
sndfile_env = Environment(tools=['default', ('matlab', {'mex': True})])

# this tells SCons where to find mexversion.c
Repository(sndfile_env["MATLAB"]["SRC"])

# define operating system independent options and dependencies
sndfile_env.Append(
        CPPPATH = "include",
        WINDOWS_INSERT_MANIFEST = True,
        LIBS = ["m", "mex", "mx"]
        )

# OS dependent stuff
if os.name == "posix":
    # add "exceptions" option, without which any mex function that raises an
    # exception (e.g., mexErrMsgTxt()) causes Matlab to crash
    sndfile_env.Append(LIBPATH="Linux",
            CCFLAGS = "-fexceptions -std=c99 -pedantic -Wall -Wextra -Wpadded -dr")
    if GetOption('linux32'):
        sndfile_env.Append(CCFLAGS="-m32", LINKFLAGS="-m32")
elif os.name == "nt":
    sndfile_env.Append(LIBPATH="Win", CPPPATH="Win")
elif os.name == "mac":
    sndfile_env.Append(LIBPATH="Mac",
            CCFLAGS="-fexceptions -std=c99 -pedantic")
else:
    exit("Oops, not a supported platform.")

# clone environment from msndfile to mexversion
mexversion_env = sndfile_env.Clone()

# do env dependent stuff
sndfile_env.Append(LIBS = "sndfile")

# add targets
mexversion = mexversion_env.SharedObject("mexversion.c")
sndfile_env.SharedLibrary("msndfile", ["msndfile.c", mexversion])
