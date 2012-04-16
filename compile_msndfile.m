% Simple script for compiling msndfile.

%
%% build variables
%

% define whether stdint.h is available
have_stdint_h = false;
% define whether to create a debug build
do_debug      = false;
% the installation prefix
inst_prefix   = '.';
% the name of the package directory
pkg_dir       = '+msndfile';

%
%% compile
%

mex_opts = '';
if ~verLessThan('matlab', '7.1')
    mex_opts = [mex_opts ' -largeArrayDims'];
end

extra_flags = '';
if ispc && have_stdint_h
    extra_flags = '-DHAVE_STDINT_H';
elseif ~ispc
    % assume stdint.h; needed with GCC >= 4.5, otherwise Matlab crashes
    extra_flags = '-DHAVE_STDINT_H -fno-reorder-blocks';
end

src_dir     = 'src/+msndfile';

% the build directory
if do_debug
    mex_opts     = [mex_opts ' -g'];
    build_prefix = 'debug';
else
    build_prefix = 'build';
end
out_dir = [build_prefix '/' pkg_dir];

src1 = [src_dir '/read.c ' ...
        src_dir '/utils.c ' ...
        src_dir '/read_utils.c'];
src2 = [src_dir '/blockread.c ' ...
        src_dir '/utils.c ' ...
        src_dir '/audio_files.c'];

if ispc
    cmd1 = ['mex ' mex_opts ' -LWin -l''sndfile-1'' -IWin ' extra_flags ' ' src1 ' -outdir ''' out_dir ''''];
    cmd2 = ['mex ' mex_opts ' -LWin -l''sndfile-1'' -IWin ' extra_flags ' ' src2 ' -outdir ''' out_dir ''''];
elseif isunix
    cmd1 = ['mex ' mex_opts ' -lsndfile ' src1 ' -outdir ' out_dir ' CFLAGS="\$CFLAGS ' extra_flags '"'];
    cmd2 = ['mex ' mex_opts ' -lsndfile ' src2 ' -outdir ' out_dir ' CFLAGS="\$CFLAGS ' extra_flags '"'];
elseif ismac
    cmd1 = ['mex ' mex_opts ' -lsndfile ' src1 ' -outdir ' out_dir ' CFLAGS="\$CFLAGS ' extra_flags '"'];
    cmd2 = ['mex ' mex_opts ' -lsndfile ' src2 ' -outdir ' out_dir ' CFLAGS="\$CFLAGS ' extra_flags '"'];
end

if ~exist(out_dir, 'dir')
    mkdir('.', out_dir);
end

eval(cmd1);
eval(cmd2);

%
%% install
%

install_dir = [inst_prefix '/' pkg_dir];
copyfile(out_dir, install_dir);
copyfile([src_dir '/*.m'], install_dir);

if ispc
    disp('Copying libsndfile...');
    copyfile('Win/libsndfile-1.dll', [inst_prefix '/' pkg_dir]);
end
disp('Done!');
