function [OutData,fs] = msndfile(fname)
% function to change duration of Audiodata without changing the pitch
% by using the dirac library
% Usage: OutData = TimeStretchDirac(InData,fs,TimeStretchFaktor, Mode)
% InParamter:   
%      fname:      a string describing an audio file
% OutParam:
%      OutData:     The new data vector (Len x Chns)
%      fs:     The sampling rate of the output

% (c) Author, Jade-Hochschule, Institut für Hoertechnik und Audiologie,
%             www.hoertechnik-audiologie.de
% Datum, Versions-History
% Licence:  see end of file (MIT licence)
