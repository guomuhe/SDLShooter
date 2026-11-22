#include "SceneMain.h"
#include "SceneTitle.h"
#include "SceneEnd.h"
#include "SceneHelp.h"
#include "Game.h"
#include <SDL.h>
#include <SDL_image.h>
#include "assets_embed.h"
#include <random>
#include <cmath>

// 辅助：遍历 list 对每个指针做 delete 并 clear
template <typename ListT>
static void deleteList(ListT& list)
{
    for (auto* p : list) delete p;
    list.clear();
}

void SceneMain::applyDifficulty(Difficulty d)
{
    // 先恢复模板的"普通"值，再按难度覆盖
    enemyTemplate.speed = 150;
    enemyTemplate.currentHealth = 2;
    enemyTemplate.coolDown = 2000;
    playerMaxHealth = 3;
    playerCoolDown = 300;
    enemySpawnInterval = 0.8f;
    itemDropChance = 0.5f;

    switch (d){
        case Difficulty::Easy:
            enemyTemplate.speed = 105;                    // 慢 30%
            enemyTemplate.currentHealth = 1;              // 脆
            enemyTemplate.coolDown = 2500;                // 少射击
            playerMaxHealth = 5;                          // 多血
            playerCoolDown = 400;                         // 射击稍慢（更容易打出去）
            enemySpawnInterval = 1.2f;                   // 生成慢
            itemDropChance = 0.6f;
            break;
        case Difficulty::Normal:
            break;                                       // 保持普通
        case Difficulty::Hard:
            enemyTemplate.speed = 210;                    // 快 40%
            enemyTemplate.currentHealth = 3;              // 厚血
            enemyTemplate.coolDown = 1200;                // 高频射击
            playerMaxHealth = 1;                          // 一击死
            playerCoolDown = 200;                         // 快射
            enemySpawnInterval = 0.5f;                    // 高频生成
            itemDropChance = 0.35f;
            break;
    }
    // 写回玩家初始值
    player.currentHealth = playerMaxHealth;
    player.maxHealth = playerMaxHealth;
    player.coolDown = playerCoolDown;
}

void SceneMain::init()
{
    // 音频
    bgm = Asset_LoadMusic("assets/music/03_Racing_Through_Asteroids_Loop.ogg");
    if (bgm) Mix_PlayMusic(bgm, -1); else SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to load music: %s", Mix_GetError());

    // UI / 字体
    uiHealth = Asset_LoadTexture(game.getRenderer(), "assets/image/Health UI Black.png");
    scoreFont = Asset_LoadFont("assets/font/VonwaonBitmap-12px.ttf", 24);

    // 音效
    sounds["player_shoot"]    = Asset_LoadWAV("assets/sound/laser_shoot4.wav");
    sounds["enemy_shoot"]     = Asset_LoadWAV("assets/sound/xs_laser.wav");
    sounds["player_explode"]  = Asset_LoadWAV("assets/sound/explosion1.wav");
    sounds["enemy_explode"]   = Asset_LoadWAV("assets/sound/explosion3.wav");
    sounds["hit"]             = Asset_LoadWAV("assets/sound/eff11.wav");
    sounds["get_item"]        = Asset_LoadWAV("assets/sound/eff5.wav");

    // 随机
    std::random_device rd;
    gen = std::mt19937(rd());
    dis = std::uniform_real_distribution<float>(0.0f, 1.0f);

    // 玩家
    player.texture = Asset_LoadTexture(game.getRenderer(), "assets/image/SpaceShip.png");
    if (!player.texture) SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to load player texture: %s", SDL_GetError());
    SDL_QueryTexture(player.texture, NULL, NULL, &player.width, &player.height);
    player.width  /= 5;
    player.height /= 5;
    player.position.x = game.getWindowWidth() / 2 - player.width / 2;
    player.position.y = game.getWindowHeight() - player.height;

    // 模板纹理
    projectilePlayerTemplate.texture = Asset_LoadTexture(game.getRenderer(), "assets/image/laser-1.png");
    SDL_QueryTexture(projectilePlayerTemplate.texture, NULL, NULL, &projectilePlayerTemplate.width, &projectilePlayerTemplate.height);
    projectilePlayerTemplate.width  /= 4;
    projectilePlayerTemplate.height /= 4;

    enemyTemplate.texture = Asset_LoadTexture(game.getRenderer(), "assets/image/insect-2.png");
    SDL_QueryTexture(enemyTemplate.texture, NULL, NULL, &enemyTemplate.width, &enemyTemplate.height);
    enemyTemplate.width  /= 4;
    enemyTemplate.height /= 4;

    projectileEnemyTemplate.texture = Asset_LoadTexture(game.getRenderer(), "assets/image/bullet-1.png");
    SDL_QueryTexture(projectileEnemyTemplate.texture, NULL, NULL, &projectileEnemyTemplate.width, &projectileEnemyTemplate.height);
    projectileEnemyTemplate.width  /= 2;
    projectileEnemyTemplate.height /= 2;

    explosionTemplate.texture = Asset_LoadTexture(game.getRenderer(), "assets/effect/explosion.png");
    SDL_QueryTexture(explosionTemplate.texture, NULL, NULL, &explosionTemplate.width, &explosionTemplate.height);
    explosionTemplate.totalFrame = explosionTemplate.width / explosionTemplate.height;
    explosionTemplate.height *= 2;
    explosionTemplate.width  = explosionTemplate.height;

    itemLifeTemplate.texture = Asset_LoadTexture(game.getRenderer(), "assets/image/bonus_life.png");
    SDL_QueryTexture(itemLifeTemplate.texture, NULL, NULL, &itemLifeTemplate.width, &itemLifeTemplate.height);
    itemLifeTemplate.width  /= 4;
    itemLifeTemplate.height /= 4;

    // 应用难度系数（必须在 enemyTemplate 就绪之后）
    applyDifficulty(game.getDifficulty());
}

