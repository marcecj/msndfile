#!/bin/sh

# a trivial script to upload releases to sf.net
cp CHANGELOG releases/README.md
rsync -avP -e ssh releases/* marcecj@frs.sourceforge.net:/home/frs/project/msndfile/
