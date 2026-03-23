@echo off
setlocal

:: Paths
set SVG_PATH=resources\themes\icons\appIcon.png
set ICO_PATH=resources\themes\icons\appIcon.ico

:: Check if ffmpeg is available
where ffmpeg >nul 2>nul
if %ERRORLEVEL% neq 0 (
    echo Error: ffmpeg is not installed or not in PATH.
    pause
    exit /b 1
)

echo Generating %ICO_PATH% from %SVG_PATH%...

:: Generate multi-resolution ICO file using ffmpeg
:: Including 16, 32, 48, 64, 128 for best Windows compatibility
ffmpeg -y -i "%SVG_PATH%" -filter_complex "[0:v]scale=16:16[s16];[0:v]scale=32:32[s32];[0:v]scale=48:48[s48];[0:v]scale=64:64[s64];[0:v]scale=128:128[s128]" -map "[s16]" -map "[s32]" -map "[s48]" -map "[s64]" -map "[s128]" "%ICO_PATH%"

if %ERRORLEVEL% equ 0 (
    echo Success! %ICO_PATH% has been updated.
) else (
    echo Error: Failed to generate icon.
)

pause
