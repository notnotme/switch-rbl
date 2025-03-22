#pragma once

#include "render/petit2d.h"
#include "Screen.h"
#include "DemoContext.h"

class ScreenIntro : public Screen<DemoContext>
{

private:
    Petit2D::Sprite::Sprite logo1;
    Petit2D::Sprite::Sprite logo2;
    float time;
    int step;

public:
    ScreenIntro();
    virtual ~ScreenIntro();

    virtual void enter(DemoContext& context) override;
    virtual void exit(DemoContext& context)override;
    virtual bool render(DemoContext& context, float dt) override;

private:
    bool update(DemoContext& context, float dt);

};
