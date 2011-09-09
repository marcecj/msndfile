% This test script runs a performance comparison between msndblockread (WAV and
% FLAC files) and a pure Matlab solution that only supports WAV files.  The
% Matlab solution is not mine, therefor I cannot publish it.

block_sizes = 2.^(8:16).';
num_run     = 1000;
file_size   = wavread('test.wav', 'size');

t_mf = zeros(length(block_sizes), 1);
s_mf = t_mf;
t_mw = t_mf;
s_mw = t_mf;
t_ww = t_mf;
s_ww = t_mf;
t_mwnt = t_mf;
s_mwnt = t_mf;

%
%% Test 1: read an entire file block-wise with varying block length N.
%

for aa=1:length(block_sizes)
    fprintf('\n');

    b = block_sizes(aa);

    disp(['Conducting performance comparison (' num2str(num_run) ' full reads, ' num2str(b) ' samples at a time).']);

    for kk=1:num_run
        msndblockread('open', 'test.flac');
        tic,
        for ll=1:b:file_size(1)-b
            msndblockread('read', 'test.flac', b);
        end
        msndblockread('read', 'test.flac', [ll file_size(1)]);
        t_e(kk) = toc;
        msndblockread('close', 'test.flac');
    end
    t_mf(aa) = mean(t_e);
    s_mf(aa) = std(t_e);
    disp(sprintf('mean time taken by msndblockread (FLAC):\t%.6f +- %.6f', mean(t_e), std(t_e)));

    for kk=1:num_run
        msndblockread('open', 'test.wav');
        tic,
        for ll=1:b:file_size(1)-b
            msndblockread('read', 'test.wav', b);
        end
        msndblockread('read', 'test.wav', [ll file_size(1)]);
        t_e(kk) = toc;
        msndblockread('close', 'test.wav');
    end
    t_mw(aa) = mean(t_e);
    s_mw(aa) = std(t_e);
    disp(sprintf('mean time taken by msndblockread (WAV): \t%.6f +- %.6f', mean(t_e), std(t_e)));

    for kk=1:num_run
        msndblockread('open', 'test.wav');
        tic,
        for ll=1:b:file_size(1)-b
            msndblockread('read', 'test.wav', b, false);
        end
        msndblockread('read', 'test.wav', [ll file_size(1)], false);
        t_e(kk) = toc;
        msndblockread('close', 'test.wav');
    end
    t_mwnt(aa) = mean(t_e);
    s_mwnt(aa) = std(t_e);
    disp(sprintf('mean time taken by msndblockread (WAV): \t%.6f +- %.6f', mean(t_e), std(t_e)));

    for kk=1:num_run
        file_h = wavReaderOpen('test.wav');
        tic,
        for ll=1:b:file_size(1)-b
            wavReaderReadBlock(file_h, ll, ll+b-1);
        end
        wavReaderReadBlock(file_h, ll, file_size(1));
        t_e(kk) = toc;
        wavReaderClose(file_h);
    end
    t_ww(aa) = mean(t_e);
    s_ww(aa) = std(t_e);
    disp(sprintf('mean time taken by wavReader:\t\t\t%.6f +- %.6f', mean(t_e), std(t_e)));
end

f_h = figure;
title('Time taken to read an entire 9.5 s audio file @44.1 kHz with varying block size.');
errorbar([block_sizes block_sizes block_sizes block_sizes], ...
         [t_mf t_mw t_mwnt t_ww].*1e3, [s_mf s_mw s_mwnt s_ww].*1e3, ...
         '-o');
set(gca, 'XScale', 'Log');
xlabel('Block size [samples]');
ylabel('Average read time +/- STD [ms]');
legend({'msndblockread (FLAC)', 'msndblockread (WAV)', ...
        'msndblockread (WAV, no transposition)', 'wavReader'});

%
%% Test 2: read the first N samples with varying N.
%

% reinitialise to zero
t_mf = zeros(length(block_sizes), 1);
s_mf = t_mf;
t_mw = t_mf;
s_mw = t_mf;
t_ww = t_mf;
s_ww = t_mf;
t_mwnt = t_mf;
s_mwnt = t_mf;

for aa=1:length(block_sizes)
    fprintf('\n');

    b = block_sizes(aa);

    disp(['Conducting performance comparison (' num2str(num_run) ' partial reads, first ' num2str(b) ' samples).']);

    msndblockread('open', 'test.flac');
    for kk=1:num_run
        tic, msndblockread('read', 'test.flac', [1 b]);
        t_e(kk) = toc;
    end
    msndblockread('close', 'test.flac');
    t_mf(aa) = mean(t_e);
    s_mf(aa) = std(t_e);
    disp(sprintf('mean time taken by msndblockread (FLAC):\t%.6f +- %.6f', mean(t_e), std(t_e)));

    msndblockread('open', 'test.wav');
    for kk=1:num_run
        tic, msndblockread('read', 'test.wav', [1 b]);
        t_e(kk) = toc;
    end
    msndblockread('close', 'test.wav');
    t_mw(aa) = mean(t_e);
    s_mw(aa) = std(t_e);
    disp(sprintf('mean time taken by msndblockread (WAV): \t%.6f +- %.6f', mean(t_e), std(t_e)));

    msndblockread('open', 'test.wav');
    for kk=1:num_run
        tic, msndblockread('read', 'test.wav', [1 b], false);
        t_e(kk) = toc;
    end
    msndblockread('close', 'test.wav');
    t_mwnt(aa) = mean(t_e);
    s_mwnt(aa) = std(t_e);
    disp(sprintf('mean time taken by msndblockread (WAV): \t%.6f +- %.6f', mean(t_e), std(t_e)));

    file_h = wavReaderOpen('test.wav');
    for kk=1:num_run
        tic, wavReaderReadBlock(file_h, 1, b);
        t_e(kk) = toc;
    end
    wavReaderClose(file_h);
    t_ww(aa) = mean(t_e);
    s_ww(aa) = std(t_e);
    disp(sprintf('mean time taken by wavReader:\t\t\t%.6f +- %.6f', mean(t_e), std(t_e)));
end

f_h(2) = figure;
title('Time taken to read an entire 9.5 s audio file @44.1 kHz with varying block size.');
errorbar([block_sizes block_sizes block_sizes block_sizes], ...
         [t_mf t_mw t_mwnt t_ww].*1e3, [s_mf s_mw s_mwnt s_ww].*1e3, ...
         '-o');
set(gca, 'XScale', 'Log');
xlabel('Block size [samples]');
ylabel('Average read time +/- STD [ms]');
legend({'msndblockread (FLAC)', 'msndblockread (WAV)', ...
        'msndblockread (WAV, no transposition)', 'wavReader'});

% print figures
fnames = {'perf_comp_whole', 'perf_comp_partial'};
for k=1:length(f_h)
    print(f_h(k), '-depsc2', ['graphics' filesep fnames{k}]);
end
