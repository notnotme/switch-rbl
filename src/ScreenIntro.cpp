#include "ScreenIntro.h"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

ScreenIntro::ScreenIntro() :
Screen()
{
}

ScreenIntro::~ScreenIntro()
{
}


void ScreenIntro::enter(DemoContext& context)
{
    auto screenWidth = Petit2D::Texture::GetWidth(context.frameBufferTexture);
    auto screenHeight = Petit2D::Texture::GetHeight(context.frameBufferTexture);
    
    Petit2D::Catalog::Set(context.spriteCatalog, "splash_1", logo1);
    logo1.x = screenWidth * 0.5f;
    logo1.y = screenHeight * 0.5f;
    logo1.r = 1.0f;
    logo1.g = 1.0f;
    logo1.b = 1.0f;
    logo1.a = 0.0f;
    logo1.scale_x = 1.0f;
    logo1.scale_y = 1.0f;
    logo1.rotation = 0.0f;

    Petit2D::Catalog::Set(context.spriteCatalog, "splash_2", logo2);
    logo2.x = screenWidth * 0.5f;
    logo2.y = screenHeight * 0.5f;
    logo2.r = 1.0f;
    logo2.g = 1.0f;
    logo2.b = 1.0f;
    logo2.a = 0.0f;
    logo2.scale_x = 1.0f;
    logo2.scale_y = 1.0f;
    logo2.rotation = 0.0f;
    
    time = 0.0f;
    step = -1;
}

void ScreenIntro::exit(DemoContext& context)
{
}

bool ScreenIntro::render(DemoContext& context, float dt)
{
    auto screenWidth = Petit2D::Texture::GetWidth(context.frameBufferTexture);
    auto screenHeight = Petit2D::Texture::GetHeight(context.frameBufferTexture);
    auto ended = update(context, dt);

    Petit2D::SetFrameBuffer(context.frameBuffer);
    Petit2D::SetViewport(0, 0, screenWidth, screenHeight);
    Petit2D::SetBlending(Petit2D::BlendMode::ALPHA);
    Petit2D::SetTexture(context.spriteTexture, Petit2D::Texture::TextureUnit::UNIT_0);

    Petit2D::SetClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    Petit2D::Clear();

    Petit2D::Sprite::Use();
    Petit2D::Sprite::SetMatrix(glm::value_ptr(context.matrix));
    Petit2D::Sprite::SetTexture(Petit2D::Texture::TextureUnit::UNIT_0);
    Petit2D::Sprite::Begin();
    Petit2D::Sprite::Add(step <= 2 ? logo1 : logo2);
    Petit2D::Sprite::End();
    Petit2D::Sprite::Render();

    time += dt;
    return ended;
}

bool ScreenIntro::update(DemoContext& context, float dt)
{
    switch (step)
    {
    case -1:
        if (time >= 0.25f)
        {
            step++;
        }
    break;

    case 0:
        if (logo1.a < 1.0f)
        {
            logo1.a = glm::clamp(logo1.a + dt, 0.0f, 1.0f);
        }
        else
        {
            time = 0.0f;
            step++;
        }
    break;

    case 1:
        if (time > 1.5f)
        {
            step++;
        }
    break;

    case 2:
        if (logo1.a > 0.0f)
        {
            logo1.a = glm::clamp(logo1.a - dt, 0.0f, 1.0f);
        }
        else
        {
            time = 0.0f;
            step++;
        }
    break;

    case 3:
        if (logo2.a < 1.0f)
        {
            logo2.a = glm::clamp(logo2.a + dt, 0.0f, 1.0f);
        }
        else
        {
            time = 0.0f;
            step++;
        }
    break;

    case 4:
        if (time >= 1.5f)
        {
            step++;
        }
    break;

    case 5:
        if (logo2.a > 0.0f)
        {
            logo2.a = glm::clamp(logo2.a - dt, 0.0f, 1.0f);
        }
        else if (context.module->get_current_pattern() >= 1)
        {
            logo2.a = 0.0f;
            return false;
        }
    break;
        
    default:
    break;
    }

    return true;
}
