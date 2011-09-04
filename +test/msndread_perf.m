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
