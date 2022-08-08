@echo off
cls
echo Pending...
taskkill /f /im cloudmusic.exe>nul

ping localhost -n 2

del "C:\Program Files (x86)\Netease\CloudMusic\msimg32.dll"
copy I:\EasyCEFInject\Release\EasyCefInject.dll "C:\Program Files (x86)\Netease\CloudMusic\msimg32.dll">nul