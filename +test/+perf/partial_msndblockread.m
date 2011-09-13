function [timings_mean, timings_std, timings_labels] = partial_msndblockread(num_run, block_sizes)

fprintf('\n');
disp(['*** Conducting msndblockread performance comparison (partial reads) ***']);

timings_mean = zeros(length(block_sizes), 4);
timings_std  = zeros(length(block_sizes), 4);
timings_labels = {'msndblockread (FLAC)', ...
                  'msndblockread (WAV)', ...
                  'msndblockread (WAV, no transposition)', ...
                  'wavReader'};

t_e = zeros(num_run, 1);

for aa=1:length(block_sizes)
    fprintf('\n');

    b = block_sizes(aa);

    disp(['Conducting performance comparison (' num2str(num_run) ' reads, first ' num2str(b) ' samples)']);

    msndblockread('open', 'test.flac');
    for kk=1:num_run
        tic, msndblockread('read', 'test.flac', [1 b]);
        t_e(kk) = toc;
    end
    msndblockread('close', 'test.flac');
    timings_mean(aa,1) = mean(t_e);
    timings_std(aa,1)  = std(t_e);
    disp(sprintf('mean time taken by msndblockread (FLAC):\t%.6f +- %.6f', mean(t_e), std(t_e)));

    msndblockread('open', 'test.wav');
    for kk=1:num_run
        tic, msndblockread('read', 'test.wav', [1 b]);
        t_e(kk) = toc;
    end
    msndblockread('close', 'test.wav');
    timings_mean(aa,2) = mean(t_e);
    timings_std(aa,2)  = std(t_e);
    disp(sprintf('mean time taken by msndblockread (WAV): \t%.6f +- %.6f', mean(t_e), std(t_e)));

    msndblockread('open', 'test.wav');
    for kk=1:num_run
        tic, msndblockread('read', 'test.wav', [1 b], false);
        t_e(kk) = toc;
    end
    msndblockread('close', 'test.wav');
    timings_mean(aa,3) = mean(t_e);
    timings_std(aa,3)  = std(t_e);
    disp(sprintf('mean time taken by msndblockread (WAV): \t%.6f +- %.6f', mean(t_e), std(t_e)));

    file_h = wavReaderOpen('test.wav');
    for kk=1:num_run
        tic, wavReaderReadBlock(file_h, 1, b);
        t_e(kk) = toc;
    end
    wavReaderClose(file_h);
    timings_mean(aa,4) = mean(t_e);
    timings_std(aa,4)  = std(t_e);
    disp(sprintf('mean time taken by wavReader:\t\t\t%.6f +- %.6f', mean(t_e), std(t_e)));
end
