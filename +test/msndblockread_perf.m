block_sizes = 2.^(8:16).';
num_run     = 1000;

t_mf = zeros(length(block_sizes), 1);
t_mw = t_mf;
t_ww = t_mf;
s_mf = t_mf;
s_mw = t_mf;
s_ww = t_mf;

msndblockread('open', 'test.wav');
for aa=1:length(block_sizes)
    fprintf('\n');

    b = block_sizes(aa);

    disp(['Conducting performance comparison (' num2str(num_run) ' reads, first ' num2str(b) ' samples)']);

    for kk=1:num_run
        tic, msndblockread('read', 'test.wav', [1 b]);
        t_e(kk) = toc;
    end
    t_mf(aa) = mean(t_e);
    s_mf(aa) = std(t_e);
    disp(sprintf('mean time taken by msndblockread:\t%.6f +- %.6f', mean(t_e), std(t_e)));

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
msndblockread('close', 'test.wav');

figure;
errorbar([block_sizes block_sizes block_sizes], ...
         [t_mf t_mw t_ww].*1e3, [s_mf s_mw s_ww].*1e3, ...
         '-o');
set(gca, 'XScale', 'Log');
xlabel('Block size [samples]');
ylabel('Average read time +/- STD [ms]');
legend({'msndblockread', 'msndread (WAV)', 'wavread'});
