#pragma once

#include <string>
#include <vector>
#include "render/petit2d.h"

class TextLine
{

public:
    TextLine(std::string text, float delay);
    virtual ~TextLine();

    virtual bool update(float dt) = 0;
    virtual void render(const std::vector<Petit2D::Catalog::SpriteDef>& glyphs, float x, float y) = 0;

protected:
    Petit2D::Sprite::Sprite stamp;
    std::string text;
    float delay;
    float timer;
    int textWidth;

    void addText(const std::vector<Petit2D::Catalog::SpriteDef>& glyphs, float x, float y, const std::string text);
    void addChar(const std::vector<Petit2D::Catalog::SpriteDef>& glyphs, float x, float y, const char character);
    int getTextWidth(const std::vector<Petit2D::Catalog::SpriteDef>& glyphs, const std::string text);
    int getCharWidth(const std::vector<Petit2D::Catalog::SpriteDef>& glyphs, const char character);

private:
    TextLine(const TextLine& copy);

};

class TextLineBlink : public TextLine
{

private:
    enum Status
    {
        SHOW,
        HIDE,
        END
    } status;

public:
    TextLineBlink(std::string text, float delay);
    virtual ~TextLineBlink();

    virtual bool update(float dt) override;
    virtual void render(const std::vector<Petit2D::Catalog::SpriteDef>& glyphs, float x, float y) override;

};

class TextLineScroll : public TextLine
{

private:
    enum Status
    {
        START,
        STOP,
        END,
        SCROLL
    } status;

    float charWidth;
    float stop;
    float offset;
    float screenWidth;

public:
    TextLineScroll(std::string text, float delay, float stop, float screenWidth);
    virtual ~TextLineScroll();

    virtual bool update(float dt) override;
    virtual void render(const std::vector<Petit2D::Catalog::SpriteDef>& glyphs, float x, float y) override;

};
