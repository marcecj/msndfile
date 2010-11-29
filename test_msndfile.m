% Simple script for testing msndfile

clear;
close all;

% the reference: the entire file imported by wavread
[in_wav, fs] = wavread('test.wav');

disp('Testing msndfile...')
disp('Test file used in all tests: test.wav (also as RAW and FLAC)')

%
%% Test 1: importing RAW files
%

% [input, fs] = msndfile('test.aif');

file_info.samplerate   = 44100;
file_info.channels     = 2;
file_info.format       = 'RAW';
file_info.sampleformat = 'PCM_16';
% file_info.endianness   = 'LITTLE'; % defaults to 'FILE'

% test the raw file import
[in_raw, fs] = msndfile('test.raw', [], file_info);

num_unequal = sum(abs(in_wav - in_raw) > 0);

fprintf('\n');
disp('Comparing RAW (msndfile) and WAV (wavread)');
disp(['There are ' num2str(num_unequal) ' incorrect samples']);

%
%% Test 2: block-wise reading
%

num_samples  = 16384;
in_blockwise     = zeros(num_samples, 2);
in_raw_blockwise = zeros(num_samples, 2);
for kk = 1:1024:16384
    in_blockwise(kk:kk+1023, :)     = msndfile('test.wav', [kk kk+1023]);
    in_raw_blockwise(kk:kk+1023, :) = msndfile('test.raw', [kk kk+1023], file_info);
end

num_unequal = sum(in_blockwise - in_wav(1:num_samples,:));

fprintf('\n');
disp('Comparing WAV (msndfile, blockwise) and WAV (wavread)');
disp(['There are ' num2str(num_unequal) ' incorrect samples']);

num_unequal = sum(in_raw_blockwise - in_wav(1:num_samples,:));

fprintf('\n');
disp('Comparing RAW (msndfile, blockwise) and WAV (wavread)');
disp(['There are ' num2str(num_unequal) ' incorrect samples']);

%
%% Test 3: test 'size' command
%

[file_size, fs] = wavread('test.wav', 'size');
disp(sprintf('wavread   (WAV):\tLength = %i,\tNChns = %i,\tFS = %i', file_size, fs));

[file_size, fs] = msndfile('test.wav', 'size');
disp(sprintf('msndfile  (WAV):\tLength = %i,\tNChns = %i,\tFS = %i', file_size, fs));

[file_size, fs] = msndfile('test.raw', 'size', file_info);
disp(sprintf('msndfile  (RAW):\tLength = %i,\tNChns = %i,\tFS = %i', file_size, fs));

[file_size, fs] = msndfile('test.flac', 'size');
disp(sprintf('msndfile (FLAC):\tLength = %i,\tNChns = %i,\tFS = %i', file_size, fs));

%
%% Test 4: performance comparisons
%

fprintf('\n');
disp('Conducting performance comparison (1000 reads, first 1024 samples)');

for kk=1:1000
    tic, msndfile('test.flac', 1024);
    t_e(kk) = toc;
end
disp(sprintf('mean time taken by msndfile (FLAC):\t%.6f +- %.6f', mean(t_e), std(t_e)));

for kk=1:1000
    tic, msndfile('test.wav', 1024);
    t_e(kk) = toc;
end
disp(sprintf('mean time taken by msndfile (WAV):\t%.6f +- %.6f', mean(t_e), std(t_e)));

for kk=1:1000
    tic, wavread('test.wav', 1024);
    t_e(kk) = toc;
end
disp(sprintf('mean time taken by wavread:\t%.6f +- %.6f', mean(t_e), std(t_e)));

fprintf('\n');
disp('Conducting performance comparison (WAV vs. WAV, 1000 reads, whole file)');

for kk=1:1000
    tic, msndfile('test.wav');
    t_e(kk) = toc;
end
disp(sprintf('mean time taken by msndfile:\t%.6f +- %.6f', mean(t_e), std(t_e)));

for kk=1:1000
    tic, wavread('test.wav');
    t_e(kk) = toc;
end
disp(sprintf('mean time taken by wavread:\t%.6f +- %.6f', mean(t_e), std(t_e)));
