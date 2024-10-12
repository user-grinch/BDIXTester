@echo off
premake5.exe vs2022
call "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat"
call MsBuild "BDIXTester.sln"
