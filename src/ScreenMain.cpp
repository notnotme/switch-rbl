#include "ScreenMain.h"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STAR_COUNT 255
#define RASTER_COUNT 5

ScreenMain::ScreenMain() :
Screen()
{
}

ScreenMain::~ScreenMain()
{
}

void ScreenMain::enter(DemoContext& context)
{
    auto screenWidth = Petit2D::Texture::GetWidth(context.frameBufferTexture);
    auto screenHeight = Petit2D::Texture::GetHeight(context.frameBufferTexture);

    textLineOffset = 0;
    textLines.push_back(std::unique_ptr<TextLineBlink>(new TextLineBlink("Hello I'm blinking !", 5.0f)));
    textLines.push_back(std::unique_ptr<TextLineScroll>(new TextLineScroll("Hello I'm scrolling !", 1.0f, 0.0f, screenWidth)));
    textLines.push_back(std::unique_ptr<TextLineScroll>(new TextLineScroll("Hello I'm scrolling with a stop !", 1.0f, 3.0f, screenWidth)));
    //mScroller.add("Hello I'm scrolling !", Scroller::SCROLL, 1.0f);
    //mScroller.add("Hello I'm scrolling, stopping, and scrolling again !", Scroller::SCROLL_STOP, 1.0f);

    Petit2D::Catalog::Set(context.spriteCatalog, "xrtitle_1", title1);
    title1.x = -title1.width * 1.25f;
    title1.y = 8 + (title1.height * 0.5f);

    Petit2D::Catalog::Set(context.spriteCatalog, "xrtitle_2", title2);
    title2.x = screenWidth + title2.width * 1.25f;
    title2.y = 8 + (title2.height * 0.5f);

    Petit2D::Catalog::Set(context.spriteCatalog, "xrlogo", logo);
    logo.x = screenWidth * 0.5f;
    logo.y = screenHeight * 0.63f;
    logo.a = 0.0f;

    Petit2D::Catalog::Set(context.spriteCatalog, "border", border);

    Petit2D::Catalog::Set(context.spriteCatalog, "pixel", borderBackground);
    borderBackground.width = screenWidth;
    borderBackground.height = 18.0f;
    borderBackground.x = screenWidth * 0.5f;
    borderBackground.y = 0.0f;
    borderBackground.scale_y = 0.0f;

    Petit2D::Catalog::Set(context.spriteCatalog, "pixel", star);
    star.width = 2.0f;
    star.height = 2.0f;
    star.a = 0.0f;

    stars.clear();
    for (int i=0; i<STAR_COUNT; i++)
    {
        Star star;
        star.position.x = rand() % screenWidth;
        star.position.y = 4 + (rand() % (int) (8 + title1.height));
        star.speed = 1.0f + (rand() % 3);

        stars.push_back(star);
    }

    Petit2D::Catalog::Set(context.spriteCatalog, "raster", raster);
    raster.x = screenWidth * 0.5f;
    raster.y = -raster.height;
    raster.width = screenWidth;
    raster.scale_y = 3.0f;
    
    rasters.push_back(glm::vec4((float) 117/255, (float) 208/255, (float) 241/255, 1.0f));
    rasters.push_back(glm::vec4(1.0f, (float) 193/255, (float) 30/255, 1.0f));
    rasters.push_back(glm::vec4((float) 220/255, (float) 79/255, 0.0f, 1.0f));
    rasters.push_back(glm::vec4((float) 152/255, (float) 98/255, (float) 151/255, 1.0f));
    rasters.push_back(glm::vec4((float) 220/255, 0.0f, (float) 130/255, 1.0f));

    rasterOffset = -title1.height * 2;
    time = 0.0f;
    step = 0;
}

void ScreenMain::exit(DemoContext& context)
{
    stars.clear();
    rasters.clear();
    textLines.clear();
}

