fprintf('\n*** Testing msndread ***\n\n');

file_info = [];

% verify that msndread raises an error when called without input arguments
try
    disp('* Calling without arguments...');
    msndfile.read;
    warning('test:err', '... error should be thrown!');
catch ME
    disp('  ... error correctly raised.');
end

%
%% Test 1: importing RAW files
%

% verify that msndread raises an error when called with insufficient arguments

try
    disp('* Attemting to read test.raw with empty range...\n');
    [in_raw, fs] = msndfile.read('test.raw', []);
    warning('test:err', '... error should be thrown!');
catch ME
    disp('  ... error correctly raised.');
end

try
    disp('* Attemting to read test.raw with empty range and fmt...');
    [in_raw, fs] = msndfile.read('test.raw', [], []);
    warning('test:err', '... error should be thrown!');
catch ME
    disp('  ... error correctly raised.');
end

try
    disp('* Attemting to read test.raw with empty range, fmt and empty file_info struct...');
    [in_raw, fs] = msndfile.read('test.raw', [], [], []);
    warning('test:err', '... error should be thrown!');
catch ME
    disp('  ... error correctly raised.');
end


% verify that msndread raises an error when file_info is incomplete

disp('* Attemting to read test.raw with file_info struct...');

file_info.samplerate   = 44100;
try
    disp('  with field ''samplerate''...');
    [in_raw, fs] = msndfile.read('test.raw', [], [], file_info);
    warning('test:err', '... error should be thrown!');
catch ME
    disp('  ... error correctly raised.');
end

file_info.channels     = 2;
try
    disp('  with fields ''samplerate'' and ''channels''...');
    [in_raw, fs] = msndfile.read('test.raw', [], [], file_info);
    warning('test:err', '... error should be thrown!');
catch ME
    disp('  ... error correctly raised.');
end

file_info.format       = 'RAW';
try
    disp('  with fields ''samplerate'', ''channels'' and ''format''...');
    [in_raw, fs] = msndfile.read('test.raw', [], [], file_info);
    warning('test:err', '... error should be thrown!');
catch ME
    disp('  ... error correctly raised.');
end

disp('* Attemting to read test.raw with complete file_info struct...');
file_info.sampleformat = 'PCM_16';
% file_info.endianness   = 'LITTLE'; % defaults to 'FILE'

% test the RAW file import

[in_raw, fs] = msndfile.read('test.raw', [], [], file_info);
disp('  ... it worked!');

num_unequal = sum(abs(in_wav - in_raw) > 0);

disp('* Comparing RAW (msndread) and WAV (wavread)');
disp(['  There are ' num2str(num_unequal) ' incorrect samples']);

%
%% Test 2: block-wise reading
%

num_samples  = 16384;
in_blockwise     = zeros(num_samples, 2);
in_raw_blockwise = zeros(num_samples, 2);
for kk = 1:1024:16384
    in_blockwise(kk:kk+1023, :)     = msndfile.read('test.wav', [kk kk+1023]);
    in_raw_blockwise(kk:kk+1023, :) = msndfile.read('test.raw', [kk kk+1023], [], file_info);
end

num_unequal  = sum(in_blockwise  - in_wav(1:num_samples,:));

disp('* Comparing WAV (msndread, blockwise) and WAV (wavread)');
disp(['  There are ' num2str(num_unequal) ' incorrect samples']);

num_unequal = sum(in_raw_blockwise - in_wav(1:num_samples,:));

disp('* Comparing RAW (msndread, blockwise) and WAV (wavread)');
disp(['  There are ' num2str(num_unequal) ' incorrect samples']);

%
%% Test 3: test 'size' command
%

disp('* Comparing output of ''size'' command...');

[file_size, fs] = wavread('test.wav', 'size');
fprintf('  wavread   (WAV):\tLength = %i,\tNChns = %i,\tFS = %i\n', file_size, fs);

[file_size, fs] = msndfile.read('test.wav', 'size');
fprintf('  msndread  (WAV):\tLength = %i,\tNChns = %i,\tFS = %i\n', file_size, fs);

[file_size, fs] = msndfile.read('test.raw', 'size', [], file_info);
fprintf('  msndread  (RAW):\tLength = %i,\tNChns = %i,\tFS = %i\n', file_size, fs);

[file_size, fs] = msndfile.read('test.flac', 'size');
fprintf('  msndread (FLAC):\tLength = %i,\tNChns = %i,\tFS = %i\n', file_size, fs);
