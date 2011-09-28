fprintf('\n*** Testing msndread ***\n');

file_info = [];

% verify that msndread raises an error when called without input arguments
try
    fprintf('\n* Calling without arguments...\n\n');
    msndfile.read;
    warning('... error should be thrown!');
catch
    disp('... error correctly raised.');
end

%
%% Test 1: importing RAW files
%

% verify that msndread raises an error when called with insufficient arguments

try
    fprintf('\n* Attemting to read test.raw with empty range...\n\n');
    [in_raw, fs] = msndfile.read('test.raw', []);
    warning('... error should be thrown!');
catch
    disp('... error correctly raised.');
end

try
    fprintf('\n* Attemting to read test.raw with empty range and file_info struct...\n\n');
    [in_raw, fs] = msndfile.read('test.raw', [], []);
    warning('... error should be thrown!');
catch
    disp('... error correctly raised.');
end

% verify that msndread raises an error when file_info is incomplete

fprintf('\n* Attemting to read test.raw with file_info struct...\n');

file_info.samplerate   = 44100;
try
    fprintf('\nwith field ''samplerate''...\n');
    [in_raw, fs] = msndfile.read('test.raw', [], file_info);
    warning('... error should be thrown!');
catch
    disp('... error correctly raised.');
end

file_info.channels     = 2;
try
    fprintf('\nwith fields ''samplerate'' and ''channels''...\n');
    [in_raw, fs] = msndfile.read('test.raw', [], file_info);
    warning('... error should be thrown!');
catch
    disp('... error correctly raised.');
end

file_info.format       = 'RAW';
try
    fprintf('\nwith fields ''samplerate'', ''channels'' and ''format''...\n');
    [in_raw, fs] = msndfile.read('test.raw', [], file_info);
    warning('... error should be thrown!');
catch
    disp('... error correctly raised.');
end

fprintf('\n* Attemting to read test.raw with complete file_info struct...\n');
file_info.sampleformat = 'PCM_16';
% file_info.endianness   = 'LITTLE'; % defaults to 'FILE'

% test the RAW file import

[in_raw, fs] = msndfile.read('test.raw', [], file_info);
fprintf('\n... it worked!\n');

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
    in_blockwise(kk:kk+1023, :)     = msndfile.read('test.wav', [kk kk+1023]);
    in_raw_blockwise(kk:kk+1023, :) = msndfile.read('test.raw', [kk kk+1023], file_info);
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

fprintf('\n* Comparing output of ''size'' command...\n\n');

[file_size, fs] = wavread('test.wav', 'size');
disp(sprintf('wavread   (WAV):\tLength = %i,\tNChns = %i,\tFS = %i', file_size, fs));

[file_size, fs] = msndfile.read('test.wav', 'size');
disp(sprintf('msndread  (WAV):\tLength = %i,\tNChns = %i,\tFS = %i', file_size, fs));

[file_size, fs] = msndfile.read('test.raw', 'size', file_info);
disp(sprintf('msndread  (RAW):\tLength = %i,\tNChns = %i,\tFS = %i', file_size, fs));

[file_size, fs] = msndfile.read('test.flac', 'size');
disp(sprintf('msndread (FLAC):\tLength = %i,\tNChns = %i,\tFS = %i', file_size, fs));
