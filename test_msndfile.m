% Simple script for testing msndfile

clear;
close all;

do_perf_tests = false;

addpath('build');

disp('Test file used in all tests: test.wav (also as RAW and FLAC)')

test.msndread;
test.msndblockread;

%
%% Test 4: performance comparisons
%

if do_perf_tests
    % conduct performance tests and save plots ("true")
    test.performance(true);
end
