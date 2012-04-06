fprintf('\n*** Testing msndblockread ***\n');

[in_wav, fs] = wavread('test.wav');

file_info = [];
file_info.samplerate   = 44100;
file_info.channels     = 2;
file_info.format       = 'RAW';
file_info.sampleformat = 'PCM_16';

file_size    = wavread('test.wav', 'size');
block_size   = 1024;
num_samples  = 16384;

%
%% Test 1: reading from various files
%

in_blockwise1 = zeros(num_samples, file_size(2));
in_blockwise2 = zeros(num_samples, file_size(2));

fprintf('\n* Opening and reading from test.wav and test.flac...\n');

msndfile.blockread('open', 'test.wav');
msndfile.blockread('open', 'test.flac');
for kk = 1:block_size:num_samples
    in_blockwise1(kk:kk+block_size-1, :)    = msndfile.blockread('read', 'test.wav', [kk kk+block_size-1]);
    in_blockwise2(kk:kk+block_size-1, :)    = msndfile.blockread('read', 'test.flac', [kk kk+block_size-1]);
end
fprintf('\n* Closing test.wav...\n\n');
msndfile.blockread('close', 'test.wav');

num_unequal  = sum(in_blockwise1  - in_wav(1:num_samples,:));
disp('Comparing WAV (msndblockread) and WAV (wavread)');
disp(['There are ' num2str(num_unequal) ' incorrect samples']);

fprintf('\n* Opening test.wav again...\n');
msndfile.blockread('open', 'test.wav');
fprintf('\n* Reading test.wav without transposing...\n');
for kk = 1:block_size:num_samples
    in_blockwise1(kk:kk+block_size-1, :) = msndfile.blockread('read', 'test.wav', [kk kk+block_size-1], false).';
end

fprintf('\n* Closing test.wav again...\n');
msndfile.blockread('close', 'test.wav');

num_unequal  = sum(in_blockwise1  - in_wav(1:num_samples,:));
fprintf('\n');
disp('Comparing WAV (msndblockread, no transpose) and WAV (wavread)');
disp(['There are ' num2str(num_unequal) ' incorrect samples']);

num_unequal = sum(in_blockwise2 - in_wav(1:num_samples,:));
fprintf('\n');
disp('Comparing FLAC (msndblockread) and WAV (wavread)');
disp(['There are ' num2str(num_unequal) ' incorrect samples']);

for kk = 1:block_size:num_samples
    in_blockwise2(kk:kk+block_size-1, :)    = msndfile.blockread('read', 'test.flac', [kk kk+block_size-1]);
end
msndfile.blockread('close', 'test.flac');

in_blockwise = zeros(file_size);
msndfile.blockread('open', 'test.wav');
for kk = 1:block_size:file_size(1)-block_size
    in_blockwise(kk:kk+block_size-1, :) = msndfile.blockread('read', 'test.wav', [kk kk+block_size-1]);
end
in_blockwise(kk:end, :) = msndfile.blockread('read', 'test.wav', [kk file_size(1)]);
msndfile.blockread('close', 'test.wav');

num_unequal = sum(in_blockwise - in_wav);
fprintf('\n');
disp('Comparing FLAC (msndblockread) and WAV (wavread)');
disp(['There are ' num2str(num_unequal) ' incorrect samples']);

%
%% Test 2: opening and closing files
%

fprintf('\n* Opening three files...\n');

in_blockwise1 = zeros(file_size);
in_blockwise2 = zeros(file_size);
in_blockwise3 = zeros(file_size);

fprintf('\n* Reading from these three files...\n');

msndfile.blockread('open', 'test.wav');
msndfile.blockread('open', 'test.flac');
msndfile.blockread('open', 'test.raw', file_info);
for kk = 1:block_size:file_size(1)-block_size
    in_blockwise1(kk:kk+block_size-1, :) = msndfile.blockread('read', 'test.wav', [kk kk+block_size-1]);
    in_blockwise2(kk:kk+block_size-1, :) = msndfile.blockread('read', 'test.flac', [kk kk+block_size-1]);
    in_blockwise3(kk:kk+block_size-1, :) = msndfile.blockread('read', 'test.raw', [kk kk+block_size-1]);
