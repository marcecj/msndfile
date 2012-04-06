function OutData = msndblockread(cmd, file_name, idx_range, file_info)
%MSNDBLOCKREAD Read audio files block-wise using the libsndfile C library.
%
% MSNDBLOCKREAD('open', FILE_NAME) will open the file specified by FILE_NAME for
% reading.
%
% OUTDATA = MSNDREAD('read', FILE_NAME, [A B]) returns the samples in the range
% A...B.  OUTDATA = MSNDREAD(FILE_NAME, N) will read the next N samples,
% starting from wherever left off.  For example, if no samples have been read
% yet, the first N samples are read.
%
% If FILE_NAME is a RAW audio file, then a fourth input argument, FILE_INFO,
% must be specified (see section "The file_info struct" below).
%
% MSNDBLOCKREAD('close', FILE_NAME) will close an open file specified by
% FILE_NAME. MSNDBLOCKREAD('closeall') closes all open files.
%
% Input parameters
% ----------------
%
%      cmd:         The command (a string). Must be one of "open", "read",
%                   "close", or "closeall" (see section "Commands" below).
%      file_name:   The audio file name (a string).
%      idx_range:   A row vector defining the range of samples to be read. If it
%                   only has one element, the file will read from the current
%                   position up to the specified index.
%      file_info:   (RAW files only) A struct containing the file information
%                   (see section "The file_info struct" below).
%
% Output parameters
% -----------------
%
%      OutData:   The new data vector (Len x Chns).
%
% Commands
% --------
%
% The following are valid commands to MSNDBLOCKREAD:
%
%   open:       Opens the audio file. Several files may be opened at once,
%               however each individual file may only be opened once.
%   read:       Reads data from an open file.
%   close:      Closes an open file.
%   closeall:   Closes all currently open files.
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
%     format:       the file format ("major format") (defaults to "RAW")
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