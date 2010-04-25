# TODO: test Mac and Windows

import os

# Go ahead and define operating system independent options and dependencies; the
# 'mex' tool automatically sets various environment variables
sndfile_env = Environment(tools=['default', 'mex'])
sndfile_env.Append(
        CPPPATH = "include",
        WINDOWS_INSERT_MANIFEST = True,
        LIBS = ["m", "mex", "mx"])

Execute(Copy("mexversion.c", sndfile_env["MATLAB"]["SRC"] + os.sep + "mexversion.c"))

# do OS dependent stuff, e.g., Matlab path
# TODO: compare with env['MATLAB']['ARCH'] instead of os.name
if os.name == "posix":
    # enforce 32 bit compilation; add "exceptions" option, without which
    # any mex function that raises an exception (e.g. mexErrMsgTxt())
    # causes matlab to crash
    sndfile_env.Append(
            LIBPATH     = "Linux",
            CCFLAGS     = "-m32 -fexceptions -std=c99 -pedantic -Wall -Wextra -Wpadded -dr",
            LINKFLAGS   = "-m32"
            )
elif os.name == "nt":
    sndfile_env.Append(LIBPATH="Win", CPPPATH="Win")
elif os.name == "mac":
    sndfile_env.Append(LIBPATH="Mac")
else:
    exit("Oops, not a supported platform.")

# clone environment from msndfile to mexversion
mexversion_env = sndfile_env.Clone()
mexversion     = mexversion_env.SharedObject("mexversion.c")

# add msndfile target
sndfile_env.Append(LIBS = "sndfile")
sndfile_env.SharedLibrary("msndfile", ["msndfile.c", mexversion])
