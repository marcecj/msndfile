#/bin/sh
#This script runs msndfile through valgrind

valgrind_opts="--error-limit=no --tool=memcheck --track-origins=yes --show-reachable=yes --leak-check=full -v"
matlab_opts="-nojvm -nosplash"

matlab $matlab_opts -r "run_valgrind;exit" -D"valgrind $valgrind_opts --log-file=valgrind.log"
