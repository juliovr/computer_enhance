@echo off

set common_compiler_flags=-Od -fp:fast -nologo -MT -Oi -Gm- -GR- -EHa- -W4 -WX -wd4201 -wd4100 -wd4189 -wd4505 -wd4456 -wd4459 -wd4311 -wd4312 -wd4302 -wd4706 -wd4127 -FC -Z7

IF NOT EXIST ..\bin mkdir ..\bin
pushd ..\bin

REM cl %common_compiler_flags% ..\src\sim8086.cpp -Fmsim8086.map /link -incremental:no -opt:ref
cl -Od  -nologo -MT -Oi -Gm- -GR- -EHa- -FC -Z7 ..\src\sim8086.cpp

popd

