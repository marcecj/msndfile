% Simple script for compiling msndfile.

if strcmp(computer, 'PCWIN') || strcmp(computer, 'PCWIN64') 
    mex -LWin -l'sndfile-1' -Iinclude -IWin msndfile.c
elseif strcmp(computer, 'GLNX86') || strcmp(computer, 'GLNXA64') 
    mex -LLinux -lsndfile -Iinclude msndfile.c
elseif strcmp(computer, 'MACI') || strcmp(computer, 'MACI')
    mex -LMac -lsndfile -Iinclude msndfile.c
end