end
in_blockwise1(kk:end, :) = msndfile.blockread('read', 'test.wav', [kk file_size(1)]);
in_blockwise2(kk:end, :) = msndfile.blockread('read', 'test.flac', [kk file_size(1)]);
in_blockwise3(kk:end, :) = msndfile.blockread('read', 'test.raw', [kk file_size(1)]);

num_unequal = sum(in_blockwise1 - in_wav);
fprintf('\n');
disp('Comparing WAV (msndblockread) and WAV (wavread)');
disp(['There are ' num2str(num_unequal) ' incorrect samples']);

num_unequal = sum(in_blockwise2 - in_wav);
fprintf('\n');
disp('Comparing FLAC (msndblockread) and WAV (wavread)');
disp(['There are ' num2str(num_unequal) ' incorrect samples']);

num_unequal = sum(in_blockwise3 - in_wav);
fprintf('\n');
disp('Comparing RAW (msndblockread) and WAV (wavread)');
disp(['There are ' num2str(num_unequal) ' incorrect samples']);

fprintf('\n* Closing test.raw...\n');

msndfile.blockread('close', 'test.raw');

try
    fprintf('\n* Attemting to read test.raw...\n\n');
    msndfile.blockread('read', 'test.raw', [kk kk+block_size-1]);
    warning('test:err', 'File not closed properly!');
catch ME
    disp('Error correctly raised...');
end

fprintf('\n* Reading from test.wav and test.flac...\n');

in_blockwise1 = zeros(file_size);
in_blockwise2 = zeros(file_size);
for kk = 1:block_size:file_size(1)-block_size
    in_blockwise1(kk:kk+block_size-1, :) = msndfile.blockread('read', 'test.wav', [kk kk+block_size-1]);
    in_blockwise2(kk:kk+block_size-1, :) = msndfile.blockread('read', 'test.flac', [kk kk+block_size-1]);
end
in_blockwise1(kk:end, :) = msndfile.blockread('read', 'test.wav', [kk file_size(1)]);
in_blockwise2(kk:end, :) = msndfile.blockread('read', 'test.flac', [kk file_size(1)]);

num_unequal = sum(in_blockwise1 - in_wav);
fprintf('\n');
disp('Comparing WAV (msndblockread) and WAV (wavread)');
disp(['There are ' num2str(num_unequal) ' incorrect samples']);

num_unequal = sum(in_blockwise2 - in_wav);
fprintf('\n');
disp('Comparing FLAC (msndblockread) and WAV (wavread)');
disp(['There are ' num2str(num_unequal) ' incorrect samples']);

fprintf('\n* Closing all files...\n');

msndfile.blockread('closeall');

try
    fprintf('\n* Attemting to read test.wav...\n\n');
    msndfile.blockread('read', 'test.wav', [kk kk+block_size-1]);
    warning('test:err', 'File not closed properly!');
catch ME
    disp('Error correctly raised...');
end

try
    fprintf('\n* Attemting to read test.flac...\n\n');
    msndfile.blockread('read', 'test.flac', [kk kk+block_size-1]);
    warning('test:err', 'File not closed properly!');
catch ME
    disp('Error correctly raised...');
end

try
    fprintf('\n* Attemting to read test.raw...\n\n');
    msndfile.blockread('read', 'test.raw', [kk kk+block_size-1]);
    warning('test:err', 'File not closed properly!');
catch ME
    disp('Error correctly raised...');
end

%
%% Test 3: passing only the block size instead of the full range
%

in_blockwise1 = zeros(file_size);

fprintf('\n* Opening test.wav...\n');

msndfile.blockread('open', 'test.wav');

fprintf('\n* Reading from test.wav (using only the block size instead of a block range)...\n');
for kk = 1:block_size:file_size(1)-block_size
    in_blockwise1(kk:kk+block_size-1, :) = msndfile.blockread('read', 'test.wav', block_size);
end
in_blockwise1(kk+block_size:end, :) = msndfile.blockread('read', 'test.wav', file_size(1)-(kk+block_size)+1);

num_unequal = sum(in_blockwise1 - in_wav);
fprintf('\n');
disp('Comparing WAV (msndblockread) and WAV (wavread)');
disp(['There are ' num2str(num_unequal) ' incorrect samples']);

msndfile.blockread('close', 'test.wav');
