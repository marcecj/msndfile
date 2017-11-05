function performance(do_write_plots)

file_info = audioinfo('test_files/test.wav');
block_sizes = 2.^(8:16).';
num_run     = 1000;

%
%% Test 1: read the first N samples of a file with varying N.
%

[tm1, ts1, tl1] = test.perf.partial_msndread(num_run, block_sizes);
[tm2, ts2, tl2] = test.perf.partial_msndblockread(num_run, block_sizes);

tm = [tm1 tm2];
ts = [ts1 ts2];
tl = [tl1 tl2];

perf_h = figure;
errorbar(repmat(block_sizes, 1, size(tm,2)), tm.*1e3, ts.*1e3);
title('Time taken to read the first N samples of an audio file.');
set(gca, 'XScale', 'Log');
xlabel('Block size [samples]');
ylabel('Average read time +/- STD [ms]');
legend(tl);

%
%% Test 2: read an entire file
%

[tm, ts, tl] = test.perf.whole_msndread(100);

perf_h(2) = figure;
errorbar(tm, ts, 'o');
title(sprintf('Time taken to read an entire %.2f s audio file @%.1f kHz.', file_info.Duration, file_info.SampleRate/1e3));
set(gca, ...
    'XTick', [1 2 3], ...
    'XTickLabel', tl);
xlabel('Function');
ylabel('Average read time +/- STD [s]');

%
%% Test 3: read an entire file block-wise with varying block length N.
%

[tm, ts, tl] = test.perf.whole_msndblockread(num_run, block_sizes);

perf_h(3) = figure;
errorbar(repmat(block_sizes, 1, size(tm,2)), tm.*1e3, ts.*1e3, '-o');
title('Time taken to read an audio file (9.5 s, @44.1 kHz) block-wise with varying block size.');
xlabel('Block size [samples]');
ylabel('Average read time +/- STD [ms]');
set(gca, 'XScale', 'Log');
legend(tl);

% print figures
if do_write_plots
    if ~exist('./graphics', 'dir')
        mkdir('.', 'graphics');
    end

    fnames = {'perf_partial', ...
              'perf_msndread_whole', ...
              'perf_msndblockread_whole'};
    for k=1:length(perf_h)
        print(perf_h(k), '-depsc2', ['graphics' filesep fnames{k}]);
    end
end
