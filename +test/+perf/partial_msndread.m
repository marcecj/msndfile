function [timings_mean, timings_std, timings_labels] = partial_msndread(num_run, block_sizes)

fprintf('\n');
disp(['*** Conducting msndread performance comparison (partial reads) ***']);

timings_mean = zeros(length(block_sizes), 3);
timings_std  = zeros(length(block_sizes), 3);
timings_labels = {'msndread (FLAC)', 'msndread (WAV)', 'wavread'};


for aa=1:length(block_sizes)
    fprintf('\n');

    b = block_sizes(aa);

    disp(['Conducting performance comparison (' num2str(num_run) ' reads, first ' num2str(b) ' samples)']);

    for kk=1:num_run
        tic, msndread('test.flac', b);
        t_e(kk) = toc;
    end
    timings_mean(aa,1) = mean(t_e);
    timings_std(aa,1) = std(t_e);
    disp(sprintf('mean time taken by msndread (FLAC):\t%.6f +- %.6f', mean(t_e), std(t_e)));

    for kk=1:num_run
        tic, msndread('test.wav', b);
        t_e(kk) = toc;
    end
    timings_mean(aa,2) = mean(t_e);
    timings_std(aa,2) = std(t_e);
    disp(sprintf('mean time taken by msndread (WAV):\t%.6f +- %.6f', mean(t_e), std(t_e)));

    for kk=1:num_run
        tic, wavread('test.wav', b);
        t_e(kk) = toc;
    end
    timings_mean(aa,3) = mean(t_e);
    timings_std(aa,3) = std(t_e);
    disp(sprintf('mean time taken by wavread:\t\t%.6f +- %.6f', mean(t_e), std(t_e)));
end
