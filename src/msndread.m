function [OutData,fs] = msndread(file_name, idx_range, file_info)
%MSNDREAD Read audio files using the libsndfile C library.
%
% OUTDATA = MSNDREAD(FILE_NAME) will read the file specified by FILE_NAME and
% returns the signal OUTDATA.
%
% OUTDATA = MSNDREAD(FILE_NAME, N) returns the first N samples.  OUTDATA =
% MSNDREAD(FILE_NAME, [START END]) will read the samples in the range
% START...END.
%
% If FILE_NAME is a RAW audio file, then a third input argument, FILE_INFO, must
% be specified (see section "The file_info struct" below).
%
% SIZE = MSNDREAD(FILE_NAME, 'size') returns a two-element row vector SIZE
% containing the number of samples per channel and the number of channels.
%
% [..., FS] = MSNDREAD(...) returns the sampling rate FS.
%
% Input parameters
% ----------------
%
%      file_name:   The audio file name (a string).
%      idx_range:   A row vector defining the range of samples to be read. If it
%                   only has one element, the file will read from the beginning
%                   up to the specified index.
%      file_info:   (RAW files only) A struct containing the file information.
%                   (see section "The file_info struct" below).
%
% Output parameters
% -----------------
%
%      OutData:   The new data vector (Len x Chns).
%      fs:        The sampling rate of the audio file.
%
% The file_info struct
% --------------------
%
% The file_info struct must be passed if the file to be read is a RAW file.
%
% The file_info struct must contain the following fields:
%     samplerate:   the sampling rate of the audio file
%     channels:     the number of channels of the audio file
%     sampleformat: the sample format (format "subtype")
%
% The file_info struct may optionally contain the following fields:
%     format:       the file format ("major format"), currently ignored
%     endianness:   the sample endian-ness (defaults to "FILE")
%
% Following is a list of valid values for the format specifiers "format",
% "sampleformat" and "endianness".  The descriptions are taken from the official
% libsndfile API documentation (see http://www.mega-nerd.com/libsndfile).  Note
% that the "format" field is (as mentioned above) currently ignored.  It is
% intended to be utilised for future write support.
%
%     Valid values for "format" are:
%
%           WAV          Microsoft WAV format (little endian).
%           AIFF         Apple/SGI AIFF format (big endian).
%           AU           Sun/NeXT AU format (big endian).
%           RAW          RAW PCM data.
%           PAF          Ensoniq PARIS file format.
%           SVX          Amiga IFF / SVX8 / SV16 format.
%           NIST         Sphere NIST format.
%           VOC          VOC files.
%           IRCAM        Berkeley/IRCAM/CARL
%           W64          Sonic Foundry's 64 bit RIFF/WAV
%           MAT4         Matlab (tm) V4.2 / GNU Octave 2.0
%           MAT5         Matlab (tm) V5.0 / GNU Octave 2.1
%           PVF          Portable Voice Format
%           XI           Fasttracker 2 Extended Instrument
%           HTK          HMM Tool Kit format
%           SDS          Midi Sample Dump Standard
%           AVR          Audio Visual Research
%           WAVEX        MS WAVE with WAVEFORMATEX
%           SD2          Sound Designer 2
%           FLAC         FLAC lossless file format
%           CAF          Core Audio File format%
%
%     Valid values for "sampleformat" are:
%
%           PCM_S8       Signed 8 bit data
%           PCM_16       Signed 16 bit data
%           PCM_24       Signed 24 bit data
%           PCM_32       Signed 32 bit data
%
%           PCM_U8       Unsigned 8 bit data (WAV and RAW only)
%
%           FLOAT        32 bit float data
%           DOUBLE       64 bit float data
%
%           ULAW         U-Law encoded.
%           ALAW         A-Law encoded.
%           IMA_ADPCM    IMA ADPCM.
%           MS_ADPCM     Microsoft ADPCM.
%
%           GSM610       GSM 6.10 encoding.
%           VOX_ADPCM    Oki Dialogic ADPCM encoding.
%
%           G721_32      32kbs G721 ADPCM encoding.
%           G723_24      24kbs G723 ADPCM encoding.
%           G723_40      40kbs G723 ADPCM encoding.
%
%           DWVW_12      12 bit Delta Width Variable Word encoding.
%           DWVW_16      16 bit Delta Width Variable Word encoding.
%           DWVW_24      24 bit Delta Width Variable Word encoding.
%           DWVW_N       N bit Delta Width Variable Word encoding.
%
%           DPCM_8       8 bit differential PCM (XI only)
%           DPCM_16      16 bit differential PCM (XI only)
%
%     Valid values for "endianness" are:
%
%           FILE         Default file endian-ness.
%           LITTLE       Force little endian-ness.
%           BIG          Force big endian-ness.
%           CPU          Force CPU endian-ness.

% (c) Marc Joliet <marcec@gmx.de>
%
% Licence: see file 'LICENSE'
%
% Last modified: 24th November, 2010
%
% History: 1.0 - first properly working version
% History: 1.1 - added support for RAW files
% History: 1.2 - added support for reading a range of samples and for obtaining
%                the size of the audio data. Also fixed a stupid bug.
