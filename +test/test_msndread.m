function test_suite = test_msndread()
initTestSuite;

function ref_data = setup()

% the reference: the entire file imported by wavread
ref_data.in_wav = wavread('test_files/test.wav');

ref_data.file_info.samplerate   = 44100;
ref_data.file_info.channels     = 2;
ref_data.file_info.format       = 'RAW';
ref_data.file_info.sampleformat = 'PCM_16';

ref_data.file_size = wavread('test_files/test.wav', 'size');

function test_no_args(~)
% verify that msndread raises an error when called without input arguments

assertExceptionThrown(@msndfile.read, 'msndfile:argerror');

function test_wav_filename(ref_data)
% verify that file names without a suffix will have ".wav" appended and that an
% appropriate errors are thrown

% non-existent files should throw exceptions
assertExceptionThrown(@() msndfile.read('test_files/bla'), 'msndfile:sndfile');
assertExceptionThrown(@() msndfile.read('test_files/bla.wav'), 'msndfile:sndfile');

warning('off', 'msndfile:read:ambiguousname');
msndfile.read('test_files/test'); % should default to test.wav file
warning('on', 'msndfile:read:ambiguousname');
msndfile.read('test_files/only_wav/test');
msndfile.read('test_files/only_raw/test', [], [], ref_data.file_info);

% ambiguous file names should throw exceptions
assertExceptionThrown(@() msndfile.read('test_files/no_wav/test'), ...
                      'msndfile:read:ambiguousname');

function test_wav_multibyte_filename(ref_data)
% test multi-byte file name support

% UTF-8 encoded file name 'test_files/bläßgans'
utf8_bytes = [98 108 195 164 195 159]; % bläß
fname = ['test_files/' native2unicode(utf8_bytes, 'UTF-8') 'gans'];

% test multibyte file name support
warning('off', 'msndfile:read:ambiguousname');
msndfile.read(fname);
warning('on', 'msndfile:read:ambiguousname');
msndfile.read([fname '.wav']);

function test_wav_read(ref_data)
% verify that data is read correctly

test_wav  = msndfile.read('test_files/test.wav');
assertEqual(test_wav, ref_data.in_wav);

function test_wav_read_blockwise(ref_data)
% test block-wise reading

num_samples  = 16384;

in_blockwise     = zeros(num_samples, 2);
in_raw_blockwise = zeros(num_samples, 2);
in_blockwise(1:1024, :)     = msndfile.read('test_files/test.wav', 1024);
in_raw_blockwise(1:1024, :) = msndfile.read('test_files/test.raw', 1024, [], ref_data.file_info);
for kk = 1025:1024:num_samples
    in_blockwise(kk:kk+1023, :)     = msndfile.read('test_files/test.wav', [kk kk+1023]);
    in_raw_blockwise(kk:kk+1023, :) = msndfile.read('test_files/test.raw', [kk kk+1023], [], ref_data.file_info);
end

assertEqual(in_blockwise, ref_data.in_wav(1:num_samples,:));
assertEqual(in_raw_blockwise, ref_data.in_wav(1:num_samples,:));

function test_wav_read_ranges(ref_data)
% test reading ranges

% test if invalid ranges throw an error
msndfile.read('test_files/test.wav', [1 ref_data.file_size(1)]);
assertExceptionThrown(@() msndfile.read('test_files/test.wav', [0 ref_data.file_size(1)]), 'msndfile:argerror');
assertExceptionThrown(@() msndfile.read('test_files/test.wav', [-1 ref_data.file_size(1)]), 'msndfile:argerror');
assertExceptionThrown(@() msndfile.read('test_files/test.wav', [1 ref_data.file_size(1)+1]), 'msndfile:argerror');
msndfile.read('test_files/test.wav', ref_data.file_size(1));

% negative read range yields an error
if verLessThan('matlab', '7.1')
    assertExceptionThrown(@() msndfile.read('test_files/test.wav', -1), 'MATLAB:p32bitsize');
else
    assertExceptionThrown(@() msndfile.read('test_files/test.wav', -1), 'MATLAB:nomem');
end

% reading zero samples should yield an empty matrix
warning('off', 'msndfile:sndfile');
assertTrue(isempty(msndfile.read('test_files/test.wav', 0)));
warning('on', 'msndfile:sndfile');
assertExceptionThrown(@() msndfile.read('test_files/test.wav', ref_data.file_size(1)+1), 'msndfile:argerror');

function test_wav_input_size(ref_data)
% test 'size' input argument

file_size = msndfile.read('test_files/test.wav', 'size');
assertEqual(ref_data.file_size, file_size);

