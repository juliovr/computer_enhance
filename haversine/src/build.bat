@echo off

set common_compiler_flags=-Od -fp:fast -nologo -MT -Oi -Gm- -GR- -EHa- -W4 -WX -wd4100 -wd4996 -FC -Z7

IF NOT EXIST ..\bin mkdir ..\bin
pushd ..\bin

cl %common_compiler_flags% ..\src\haversine.cpp -Fmhaversine.map /link -incremental:no -opt:ref

popd

