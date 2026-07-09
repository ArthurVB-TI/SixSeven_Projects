@echo off
REM ============================================================
REM  Sobe o backend do ESP32 (backend_novo.exe, estatico).
REM  Le a config do config.json (mesma pasta). Porta 3000.
REM  ATENCAO: nao rodar junto com o backend principal (Drogon),
REM  pois os dois usam a porta 3000.
REM ============================================================
cd /d "%~dp0"
backend_novo.exe
