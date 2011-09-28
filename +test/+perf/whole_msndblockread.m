function [timings_mean, timings_std, timings_labels] = partial_msndblockread(num_run, block_sizes)

% This test script runs a performance comparison between msndblockread (WAV and
% FLAC files) and a pure Matlab solution that only supports WAV files.  The
% Matlab solution is not mine, therefor I cannot publish it.

fprintf('\n');
disp(['*** Conducting msndblockread performance comparison (whole reads) ***']);

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

file_size = wavread('test.wav', 'size');

for aa=1:length(block_sizes)
    fprintf('\n');

    b = block_sizes(aa);

    disp(['Conducting performance comparison (' num2str(num_run) ' full reads, ' num2str(b) ' samples at a time).']);

    for kk=1:num_run
        msndfile.blockread('open', 'test.flac');
        tic,
        for ll=1:b:file_size(1)-b
            msndfile.blockread('read', 'test.flac', b);
        end
        msndfile.blockread('read', 'test.flac', [ll file_size(1)]);
        t_e(kk) = toc;
        msndfile.blockread('close', 'test.flac');
    end
    timings_mean(aa,1) = mean(t_e);
    timings_std(aa,1)  = std(t_e);
    disp(sprintf('mean time taken by msndblockread (FLAC):\t%.6f +- %.6f', mean(t_e), std(t_e)));

    for kk=1:num_run
        msndfile.blockread('open', 'test.wav');
        tic,
        for ll=1:b:file_size(1)-b
            msndfile.blockread('read', 'test.wav', b);
        end
        msndfile.blockread('read', 'test.wav', [ll file_size(1)]);
        t_e(kk) = toc;
        msndfile.blockread('close', 'test.wav');
    end
    timings_mean(aa,2) = mean(t_e);
    timings_std(aa,2)  = std(t_e);
    disp(sprintf('mean time taken by msndblockread (WAV): \t%.6f +- %.6f', mean(t_e), std(t_e)));

    for kk=1:num_run
        msndfile.blockread('open', 'test.wav');
        tic,
        for ll=1:b:file_size(1)-b
            msndfile.blockread('read', 'test.wav', b, false);
        end
        msndfile.blockread('read', 'test.wav', [ll file_size(1)], false);
        t_e(kk) = toc;
        msndfile.blockread('close', 'test.wav');
    end
    timings_mean(aa,3) = mean(t_e);
    timings_std(aa,3)  = std(t_e);
    disp(sprintf('mean time taken by msndblockread (WAV): \t%.6f +- %.6f', mean(t_e), std(t_e)));

    if exist('wavReaderOpen', 'file')
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
        timings_mean(aa,4) = mean(t_e);
        timings_std(aa,4)  = std(t_e);
        disp(sprintf('mean time taken by wavReader:\t\t\t%.6f +- %.6f', mean(t_e), std(t_e)));
    end
end
