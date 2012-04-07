function test_suite = test_msndblockread()
initTestSuite;

function ref_data = setup()

% the reference: the entire file imported by wavread
ref_data.in_wav = wavread('test_files/test.wav');

ref_data.file_info.samplerate   = 44100;
ref_data.file_info.channels     = 2;
ref_data.file_info.format       = 'RAW';
ref_data.file_info.sampleformat = 'PCM_16';

ref_data.file_size    = wavread('test_files/test.wav', 'size');
ref_data.block_size   = 1024;

function teardown(~)

% make sure there are no open files in case of an error to prevent spurious
% "File already open" errors
msndfile.blockread('closeall');

function test_open(ref_data)
% test open command

msndfile.blockread('open', 'test_files/test.wav');
msndfile.blockread('open', 'test_files/test.flac');
msndfile.blockread('open', 'test_files/test.raw', ref_data.file_info);

assertExceptionThrown(@() msndfile.blockread('open', 'test_files/test.wav'), '');
assertExceptionThrown(@() msndfile.blockread('open', 'test_files/test.flac'), '');
assertExceptionThrown(@() msndfile.blockread('open', 'test_files/test.raw', ref_data.file_info), '');

function test_close(ref_data)
% test close command

msndfile.blockread('open', 'test_files/test.wav');
msndfile.blockread('open', 'test_files/test.flac');
msndfile.blockread('open', 'test_files/test.raw', ref_data.file_info);

msndfile.blockread('close', 'test_files/test.wav');
msndfile.blockread('close', 'test_files/test.flac');
msndfile.blockread('close', 'test_files/test.raw', ref_data.file_info);

% should not throw an exception
warning('off', 'blockread:filenotopen');
msndfile.blockread('close', 'test_files/test.wav');
msndfile.blockread('close', 'test_files/test.flac');
msndfile.blockread('close', 'test_files/test.raw', ref_data.file_info);
warning('on', 'blockread:filenotopen');

function test_closeall(ref_data)
% test read command

msndfile.blockread('open', 'test_files/test.wav');
msndfile.blockread('open', 'test_files/test.flac');
msndfile.blockread('open', 'test_files/test.raw', ref_data.file_info);

msndfile.blockread('closeall');

assertExceptionThrown(@() msndfile.blockread('read', 'test_files/test.wav', ref_data.block_size), '');
assertExceptionThrown(@() msndfile.blockread('read', 'test_files/test.flac', ref_data.block_size), '');
assertExceptionThrown(@() msndfile.blockread('read', 'test_files/test.raw', ref_data.block_size, ref_data.file_info), '');

% should not throw an exception
warning('off', 'blockread:filenotopen');
msndfile.blockread('close', 'test_files/test.wav');
msndfile.blockread('close', 'test_files/test.flac');
msndfile.blockread('close', 'test_files/test.raw', ref_data.file_info);
warning('on', 'blockread:filenotopen');

function test_read(ref_data)
% test read command

block_size = ref_data.block_size;
in_wav     = ref_data.in_wav;

num_samples   = 16384;
in_blockwise1 = zeros(num_samples, ref_data.file_size(2));
in_blockwise2 = zeros(num_samples, ref_data.file_size(2));

% open two test files and read from them
msndfile.blockread('open', 'test_files/test.wav');
msndfile.blockread('open', 'test_files/test.flac');
for kk = 1:block_size:num_samples
    in_blockwise1(kk:kk+block_size-1, :)    = msndfile.blockread('read', 'test_files/test.wav', [kk kk+block_size-1]);
    in_blockwise2(kk:kk+block_size-1, :)    = msndfile.blockread('read', 'test_files/test.flac', [kk kk+block_size-1]);
end

% compare the outputs
assertEqual(in_blockwise1, in_wav(1:num_samples,:));
assertEqual(in_blockwise2, in_wav(1:num_samples,:));

% close the WAV file and try to read from it
msndfile.blockread('close', 'test_files/test.wav');
assertExceptionThrown(@() msndfile.blockread('read', 'test_files/test.wav', block_size), '');

