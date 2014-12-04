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

function test_missing_command(ref_data)
% test that a missing command will lead to an exception

assertExceptionThrown(@() msndfile.blockread(), 'msndfile:argerror');

function test_empty_command(ref_data)
% test that an empty command will lead to an exception

assertExceptionThrown(@() msndfile.blockread(''), 'msndfile:argerror');

function test_wav_open(ref_data)
% test open command

msndfile.blockread('open', 'test_files/test.wav');
msndfile.blockread('open', 'test_files/test.raw', ref_data.file_info);

assertExceptionThrown(@() msndfile.blockread('open', 'test_files/test.wav'), 'msndfile:blockread:fileopen');
assertExceptionThrown(@() msndfile.blockread('open', 'test_files/test.raw', ref_data.file_info), 'msndfile:blockread:fileopen');

function test_wav_close(ref_data)
% test close command

msndfile.blockread('open', 'test_files/test.wav');
msndfile.blockread('open', 'test_files/test.raw', ref_data.file_info);

msndfile.blockread('close', 'test_files/test.wav');
msndfile.blockread('close', 'test_files/test.raw', ref_data.file_info);

% due to the nature of the implementation, also test closing in reverse order
msndfile.blockread('open', 'test_files/test.wav');
msndfile.blockread('open', 'test_files/test.flac');
msndfile.blockread('open', 'test_files/test.raw', ref_data.file_info);

msndfile.blockread('close', 'test_files/test.raw', ref_data.file_info);
msndfile.blockread('close', 'test_files/test.flac');
msndfile.blockread('close', 'test_files/test.wav');

% should throw an exception
assertExceptionThrown(@() msndfile.blockread('close', 'test_files/test.wav'), 'msndfile:blockread:filenotopen');
assertExceptionThrown(@() msndfile.blockread('close', 'test_files/test.raw', ref_data.file_info), 'msndfile:blockread:filenotopen');

function test_wav_read(ref_data)
% test read command

block_size = ref_data.block_size;
in_wav     = ref_data.in_wav;

num_samples  = 16384;
in_blockwise = zeros(num_samples, ref_data.file_size(2));

% open a test file and read from it
msndfile.blockread('open', 'test_files/test.wav');
for kk = 1:block_size:num_samples
    in_blockwise(kk:kk+block_size-1, :)    = msndfile.blockread('read', 'test_files/test.wav', [kk kk+block_size-1]);
end

% compare the outputs
assertEqual(in_blockwise, in_wav(1:num_samples,:));

function test_wav_read_ranges(ref_data)
% test if invalid ranges throw an error

msndfile.blockread('open', 'test_files/test.wav');
msndfile.blockread('read', 'test_files/test.wav', [1 ref_data.file_size(1)]);
assertExceptionThrown(@() msndfile.blockread('read', 'test_files/test.wav', [0 ref_data.file_size(1)]), 'msndfile:argerror');
assertExceptionThrown(@() msndfile.blockread('read', 'test_files/test.wav', [-1 ref_data.file_size(1)]), 'msndfile:argerror');
assertExceptionThrown(@() msndfile.blockread('read', 'test_files/test.wav', [1 ref_data.file_size(1)+1]), 'msndfile:argerror');
assertExceptionThrown(@() msndfile.blockread('read', 'test_files/test.wav', [1 ref_data.file_size(1)+1]), 'msndfile:argerror');

assertExceptionThrown(@() msndfile.blockread('read', 'test_files/test.wav', ref_data.file_size(1)+1), 'msndfile:argerror');

function test_wav_read_backwards(ref_data)
% test if reading backwards fails

msndfile.blockread('open', 'test_files/test.wav');

% reading backwards from the start should fail
if verLessThan('matlab', '7.1')
    assertExceptionThrown(@() msndfile.blockread('read', 'test_files/test.wav', -1), 'MATLAB:p32bitsize');
else
    assertExceptionThrown(@() msndfile.blockread('read', 'test_files/test.wav', -1), 'MATLAB:nomem');
end

% reading backwards from the end should also fail
msndfile.blockread('seek', 'test_files/test.wav', ref_data.file_size(1));
if verLessThan('matlab', '7.1')
    assertExceptionThrown(@() msndfile.blockread('read', 'test_files/test.wav', -1), 'MATLAB:p32bitsize');
