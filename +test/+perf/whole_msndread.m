function [timings_mean, timings_std, timings_labels] = partial_msndblockread(num_run)

fprintf('\n');
disp(['*** Conducting msndread performance comparison (whole reads) ***']);

t_e          = zeros(num_run, 1);
timings_mean = zeros(3, 1);
timings_std  = zeros(3, 1);
% timings_labels = {'msndread (FLAC)', ...
%                   'msndread (WAV)', ...
%                   'msndread (WAV, no transposition)', ...
%                   'wavread'});
timings_labels = {'msndread (FLAC)', ...
                  'msndread (WAV)', ...
                  'wavread'};

fprintf('\n');
disp(['Conducting performance comparison (' num2str(num_run) ' full reads).']);

for kk=1:num_run
    tic, msndfile.read('test_files/test.flac');
    t_e(kk) = toc;
end
timings_mean(1)     = mean(t_e);
timings_std(1) = std(t_e);
disp(sprintf('mean time taken by msndread (FLAC):\t%.6f +- %.6f', mean(t_e), std(t_e)));

for kk=1:num_run
    tic, msndfile.read('test_files/test.wav');
    t_e(kk) = toc;
end
timings_mean(2)     = mean(t_e);
timings_std(2) = std(t_e);
disp(sprintf('mean time taken by msndread (WAV): \t%.6f +- %.6f', mean(t_e), std(t_e)));

% for kk=1:num_run
%     tic, msndfile.read('test_files/test.wav', false);
%     t_e(kk) = toc;
% end
% t_mwnt(aa) = mean(t_e);
% s_mwnt(aa) = std(t_e);
% disp(sprintf('mean time taken by msndread (WAV): \t%.6f +- %.6f', mean(t_e), std(t_e)));

for kk=1:num_run
    tic, wavread('test_files/test.wav');
    t_e(kk) = toc;
end
timings_mean(3)     = mean(t_e);
timings_std(3) = std(t_e);
disp(sprintf('mean time taken by wavread:\t\t%.6f +- %.6f', mean(t_e), std(t_e)));
