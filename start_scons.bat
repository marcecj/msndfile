start /b cmd /k C:/Python27/Scripts/scons

REM TODO: It must be possible to find Pythons root directory automatically?
REM change to wherever SCons is installed on your system.
doskey scons=C:/Python27/Scripts/scons $*
