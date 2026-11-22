#ifndef SCENE_MAIN_H
#define SCENE_MAIN_H

#include "Scene.h"
#include "Object.h"
#include <list>
#include <random>
#include <map>
#include <SDL.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>


class Game;

class SceneMain : public Scene {
public:
    ~SceneMain() override = default;

    void update(float deltaTime) override;
    void render() override;
    void handleEvent(SDL_Event* event) override;
    void init() override;
    void clean() override;

private:
    Player player;
    Mix_Music* bgm = nullptr;
    SDL_Texture* uiHealth = nullptr;
    TTF_Font* scoreFont = nullptr;

    // 分数 UI 缓存（性能优化：只在分数变化时重建纹理）
    SDL_Texture* scoreTexture = nullptr;
    std::string  scoreCacheText;

    int score = 0;
    float timerEnd = 0.0f;

    bool isDead = false;

    // ===== 作弊器状态 =====
    bool cheatGodMode      = false;  // F1 无敌模式：玩家免疫一切伤害
    bool cheatOneHitKill   = false;  // F2 一击必杀：任意命中即击毁敌机
    bool cheatRapidFire    = false;  // F3 极速射击：玩家射击无冷却
    bool cheatFreezeEnemy  = false;  // F5 冻结敌机：敌机不移动不开火
    bool cheatSlowBullets  = false;  // F6 慢速敌弹：敌机子弹速度降为 10%

    // 随机数
    std::mt19937 gen;
    std::uniform_real_distribution<float> dis;

    // 模板（init 里用难度系数覆盖）
    Enemy enemyTemplate;
    ProjectilePlayer projectilePlayerTemplate;
    ProjectileEnemy projectileEnemyTemplate;
    Explosion explosionTemplate;
    Item itemLifeTemplate;

    // 物体容器
    std::list<Enemy*> enemies;
    std::list<ProjectilePlayer*> projectilesPlayer;
    std::list<ProjectileEnemy*> projectilesEnemy;
    std::list<Explosion*> explosions;
    std::list<Item*> items;
    std::map<std::string, Mix_Chunk*> sounds;

    // 难度参数
    float enemySpawnInterval = 0.8f;  // 敌机生成间隔（秒）
    float spawnTimer = 0.0f;          // 生成计时器
    float itemDropChance = 0.5f;      // 道具掉落概率
    int   playerMaxHealth = 3;
    Uint32 playerCoolDown = 300;

    // 每帧缓存的 SDL_GetTicks()，避免多处重复调用
    Uint32 now = 0;

    // 渲染
    void renderItems();
    void renderUI();
    void renderCheatStatus();
    void renderExplosions();
    void renderPlayerProjectiles();
    void renderEnemyProjectiles();
    void renderEnemies();

    // 更新
    void updateEnemies(float deltaTime);
    void updateEnemyProjectiles(float deltaTime);
    void updatePlayer(float deltaTime);
    void updateItems(float deltaTime);
    void updateExplosions(float deltaTime);
    void updatePlayerProjectiles(float deltaTime);
    void keyboardControl(float deltaTime);
    void spawnEnemy();
    void changeSceneDelayed(float deltaTime, float delay);

    // 其它
    void playerGetItem(Item* item);
    void shootPlayer();
    void shootEnemy(Enemy* enemy);
    SDL_FPoint getDirection(Enemy* enemy);
    void enemyExplode(Enemy* enemy);
    void dropItem(Enemy* enemy);

    // 难度系数应用（在 init 里调用一次）
    void applyDifficulty(Difficulty d);
};


#endif // SCENE_MAIN_H