% close the FLAC file and try to read from it
msndfile.blockread('close', 'test_files/test.flac');
assertExceptionThrown(@() msndfile.blockread('read', 'test_files/test.flac', block_size), '');

function test_read_only_blocksize(ref_data)
% test passing only the block size instead of the full range

block_size = ref_data.block_size;
in_wav     = ref_data.in_wav;

in_blockwise1 = zeros(ref_data.file_size);

msndfile.blockread('open', 'test_files/test.wav');

for kk = 1:block_size:ref_data.file_size(1)-block_size
    in_blockwise1(kk:kk+block_size-1, :) = msndfile.blockread('read', 'test_files/test.wav', block_size);
end
in_blockwise1(kk+block_size:end, :) = msndfile.blockread('read', 'test_files/test.wav', ref_data.file_size(1)-(kk+block_size)+1);

assertEqual(in_blockwise1, in_wav);

msndfile.blockread('close', 'test_files/test.wav');

function test_reopen(ref_data)
% test opening and closing files with repeated reading

block_size = ref_data.block_size;
in_wav     = ref_data.in_wav;

in_blockwise1 = zeros(ref_data.file_size);
in_blockwise2 = zeros(ref_data.file_size);
in_blockwise3 = zeros(ref_data.file_size);

msndfile.blockread('open', 'test_files/test.wav');
msndfile.blockread('open', 'test_files/test.flac');
msndfile.blockread('open', 'test_files/test.raw', ref_data.file_info);
for kk = 1:block_size:ref_data.file_size(1)-block_size
    in_blockwise1(kk:kk+block_size-1, :) = msndfile.blockread('read', 'test_files/test.wav', [kk kk+block_size-1]);
    in_blockwise2(kk:kk+block_size-1, :) = msndfile.blockread('read', 'test_files/test.flac', [kk kk+block_size-1]);
    in_blockwise3(kk:kk+block_size-1, :) = msndfile.blockread('read', 'test_files/test.raw', [kk kk+block_size-1]);
end
in_blockwise1(kk:end, :) = msndfile.blockread('read', 'test_files/test.wav', [kk ref_data.file_size(1)]);
in_blockwise2(kk:end, :) = msndfile.blockread('read', 'test_files/test.flac', [kk ref_data.file_size(1)]);
in_blockwise3(kk:end, :) = msndfile.blockread('read', 'test_files/test.raw', [kk ref_data.file_size(1)]);

assertEqual(in_blockwise1, in_wav);
assertEqual(in_blockwise2, in_wav);
assertEqual(in_blockwise3, in_wav);

msndfile.blockread('close', 'test_files/test.raw');

assertExceptionThrown(@() msndfile.blockread('read', 'test_files/test.raw', [kk kk+block_size-1]), '');

in_blockwise1(:) = 0;
in_blockwise2(:) = 0;
for kk = 1:block_size:ref_data.file_size(1)-block_size
    in_blockwise1(kk:kk+block_size-1, :) = msndfile.blockread('read', 'test_files/test.wav', [kk kk+block_size-1]);
    in_blockwise2(kk:kk+block_size-1, :) = msndfile.blockread('read', 'test_files/test.flac', [kk kk+block_size-1]);
end
in_blockwise1(kk:end, :) = msndfile.blockread('read', 'test_files/test.wav', [kk ref_data.file_size(1)]);
in_blockwise2(kk:end, :) = msndfile.blockread('read', 'test_files/test.flac', [kk ref_data.file_size(1)]);

assertEqual(in_blockwise1, in_wav);
assertEqual(in_blockwise2, in_wav);

msndfile.blockread('closeall');

assertExceptionThrown(@() msndfile.blockread('read', 'test_files/test.wav', [kk kk+block_size-1]), '');
assertExceptionThrown(@() msndfile.blockread('read', 'test_files/test.flac', [kk kk+block_size-1]), '');
assertExceptionThrown(@() msndfile.blockread('read', 'test_files/test.raw', [kk kk+block_size-1]), '');
