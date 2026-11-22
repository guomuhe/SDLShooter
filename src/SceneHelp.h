#ifndef SCENE_HELP_H
#define SCENE_HELP_H

#include "Scene.h"

enum class HelpOrigin {
    Title,
    Main
};

class SceneHelp : public Scene
{
public:
    SceneHelp(HelpOrigin origin = HelpOrigin::Title);

    virtual void init() override;
    virtual void update(float deltaTime) override;
    virtual void render() override;
    virtual void clean() override;
    virtual void handleEvent(SDL_Event* event) override;

private:
    HelpOrigin origin;
    float blinkTimer = 0.0f;
};

#endif // SCENE_HELP_H
