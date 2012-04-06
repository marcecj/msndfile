% Simple script used when running msndfile through valgrind

clear;
close all;

addpath('debug');

test.msndread;
test.msndblockread;
