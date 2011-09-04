% Simple script for testing msndfile

clear;
close all;

do_perf_tests = false;

addpath('build');

% the reference: the entire file imported by wavread
[in_wav, fs] = wavread('test.wav');

disp('Test file used in all tests: test.wav (also as RAW and FLAC)')

test_msndread;
test_msndblockread;

%
%% Test 4: performance comparisons
%

if do_perf_tests
    test_msndread_perf;
    test_msndblockread_perf;
end
