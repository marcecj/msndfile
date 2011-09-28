% Simple script for compiling msndfile.

%
%% compile
%

% needed with GCC >= 4.5, otherwise Matlab crashes
extra_flags = '-fno-reorder-blocks';
out_dir     = 'build';

if strcmp(computer, 'PCWIN') || strcmp(computer, 'PCWIN64')
    cmd1 = ['mex -LWin -l''sndfile-1'' -Iinclude -IWin src/msndread.c src/utils.c -outdir ''' out_dir '''']
    cmd2 = ['mex -LWin -l''sndfile-1'' -Iinclude -IWin src/msndblockread.c src/utils.c -outdir ''' out_dir '''']
elseif strcmp(computer, 'GLNX86') || strcmp(computer, 'GLNXA64')
    default_flags = '-LLinux -lsndfile -Iinclude';
    cmd1 = ['mex ' default_flags ' src/msndread.c src/utils.c -outdir ' out_dir ' CFLAGS=''\$CFLAGS''' extra_flags];
    cmd2 = ['mex ' default_flags ' src/msndblockread.c src/utils.c -outdir ''' out_dir ''' CFLAGS=''\$CFLAGS ''' extra_flags];
elseif strcmp(computer, 'MACI') || strcmp(computer, 'MACI')
    default_flags = '-LMac -lsndfile -Iinclude';
    cmd1 = ['mex ' default_flags ' src/msndread.c src/utils.c -outdir ''' out_dir ''' CFLAGS=''\$CFLAGS ''' extra_flags];
    cmd2 = ['mex ' default_flags ' src/msndblockread.c src/utils.c -outdir ''' out_dir ''' CFLAGS=''\$CFLAGS ''' extra_flags];
end

if ~exist(out_dir, 'dir')
    mkdir('.', out_dir);
end

eval(cmd1); eval(cmd2);

%
%% install
%

inst_dir = '+msndfile';

if ~exist(inst_dir, 'dir')
    mkdir('.', inst_dir);
end

sources = {'msndread', 'msndblockread'};
targets = {'read', 'blockread'};
for kk=1:length(targets)
    copyfile(fullfile(out_dir, [sources{kk} '.' mexext]), ...
             fullfile(inst_dir, [targets{kk} '.' mexext]))
    copyfile(fullfile('src', [sources{kk} '.m']), ...
             fullfile(inst_dir, [targets{kk} '.m']))
end
