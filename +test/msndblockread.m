fprintf('\n*** Testing msndblockread ***\n\n');

[in_wav, fs] = wavread('test.wav');

file_info = [];
file_info.samplerate   = 44100;
file_info.channels     = 2;
file_info.format       = 'RAW';
file_info.sampleformat = 'PCM_16';

file_size    = wavread('test.wav', 'size');
block_size   = 1024;

num_samples  = 16384;
in_blockwise1 = zeros(num_samples, file_size(2));
in_blockwise2 = zeros(num_samples, file_size(2));
msndblockread('open', 'test.wav');
msndblockread('open', 'test.flac');
for kk = 1:block_size:num_samples
    in_blockwise1(kk:kk+block_size-1, :)    = msndblockread('read', 'test.wav', [kk kk+block_size-1]);
    in_blockwise2(kk:kk+block_size-1, :)    = msndblockread('read', 'test.flac', [kk kk+block_size-1]);
end
msndblockread('close', 'test.wav');

num_unequal  = sum(in_blockwise1  - in_wav(1:num_samples,:));
disp('Comparing WAV (msndblockread) and WAV (wavread)');
disp(['There are ' num2str(num_unequal) ' incorrect samples']);

num_unequal = sum(in_blockwise2 - in_wav(1:num_samples,:));
fprintf('\n');
disp('Comparing FLAC (msndblockread) and WAV (wavread)');
disp(['There are ' num2str(num_unequal) ' incorrect samples']);

for kk = 1:block_size:num_samples
    in_blockwise2(kk:kk+block_size-1, :)    = msndblockread('read', 'test.flac', [kk kk+block_size-1]);
end
msndblockread('close', 'test.flac');

in_blockwise = zeros(file_size);
msndblockread('open', 'test.wav');
for kk = 1:block_size:file_size(1)-block_size
    in_blockwise(kk:kk+block_size-1, :) = msndblockread('read', 'test.wav', [kk kk+block_size-1]);
end
in_blockwise(kk:end, :) = msndblockread('read', 'test.wav', [kk file_size(1)]);
msndblockread('close', 'test.wav');

num_unequal = sum(in_blockwise - in_wav);
fprintf('\n');
disp('Comparing FLAC (msndblockread) and WAV (wavread)');
disp(['There are ' num2str(num_unequal) ' incorrect samples']);

% Test opening/closing functionality

msndblockread('open', 'test.wav');
msndblockread('open', 'test.flac');
msndblockread('open', 'test.raw', file_info);

in_blockwise1 = zeros(file_size);
in_blockwise2 = zeros(file_size);
in_blockwise3 = zeros(file_size);
for kk = 1:block_size:file_size(1)-block_size
    in_blockwise1(kk:kk+block_size-1, :) = msndblockread('read', 'test.wav', [kk kk+block_size-1]);
    in_blockwise2(kk:kk+block_size-1, :) = msndblockread('read', 'test.flac', [kk kk+block_size-1]);
    in_blockwise3(kk:kk+block_size-1, :) = msndblockread('read', 'test.raw', [kk kk+block_size-1]);
end
in_blockwise1(kk:end, :) = msndblockread('read', 'test.wav', [kk file_size(1)]);
in_blockwise2(kk:end, :) = msndblockread('read', 'test.flac', [kk file_size(1)]);
in_blockwise3(kk:end, :) = msndblockread('read', 'test.raw', [kk file_size(1)]);

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

msndblockread('close', 'test.raw');

fprintf('\n');
try
    msndblockread('read', 'test.raw', [kk kk+block_size-1]);
    warning('File not closed properly!');
catch
    disp('Error correctly raised...');
end

in_blockwise1 = zeros(file_size);
in_blockwise2 = zeros(file_size);
for kk = 1:block_size:file_size(1)-block_size
    in_blockwise1(kk:kk+block_size-1, :) = msndblockread('read', 'test.wav', [kk kk+block_size-1]);
    in_blockwise2(kk:kk+block_size-1, :) = msndblockread('read', 'test.flac', [kk kk+block_size-1]);
end
in_blockwise1(kk:end, :) = msndblockread('read', 'test.wav', [kk file_size(1)]);
in_blockwise2(kk:end, :) = msndblockread('read', 'test.flac', [kk file_size(1)]);

num_unequal = sum(in_blockwise1 - in_wav);
fprintf('\n');
disp('Comparing WAV (msndblockread) and WAV (wavread)');
disp(['There are ' num2str(num_unequal) ' incorrect samples']);

num_unequal = sum(in_blockwise2 - in_wav);
fprintf('\n');
disp('Comparing FLAC (msndblockread) and WAV (wavread)');
disp(['There are ' num2str(num_unequal) ' incorrect samples']);

msndblockread('closeall');

fprintf('\n');
try
    msndblockread('read', 'test.wav', [kk kk+block_size-1]);
    warning('File not closed properly!');
catch
    disp('Error correctly raised...');
end
try
    msndblockread('read', 'test.flac', [kk kk+block_size-1]);
    warning('File not closed properly!');
catch
    disp('Error correctly raised...');
end
try
    msndblockread('read', 'test.raw', [kk kk+block_size-1]);
    warning('File not closed properly!');
catch
    disp('Error correctly raised...');
end

msndblockread('open', 'test.wav');

in_blockwise1 = zeros(file_size);
for kk = 1:block_size:file_size(1)-block_size
    in_blockwise1(kk:kk+block_size-1, :) = msndblockread('read', 'test.wav', block_size);
end
in_blockwise1(kk:end, :) = msndblockread('read', 'test.wav', [kk file_size(1)]);

num_unequal = sum(in_blockwise1 - in_wav);
fprintf('\n');
disp('Comparing WAV (msndblockread) and WAV (wavread)');
disp(['There are ' num2str(num_unequal) ' incorrect samples']);

msndblockread('close', 'test.wav');