file_size = msndfile.read('test_files/test.raw', 'size', [], ref_data.file_info);
assertEqual(ref_data.file_size, file_size);

function test_wav_input_fmt(ref_data)
% test fmt input argument; doesn't make sense with FLAC

in_test = msndfile.read('test_files/test.wav', 'double');
in_wav  = wavread('test_files/test.wav', 'double');

assertEqual(in_test, in_wav);

in_test = msndfile.read('test_files/test.wav', 'native');
in_wav  = wavread('test_files/test.wav', 'native');

assertEqual(in_test, in_wav);

in_test = msndfile.read('test_files/test.raw', [], 'native', ref_data.file_info);

assertEqual(in_test, in_wav);

assertExceptionThrown(@() msndfile.read('test_files/test.wav', 'bla'), 'msndfile:argerror');

function test_wav_output_fs(ref_data)
% test 'fs' return value

[~, fs_ref] = wavread('test_files/test.wav', 'size');

[~, fs] = msndfile.read('test_files/test.wav', 'size');
assertEqual(fs_ref, fs);

[~, fs] = msndfile.read('test_files/test.raw', 'size', [], ref_data.file_info);
assertEqual(fs_ref, fs);

function test_wav_output_bits(ref_data)
% test 'nbits' return value

[~, ~, nbits_ref] = wavread('test_files/test.wav', 'size');

[~, ~, nbits] = msndfile.read('test_files/test.wav', 'size');
assertEqual(nbits_ref, nbits);

[~, ~, nbits] = msndfile.read('test_files/test.raw', 'size', [], ref_data.file_info);
assertEqual(nbits_ref, nbits);

function test_wav_output_opts(~)
% test opts return value; doesn't make sense with FLAC because the meta-data
% differs

[~, ~, ~, opts_ref]  = wavread('test_files/test.wav', 'size');
[~, ~, ~, opts]      = msndfile.read('test_files/test.wav', 'size');

% compare fmt fields
fmt_fields_ref = fieldnames(opts_ref.fmt);
fmt_fields     = fieldnames(opts.fmt);
assertEqual(fmt_fields_ref, fmt_fields);
assertEqual(length(fmt_fields_ref), length(fmt_fields));
for k=1:length(fmt_fields_ref)
    assertEqual(fmt_fields_ref{k}, fmt_fields{k});
    assertEqual(opts_ref.fmt.(fmt_fields_ref{k}), opts.fmt.(fmt_fields{k}));
end

% compare info fields
info_fields_ref = fieldnames(opts_ref.info);
info_fields     = fieldnames(opts.info);
assertEqual(info_fields_ref, info_fields);
assertEqual(length(info_fields_ref), length(info_fields));
for k=1:length(info_fields_ref)
    assertEqual(info_fields_ref{k}, info_fields{k});
    assertEqual(opts_ref.info.(info_fields_ref{k}), opts.info.(info_fields{k}));
end

function test_flac_output_opts(~)
% test opts return value; doesn't make sense with FLAC because the meta-data
% differs

[~, ~, ~, opts_ref]  = wavread('test_files/test.wav', 'size');
[~, ~, ~, opts]      = msndfile.read('test_files/test.flac', 'size');

% compare fmt fields
fmt_fields = fieldnames(opts_ref.fmt);

for k=1:length(fmt_fields)
    if strcmp(fmt_fields{k}, 'wFormatTag')
        continue;
    end
    assertEqual(opts_ref.fmt.(fmt_fields{k}), opts.fmt.(fmt_fields{k}));
end

% compare info fields
info_fields = fieldnames(opts_ref.info);
for k=1:length(info_fields)
    assertEqual(opts_ref.info.(info_fields{k}), opts.info.(info_fields{k}));
end

function test_flac_filename(ref_data)
% verify that file names without a suffix will have ".wav" appended and that an
% appropriate errors are thrown - FLAC only

msndfile.read('test_files/only_flac/test');

% ambiguous file names should throw exceptions
assertExceptionThrown(@() msndfile.read('test_files/no_wav/test'), ...
                      'msndfile:read:ambiguousname');

function test_flac_multibyte_filename(ref_data)
% test multi-byte file name support - FLAC only

% UTF-8 encoded file name 'test_files/bläßgans'
utf8_bytes = [98 108 195 164 195 159]; % bläß
fname = ['test_files/' native2unicode(utf8_bytes, 'UTF-8') 'gans'];

msndfile.read([fname '.flac']);

function test_flac_read(ref_data)
% verify that data is read correctly - FLAC only

test_flac = msndfile.read('test_files/test.flac');
assertEqual(test_flac, ref_data.in_wav);