void SceneMain::clean()
{
    for (auto& kv : sounds) if (kv.second) Mix_FreeChunk(kv.second);
    sounds.clear();

    deleteList(projectilesPlayer);
    deleteList(enemies);
    deleteList(projectilesEnemy);
    deleteList(explosions);
    deleteList(items);

    SDL_DestroyTexture(uiHealth);          uiHealth = nullptr;
    TTF_CloseFont(scoreFont);              scoreFont = nullptr;
    SDL_DestroyTexture(scoreTexture);      scoreTexture = nullptr;

    SDL_DestroyTexture(player.texture);                  player.texture = nullptr;
    SDL_DestroyTexture(projectilePlayerTemplate.texture);   projectilePlayerTemplate.texture = nullptr;
    SDL_DestroyTexture(enemyTemplate.texture);              enemyTemplate.texture = nullptr;
    SDL_DestroyTexture(projectileEnemyTemplate.texture);    projectileEnemyTemplate.texture = nullptr;
    SDL_DestroyTexture(explosionTemplate.texture);          explosionTemplate.texture = nullptr;
    SDL_DestroyTexture(itemLifeTemplate.texture);           itemLifeTemplate.texture = nullptr;

    if (bgm){ Mix_HaltMusic(); Mix_FreeMusic(bgm); bgm = nullptr; }
}

// ---------- update ----------

void SceneMain::update(float deltaTime)
{
    now = SDL_GetTicks();  // 本帧统一时间戳

    keyboardControl(deltaTime);
    updatePlayerProjectiles(deltaTime);
    updateEnemyProjectiles(deltaTime);
    spawnTimer += deltaTime;
    if (spawnTimer >= enemySpawnInterval){
        spawnTimer = 0.0f;
        spawnEnemy();
    }
    updateEnemies(deltaTime);
    updatePlayer(deltaTime);
    updateExplosions(deltaTime);
    updateItems(deltaTime);
    if (isDead){
        changeSceneDelayed(deltaTime, 3);
    }
}

void SceneMain::keyboardControl(float deltaTime)
{
    if (isDead) return;
    const Uint8* key = SDL_GetKeyboardState(nullptr);
    if (key[SDL_SCANCODE_W]) player.position.y -= deltaTime * player.speed;
    if (key[SDL_SCANCODE_S]) player.position.y += deltaTime * player.speed;
    if (key[SDL_SCANCODE_A]) player.position.x -= deltaTime * player.speed;
    if (key[SDL_SCANCODE_D]) player.position.x += deltaTime * player.speed;

    // 钳制边界
    if (player.position.x < 0) player.position.x = 0;
    if (player.position.x > game.getWindowWidth()  - player.width)  player.position.x = game.getWindowWidth()  - player.width;
    if (player.position.y < 0) player.position.y = 0;
    if (player.position.y > game.getWindowHeight() - player.height) player.position.y = game.getWindowHeight() - player.height;

    // 极速射击作弊：跳过冷却检测
    if (key[SDL_SCANCODE_J] && (cheatRapidFire || now - player.lastShootTime > player.coolDown)){
        shootPlayer();
        player.lastShootTime = now;
    }
}

