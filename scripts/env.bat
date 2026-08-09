@echo off
rem Activate the project-local toolchain (portable MSVC + CMake + Ninja).
rem Everything lives under .toolchain\ and is deleted together with the project.
call "%~dp0..\.toolchain\msvc\setup_x64.bat"
set "PATH=%~dp0..\.toolchain\cmake\bin;%~dp0..\.toolchain\ninja;%PATH%"
