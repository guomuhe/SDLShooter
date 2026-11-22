#include "SceneTitle.h"
#include "SceneMain.h"
#include "SceneHelp.h"
#include "Game.h"
#include "assets_embed.h"
#include <string>

static const char* diffName(Difficulty d)
{
    switch (d){
        case Difficulty::Easy:   return "简单";
        case Difficulty::Normal: return "普通";
        case Difficulty::Hard:   return "困难";
    }
    return "";
}

static Difficulty nextDiff(Difficulty d)
{
    switch (d){
        case Difficulty::Easy:   return Difficulty::Normal;
        case Difficulty::Normal: return Difficulty::Hard;
        case Difficulty::Hard:   return Difficulty::Easy;
    }
    return d;
}

static Difficulty prevDiff(Difficulty d)
{
    switch (d){
        case Difficulty::Easy:   return Difficulty::Hard;
        case Difficulty::Normal: return Difficulty::Easy;
        case Difficulty::Hard:   return Difficulty::Normal;
    }
    return d;
}

void SceneTitle::init()
{
    bgm = Asset_LoadMusic("assets/music/06_Battle_in_Space_Intro.ogg");
    if (bgm == nullptr){
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to load music: %s", Mix_GetError());
    }
    Mix_PlayMusic(bgm, -1);
}

void SceneTitle::update(float deltaTime)
{
    timer += deltaTime;
    if (timer > 1.0f){
        timer -= 1.0f;
    }
}

void SceneTitle::render()
{
    Difficulty d = game.getDifficulty();

    // 标题
    game.renderTextCentered("SDL太空战机", 0.32, true);

    // 难度选择
    game.renderTextCentered("难度", 0.48, false);
    // 上一个（上箭头提示）
    game.renderTextCentered("[  " + std::string(diffName(prevDiff(d))) + "  ]", 0.54, false);
    // 当前高亮（闪烁）
    game.renderTextCentered("> " + std::string(diffName(d)) + " <", 0.60, true);
    // 下一个（下箭头提示）
    game.renderTextCentered("[  " + std::string(diffName(nextDiff(d))) + "  ]", 0.66, false);

    // 底部提示（闪烁）
    if (timer < 0.5f){
        game.renderTextCentered("按 J 键开始游戏", 0.82, false);
        game.renderTextCentered("按 R 查看帮助   上下键切换难度", 0.88, false);
    }
}

void SceneTitle::clean()
{
    if (bgm != nullptr){
        Mix_HaltMusic();
        Mix_FreeMusic(bgm);
    }
}

void SceneTitle::handleEvent(SDL_Event *event)
{
    if (event->type == SDL_KEYDOWN){
        switch (event->key.keysym.scancode){
            case SDL_SCANCODE_UP:
            case SDL_SCANCODE_W:
                game.setDifficulty(prevDiff(game.getDifficulty()));
                break;
            case SDL_SCANCODE_DOWN:
            case SDL_SCANCODE_S:
                game.setDifficulty(nextDiff(game.getDifficulty()));
                break;
            case SDL_SCANCODE_J:
                game.changeScene(new SceneMain());
                break;
            case SDL_SCANCODE_R:
                game.changeScene(new SceneHelp(HelpOrigin::Title));
                break;
            default: break;
        }
    }
}