void SceneMain::shootPlayer()
{
    auto* p = new ProjectilePlayer(projectilePlayerTemplate);
    p->position.x = player.position.x + player.width / 2 - p->width / 2;
    p->position.y = player.position.y;
    projectilesPlayer.push_back(p);
    Mix_PlayChannel(0, sounds["player_shoot"], 0);
}

void SceneMain::shootEnemy(Enemy* enemy)
{
    auto* p = new ProjectileEnemy(projectileEnemyTemplate);
    p->position.x = enemy->position.x + enemy->width  / 2 - p->width  / 2;
    p->position.y = enemy->position.y + enemy->height / 2 - p->height / 2;
    p->direction = getDirection(enemy);
    projectilesEnemy.push_back(p);
    Mix_PlayChannel(-1, sounds["enemy_shoot"], 0);
}

SDL_FPoint SceneMain::getDirection(Enemy* enemy)
{
    float dx = (player.position.x + player.width  / 2) - (enemy->position.x + enemy->width  / 2);
    float dy = (player.position.y + player.height / 2) - (enemy->position.y + enemy->height / 2);
    float len = std::sqrt(dx*dx + dy*dy);
    if (len < 0.0001f) return {0.0f, 1.0f};  // 避免除零
    return {dx/len, dy/len};
}

void SceneMain::spawnEnemy()
{
    // 简单防顶：敌人过多时跳过
    if (enemies.size() >= 30) return;

    auto* e = new Enemy(enemyTemplate);
    e->position.x = dis(gen) * (game.getWindowWidth() - e->width);
    e->position.y = -e->height;
    e->lastShootTime = now;
    enemies.push_back(e);
}

void SceneMain::updateEnemies(float deltaTime)
{
    for (auto it = enemies.begin(); it != enemies.end();){
        Enemy* enemy = *it;
        // 冻结敌机作弊：敌机不移动、不开火
        if (!cheatFreezeEnemy){
            enemy->position.y += enemy->speed * deltaTime;
        }
        if (enemy->position.y > game.getWindowHeight()){
            delete enemy;
            it = enemies.erase(it);
        } else {
            if (!cheatFreezeEnemy && now - enemy->lastShootTime > enemy->coolDown && !isDead){
                shootEnemy(enemy);
                enemy->lastShootTime = now;
            }
            if (enemy->currentHealth <= 0){
                enemyExplode(enemy);
                it = enemies.erase(it);
            } else {
                ++it;
            }
        }
    }
}

void SceneMain::updatePlayerProjectiles(float deltaTime)
{
    const int margin = 32;
    for (auto it = projectilesPlayer.begin(); it != projectilesPlayer.end();){
        ProjectilePlayer* pr = *it;
        pr->position.y -= pr->speed * deltaTime;
        if (pr->position.y + margin < 0){
            delete pr;
            it = projectilesPlayer.erase(it);
        } else {
            bool hit = false;
            for (auto* enemy : enemies){
                SDL_Rect er = {static_cast<int>(enemy->position.x), static_cast<int>(enemy->position.y), enemy->width, enemy->height};
                SDL_Rect prr = {static_cast<int>(pr->position.x), static_cast<int>(pr->position.y), pr->width, pr->height};
                if (SDL_HasIntersection(&er, &prr)){
                    // 一击必杀作弊：直接将敌机血量清零
                    enemy->currentHealth = cheatOneHitKill ? 0 : enemy->currentHealth - pr->damage;
                    delete pr;
                    it = projectilesPlayer.erase(it);
                    hit = true;
                    Mix_PlayChannel(-1, sounds["hit"], 0);
                    break;
                }
            }
            if (!hit) ++it;
        }
    }
}

void SceneMain::updateEnemyProjectiles(float deltaTime)
{
    const int margin = 32;
    // 慢速敌弹作弊：子弹速度降为 10%
    const float speedMul = cheatSlowBullets ? 0.1f : 1.0f;
    SDL_Rect pr, pl;
    for (auto it = projectilesEnemy.begin(); it != projectilesEnemy.end();){
        ProjectileEnemy* prj = *it;
        prj->position.x += prj->speed * prj->direction.x * deltaTime * speedMul;
        prj->position.y += prj->speed * prj->direction.y * deltaTime * speedMul;
        if (prj->position.y > game.getWindowHeight() + margin ||
            prj->position.y < -margin ||
            prj->position.x < -margin ||
            prj->position.x > game.getWindowWidth() + margin){
            delete prj;
            it = projectilesEnemy.erase(it);
        } else {
            pr = {static_cast<int>(prj->position.x), static_cast<int>(prj->position.y), prj->width, prj->height};
            pl = {static_cast<int>(player.position.x), static_cast<int>(player.position.y), player.width, player.height};
            if (SDL_HasIntersection(&pr, &pl) && !isDead){
                // 无敌模式作弊：免疫敌机子弹伤害
                if (!cheatGodMode){
                    player.currentHealth -= prj->damage;
                }
                delete prj;
                it = projectilesEnemy.erase(it);
                Mix_PlayChannel(-1, sounds["hit"], 0);
            } else {
                ++it;
            }
        }
    }
}

