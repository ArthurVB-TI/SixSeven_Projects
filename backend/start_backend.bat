@echo off
REM ============================================================
REM  Sobe o backend principal (Drogon) compilado em build\.
REM  Requer o MSYS2 em C:\Users\verso\msys64 (DLLs do MinGW64).
REM  Se mudar o MSYS2 de lugar, ajuste o PATH abaixo.
REM ============================================================
set PATH=C:\Users\verso\msys64\mingw64\bin;%PATH%
cd /d "%~dp0build"
sixseven_backend.exe
