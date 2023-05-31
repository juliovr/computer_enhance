@echo off

..\bin\sim8086 %1 > generated_%1.asm
nasm generated_%1.asm
fc %1 generated_%1