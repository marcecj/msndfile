% Simple script for compiling msndfile.

% define whether stdint.h is available
have_stdint_h = false;

%
%% compile
%

win_flags = '';
if ~have_stdint_h
    win_flags = '-DNOT_HAVE_STDINT_H';
end

% needed with GCC >= 4.5, otherwise Matlab crashes
extra_flags = '-fno-reorder-blocks';
src_dir     = 'src/+msndfile';
out_dir     = 'build/+msndfile';

src1 = [src_dir '/read.c ' ...
        src_dir '/utils.c ' ...
        src_dir '/read_utils.c'];
src2 = [src_dir '/blockread.c ' ...
        src_dir '/utils.c ' ...
        src_dir '/audio_files.c'];

if strcmp(computer, 'PCWIN') || strcmp(computer, 'PCWIN64')
    cmd1 = ['mex -LWin -l''sndfile-1'' -IWin ' win_flags ' ' src1 ' -outdir ''' out_dir ''''];
    cmd2 = ['mex -LWin -l''sndfile-1'' -IWin ' win_flags ' ' src2 ' -outdir ''' out_dir ''''];
elseif strcmp(computer, 'GLNX86') || strcmp(computer, 'GLNXA64')
    cmd1 = ['mex -lsndfile ' src1 ' -outdir ' out_dir ' CFLAGS=''\$CFLAGS''' extra_flags];
    cmd2 = ['mex -lsndfile ' src2 ' -outdir ' out_dir ' CFLAGS=''\$CFLAGS''' extra_flags];
elseif strcmp(computer, 'MACI') || strcmp(computer, 'MACI')
    cmd1 = ['mex -lsndfile ' src1 ' -outdir ' out_dir ' CFLAGS=''\$CFLAGS ''' extra_flags];
    cmd2 = ['mex -lsndfile ' src2 ' -outdir ' out_dir ' CFLAGS=''\$CFLAGS ''' extra_flags];
end

if ~exist(out_dir, 'dir')
    mkdir('.', out_dir);
end

disp(cmd1);
eval(cmd1);
disp(cmd2);
eval(cmd2);

if strcmp(computer, 'PCWIN') || strcmp(computer, 'PCWIN64')
    disp('Copying libsndfile...');
    copyfile('Win/libsndfile-1.dll', out_dir);
end
disp('Done!');
