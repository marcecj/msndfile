% Simple script for testing msndfile

clear;
close all;

do_perf_tests = false;

addpath('build');

% the reference: the entire file imported by wavread
[in_wav, fs] = wavread('test.wav');

disp('Testing msndread...')
disp('Test file used in all tests: test.wav (also as RAW and FLAC)')

% verify that msndread raises an error when called without input arguments
try
    msndread;
catch
    disp('All in order...');
end

%
%% Test 1: importing RAW files
%

% verify that msndread raises an error when called with insufficient arguments
try
    [in_raw, fs] = msndread('test.raw', []);
catch
    disp('All in order...');
end
try
    [in_raw, fs] = msndread('test.raw', [], []);
catch
    disp('All in order...');
end

% verify that msndread raises an error when file_info is incomplete
file_info.samplerate   = 44100;
try
    [in_raw, fs] = msndread('test.raw', [], file_info);
catch
    disp('All in order...');
end

file_info.channels     = 2;
try
    [in_raw, fs] = msndread('test.raw', [], file_info);
catch
    disp('All in order...');
end

file_info.format       = 'RAW';
try
    [in_raw, fs] = msndread('test.raw', [], file_info);
catch
    disp('All in order...');
end

file_info.sampleformat = 'PCM_16';
% file_info.endianness   = 'LITTLE'; % defaults to 'FILE'

% test the raw file import
[in_raw, fs] = msndread('test.raw', [], file_info);

num_unequal = sum(abs(in_wav - in_raw) > 0);

fprintf('\n');
disp('Comparing RAW (msndread) and WAV (wavread)');
disp(['There are ' num2str(num_unequal) ' incorrect samples']);

%
%% Test 2: block-wise reading
%

num_samples  = 16384;
in_blockwise     = zeros(num_samples, 2);
in_raw_blockwise = zeros(num_samples, 2);
for kk = 1:1024:16384
    in_blockwise(kk:kk+1023, :)     = msndread('test.wav', [kk kk+1023]);
    in_raw_blockwise(kk:kk+1023, :) = msndread('test.raw', [kk kk+1023], file_info);
end

num_unequal = sum(in_blockwise - in_wav(1:num_samples,:));

fprintf('\n');
disp('Comparing WAV (msndread, blockwise) and WAV (wavread)');
disp(['There are ' num2str(num_unequal) ' incorrect samples']);

num_unequal = sum(in_raw_blockwise - in_wav(1:num_samples,:));

fprintf('\n');
disp('Comparing RAW (msndread, blockwise) and WAV (wavread)');
disp(['There are ' num2str(num_unequal) ' incorrect samples']);

%
%% Test 3: test 'size' command
%

[file_size, fs] = wavread('test.wav', 'size');
disp(sprintf('wavread   (WAV):\tLength = %i,\tNChns = %i,\tFS = %i', file_size, fs));

[file_size, fs] = msndread('test.wav', 'size');
disp(sprintf('msndread  (WAV):\tLength = %i,\tNChns = %i,\tFS = %i', file_size, fs));

[file_size, fs] = msndread('test.raw', 'size', file_info);
disp(sprintf('msndread  (RAW):\tLength = %i,\tNChns = %i,\tFS = %i', file_size, fs));

[file_size, fs] = msndread('test.flac', 'size');
disp(sprintf('msndread (FLAC):\tLength = %i,\tNChns = %i,\tFS = %i', file_size, fs));

%
%% Test 4: performance comparisons
%

if ~do_perf_tests
    break;
end

block_sizes = 2.^(8:16).';
t_mf = zeros(length(block_sizes), 1);
t_mw = t_mf;
t_ww = t_mf;
s_mf = t_mf;
s_mw = t_mf;
s_ww = t_mf;

for aa=1:length(block_sizes)
    fprintf('\n');

    b = block_sizes(aa);

    num_run = 1000;
    disp(['Conducting performance comparison (' num2str(num_run) ' reads, first ' num2str(b) ' samples)']);

    for kk=1:num_run
        tic, msndread('test.flac', b);
        t_e(kk) = toc;
    end
    t_mf(aa) = mean(t_e);
    s_mf(aa) = std(t_e);
    disp(sprintf('mean time taken by msndread (FLAC):\t%.6f +- %.6f', mean(t_e), std(t_e)));

    for kk=1:num_run
        tic, msndread('test.wav', b);
        t_e(kk) = toc;
    end
    t_mw(aa) = mean(t_e);
    s_mw(aa) = std(t_e);
    disp(sprintf('mean time taken by msndread (WAV):\t%.6f +- %.6f', mean(t_e), std(t_e)));

    for kk=1:num_run
        tic, wavread('test.wav', b);
        t_e(kk) = toc;
    end
    t_ww(aa) = mean(t_e);
    s_ww(aa) = std(t_e);
    disp(sprintf('mean time taken by wavread:\t%.6f +- %.6f', mean(t_e), std(t_e)));
end

figure;
errorbar([block_sizes block_sizes block_sizes], ...
         [t_mf t_mw t_ww].*1e3, [s_mf s_mw s_ww].*1e3, ...
         '-o');
set(gca, 'XScale', 'Log');
xlabel('Block size [samples]');
ylabel('Average read time +/- STD [ms]');
legend({'msndread (FLAC)', 'msndread (WAV)', 'wavread'});

fprintf('\n');
disp(['Conducting performance comparison (WAV vs. WAV, ' num2str(num_run) ' reads, whole file)']);

for kk=1:num_run
    tic, msndread('test.wav');
    t_e(kk) = toc;
end
disp(sprintf('mean time taken by msndread:\t%.6f +- %.6f', mean(t_e), std(t_e)));

for kk=1:1000
    tic, wavread('test.wav');
    t_e(kk) = toc;
end
disp(sprintf('mean time taken by wavread:\t%.6f +- %.6f', mean(t_e), std(t_e)));