else
    assertExceptionThrown(@() msndfile.blockread('read', 'test_files/test.wav', -1), 'MATLAB:nomem');
end

function test_wav_read_zero_samples(ref_data)
% test reading zero samples

msndfile.blockread('open', 'test_files/test.wav');

% reading zero samples should yield an empty matrix
warning('off', 'msndfile:sndfile');
assertTrue(isempty(msndfile.blockread('read', 'test_files/test.wav', 0)));
warning('on', 'msndfile:sndfile');

function test_wav_read_only_blocksize(ref_data)
% test passing only the block size instead of the full range

block_size = ref_data.block_size;
in_wav     = ref_data.in_wav;

in_blockwise = zeros(ref_data.file_size);

msndfile.blockread('open', 'test_files/test.wav');

for kk = 1:block_size:ref_data.file_size(1)-block_size
    in_blockwise(kk:kk+block_size-1, :) = msndfile.blockread('read', 'test_files/test.wav', block_size);
end
in_blockwise(kk+block_size:end, :) = msndfile.blockread('read', 'test_files/test.wav', ref_data.file_size(1)-(kk+block_size)+1);

assertEqual(in_blockwise, in_wav);

function test_wav_read_transpose(ref_data)
% test the "transpose" option

block_size = ref_data.block_size;
in_wav     = ref_data.in_wav;

in_blockwise = zeros(fliplr(ref_data.file_size));

msndfile.blockread('open', 'test_files/test.wav');

% read in the whole file, but without transposing the output
for kk = 1:block_size:ref_data.file_size(1)-block_size
    in_blockwise(:, kk:kk+block_size-1) = msndfile.blockread('read', 'test_files/test.wav', block_size, false);
end
in_blockwise(:, kk+block_size:end) = msndfile.blockread('read', 'test_files/test.wav', ref_data.file_size(1)-(kk+block_size)+1, false);

