% Simple script for compiling msndfile.

if strcmp(computer, 'PCWIN') || strcmp(computer, 'PCWIN64')
    mex -LWin -l'sndfile-1' -Iinclude -IWin src/msndread.c src/utils.c -outdir 'build'
    mex -LWin -l'sndfile-1' -Iinclude -IWin src/msndblockread.c src/utils.c -outdir 'build'
elseif strcmp(computer, 'GLNX86') || strcmp(computer, 'GLNXA64')
    mex -LLinux -lsndfile -Iinclude src/msndread.c src/utils.c -outdir 'build' ...
        CFLAGS='\$CFLAGS -std=c99'
    mex -LLinux -lsndfile -Iinclude src/msndblockread.c src/utils.c -outdir 'build' ...
        CFLAGS='\$CFLAGS -std=c99'
elseif strcmp(computer, 'MACI') || strcmp(computer, 'MACI')
    mex -LMac -lsndfile -Iinclude src/msndread.c src/utils.c -outdir 'build' ...
        CFLAGS='\$CFLAGS -std=c99'
    mex -LMac -lsndfile -Iinclude src/msndblockread.c src/utils.c -outdir 'build' ...
        CFLAGS='\$CFLAGS -std=c99'
end