bool ScreenMain::render(DemoContext& context, float dt)
{
    auto screenWidth = Petit2D::Texture::GetWidth(context.frameBufferTexture);
    auto screenHeight = Petit2D::Texture::GetHeight(context.frameBufferTexture);
    auto ended = update(context, dt);

    Petit2D::SetFrameBuffer(context.frameBuffer);
    Petit2D::SetViewport(0, 0, screenWidth, screenHeight);
    Petit2D::SetClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    Petit2D::Clear();
    Petit2D::SetTexture(context.spriteTexture, Petit2D::Texture::TextureUnit::UNIT_0);
   
    Petit2D::Sprite::Use();
    Petit2D::Sprite::SetMatrix(glm::value_ptr(context.matrix));
    Petit2D::Sprite::SetTexture(Petit2D::Texture::TextureUnit::UNIT_0);

    Petit2D::SetBlending(Petit2D::BlendMode::ALPHA);
    Petit2D::Sprite::Begin();
    for (int i=0; i<STAR_COUNT; i++)
    {
        auto item = &stars[i];
        auto color = glm::clamp(item->speed / 3.0f, 0.1f, 1.0f);
        star.x = item->position.x;
        star.y = item->position.y;
        star.r = color;
        star.g = color;
        star.b = color;
        Petit2D::Sprite::Add(star);
    }
    Petit2D::Sprite::End();
    Petit2D::Sprite::Render();

    Petit2D::SetBlending(Petit2D::BlendMode::ADDITIVE);
    Petit2D::Sprite::Begin();
    for (int i=0; i<RASTER_COUNT; i++)
    {
        auto item = &rasters[i];
        raster.y = rasterOffset + glm::cos(((time * 75) + (i * 18.0f)) * (3.14f / 180.0f)) * (title1.height * 0.425f);
        raster.r = item->r;
        raster.g = item->g;
        raster.b = item->b;
        raster.a = item->a;
        Petit2D::Sprite::Add(raster);
    }
    Petit2D::Sprite::Add(title1);
    Petit2D::Sprite::Add(title2);
    Petit2D::Sprite::End();
    Petit2D::Sprite::Render();

    Petit2D::SetBlending(Petit2D::BlendMode::ALPHA);
    Petit2D::Sprite::Begin();
    Petit2D::Sprite::Add(logo);
    Petit2D::Sprite::Add(borderBackground);
    auto offset = borderOffset;
    while (offset < screenWidth * 1.5f)
    {
        border.x = offset;
        border.y = borderBackground.y - ((borderBackground.height * borderBackground.scale_y) * 0.5f) - 1;
        Petit2D::Sprite::Add(border);

        border.x = offset + border.width * 0.5f;
        border.y = borderBackground.y + ((borderBackground.height * borderBackground.scale_y) * 0.5f) + 1;
        Petit2D::Sprite::Add(border);

        offset += border.width;
    }
    Petit2D::Sprite::End();
    Petit2D::Sprite::Render();

    if (step == 1)
    {
        Petit2D::SetTexture(context.fontTexture, Petit2D::Texture::TextureUnit::UNIT_0);
        Petit2D::Sprite::Begin();
        textLines[textLineOffset]->render(context.fontGlyphs, screenWidth * 0.5f, borderBackground.y);
        Petit2D::Sprite::End();
        Petit2D::Sprite::Render();
    }
    
    time += dt;
    return ended;
}

bool ScreenMain::update(DemoContext& context, float dt)
{
    auto screenWidth = Petit2D::Texture::GetWidth(context.frameBufferTexture);
    auto screenHeight = Petit2D::Texture::GetHeight(context.frameBufferTexture);

    borderOffset -= 300.0f * dt;
    if (borderOffset < -screenWidth)
    {
        borderOffset += screenWidth;
    }

    switch (step)
    {
    case 0:
        if (title1.x < screenWidth * 0.5f)
        {
            title1.x += 800.0f * dt;
            title2.x -= 800.0f * dt;
        }
        else
        {
            title1.x = screenWidth * 0.5f;
            title2.x = screenWidth * 0.5f;
        }

        if (borderBackground.y < (screenHeight - border.height) - borderBackground.height * 0.5f)
        {
            borderBackground.y += 375.0f * dt;
        }
        else
        {
            borderBackground.y = (screenHeight - border.height) - borderBackground.height * 0.5f;
        }

        if (borderBackground.scale_y < 1.0f)
        {
            borderBackground.scale_y += dt;
        }
        else
        {
            borderBackground.scale_y = 1.0f;
        }

        if (borderBackground.scale_y == 1.0f && title1.x == screenWidth * 0.5f)
        {
            logo.a = 0.06f;
            step++;
        }
    break;
    
    case 1:
        if (! textLines[textLineOffset]->update(dt))
        {
            textLineOffset = (textLineOffset + 1) % textLines.size();
        }

        for (int i=0; i<STAR_COUNT; i++)
        {
            auto& item = stars[i]; 
            if (item.position.y < title1.y)
            {
                item.position.x = item.position.x - (item.speed * dt * 25);
                if (item.position.x < -star.width)
                {
                    item.position.x += screenWidth + star.width;
                }
            }
            else
            {
                item.position.x = item.position.x + (item.speed * dt * 25);
                if (item.position.x > screenWidth + star.width)
                {
                    item.position.x = -star.width;
                }
            }
        }
        if (star.a < 1.0f)
        {
            star.a = glm::clamp(star.a + (dt * 0.20f), 0.0f, 1.0f);
        }
        
        if (logo.a != 0.05f && logo.a < 0.13f)
        {
            logo.a = glm::clamp(logo.a * (1.0f + dt * 3.0f), 0.0f, 0.13f);
        }
        else
        {
            logo.a = 0.05f;
        }

        if (context.module->get_current_pattern() < 0x0F && (context.module->get_current_row() + 4) % 8 == 0)
        {
            titleWobble = 360.0f;
        }
        else
        {
            titleWobble *= 0.75f;
        }

        if (rasterOffset < title1.y)
        {
            rasterOffset += dt * 100;
        }

        logo.y = (screenHeight * 0.63f) + glm::sin(titleWobble * 3.14f / 180.0f) * 3.0f;
        title1.x = (screenWidth * 0.5f) + glm::sin(titleWobble * 3.14f / 180.0f) * 2.0f;
        title2.x = (screenWidth * 0.5f) + glm::sin(180.0f + titleWobble * 3.14f / 180.0f) * 2.0f;
    break;

    default:
    break;
    }
    
    return true;
}
