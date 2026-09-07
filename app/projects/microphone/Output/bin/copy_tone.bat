@echo off
cd /d %~dp0
set tone_dir=..\bin\res_tone
set res_dir=..\bin\res
set tone_zh_dir=%tone_dir%\zh
set tone_en_dir=%tone_dir%\en
set res_zh_dir=%res_dir%\zh
set res_en_dir=%res_dir%\en
if exist "%res_dir%" (
echo del /s %res_dir%\*.mp3
echo del /s %res_dir%\*.sbc
echo del /s %res_dir%\*.wav
del /s %res_dir%\*.mp3 > NUL
del /s %res_dir%\*.sbc > NUL
) else (
mkdir %res_dir%
mkdir %res_zh_dir%
mkdir %res_en_dir%
)
echo copy %tone_dir% file to %res_dir%
copy /y %tone_dir%\ring.mp3 %res_dir%\ring.mp3 > NUL
copy /y %tone_dir%\update.mp3 %res_dir%\update.mp3 > NUL
copy /y %tone_dir%\update_done.mp3 %res_dir%\update_done.mp3 > NUL
copy /y %tone_en_dir%\language.mp3 %res_en_dir%\language.mp3 > NUL
copy /y %tone_zh_dir%\language.mp3 %res_zh_dir%\language.mp3 > NUL
copy /y %tone_en_dir%\poweron.mp3 %res_en_dir%\poweron.mp3 > NUL
copy /y %tone_zh_dir%\poweron.mp3 %res_zh_dir%\poweron.mp3 > NUL
copy /y %tone_en_dir%\poweroff.mp3 %res_en_dir%\poweroff.mp3 > NUL
copy /y %tone_zh_dir%\poweroff.mp3 %res_zh_dir%\poweroff.mp3 > NUL
