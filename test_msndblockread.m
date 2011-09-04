fprintf('\n*** Testing msndblockread ***\n\n');

if ~exist('in_wav', 'var')
	[in_wav, fs] = wavread('test.wav');
end

file_size    = wavread('test.wav', 'size');
block_size   = 1024;

num_samples  = 16384;
in_blockwise1 = zeros(num_samples, file_size(2));
in_blockwise2 = zeros(num_samples, file_size(2));
msndblockread('open', 'test.wav');
msndblockread('open', 'test.flac');
for kk = 1:block_size:num_samples
    in_blockwise1(kk:kk+1023, :)    = msndblockread('read', 'test.wav', [kk kk+1023]);
    in_blockwise2(kk:kk+1023, :)    = msndblockread('read', 'test.flac', [kk kk+1023]);
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
    in_blockwise2(kk:kk+1023, :)    = msndblockread('read', 'test.flac', [kk kk+1023]);
end
msndblockread('close', 'test.flac');

in_blockwise = zeros(file_size);
msndblockread('open', 'test.wav');
for kk = 1:block_size:file_size(1)-block_size
    in_blockwise(kk:kk+1023, :) = msndblockread('read', 'test.wav', [kk kk+1023]);
end
in_blockwise(kk:end, :) = msndblockread('read', 'test.wav', [kk file_size(1)]);
msndblockread('close', 'test.wav');

num_unequal = sum(in_blockwise - in_wav);
fprintf('\n');
disp('Comparing FLAC (msndblockread) and WAV (wavread)');
disp(['There are ' num2str(num_unequal) ' incorrect samples']);
