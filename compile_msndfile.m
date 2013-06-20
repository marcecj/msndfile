function compile_msndfile(varargin)
% COMPILE_MSNDFILE Compile msndfile.
%
% COMPILE_MSNDFILE will compile the msndfile suite of Mex extensions and copy
% the resulting binaries into the package directory '+msndfile'.
%
% Input parameters
% ----------------
%
% COMPILE_MSNDFILE has the following options (passed as parameter/value pairs):
%
%   Name        | Type    | Description                                  | Default
%   ==================================================================================
%   HaveStdintH | logical | whether the system has the stdint.h C header | true
%   Debug       | logical | whether to build msndfile with debug symbols | false
%   Destdir     | char    | where to install the package directory       | '.'
%   PkgDir      | char    | the name of the package directory            | '+msndfile'

%
%% input parsing
%

p = inputParser();
p.addParamValue('HaveStdintH' , true        , @islogical);
p.addParamValue('Debug'       , false       , @islogical);
p.addParamValue('Destdir'     , '.'         , @(x) ischar(x) && isdir(x));
p.addParamValue('PkgDir'      , '+msndfile' , @ischar);
p.parse(varargin{:});

%
%% build variables
%

% define whether stdint.h is available
have_stdint_h = p.Results.HaveStdintH;
% define whether to create a debug build
do_debug      = p.Results.Debug;
% the installation prefix
inst_prefix   = p.Results.Destdir;
% the name of the package directory
pkg_dir       = p.Results.PkgDir;

%
%% compile
%

mex_opts = '';
if ~verLessThan('matlab', '7.1')
    mex_opts = [mex_opts ' -largeArrayDims'];
end

% -fno-reorder-blocks needed with GCC >= 4.5, otherwise Matlab crashes
extra_flags = '-std=c99 -fno-reorder-blocks';
if have_stdint_h
    extra_flags = [extra_flags ' -DHAVE_STDINT_H'];
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
    cmd1 = ['mex ' mex_opts ' -LWin -l''sndfile-1'' -IWin ' src1 ' -outdir ' out_dir ' COMPFLAGS="$COMPFLAGS ' extra_flags '"'];
    cmd2 = ['mex ' mex_opts ' -LWin -l''sndfile-1'' -IWin ' src2 ' -outdir ' out_dir ' COMPFLAGS="$COMPFLAGS ' extra_flags '"'];
else
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
    copyfile('Win/libsndfile-1.dll', install_dir);
end
disp('Done!');