% now check for correct matrix dimensions
assertEqual(in_blockwise.', in_wav);

function test_wav_reopen(ref_data)
% test opening and closing files with repeated reading

block_size = ref_data.block_size;
in_wav     = ref_data.in_wav;

in_blockwise1 = zeros(ref_data.file_size);
in_blockwise2 = zeros(ref_data.file_size);

% open two files and read their entire contents block-wise
msndfile.blockread('open', 'test_files/test.wav');
msndfile.blockread('open', 'test_files/test.raw', ref_data.file_info);
for kk = 1:block_size:ref_data.file_size(1)-block_size
    in_blockwise1(kk:kk+block_size-1, :) = msndfile.blockread('read', 'test_files/test.wav', [kk kk+block_size-1]);
    in_blockwise2(kk:kk+block_size-1, :) = msndfile.blockread('read', 'test_files/test.raw', [kk kk+block_size-1]);
end
in_blockwise1(kk:end, :) = msndfile.blockread('read', 'test_files/test.wav', [kk ref_data.file_size(1)]);
in_blockwise2(kk:end, :) = msndfile.blockread('read', 'test_files/test.raw', [kk ref_data.file_size(1)]);

assertEqual(in_blockwise1, in_wav);
assertEqual(in_blockwise2, in_wav);

% close *one* of the files
msndfile.blockread('close', 'test_files/test.raw');
assertExceptionThrown(@() msndfile.blockread('read', 'test_files/test.raw', [kk kk+block_size-1]), 'msndfile:blockread:filenotopen');

% now read from the remaining file
in_blockwise1(:) = 0;
for kk = 1:block_size:ref_data.file_size(1)-block_size
    in_blockwise1(kk:kk+block_size-1, :) = msndfile.blockread('read', 'test_files/test.wav', [kk kk+block_size-1]);
end
in_blockwise1(kk:end, :) = msndfile.blockread('read', 'test_files/test.wav', [kk ref_data.file_size(1)]);

assertEqual(in_blockwise1, in_wav);

msndfile.blockread('closeall');

assertExceptionThrown(@() msndfile.blockread('read', 'test_files/test.wav', [kk kk+block_size-1]), 'msndfile:blockread:filenotopen');
assertExceptionThrown(@() msndfile.blockread('read', 'test_files/test.raw', [kk kk+block_size-1]), 'msndfile:blockread:filenotopen');

function test_wav_missing_filename(ref_data)
% test that a missing file name will lead to an exception

assertExceptionThrown(@() msndfile.blockread('open'), 'msndfile:argerror');

function test_wav_empty_filename(ref_data)
% test that an empty file name will lead to an exception

assertExceptionThrown(@() msndfile.blockread('open', ''), 'msndfile:sndfile');

function test_wav_multibyte_filename(ref_data)
% test multibyte file name support; just do some regular stuff

% UTF-8 encoded file name 'test_files/bläßgans'
utf8_bytes = [98 108 195 164 195 159]; % bläß
fname = ['test_files/' native2unicode(utf8_bytes, 'UTF-8') 'gans.wav'];

msndfile.blockread('open', fname);
assertExceptionThrown(@() msndfile.blockread('open', fname), 'msndfile:blockread:fileopen');
msndfile.blockread('read', fname, [1 ref_data.file_size(1)]);
msndfile.blockread('close', fname);
assertExceptionThrown(@() msndfile.blockread('close', fname), 'msndfile:blockread:filenotopen');

function test_seek(ref_data)
% test seek command

file_len = ref_data.file_size(1);

msndfile.blockread('open', 'test_files/test.wav');

% Invalid sample positions should raise errors
assertExceptionThrown(@() msndfile.blockread('seek', 'test_files/test.wav', -1), 'msndfile:argerror');
assertExceptionThrown(@() msndfile.blockread('seek', 'test_files/test.wav', 0), 'msndfile:argerror');
assertExceptionThrown(@() msndfile.blockread('seek', 'test_files/test.wav', file_len+1), 'msndfile:argerror');
assertExceptionThrown(@() msndfile.blockread('seek', 'test_files/test.wav', file_len+2), 'msndfile:argerror');

% These indices should be fine
msndfile.blockread('seek', 'test_files/test.wav', 1);
msndfile.blockread('seek', 'test_files/test.wav', file_len);

% Read 100 random samples and compare with wavread() to verify that "seek"
% actually seeks to the correct position.
for k=1:100
    pos = randi([0 file_len], 1);

    msndfile.blockread('seek', 'test_files/test.wav', pos);

    data1 = msndfile.blockread('read', 'test_files/test.wav', 1);
    data2 = wavread('test_files/test.wav', [pos pos]);

    assertEqual(data1, data2);
end

function test_tell(ref_data)
% test tell command

file_len = ref_data.file_size(1);

msndfile.blockread('open', 'test_files/test.wav');

% verify that the current position is unaltered after seek errors
assertEqual(1, msndfile.blockread('tell', 'test_files/test.wav'));
assertExceptionThrown(@() msndfile.blockread('seek', 'test_files/test.wav', 0), 'msndfile:argerror');
assertEqual(1, msndfile.blockread('tell', 'test_files/test.wav'));
assertExceptionThrown(@() msndfile.blockread('seek', 'test_files/test.wav', file_len+1), 'msndfile:argerror');
assertEqual(1, msndfile.blockread('tell', 'test_files/test.wav'));

% simple "tell" checks
msndfile.blockread('seek', 'test_files/test.wav', file_len);
assertEqual(file_len, msndfile.blockread('tell', 'test_files/test.wav'));
msndfile.blockread('seek', 'test_files/test.wav', 1);
assertEqual(1, msndfile.blockread('tell', 'test_files/test.wav'));

% seek to 100 random positions and verify that "tell" correctly reports the
% position we seeked to
for k=1:100
    pos = randi([0 file_len], 1);

    msndfile.blockread('seek', 'test_files/test.wav', pos);

    assertEqual(pos, msndfile.blockread('tell', 'test_files/test.wav'));
end

function test_seek_raw(ref_data)
% test seek command with RAW files

file_len = ref_data.file_size(1);

msndfile.blockread('open', 'test_files/test.raw', ref_data.file_info);

% Read 100 random samples and compare with wavread() to verify that "seek"
% actually seeks to the correct position.
for k=1:100
    pos = randi([0 file_len], 1);

    msndfile.blockread('seek', 'test_files/test.raw', pos);

    data1 = msndfile.blockread('read', 'test_files/test.raw', 1);
    data2 = wavread('test_files/test.wav', [pos pos]);

    assertEqual(data1, data2);
end

function test_tell_raw(ref_data)
% test tell command with RAW files

file_len = ref_data.file_size(1);

msndfile.blockread('open', 'test_files/test.raw', ref_data.file_info);

% seek to 100 random positions and verify that "tell" correctly reports the
% position we seeked to
for k=1:100
    pos = randi([0 file_len], 1);

    msndfile.blockread('seek', 'test_files/test.raw', pos);

    assertEqual(pos, msndfile.blockread('tell', 'test_files/test.raw'));
end

function test_closeall(ref_data)
% test closeall command; doesn't need to have the FLAC file open

msndfile.blockread('open', 'test_files/test.wav');
msndfile.blockread('open', 'test_files/test.raw', ref_data.file_info);

msndfile.blockread('closeall');

assertExceptionThrown(@() msndfile.blockread('read', 'test_files/test.wav', ref_data.block_size), 'msndfile:blockread:filenotopen');
assertExceptionThrown(@() msndfile.blockread('read', 'test_files/test.raw', ref_data.block_size, ref_data.file_info), 'msndfile:blockread:filenotopen');

assertExceptionThrown(@() msndfile.blockread('seek', 'test_files/test.wav', 1), 'msndfile:blockread:filenotopen');
assertExceptionThrown(@() msndfile.blockread('seek', 'test_files/test.raw', 1, ref_data.file_info), 'msndfile:blockread:filenotopen');

assertExceptionThrown(@() msndfile.blockread('tell', 'test_files/test.wav'), 'msndfile:blockread:filenotopen');
assertExceptionThrown(@() msndfile.blockread('tell', 'test_files/test.raw'), 'msndfile:blockread:filenotopen');

assertExceptionThrown(@() msndfile.blockread('close', 'test_files/test.wav'), 'msndfile:blockread:filenotopen');
assertExceptionThrown(@() msndfile.blockread('close', 'test_files/test.raw', ref_data.file_info), 'msndfile:blockread:filenotopen');

function test_flac_open(ref_data)
% test open command - FLAC only

msndfile.blockread('open', 'test_files/test.flac');
assertExceptionThrown(@() msndfile.blockread('open', 'test_files/test.flac'), 'msndfile:blockread:fileopen');

function test_flac_close(ref_data)
% test close command - FLAC only

msndfile.blockread('open', 'test_files/test.flac');
msndfile.blockread('close', 'test_files/test.flac');
assertExceptionThrown(@() msndfile.blockread('close', 'test_files/test.flac'), 'msndfile:blockread:filenotopen');

function test_flac_read(ref_data)
% test read command - FLAC only

block_size = ref_data.block_size;
in_wav     = ref_data.in_wav;

num_samples   = 16384;
in_blockwise = zeros(num_samples, ref_data.file_size(2));

% open two test files and read from them
msndfile.blockread('open', 'test_files/test.flac');
for kk = 1:block_size:num_samples
    in_blockwise(kk:kk+block_size-1, :)    = msndfile.blockread('read', 'test_files/test.flac', [kk kk+block_size-1]);
end

% compare the outputs
assertEqual(in_blockwise, in_wav(1:num_samples,:));

function test_flac_read_ranges(ref_data)
% test if invalid ranges throw an error

msndfile.blockread('open', 'test_files/test.flac');
msndfile.blockread('read', 'test_files/test.flac', [1 ref_data.file_size(1)]);
assertExceptionThrown(@() msndfile.blockread('read', 'test_files/test.flac', [0 ref_data.file_size(1)]), 'msndfile:argerror');
assertExceptionThrown(@() msndfile.blockread('read', 'test_files/test.flac', [-1 ref_data.file_size(1)]), 'msndfile:argerror');
assertExceptionThrown(@() msndfile.blockread('read', 'test_files/test.flac', [1 ref_data.file_size(1)+1]), 'msndfile:argerror');
assertExceptionThrown(@() msndfile.blockread('read', 'test_files/test.flac', [1 ref_data.file_size(1)+1]), 'msndfile:argerror');

assertExceptionThrown(@() msndfile.blockread('read', 'test_files/test.flac', ref_data.file_size(1)+1), 'msndfile:argerror');

function test_flac_read_backwards(ref_data)
% test if reading backwards fails

msndfile.blockread('open', 'test_files/test.flac');

% reading backwards from the start should fail
if verLessThan('matlab', '7.1')
    assertExceptionThrown(@() msndfile.blockread('read', 'test_files/test.flac', -1), 'MATLAB:p32bitsize');
else
    assertExceptionThrown(@() msndfile.blockread('read', 'test_files/test.flac', -1), 'MATLAB:nomem');
end

% reading backwards from the end should also fail
msndfile.blockread('seek', 'test_files/test.flac', ref_data.file_size(1));
if verLessThan('matlab', '7.1')
    assertExceptionThrown(@() msndfile.blockread('read', 'test_files/test.flac', -1), 'MATLAB:p32bitsize');
else
    assertExceptionThrown(@() msndfile.blockread('read', 'test_files/test.flac', -1), 'MATLAB:nomem');
end

function test_flac_read_zero_samples(ref_data)
% test reading zero samples

msndfile.blockread('open', 'test_files/test.flac');

% reading zero samples should yield an empty matrix
warning('off', 'msndfile:sndfile');
assertTrue(isempty(msndfile.blockread('read', 'test_files/test.flac', 0)));
warning('on', 'msndfile:sndfile');

function test_flac_read_only_blocksize(ref_data)
% test passing only the block size instead of the full range - FLAC only

block_size = ref_data.block_size;
in_wav     = ref_data.in_wav;

in_blockwise = zeros(ref_data.file_size);

msndfile.blockread('open', 'test_files/test.flac');

for kk = 1:block_size:ref_data.file_size(1)-block_size
    in_blockwise(kk:kk+block_size-1, :) = msndfile.blockread('read', 'test_files/test.flac', block_size);
end
in_blockwise(kk+block_size:end, :) = msndfile.blockread('read', 'test_files/test.flac', ref_data.file_size(1)-(kk+block_size)+1);

assertEqual(in_blockwise, in_wav);

function test_flac_read_transpose(ref_data)
% test the "transpose" option

block_size = ref_data.block_size;
in_wav     = ref_data.in_wav;

in_blockwise = zeros(fliplr(ref_data.file_size));

msndfile.blockread('open', 'test_files/test.flac');

% read in the whole file, but without transposing the output
for kk = 1:block_size:ref_data.file_size(1)-block_size
    in_blockwise(:, kk:kk+block_size-1) = msndfile.blockread('read', 'test_files/test.flac', block_size, false);
end
in_blockwise(:, kk+block_size:end) = msndfile.blockread('read', 'test_files/test.flac', ref_data.file_size(1)-(kk+block_size)+1, false);

% now check for correct matrix dimensions
assertEqual(in_blockwise.', in_wav);

function test_flac_reopen(ref_data)
% test opening and closing files with repeated reading - FLAC only

block_size = ref_data.block_size;
in_wav     = ref_data.in_wav;

in_blockwise = zeros(ref_data.file_size);

msndfile.blockread('open', 'test_files/test.flac');
for kk = 1:block_size:ref_data.file_size(1)-block_size
    in_blockwise(kk:kk+block_size-1, :) = msndfile.blockread('read', 'test_files/test.flac', [kk kk+block_size-1]);
end
in_blockwise(kk:end, :) = msndfile.blockread('read', 'test_files/test.flac', [kk ref_data.file_size(1)]);

assertEqual(in_blockwise, in_wav);

in_blockwise(:) = 0;
for kk = 1:block_size:ref_data.file_size(1)-block_size
    in_blockwise(kk:kk+block_size-1, :) = msndfile.blockread('read', 'test_files/test.flac', [kk kk+block_size-1]);
end
in_blockwise(kk:end, :) = msndfile.blockread('read', 'test_files/test.flac', [kk ref_data.file_size(1)]);

assertEqual(in_blockwise, in_wav);

msndfile.blockread('closeall');

assertExceptionThrown(@() msndfile.blockread('read', 'test_files/test.flac', [kk kk+block_size-1]), 'msndfile:blockread:filenotopen');

function test_flac_multibyte_filename(ref_data)
% test multibyte file name support; just do some regular stuff - FLAC only

% UTF-8 encoded file name 'test_files/bläßgans'
utf8_bytes = [98 108 195 164 195 159]; % bläß
fname = ['test_files/' native2unicode(utf8_bytes, 'UTF-8') 'gans.flac'];

msndfile.blockread('open', fname);
assertExceptionThrown(@() msndfile.blockread('open', fname), 'msndfile:blockread:fileopen');
msndfile.blockread('read', fname, [1 ref_data.file_size(1)]);
msndfile.blockread('close', fname);
assertExceptionThrown(@() msndfile.blockread('close', fname), 'msndfile:blockread:filenotopen');
