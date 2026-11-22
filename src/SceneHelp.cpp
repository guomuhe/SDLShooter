#include "SceneHelp.h"
#include "SceneTitle.h"
#include "SceneMain.h"
#include "Game.h"
#include <string>

SceneHelp::SceneHelp(HelpOrigin origin)
    : origin(origin)
{
}

void SceneHelp::init()
{
    // 帮助页不播放音乐
}

void SceneHelp::update(float deltaTime)
{
    blinkTimer += deltaTime;
    if (blinkTimer > 1.0f){
        blinkTimer -= 1.0f;
    }
}

void SceneHelp::render()
{
    // 标题
    game.renderTextCentered("操作说明", 0.08, true);

    // 逐条居中渲染帮助文本
    game.renderTextCentered("W / A / S / D  - 移动飞船", 0.22, false);
    game.renderTextCentered("J              - 发射子弹",   0.30, false);
    game.renderTextCentered("F4             - 切换全屏",   0.38, false);
    game.renderTextCentered("ESC            - 返回标题",   0.46, false);
    game.renderTextCentered("R              - 打开/关闭帮助", 0.54, false);

    // 作弊器热键
    game.renderTextCentered("--- 作弊器 ---", 0.64, false);
    game.renderTextCentered("F1 无敌  F2 一击必杀  F3 极速射击", 0.72, false);
    game.renderTextCentered("F5 冻结敌机  F6 慢速敌弹  F7 +1000分", 0.80, false);

    // 底部闪烁提示
    if (blinkTimer < 0.5f){
        game.renderTextCentered("按 R 或 ESC 返回", 0.9, false);
    }
}

void SceneHelp::clean()
{
    // 帮助页没有需要清理的资源
}

void SceneHelp::handleEvent(SDL_Event *event)
{
    if (event->type == SDL_KEYDOWN){
        if (event->key.keysym.scancode == SDL_SCANCODE_R ||
            event->key.keysym.scancode == SDL_SCANCODE_ESCAPE){
            if (origin == HelpOrigin::Main){
                game.changeScene(new SceneMain());
            }else{
                game.changeScene(new SceneTitle());
            }
        }
    }
}
