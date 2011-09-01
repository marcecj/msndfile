% Simple script for compiling msndfile.

if strcmp(computer, 'PCWIN') || strcmp(computer, 'PCWIN64')
    mex -LWin -l'sndfile-1' -Iinclude -IWin src/msndfile.c -outdir 'build'
elseif strcmp(computer, 'GLNX86') || strcmp(computer, 'GLNXA64')
    mex -LLinux -lsndfile -Iinclude src/msndfile.c -outdir 'build' ...
        CFLAGS='\$CFLAGS -std=c99'
elseif strcmp(computer, 'MACI') || strcmp(computer, 'MACI')
    mex -LMac -lsndfile -Iinclude src/msndfile.c -outdir 'build' ...
        CFLAGS='\$CFLAGS -std=c99'
end