void SceneMain::updatePlayer(float /*deltaTime*/)
{
    if (isDead) return;
    if (player.currentHealth <= 0){
        isDead = true;
        auto* ex = new Explosion(explosionTemplate);
        ex->position.x = player.position.x + player.width  / 2 - ex->width  / 2;
        ex->position.y = player.position.y + player.height / 2 - ex->height / 2;
        ex->startTime = now;
        explosions.push_back(ex);
        Mix_PlayChannel(-1, sounds["player_explode"], 0);
        game.setFinalScore(score);
        return;
    }
    // 碰撞后 break，避免一次撞到多个敌机多次扣血
    for (auto* enemy : enemies){
        SDL_Rect er = {static_cast<int>(enemy->position.x), static_cast<int>(enemy->position.y), enemy->width, enemy->height};
        SDL_Rect pl = {static_cast<int>(player.position.x), static_cast<int>(player.position.y), player.width, player.height};
        if (SDL_HasIntersection(&pl, &er)){
            // 无敌模式作弊：免疫撞击伤害，但敌机仍被撞毁
            if (!cheatGodMode){
                player.currentHealth -= 1;
            }
            enemy->currentHealth = 0;
            break;  // 修：一次只扣一次血
        }
    }
}

void SceneMain::updateExplosions(float /*deltaTime*/)
{
    for (auto it = explosions.begin(); it != explosions.end();){
        Explosion* ex = *it;
        ex->currentFrame = (now - ex->startTime) * ex->FPS / 1000;
        if (ex->currentFrame >= ex->totalFrame){
            delete ex;
            it = explosions.erase(it);
        } else {
            ++it;
        }
    }
}

void SceneMain::updateItems(float deltaTime)
{
    for (auto it = items.begin(); it != items.end();){
        Item* item = *it;
        item->position.x += item->direction.x * item->speed * deltaTime;
        item->position.y += item->direction.y * item->speed * deltaTime;

        // 边缘反弹
        if (item->bounceCount > 0 && item->position.x < 0){
            item->direction.x = -item->direction.x;
            item->bounceCount--;
        }
        if (item->bounceCount > 0 && item->position.x + item->width > game.getWindowWidth()){
            item->direction.x = -item->direction.x;
            item->bounceCount--;
        }
        if (item->bounceCount > 0 && item->position.y < 0){
            item->direction.y = -item->direction.y;
            item->bounceCount--;
        }
        if (item->bounceCount > 0 && item->position.y + item->height > game.getWindowHeight()){
            item->direction.y = -item->direction.y;
            item->bounceCount--;
        }

        if (item->position.x + item->width < 0 || item->position.x > game.getWindowWidth() ||
            item->position.y + item->height < 0 || item->position.y > game.getWindowHeight()){
            delete item;
            it = items.erase(it);
        } else {
            SDL_Rect ir = {static_cast<int>(item->position.x), static_cast<int>(item->position.y), item->width, item->height};
            SDL_Rect pl = {static_cast<int>(player.position.x), static_cast<int>(player.position.y), player.width, player.height};
            if (SDL_HasIntersection(&ir, &pl) && !isDead){
                playerGetItem(item);
                delete item;
                it = items.erase(it);
            } else {
                ++it;
            }
        }
    }
}

void SceneMain::playerGetItem(Item* item)
{
    score += 5;
    if (item->type == ItemType::Life && player.currentHealth < player.maxHealth){
        player.currentHealth++;
    }
    Mix_PlayChannel(-1, sounds["get_item"], 0);
}

