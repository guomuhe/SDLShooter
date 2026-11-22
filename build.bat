@echo off
setlocal
set "MINGW=C:\Program Files (x86)\Dev-Cpp\MinGW64\bin"
set "PATH=%MINGW%;%PATH%"

if not exist obj mkdir obj

rem compile resource file (windres finds gcc via PATH, avoiding space-in-path bug)
windres resources.rc -O coff -o obj\resources.o || goto :fail

rem compile sources
g++ -std=c++17 -Wall -Wextra -Wpedantic -I deps\include\SDL2 -I src -c src\main.cpp -o obj\main.o || goto :fail
g++ -std=c++17 -Wall -Wextra -Wpedantic -I deps\include\SDL2 -I src -c src\Game.cpp -o obj\Game.o || goto :fail
g++ -std=c++17 -Wall -Wextra -Wpedantic -I deps\include\SDL2 -I src -c src\Scene.cpp -o obj\Scene.o || goto :fail
g++ -std=c++17 -Wall -Wextra -Wpedantic -I deps\include\SDL2 -I src -c src\SceneTitle.cpp -o obj\SceneTitle.o || goto :fail
g++ -std=c++17 -Wall -Wextra -Wpedantic -I deps\include\SDL2 -I src -c src\SceneMain.cpp -o obj\SceneMain.o || goto :fail
g++ -std=c++17 -Wall -Wextra -Wpedantic -I deps\include\SDL2 -I src -c src\SceneEnd.cpp -o obj\SceneEnd.o || goto :fail
g++ -std=c++17 -Wall -Wextra -Wpedantic -I deps\include\SDL2 -I src -c src\SceneHelp.cpp -o obj\SceneHelp.o || goto :fail

rem link
g++ obj\main.o obj\Game.o obj\Scene.o obj\SceneTitle.o obj\SceneMain.o obj\SceneEnd.o obj\SceneHelp.o obj\resources.o -L deps\lib -lmingw32 -lSDL2main -lSDL2 -lSDL2_image -lSDL2_mixer -lSDL2_ttf -limm32 -mwindows -o SDLShooter-Windows.exe || goto :fail

rem copy DLLs next to the exe
copy /Y deps\bin\SDL2.dll . >nul
copy /Y deps\bin\SDL2_image.dll . >nul
copy /Y deps\bin\SDL2_mixer.dll . >nul
copy /Y deps\bin\SDL2_ttf.dll . >nul
echo Build OK: SDLShooter-Windows.exe
goto :eof

:fail
echo Build FAILED
endlocal
exit /b 1
