# The msndfile Mex extension
Marc Joliet <marcec@gmx.de>

For more information, visit the [project
homepage](https://marcecj.github.io/msndfile/).

## Introduction

msndfile is a suite of MATLAB Mex wrappers of the
[libsndfile](http://www.mega-nerd.com/libsndfile) C library for reading and
writing audio files.  Currently, it consists of of two functions:
`msndfile.read` and `msndfile.blockread`.  The former is intended as an API
compatible replacement for `wavread()`, latter is intended for reading audio
files block-wise, for instance to process large files that don't fit in RAM.

Due to the use of [libsndfile](http://www.mega-nerd.com/libsndfile), msndfile
supports more formats than `wavread`, such as various WAV version not supported by
`wavread` (e.g., ADPCM, µ-law and A-law), [OGG Vorbis](http://www.vorbis.com/)
and (in my opinion much more interesting) the Free Lossless Audio Codec
[FLAC](https://xiph.org/flac/).

Still missing are the corresponding functions for writing data: `msndfile.write`
and `msndfile.blockwrite`.  Just as with `msndfile.read`, `msndfile.write` is
supposed to be API-compatible with `wavwrite`.

## Installation from source

There are two ways to compile msndfile.  The easy way, in Matlab, is to type
`compile_msndfile` to compile all Mex extensions.  The resulting package
directory `+msndfile` should be copied to a location in MATLABs path.

There is also a more flexible build system based on SCons that can also
generate a Visual Studio IDE project file and can be integrated into other IDEs
like Eclipse.  This build system is explained in more detail in the full
documentation on the [project homepage](https://marcecj.github.io/msndfile/)
(or in the `doc` directory in the source repository).

## Copyrights

msndfile is Copyright (C) 2010-2015 Marc Joliet <marcec@gmx.de> and licenced
under the MIT licence.  See the file [LICENSE](LICENSE) in the project
repository.

The documentation of msndfile (including this README) is Copyright (C) 2010-2015
Marc Joliet <marcec@gmx.de> and licenced under the [Attribution-ShareAlike 3.0
Unported (CC BY-SA 3.0)](http://creativecommons.org/licenses/by-sa/3.0/)
license.

The SCons logo is licensed under the [Creative Commons
Attribution-Noncommercial-Share Alike 3.0
Unported](http://creativecommons.org/licenses/by-nc-sa/3.0/) license.

The Git Logo by [Jason Long](http://twitter.com/jasonlong) is licensed under the
[Creative Commons Attribution 3.0
Unported](http://creativecommons.org/licenses/by/3.0/) license.

The test files test.{raw,flac,wav} are an excerpt of "Glass Cafe" by Jon7 and
are licensed under the [Attribution-NonCommercial-ShareAlike 2.5 Generic (CC
BY-NC-SA 2.5)](http://creativecommons.org/licenses/by-nc-sa/2.5/) license.

[libsndfile](https://github.com/erikd/libsndfile/) is maintained by Erik de
Castro Lopo and is licensed under the LGPLv2.1 (included in the source
directory, see the file [LGPL-2.1](LGPL-2.1).
