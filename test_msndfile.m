% Simple script for testing msndfile

clear;
close all;

% the reference: the entire file imported by wavread
[in_wav, fs] = wavread('test.wav');

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
%% Test 3: performance comparisons
%

fprintf('\n');
disp('Conducting performance comparison (1000 reads, first 1024 samples)');

for kk=1:1000
    tic, msndfile('test.flac', 1024);
    t_e(kk) = toc;
end
disp(['mean time taken by msndfile (FLAC): ' num2str(mean(t_e))]);
disp(['(standard deviation: ' num2str(std(t_e)) ')']);

for kk=1:1000
    tic, msndfile('test.wav', 1024);
    t_e(kk) = toc;
end
disp(['mean time taken by msndfile (WAV): ' num2str(mean(t_e))]);
disp(['(standard deviation: ' num2str(std(t_e)) ')']);

for kk=1:1000
    tic, wavread('test', 1024);
    t_e(kk) = toc;
end
disp(['mean time taken by wavread: ' num2str(mean(t_e))]);
disp(['(standard deviation: ' num2str(std(t_e)) ')']);

fprintf('\n');
disp('Conducting performance comparison (WAV vs. WAV, 1000 reads, whole file)');

for kk=1:1000
    tic, msndfile('test.wav', 1024);
    t_e(kk) = toc;
end
disp(['mean time taken by msndfile: ' num2str(mean(t_e))]);
disp(['(standard deviation: ' num2str(std(t_e)) ')']);

for kk=1:1000
    tic, wavread('test', 1024);
    t_e(kk) = toc;
end
disp(['mean time taken by wavread: ' num2str(mean(t_e))]);
disp(['(standard deviation: ' num2str(std(t_e)) ')']);
