function test_suite = test_msndread()
initTestSuite;

function ref_data = setup()

% the reference: the entire file imported by wavread
ref_data.in_wav = wavread('test_files/test.wav');

ref_data.file_info.samplerate   = 44100;
ref_data.file_info.channels     = 2;
ref_data.file_info.format       = 'RAW';
ref_data.file_info.sampleformat = 'PCM_16';

function test_no_args(ref_data)
% verify that msndread raises an error when called without input arguments

assertExceptionThrown(@msndfile.read, '');

function test_raw_empty_range(ref_data)
% verify that msndread raises an error when called with insufficient arguments

assertExceptionThrown(@() msndfile.read('test_files/test.raw', []), '');

function test_raw_empty_range_fmt(ref_data)
% verify that msndread raises an error when called with insufficient arguments

assertExceptionThrown(@() msndfile.read('test_files/test.raw', [], []), '');

function test_raw_empty_range_fmt_info(ref_data)
% verify that msndread raises an error when called with insufficient arguments

assertExceptionThrown(@() msndfile.read('test_files/test.raw', [], [], []), '');

function test_raw_incomplete_file_info(ref_data)
% verify that msndread raises an error when file_info is incomplete

file_info.samplerate   = 44100;
assertExceptionThrown(@() msndfile.read('test_files/test.raw', [], [], file_info), '');
file_info.channels     = 2;
assertExceptionThrown(@() msndfile.read('test_files/test.raw', [], [], file_info), '');
file_info.format       = 'RAW';
assertExceptionThrown(@() msndfile.read('test_files/test.raw', [], [], file_info), '');
file_info.sampleformat = 'PCM_16';
[in_sig, fs] = msndfile.read('test_files/test.raw', [], [], file_info);

% file_info.endianness   = 'LITTLE'; % defaults to 'FILE'


function test_raw(ref_data)
% test the RAW file import

[in_raw, fs] = msndfile.read('test_files/test.raw', [], [], ref_data.file_info);

err_sum = sum(abs(ref_data.in_wav - in_raw));
assertTrue(all(err_sum==0));

function test_blockwise(ref_data)

num_samples  = 16384;

in_blockwise     = zeros(num_samples, 2);
in_raw_blockwise = zeros(num_samples, 2);
for kk = 1:1024:num_samples
    in_blockwise(kk:kk+1023, :)     = msndfile.read('test_files/test.wav', [kk kk+1023]);
    in_raw_blockwise(kk:kk+1023, :) = msndfile.read('test_files/test.raw', [kk kk+1023], [], ref_data.file_info);
end

err_sum = sum(abs(in_blockwise - ref_data.in_wav(1:num_samples,:)));
assertTrue(all(err_sum==0));

err_sum = sum(abs(in_raw_blockwise - ref_data.in_wav(1:num_samples,:)));
assertTrue(all(err_sum==0));

function test_size(ref_data)

[file_size_ref, fs_ref, nbits_ref] = wavread('test_files/test.wav', 'size');

[file_size, fs, nbits] = msndfile.read('test_files/test.wav', 'size');
assertEqual(file_size_ref, file_size);
assertEqual(fs_ref, fs);
assertEqual(nbits_ref, nbits);

[file_size, fs, nbits] = msndfile.read('test_files/test.raw', 'size', [], ref_data.file_info);
assertEqual(file_size_ref, file_size);
assertEqual(fs_ref, fs);
assertEqual(nbits_ref, nbits);

[file_size, fs, nbits] = msndfile.read('test_files/test.flac', 'size');
assertEqual(file_size_ref, file_size);
assertEqual(fs_ref, fs);
assertEqual(nbits_ref, nbits);

function test_fmt(ref_data)
% test fmt input argument

in_test = msndfile.read('test_files/test.wav', 'double');
in_wav  = wavread('test_files/test.wav', 'double');
err_sum = sum(abs(in_test - in_wav));

assertTrue(all(err_sum==0));

in_test = msndfile.read('test_files/test.wav', 'native');
in_wav  = wavread('test_files/test.wav', 'native');
err_sum = sum(abs(in_test - in_wav));

assertTrue(all(err_sum==0));

in_test = msndfile.read('test_files/test.raw', [], 'native', ref_data.file_info);
err_sum = sum(abs(in_test - in_wav));

assertTrue(all(err_sum==0));

function test_opts(ref_data)
% test opts return value separately from the others

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
info_fields_ref = fieldnames(opts_ref.fmt);
info_fields     = fieldnames(opts.fmt);
assertEqual(info_fields_ref, info_fields);
assertEqual(length(info_fields_ref), length(info_fields));
for k=1:length(info_fields_ref)
    assertEqual(info_fields_ref{k}, info_fields{k});
    assertEqual(opts_ref.fmt.(info_fields_ref{k}), opts.fmt.(info_fields{k}));
end