void SceneMain::enemyExplode(Enemy* enemy)
{
    auto* ex = new Explosion(explosionTemplate);
    ex->position.x = enemy->position.x + enemy->width  / 2 - ex->width  / 2;
    ex->position.y = enemy->position.y + enemy->height / 2 - ex->height / 2;
    ex->startTime = now;
    explosions.push_back(ex);
    Mix_PlayChannel(-1, sounds["enemy_explode"], 0);

    if (dis(gen) < itemDropChance){
        dropItem(enemy);
    }
    score += 10;
}

void SceneMain::dropItem(Enemy* enemy)
{
    auto* item = new Item(itemLifeTemplate);
    item->position.x = enemy->position.x + enemy->width  / 2 - item->width  / 2;
    item->position.y = enemy->position.y + enemy->height / 2 - item->height / 2;
    float angle = dis(gen) * 2.0f * 3.1415926535f;  // M_PI 跨平台不稳，直接写常数
    item->direction.x = std::cos(angle);
    item->direction.y = std::sin(angle);
    items.push_back(item);
}

void SceneMain::changeSceneDelayed(float deltaTime, float delay)
{
    timerEnd += deltaTime;
    if (timerEnd > delay){
        game.changeScene(new SceneEnd());
    }
}

// ---------- render ----------

void SceneMain::render()
{
    renderPlayerProjectiles();
    renderEnemyProjectiles();
    if (!isDead){
        SDL_Rect pr = {static_cast<int>(player.position.x), static_cast<int>(player.position.y), player.width, player.height};
        SDL_RenderCopy(game.getRenderer(), player.texture, nullptr, &pr);
    }
    renderEnemies();
    renderItems();
    renderExplosions();
    renderUI();
    renderCheatStatus();
}

void SceneMain::renderPlayerProjectiles()
{
    for (auto* pr : projectilesPlayer){
        SDL_Rect r = {static_cast<int>(pr->position.x), static_cast<int>(pr->position.y), pr->width, pr->height};
        SDL_RenderCopy(game.getRenderer(), pr->texture, nullptr, &r);
    }
}

void SceneMain::renderEnemyProjectiles()
{
    constexpr float RAD_TO_DEG = 180.0f / 3.1415926535f;
    for (auto* pr : projectilesEnemy){
        SDL_Rect r = {static_cast<int>(pr->position.x), static_cast<int>(pr->position.y), pr->width, pr->height};
        float angle = std::atan2(pr->direction.y, pr->direction.x) * RAD_TO_DEG - 90.0f;
        SDL_RenderCopyEx(game.getRenderer(), pr->texture, nullptr, &r, angle, nullptr, SDL_FLIP_NONE);
    }
}

void SceneMain::renderEnemies()
{
    for (auto* e : enemies){
        SDL_Rect r = {static_cast<int>(e->position.x), static_cast<int>(e->position.y), e->width, e->height};
        SDL_RenderCopy(game.getRenderer(), e->texture, nullptr, &r);
    }
}

void SceneMain::renderItems()
{
    for (auto* it : items){
        SDL_Rect r = {static_cast<int>(it->position.x), static_cast<int>(it->position.y), it->width, it->height};
        SDL_RenderCopy(game.getRenderer(), it->texture, nullptr, &r);
    }
}

void SceneMain::renderExplosions()
{
    for (auto* ex : explosions){
        SDL_Rect src = {ex->currentFrame * ex->width / 2, 0, ex->width / 2, ex->height / 2};
        SDL_Rect dst = {static_cast<int>(ex->position.x), static_cast<int>(ex->position.y), ex->width, ex->height};
        SDL_RenderCopy(game.getRenderer(), ex->texture, &src, &dst);
    }
}

// 性能优化：score 纹理只在分数变化时重建
void SceneMain::renderUI()
{
    // 血条
    constexpr int x = 10, y = 10, size = 32, offset = 40;
    SDL_SetTextureColorMod(uiHealth, 100, 100, 100);
    for (int i = 0; i < player.maxHealth; i++){
        SDL_Rect r = {x + i * offset, y, size, size};
        SDL_RenderCopy(game.getRenderer(), uiHealth, nullptr, &r);
    }
    SDL_SetTextureColorMod(uiHealth, 255, 255, 255);
    for (int i = 0; i < player.currentHealth; i++){
        SDL_Rect r = {x + i * offset, y, size, size};
        SDL_RenderCopy(game.getRenderer(), uiHealth, nullptr, &r);
    }

    // 分数（只在内容变化时重建纹理，避免每帧创建/销毁纹理）
    std::string text = "SCORE:" + std::to_string(score);
    SDL_Rect scoreRect;
    if (text != scoreCacheText){
        scoreCacheText = text;
        SDL_DestroyTexture(scoreTexture);
        scoreTexture = nullptr;
        SDL_Surface* surface = TTF_RenderUTF8_Solid(scoreFont, text.c_str(), {255, 255, 255, 255});
        if (surface){
            scoreTexture = SDL_CreateTextureFromSurface(game.getRenderer(), surface);
            scoreRect = {game.getWindowWidth() - 10 - surface->w, 10, surface->w, surface->h};
            SDL_RenderCopy(game.getRenderer(), scoreTexture, nullptr, &scoreRect);
            SDL_FreeSurface(surface);
        }
    } else if (scoreTexture){
        int tw, th;
        SDL_QueryTexture(scoreTexture, nullptr, nullptr, &tw, &th);
        scoreRect = {game.getWindowWidth() - 10 - tw, 10, tw, th};
        SDL_RenderCopy(game.getRenderer(), scoreTexture, nullptr, &scoreRect);
    }
}

