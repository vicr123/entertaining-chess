if "%APPVEYOR_REPO_TAG_NAME%"=="continuous" (

    exit 1

)

git submodule init
git submodule update

set QTDIR=C:\Qt\5.15\msvc2019_64
set PATH=%PATH%;%QTDIR%\bin
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat"


git clone https://github.com/vicr123/contemporary-icons.git

rem Remove symlinks from contemporary-icons
cp appveyor\delink.ps1 contemporary-icons
cd contemporary-icons
powershell -executionpolicy bypass -File delink.ps1
cd ..

git clone https://github.com/discordapp/discord-rpc.git
cd discord-rpc
mkdir build
cd build
cmake -G"NMake Makefiles" .. -DBUILD_EXAMPLES=OFF -DCMAKE_POSITION_INDEPENDENT_CODE=ON -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
cmake --build . --config Release --target install
cd ..
cd ..

git clone https://github.com/vicr123/the-libs.git
cd the-libs
git checkout blueprint
qmake the-libs.pro "CONFIG+=release"
nmake release
nmake install
cd ..

git clone https://github.com/vicr123/libentertaining.git
cd libentertaining
qmake libentertaining.pro "CONFIG+=release"
nmake release
nmake install
cd ..

git clone https://github.com/vicr123/contemporary-theme.git
cd contemporary-theme
qmake Contemporary.pro "CONFIG+=release"
nmake release
cd ..

qmake entertaining-chess.pro "CONFIG+=release"
nmake release
mkdir deploy
mkdir deploy\styles
mkdir deploy\translations
mkdir deploy\audio
mkdir deploy\icons
copy "contemporary-theme\release\Contemporary.dll" deploy\styles
copy release\entertaining-chess.exe deploy
copy translations\*.qm deploy\translations
copy assets\audio\* deploy\audio
copy "C:\Program Files\thelibs\lib\the-libs.dll" deploy
copy "C:\Program Files\libentertaining\lib\entertaining.dll" deploy
copy "C:\OpenSSL-v111-Win64\bin\libssl-1_1-x64.dll" deploy
copy "C:\OpenSSL-v111-Win64\bin\libcrypto-1_1-x64.dll" deploy
copy defaults.conf deploy
robocopy contemporary-icons deploy\icons\ /mir
cd deploy
windeployqt entertaining-chess.exe -network -quickwidgets -gamepad -svg -multimedia -printsupport -concurrent
