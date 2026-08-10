@echo off
setlocal
set "PROJECT_ROOT=%~dp0.."
if not exist "%PROJECT_ROOT%\.toolchain\python-site" (
    mkdir "%PROJECT_ROOT%\.toolchain\python-site"
)
python -m pip install --upgrade --target "%PROJECT_ROOT%\.toolchain\python-site" ^
    -r "%PROJECT_ROOT%\requirements-build.txt"
