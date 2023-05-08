@echo off

nasm %1.asm
bin\sim8086 --sim %1