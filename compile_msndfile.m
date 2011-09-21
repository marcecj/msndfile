% Simple script for compiling msndfile.

% needed with GCC >= 4.5, otherwise Matlab crashes
extra_flags = '-fno-reorder-blocks';

if strcmp(computer, 'PCWIN') || strcmp(computer, 'PCWIN64')
    cmd1 = 'mex -LWin -l''sndfile-1'' -Iinclude -IWin src/msndread.c src/utils.c -outdir ''build'''
    cmd2 = 'mex -LWin -l''sndfile-1'' -Iinclude -IWin src/msndblockread.c src/utils.c -outdir ''build'''
elseif strcmp(computer, 'GLNX86') || strcmp(computer, 'GLNXA64')
    default_flags = '-LLinux -lsndfile -Iinclude';
    cmd1 = ['mex ' default_flags ' src/msndread.c src/utils.c -outdir ''build'' CFLAGS=''\$CFLAGS''' extra_flags];
    cmd2 = ['mex ' default_flags ' src/msndblockread.c src/utils.c -outdir ''build'' CFLAGS=''\$CFLAGS ''' extra_flags];
elseif strcmp(computer, 'MACI') || strcmp(computer, 'MACI')
    default_flags = '-LMac -lsndfile -Iinclude';
    cmd1 = ['mex ' default_flags ' src/msndread.c src/utils.c -outdir ''build'' CFLAGS=''\$CFLAGS ''' extra_flags];
    cmd2 = ['mex ' default_flags ' src/msndblockread.c src/utils.c -outdir ''build'' CFLAGS=''\$CFLAGS ''' extra_flags];
end

eval(cmd1);
eval(cmd2);
