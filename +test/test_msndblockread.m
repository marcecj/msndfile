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

assertExceptionThrown(@() msndfile.blockread('open', 'test_files/test.wav'), 'msndfile:blockread:fileopen');
assertExceptionThrown(@() msndfile.blockread('open', 'test_files/test.flac'), 'msndfile:blockread:fileopen');
assertExceptionThrown(@() msndfile.blockread('open', 'test_files/test.raw', ref_data.file_info), 'msndfile:blockread:fileopen');

function test_seek(ref_data)

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

function test_close(ref_data)
% test close command

msndfile.blockread('open', 'test_files/test.wav');
msndfile.blockread('open', 'test_files/test.flac');
msndfile.blockread('open', 'test_files/test.raw', ref_data.file_info);

msndfile.blockread('close', 'test_files/test.wav');
msndfile.blockread('close', 'test_files/test.flac');
msndfile.blockread('close', 'test_files/test.raw', ref_data.file_info);

% should throw an exception
assertExceptionThrown(@() msndfile.blockread('close', 'test_files/test.wav'), 'msndfile:blockread:filenotopen');
assertExceptionThrown(@() msndfile.blockread('close', 'test_files/test.flac'), 'msndfile:blockread:filenotopen');
assertExceptionThrown(@() msndfile.blockread('close', 'test_files/test.raw', ref_data.file_info), 'msndfile:blockread:filenotopen');

function test_closeall(ref_data)
% test read command

msndfile.blockread('open', 'test_files/test.wav');
msndfile.blockread('open', 'test_files/test.flac');
msndfile.blockread('open', 'test_files/test.raw', ref_data.file_info);

msndfile.blockread('closeall');

assertExceptionThrown(@() msndfile.blockread('read', 'test_files/test.wav', ref_data.block_size), 'msndfile:blockread:filenotopen');
assertExceptionThrown(@() msndfile.blockread('read', 'test_files/test.flac', ref_data.block_size), 'msndfile:blockread:filenotopen');
assertExceptionThrown(@() msndfile.blockread('read', 'test_files/test.raw', ref_data.block_size, ref_data.file_info), 'msndfile:blockread:filenotopen');

% should not throw an exception
assertExceptionThrown(@() msndfile.blockread('close', 'test_files/test.wav'), 'msndfile:blockread:filenotopen');
assertExceptionThrown(@() msndfile.blockread('close', 'test_files/test.flac'), 'msndfile:blockread:filenotopen');
assertExceptionThrown(@() msndfile.blockread('close', 'test_files/test.raw', ref_data.file_info), 'msndfile:blockread:filenotopen');

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
assertExceptionThrown(@() msndfile.blockread('read', 'test_files/test.wav', block_size), 'msndfile:blockread:filenotopen');

% close the FLAC file and try to read from it
msndfile.blockread('close', 'test_files/test.flac');
assertExceptionThrown(@() msndfile.blockread('read', 'test_files/test.flac', block_size), 'msndfile:blockread:filenotopen');

% test if invalid ranges throw an error
msndfile.blockread('open', 'test_files/test.wav');
msndfile.blockread('read', 'test_files/test.wav', [1 ref_data.file_size(1)]);
assertExceptionThrown(@() msndfile.blockread('read', 'test_files/test.wav', [0 ref_data.file_size(1)]), 'msndfile:argerror');
assertExceptionThrown(@() msndfile.blockread('read', 'test_files/test.wav', [-1 ref_data.file_size(1)]), 'msndfile:argerror');
assertExceptionThrown(@() msndfile.blockread('read', 'test_files/test.wav', [1 ref_data.file_size(1)+1]), 'msndfile:argerror');
assertExceptionThrown(@() msndfile.blockread('read', 'test_files/test.wav', [1 ref_data.file_size(1)+1]), 'msndfile:argerror');

% close and reopen to guarantee that the file is at position 1
msndfile.blockread('close', 'test_files/test.wav');
msndfile.blockread('open', 'test_files/test.wav');
msndfile.blockread('read', 'test_files/test.wav', ref_data.file_size(1));

if verLessThan('matlab', '7.1')
    assertExceptionThrown(@() msndfile.blockread('read', 'test_files/test.wav', -1), 'MATLAB:p32bitsize');
else
    assertExceptionThrown(@() msndfile.blockread('read', 'test_files/test.wav', -1), 'MATLAB:nomem');
end
warning('off', 'msndfile:sndfile');
assertTrue(isempty(msndfile.blockread('read', 'test_files/test.wav', 0)));
warning('on', 'msndfile:sndfile');
msndfile.blockread('close', 'test_files/test.wav');
msndfile.blockread('open', 'test_files/test.wav');
assertExceptionThrown(@() msndfile.blockread('read', 'test_files/test.wav', ref_data.file_size(1)+1), 'msndfile:argerror');

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

assertExceptionThrown(@() msndfile.blockread('read', 'test_files/test.raw', [kk kk+block_size-1]), 'msndfile:blockread:filenotopen');

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

assertExceptionThrown(@() msndfile.blockread('read', 'test_files/test.wav', [kk kk+block_size-1]), 'msndfile:blockread:filenotopen');
assertExceptionThrown(@() msndfile.blockread('read', 'test_files/test.flac', [kk kk+block_size-1]), 'msndfile:blockread:filenotopen');
assertExceptionThrown(@() msndfile.blockread('read', 'test_files/test.raw', [kk kk+block_size-1]), 'msndfile:blockread:filenotopen');

% test multibyte file name support; just do some regular stuff
function test_multibyte(ref_data)

% UTF-8 encoded file name 'test_files/bläßgans'
utf8_bytes = [98 108 195 164 195 159]; % bläß
fname = ['test_files/' native2unicode(utf8_bytes, 'UTF-8') 'gans.wav'];

msndfile.blockread('open', fname);
assertExceptionThrown(@() msndfile.blockread('open', fname), 'msndfile:blockread:fileopen');
msndfile.blockread('read', fname, [1 ref_data.file_size(1)]);
msndfile.blockread('close', fname);
assertExceptionThrown(@() msndfile.blockread('close', fname), 'msndfile:blockread:filenotopen');
