fprintf('\n*** Testing msndread ***\n\n');

file_info = [];

% verify that msndread raises an error when called without input arguments
try
    msndread;
    warning('Error should be thrown!');
catch
    disp('Error correctly raised...');
end

%
%% Test 1: importing RAW files
%

% verify that msndread raises an error when called with insufficient arguments
try
    [in_raw, fs] = msndread('test.raw', []);
    warning('Error should be thrown!');
catch
    disp('Error correctly raised...');
end
try
    [in_raw, fs] = msndread('test.raw', [], []);
    warning('Error should be thrown!');
catch
    disp('Error correctly raised...');
end

% verify that msndread raises an error when file_info is incomplete
file_info.samplerate   = 44100;
try
    [in_raw, fs] = msndread('test.raw', [], file_info);
    warning('Error should be thrown!');
catch
    disp('Error correctly raised...');
end

file_info.channels     = 2;
try
    [in_raw, fs] = msndread('test.raw', [], file_info);
    warning('Error should be thrown!');
catch
    disp('Error correctly raised...');
end

file_info.format       = 'RAW';
try
    [in_raw, fs] = msndread('test.raw', [], file_info);
    warning('Error should be thrown!');
catch
    disp('Error correctly raised...');
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

num_unequal  = sum(in_blockwise  - in_wav(1:num_samples,:));

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
