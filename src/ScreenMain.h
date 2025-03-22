#pragma once

#include <vector>
#include <memory>
#include "render/petit2d.h"
#include "DemoContext.h"
#include "Screen.h"
#include "TextLine.h"

class ScreenMain : public Screen<DemoContext>
{

private:
    struct Star
    {
        glm::vec2 position;
        float speed;
    };

    float time;
    int step;
    float borderOffset;
    float titleWobble;

    Petit2D::Sprite::Sprite title1;
    Petit2D::Sprite::Sprite title2;
    Petit2D::Sprite::Sprite logo;
    Petit2D::Sprite::Sprite border;
    Petit2D::Sprite::Sprite borderBackground;
    Petit2D::Sprite::Sprite star;
    Petit2D::Sprite::Sprite raster;
    std::vector<Star> stars;
    std::vector<glm::vec4> rasters;
    std::vector<std::unique_ptr<TextLine>> textLines;
    float rasterOffset;
    unsigned int textLineOffset;

public:
    ScreenMain();
    virtual ~ScreenMain();

    virtual void enter(DemoContext& context) override;
    virtual void exit(DemoContext& context)override;
    virtual bool render(DemoContext& context, float dt) override;

private:
    bool update(DemoContext& context, float dt);
    
};
