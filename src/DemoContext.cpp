#include "DemoContext.h"

#include <fstream>
#include <glm/gtc/matrix_transform.hpp>

static const int FRAMEBUFFER_WIDTH = 640;
static const int FRAMEBUFFER_HEIGHT = 360;

DemoContext::DemoContext() :
module(nullptr)
{
}

DemoContext::~DemoContext()
{
}

void DemoContext::load(int fontSize)
{
    std::ifstream audioFile = std::ifstream("romfs:/bd_int10.xm", std::ios::binary);
    module = new openmpt::module(audioFile);
    module->ctl_set_boolean("render.resampler.emulate_amiga", true);
    module->ctl_set_text("play.at_end", "continue");
    audioFile.close();

    spriteCatalog = Petit2D::Catalog::Create();
    Petit2D::Catalog::Init(spriteCatalog, "romfs:/sprites.bin");

    scanLinesTexture = Petit2D::Texture::Create();
    Petit2D::Texture::Init(scanLinesTexture, "romfs:/scanlines.png");
    Petit2D::Texture::SetWrap(scanLinesTexture, Petit2D::Texture::Wrap::REPEAT, Petit2D::Texture::Wrap::REPEAT);
    Petit2D::Texture::SetFilter(scanLinesTexture, Petit2D::Texture::Filter::NEAREST, Petit2D::Texture::Filter::NEAREST);

    fontTexture = Petit2D::Texture::Create();
    Petit2D::Texture::Init(fontTexture, "romfs:/font1.png");
    Petit2D::Texture::SetWrap(fontTexture, Petit2D::Texture::Wrap::CLAMP, Petit2D::Texture::Wrap::CLAMP);
    Petit2D::Texture::SetFilter(fontTexture, Petit2D::Texture::Filter::NEAREST, Petit2D::Texture::Filter::NEAREST);

    spriteTexture = Petit2D::Texture::Create();
    Petit2D::Texture::Init(spriteTexture, "romfs:/spritesheet.png");
    Petit2D::Texture::SetWrap(spriteTexture, Petit2D::Texture::Wrap::CLAMP, Petit2D::Texture::Wrap::CLAMP);
    Petit2D::Texture::SetFilter(spriteTexture, Petit2D::Texture::Filter::NEAREST, Petit2D::Texture::Filter::NEAREST);

    frameBufferTexture = Petit2D::Texture::Create();
    Petit2D::Texture::Init(frameBufferTexture, FRAMEBUFFER_WIDTH, FRAMEBUFFER_HEIGHT, Petit2D::Texture::InternalFormat::RGBA8, Petit2D::Texture::Format::RGBA, Petit2D::Texture::DataType::UNSIGNED_BYTE, nullptr);
    Petit2D::Texture::SetWrap(frameBufferTexture, Petit2D::Texture::Wrap::CLAMP, Petit2D::Texture::Wrap::CLAMP);
    Petit2D::Texture::SetFilter(frameBufferTexture, Petit2D::Texture::Filter::NEAREST, Petit2D::Texture::Filter::NEAREST);

    frameBuffer = Petit2D::FrameBuffer::Create();
    Petit2D::FrameBuffer::Init(frameBuffer, frameBufferTexture);
    
    matrix = glm::ortho(0.0f, (float) Petit2D::Texture::GetWidth(frameBufferTexture), (float) Petit2D::Texture::GetHeight(frameBufferTexture), 0.0f, 0.0f, 1.0f);

    auto glyphWidth = 1.0f / 16.0f;
    auto glyphHeight = 1.0f / 16.0f;
    fontGlyphs.clear();
    for (auto y=0; y<16; y++)
    {
        for (auto x=0; x<16; x++)
        {
            Petit2D::Catalog::SpriteDef spriteDef;
            spriteDef.width = fontSize;
            spriteDef.height = fontSize;
            spriteDef.s = (float) (x * glyphWidth);
            spriteDef.t = (float) (y * glyphHeight);
            spriteDef.p = (float) ((x * glyphWidth) + glyphWidth);
            spriteDef.q = (float) ((y * glyphHeight) + glyphHeight);

            fontGlyphs.push_back(spriteDef);
        }
    }
}

void DemoContext::unload()
{
    if (module != nullptr)
    {
        delete module;
        module = nullptr;
    }

    fontGlyphs.clear();
    Petit2D::Catalog::Destroy(spriteCatalog);
    Petit2D::FrameBuffer::Destroy(frameBuffer);
    Petit2D::Texture::Destroy(frameBufferTexture);
    Petit2D::Texture::Destroy(scanLinesTexture);
    Petit2D::Texture::Destroy(fontTexture);
    Petit2D::Texture::Destroy(spriteTexture);
}