// 作弊器状态显示：仅渲染当前激活的作弊项
void SceneMain::renderCheatStatus()
{
    struct CheatEntry { bool active; const char* label; };
    CheatEntry entries[] = {
        {cheatGodMode,     "F1 无敌"},
        {cheatOneHitKill,  "F2 一击必杀"},
        {cheatRapidFire,   "F3 极速射击"},
        {cheatFreezeEnemy, "F5 冻结敌机"},
        {cheatSlowBullets, "F6 慢速敌弹"},
    };

    SDL_Renderer* renderer = game.getRenderer();
    int posY = 50;  // 血条下方
    constexpr int lineHeight = 22;

    // 标题
    {
        SDL_Color yellow = {255, 220, 0, 255};
        SDL_Surface* s = TTF_RenderUTF8_Solid(scoreFont, "[作弊器]", yellow);
        if (s){
            SDL_Texture* t = SDL_CreateTextureFromSurface(renderer, s);
            SDL_Rect r = {10, posY, s->w, s->h};
            SDL_RenderCopy(renderer, t, nullptr, &r);
            SDL_DestroyTexture(t);
            SDL_FreeSurface(s);
            posY += lineHeight + 4;
        }
    }

    // 各项作弊状态
    for (const auto& e : entries){
        if (!e.active) continue;
        SDL_Color green = {0, 255, 100, 255};
        std::string line = std::string("  ON  ") + e.label;
        SDL_Surface* s = TTF_RenderUTF8_Solid(scoreFont, line.c_str(), green);
        if (s){
            SDL_Texture* t = SDL_CreateTextureFromSurface(renderer, s);
            SDL_Rect r = {10, posY, s->w, s->h};
            SDL_RenderCopy(renderer, t, nullptr, &r);
            SDL_DestroyTexture(t);
            SDL_FreeSurface(s);
            posY += lineHeight;
        }
    }

    // 加分提示（F7 是即时型，无开关状态，单独提示）
    {
        SDL_Color gray = {180, 180, 180, 255};
        SDL_Surface* s = TTF_RenderUTF8_Solid(scoreFont, "F7 +1000分", gray);
        if (s){
            SDL_Texture* t = SDL_CreateTextureFromSurface(renderer, s);
            SDL_Rect r = {10, posY + 4, s->w, s->h};
            SDL_RenderCopy(renderer, t, nullptr, &r);
            SDL_DestroyTexture(t);
            SDL_FreeSurface(s);
        }
    }
}

// ---------- input ----------

void SceneMain::handleEvent(SDL_Event* event)
{
    if (event->type == SDL_KEYDOWN && event->key.repeat == 0){
        switch (event->key.keysym.scancode){
            case SDL_SCANCODE_ESCAPE:
                game.changeScene(new SceneTitle());
                break;
            case SDL_SCANCODE_R:
                game.changeScene(new SceneHelp(HelpOrigin::Main));
                break;
            // ===== 作弊器热键 =====
            case SDL_SCANCODE_F1:
                cheatGodMode = !cheatGodMode;
                break;
            case SDL_SCANCODE_F2:
                cheatOneHitKill = !cheatOneHitKill;
                break;
            case SDL_SCANCODE_F3:
                cheatRapidFire = !cheatRapidFire;
                break;
            case SDL_SCANCODE_F5:
                cheatFreezeEnemy = !cheatFreezeEnemy;
                break;
            case SDL_SCANCODE_F6:
                cheatSlowBullets = !cheatSlowBullets;
                break;
            case SDL_SCANCODE_F7:
                score += 1000;
                break;
            default:
                break;
        }
    }
}
