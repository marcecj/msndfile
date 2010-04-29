function [OutData,fs] = msndfile(file_name)
% A function to read audio files by using the libsndfile C library.
% Usage: [OutData, fs] = msndfile(file_name)
% InParamter:   
%      file_name: a string describing an audio file
% OutParam:
%      OutData:   the new data vector (Len x Chns)
%      fs:        the sampling rate of the audio file

% (c) Marc Joliet, Jade-Hochschule, Institut für Hoertechnik und Audiologie,
% www.hoertechnik-audiologie.de
% 28th April, 2010
% History: 1.0 - first properly working version
% Licence: see file 'LICENCE'
