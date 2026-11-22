#include <iostream>
#include <SDL.h>
#include <SDL_syswm.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>
#include "Game.h"

#ifdef _WIN32
#include <windows.h>
#include <imm.h>

static void switchToEnglishIME(SDL_Window* window)
{
    // 1. 关闭 IME（切成英文直输）
    HWND hwnd = nullptr;
    SDL_SysWMinfo info;
    SDL_VERSION(&info.version);
    if (SDL_GetWindowWMInfo(window, &info)){
        hwnd = info.info.win.window;
    }else{
        hwnd = GetForegroundWindow();
    }
    if (hwnd){
        HIMC himc = ImmGetContext(hwnd);
        if (himc){
            ImmSetOpenStatus(himc, FALSE);
            ImmReleaseContext(hwnd, himc);
        }
    }
    // 2. 切换到英文键盘布局 (0x00000409 = en-US)
    ActivateKeyboardLayout((HKL)0x00000409, KLF_SETFORPROCESS);
}
#endif

int main(int, char**) {
    Game& game = Game::getInstance();
    game.init();
#ifdef _WIN32
    switchToEnglishIME(game.getWindow());
#endif
    game.run();
    
    return 0;
}