function test_flac_read_blockwise(ref_data)
% test block-wise reading - FLAC only

num_samples  = 16384;

in_blockwise = zeros(num_samples, 2);

in_blockwise(1:1024, :) = msndfile.read('test_files/test.flac', 1024);
for kk = 1025:1024:num_samples
    in_blockwise(kk:kk+1023, :) = msndfile.read('test_files/test.flac', [kk kk+1023]);
end

assertEqual(in_blockwise, ref_data.in_wav(1:num_samples,:));

function test_flac_read_ranges(ref_data)
% test reading ranges - FLAC only

% test if invalid ranges throw an error
msndfile.read('test_files/test.flac', [1 ref_data.file_size(1)]);
assertExceptionThrown(@() msndfile.read('test_files/test.flac', [0 ref_data.file_size(1)]), 'msndfile:argerror');
assertExceptionThrown(@() msndfile.read('test_files/test.flac', [-1 ref_data.file_size(1)]), 'msndfile:argerror');
assertExceptionThrown(@() msndfile.read('test_files/test.flac', [1 ref_data.file_size(1)+1]), 'msndfile:argerror');
msndfile.read('test_files/test.flac', ref_data.file_size(1));

% negative read range yields an error
if verLessThan('matlab', '7.1')
    assertExceptionThrown(@() msndfile.read('test_files/test.flac', -1), 'MATLAB:p32bitsize');
else
    assertExceptionThrown(@() msndfile.read('test_files/test.flac', -1), 'MATLAB:nomem');
end

% reading zero samples should yield an empty matrix
warning('off', 'msndfile:sndfile');
assertTrue(isempty(msndfile.read('test_files/test.flac', 0)));
warning('on', 'msndfile:sndfile');
assertExceptionThrown(@() msndfile.read('test_files/test.flac', ref_data.file_size(1)+1), 'msndfile:argerror');

function test_flac_input_size(ref_data)
% test 'size' input argument - FLAC only

file_size = msndfile.read('test_files/test.flac', 'size');
assertEqual(ref_data.file_size, file_size);

function test_flac_output_fs(ref_data)
% test 'fs' return value - FLAC only

[~, fs_ref] = wavread('test_files/test.wav', 'size');

[~, fs] = msndfile.read('test_files/test.flac', 'size');
assertEqual(fs_ref, fs);

function test_flac_output_bits(ref_data)
% test 'nbits' return value - FLAC only

[~, ~, nbits_ref] = wavread('test_files/test.wav', 'size');

[~, ~, nbits] = msndfile.read('test_files/test.flac', 'size');
assertEqual(nbits_ref, nbits);

function test_raw_empty_args(~)
% verify that msndread raises an error when called with insufficient arguments

assertExceptionThrown(@() msndfile.read('test_files/test.raw', []), 'msndfile:sndfile');
assertExceptionThrown(@() msndfile.read('test_files/test.raw', [], []), 'msndfile:sndfile');
assertExceptionThrown(@() msndfile.read('test_files/test.raw', [], [], []), 'msndfile:sndfile');

function test_raw_incomplete_file_info(~)
% verify that msndread raises an error when file_info is incomplete

file_info.samplerate   = 44100;
assertExceptionThrown(@() msndfile.read('test_files/test.raw', [], [], file_info), 'msndfile:argerror');
file_info.channels     = 2;
assertExceptionThrown(@() msndfile.read('test_files/test.raw', [], [], file_info), 'msndfile:argerror');

% only now is file_info complete
file_info.sampleformat = 'PCM_16';
[in_sig, fs] = msndfile.read('test_files/test.raw', [], [], file_info);

% reset file_info
file_info = [];

file_info.samplerate   = 44100;
file_info.channels     = 2;
file_info.format       = 'RAW';
assertExceptionThrown(@() msndfile.read('test_files/test.raw', [], [], file_info), 'msndfile:argerror');
file_info.endianness   = 'FILE';
assertExceptionThrown(@() msndfile.read('test_files/test.raw', [], [], file_info), 'msndfile:argerror');
% only now is file_info complete
file_info.sampleformat = 'PCM_16';
[in_sig, fs] = msndfile.read('test_files/test.raw', [], [], file_info);

% endianness defaults to 'FILE'; it is unused here, test anyway
file_info.endianness   = 'FILE';
[in_sig, fs] = msndfile.read('test_files/test.raw', [], [], file_info);

function test_raw_read(ref_data)
% test the RAW file import

[in_raw, fs] = msndfile.read('test_files/test.raw', [], [], ref_data.file_info);

assertEqual(ref_data.in_wav, in_raw);
