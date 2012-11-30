#!/bin/sh

# a trivial script to upload releases to sf.net
rsync -avP -e ssh releases/* marcecj@frs.sourceforge.net:/home/frs/project/msndfile/
