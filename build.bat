@echo off

REM debug build flags
set CommonCompilerFlags=-Od -MTd -Zi -nologo -GR- -EHa- -Oi -WX -Wall
set CommonLinkerFlags=user32.lib gdi32.lib Comdlg32.lib

REM release build flags
REM set CommonCompilerFlags=-O2 -MT -nologo -Oi -WX -Wall
REM set CommonLinkerFlags=user32.lib gdi32.lib

IF NOT EXIST .\build mkdir .\build
pushd .\build

REM del *.pdb > NUL 2> NUL
cl %CommonCompilerFlags% ..\main.c /link %CommonLinkerFlags%
popd
