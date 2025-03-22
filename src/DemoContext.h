#pragma once

#include <vector>
#include <glm/glm.hpp>
#include <libopenmpt/libopenmpt.hpp>
#include "render/petit2d.h"

class DemoContext
{

public:
    DemoContext();
    virtual ~DemoContext();
    void load(int fontSize);
    void unload();

    openmpt::module* module;
    glm::mat4 matrix;
    std::vector<Petit2D::Catalog::SpriteDef> fontGlyphs;
    Petit2D::Catalog::Catalog* spriteCatalog;
    Petit2D::Texture::Texture* spriteTexture;
    Petit2D::Texture::Texture* fontTexture;
    Petit2D::Texture::Texture* scanLinesTexture;
    Petit2D::Texture::Texture* frameBufferTexture;
    Petit2D::FrameBuffer::FrameBuffer* frameBuffer;

private:
    DemoContext(const DemoContext& copy);

};
