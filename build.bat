@echo off

REM debug build flags
set CommonCompilerFlags=-Od -MTd -Zi -nologo -GR- -EHa- -Oi -WX -Wall -Qspectre
set CommonLinkerFlags=user32.lib gdi32.lib comdlg32.lib winmm.lib

REM release build flags
REM set CommonCompilerFlags=-O2 -MT -nologo -Oi -WX -Wall

IF NOT EXIST .\build mkdir .\build
pushd .\build

REM del *.pdb > NUL 2> NUL
cl %CommonCompilerFlags% ..\main.c /Fe"ppm-viewer" /link %CommonLinkerFlags%
popd
