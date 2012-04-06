function [timings_mean, timings_std, timings_labels] = partial_msndblockread(num_run, block_sizes)

fprintf('\n');
disp(['*** Conducting msndblockread performance comparison (partial reads) ***']);

if exist('wavReaderOpen', 'file')
    timings_mean = zeros(length(block_sizes), 4);
    timings_std  = zeros(length(block_sizes), 4);
    timings_labels = {'msndblockread (FLAC)', ...
                  'msndblockread (WAV)', ...
                  'msndblockread (WAV, no transposition)', ...
                  'wavReader'};
else
    warning('Skipping ''wavReader'' tests.');

    timings_mean = zeros(length(block_sizes), 3);
    timings_std  = zeros(length(block_sizes), 3);
    timings_labels = {'msndblockread (FLAC)', ...
                  'msndblockread (WAV)', ...
                  'msndblockread (WAV, no transposition)'};
end

t_e = zeros(num_run, 1);

for aa=1:length(block_sizes)
    fprintf('\n');

    b = block_sizes(aa);

    disp(['Conducting performance comparison (' num2str(num_run) ' reads, first ' num2str(b) ' samples)']);

    msndfile.blockread('open', 'test_files/test.flac');
    for kk=1:num_run
        tic, msndfile.blockread('read', 'test_files/test.flac', [1 b]);
        t_e(kk) = toc;
    end
    msndfile.blockread('close', 'test_files/test.flac');
    timings_mean(aa,1) = mean(t_e);
    timings_std(aa,1)  = std(t_e);
    disp(sprintf('mean time taken by msndblockread (FLAC):\t%.6f +- %.6f', mean(t_e), std(t_e)));

    msndfile.blockread('open', 'test_files/test.wav');
    for kk=1:num_run
        tic, msndfile.blockread('read', 'test_files/test.wav', [1 b]);
        t_e(kk) = toc;
    end
    msndfile.blockread('close', 'test_files/test.wav');
    timings_mean(aa,2) = mean(t_e);
    timings_std(aa,2)  = std(t_e);
    disp(sprintf('mean time taken by msndblockread (WAV): \t%.6f +- %.6f', mean(t_e), std(t_e)));

    msndfile.blockread('open', 'test_files/test.wav');
    for kk=1:num_run
        tic, msndfile.blockread('read', 'test_files/test.wav', [1 b], false);
        t_e(kk) = toc;
    end
    msndfile.blockread('close', 'test_files/test.wav');
    timings_mean(aa,3) = mean(t_e);
    timings_std(aa,3)  = std(t_e);
    disp(sprintf('mean time taken by msndblockread (WAV): \t%.6f +- %.6f', mean(t_e), std(t_e)));

    if exist('wavReaderOpen', 'file')
        file_h = wavReaderOpen('test_files/test.wav');
        for kk=1:num_run
            tic, wavReaderReadBlock(file_h, 1, b);
            t_e(kk) = toc;
        end
        wavReaderClose(file_h);
        timings_mean(aa,4) = mean(t_e);
        timings_std(aa,4)  = std(t_e);
        disp(sprintf('mean time taken by wavReader:\t\t\t%.6f +- %.6f', mean(t_e), std(t_e)));
    end
end
