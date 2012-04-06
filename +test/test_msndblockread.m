function test_suite = test_msndblockread()
initTestSuite;

% TODO: clean this up and have the functions test separate things
function ref_data = setup()

% the reference: the entire file imported by wavread
ref_data.in_wav = wavread('test_files/test.wav');

ref_data.file_info.samplerate   = 44100;
ref_data.file_info.channels     = 2;
ref_data.file_info.format       = 'RAW';
ref_data.file_info.sampleformat = 'PCM_16';

ref_data.file_size    = wavread('test_files/test.wav', 'size');
ref_data.block_size   = 1024;

function teardown(ref_data)

% make sure there are no open files in case of an error to prevent spurious
% "File already open" errors
msndfile.blockread('closeall');

function test_read(ref_data)
% Test 1: reading from various files

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

% close the WAV file
msndfile.blockread('close', 'test_files/test.wav');

% compare the outputs
err_sum = sum(abs(in_blockwise1 - in_wav(1:num_samples,:)));
assertTrue(all(err_sum==0));
err_sum = sum(abs(in_blockwise2 - in_wav(1:num_samples,:)));
assertTrue(all(err_sum==0));

% open the WAV file again and read from it
msndfile.blockread('open', 'test_files/test.wav');
for kk = 1:block_size:num_samples
    in_blockwise1(kk:kk+block_size-1, :) = msndfile.blockread('read', 'test_files/test.wav', [kk kk+block_size-1], false).';
end

% close the WAV file again
msndfile.blockread('close', 'test_files/test.wav');

% compare the WAV outputs again
err_sum = sum(abs(in_blockwise1  - in_wav(1:num_samples,:)));
assertTrue(all(err_sum==0));

% read from the FLAC file again and close it
for kk = 1:block_size:num_samples
    in_blockwise2(kk:kk+block_size-1, :)    = msndfile.blockread('read', 'test_files/test.flac', [kk kk+block_size-1]);
end
msndfile.blockread('close', 'test_files/test.flac');

in_blockwise = zeros(ref_data.file_size);
msndfile.blockread('open', 'test_files/test.wav');
for kk = 1:block_size:ref_data.file_size(1)-block_size
    in_blockwise(kk:kk+block_size-1, :) = msndfile.blockread('read', 'test_files/test.wav', [kk kk+block_size-1]);
end
in_blockwise(kk:end, :) = msndfile.blockread('read', 'test_files/test.wav', [kk ref_data.file_size(1)]);
msndfile.blockread('close', 'test_files/test.wav');

err_sum = sum(abs(in_blockwise - in_wav));
assertTrue(all(err_sum==0));

%
%% Test 2: opening and closing files
%

function test_open_close(ref_data)

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

err_sum = sum(abs(in_blockwise1 - in_wav));
assertTrue(all(err_sum==0));
err_sum = sum(abs(in_blockwise2 - in_wav));
assertTrue(all(err_sum==0));
err_sum = sum(abs(in_blockwise3 - in_wav));
assertTrue(all(err_sum==0));

msndfile.blockread('close', 'test_files/test.raw');

assertExceptionThrown(@() msndfile.blockread('read', 'test_files/test.raw', [kk kk+block_size-1]), '');

in_blockwise1 = zeros(ref_data.file_size);
in_blockwise2 = zeros(ref_data.file_size);
for kk = 1:block_size:ref_data.file_size(1)-block_size
    in_blockwise1(kk:kk+block_size-1, :) = msndfile.blockread('read', 'test_files/test.wav', [kk kk+block_size-1]);
    in_blockwise2(kk:kk+block_size-1, :) = msndfile.blockread('read', 'test_files/test.flac', [kk kk+block_size-1]);
end
in_blockwise1(kk:end, :) = msndfile.blockread('read', 'test_files/test.wav', [kk ref_data.file_size(1)]);
in_blockwise2(kk:end, :) = msndfile.blockread('read', 'test_files/test.flac', [kk ref_data.file_size(1)]);

err_sum = sum(abs(in_blockwise1 - in_wav));
assertTrue(all(err_sum==0));

err_sum = sum(abs(in_blockwise2 - in_wav));
assertTrue(all(err_sum==0));

msndfile.blockread('closeall');

assertExceptionThrown(@() msndfile.blockread('read', 'test_files/test.wav', [kk kk+block_size-1]), '');

assertExceptionThrown(@() msndfile.blockread('read', 'test_files/test.flac', [kk kk+block_size-1]), '');

assertExceptionThrown(@() msndfile.blockread('read', 'test_files/test.raw', [kk kk+block_size-1]), '');

%
%% Test 3: passing only the block size instead of the full range
%

function test_only_blocksize(ref_data)

block_size = ref_data.block_size;
in_wav     = ref_data.in_wav;

in_blockwise1 = zeros(ref_data.file_size);

msndfile.blockread('open', 'test_files/test.wav');

for kk = 1:block_size:ref_data.file_size(1)-block_size
    in_blockwise1(kk:kk+block_size-1, :) = msndfile.blockread('read', 'test_files/test.wav', block_size);
end
in_blockwise1(kk+block_size:end, :) = msndfile.blockread('read', 'test_files/test.wav', ref_data.file_size(1)-(kk+block_size)+1);

err_sum = sum(abs(in_blockwise1 - in_wav));
assertTrue(all(err_sum==0));

msndfile.blockread('close', 'test_files/test.wav');
