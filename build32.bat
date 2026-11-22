@echo off
setlocal
REM 32 位静态构建脚本：使用 d:\code\mingw32 和 deps32（i686 SDL2 静态库）
REM 生成单文件 exe：SDL2 全家桶 + MinGW 运行时静态链接，assets 资源内嵌进 exe
set "MINGW=d:\code\mingw32\bin"
set "PATH=%MINGW%;%PATH%"

if not exist obj mkdir obj

REM 1) 把 assets/ 下的资源内嵌进 obj\assets_data.o 并生成 src\assets_embed.cpp
powershell -ExecutionPolicy Bypass -File .\embed_assets.ps1 || goto :fail

REM 2) 编译图标资源
windres resources.rc -O coff -o obj\resources.o || goto :fail

REM 3) 编译源码
g++ -std=c++17 -Wall -Wextra -Wpedantic -I deps32\include\SDL2 -I src -c src\main.cpp        -o obj\main.o        || goto :fail
g++ -std=c++17 -Wall -Wextra -Wpedantic -I deps32\include\SDL2 -I src -c src\Game.cpp        -o obj\Game.o        || goto :fail
g++ -std=c++17 -Wall -Wextra -Wpedantic -I deps32\include\SDL2 -I src -c src\Scene.cpp       -o obj\Scene.o       || goto :fail
g++ -std=c++17 -Wall -Wextra -Wpedantic -I deps32\include\SDL2 -I src -c src\SceneTitle.cpp  -o obj\SceneTitle.o  || goto :fail
g++ -std=c++17 -Wall -Wextra -Wpedantic -I deps32\include\SDL2 -I src -c src\SceneMain.cpp   -o obj\SceneMain.o   || goto :fail
g++ -std=c++17 -Wall -Wextra -Wpedantic -I deps32\include\SDL2 -I src -c src\SceneEnd.cpp    -o obj\SceneEnd.o    || goto :fail
g++ -std=c++17 -Wall -Wextra -Wpedantic -I deps32\include\SDL2 -I src -c src\SceneHelp.cpp   -o obj\SceneHelp.o   || goto :fail
g++ -std=c++17 -Wall -Wextra -Wpedantic -I deps32\include\SDL2 -I src -c src\AssetLoader.cpp -o obj\AssetLoader.o || goto :fail
g++ -std=c++17 -Wall -Wextra -Wpedantic -I deps32\include\SDL2 -I src -c src\assets_embed.cpp -o obj\assets_embed.o || goto :fail

REM 4) 完全静态链接（assets_data.o 含内嵌资源二进制）
g++ obj\main.o obj\Game.o obj\Scene.o obj\SceneTitle.o obj\SceneMain.o obj\SceneEnd.o obj\SceneHelp.o obj\AssetLoader.o obj\assets_embed.o obj\assets_data.o obj\resources.o ^
    -L deps32\lib ^
    -lmingw32 ^
    -l:libSDL2main.a ^
    -l:libSDL2_image.a -l:libSDL2_mixer.a -l:libSDL2_ttf.a -l:libSDL2.a ^
    -lwinmm -lusp10 -lgdi32 -lrpcrt4 -limm32 -lole32 -loleaut32 -lshell32 -lversion -ladvapi32 -lsetupapi ^
    -static-libgcc -static-libstdc++ -static ^
    -mwindows -o SDLShooter-Windows.exe || goto :fail

echo Build OK (32-bit static, assets embedded): SDLShooter-Windows.exe
echo 运行时无需外部 DLL，也无需 assets 目录（仅 save.dat 会在 exe 旁生成）
goto :eof

:fail
echo Build FAILED
endlocal
exit /b 1
