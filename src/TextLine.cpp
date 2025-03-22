#include "TextLine.h"

#include <string>

TextLine::TextLine(std::string text, float delay) :
stamp(),
text(text),
delay(delay),
timer(0),
textWidth(-1)
{
    stamp.r = 0.0f;
    stamp.g = 0.0f;
    stamp.b = 0.0f;
}

TextLine::~TextLine()
{
}

void TextLine::addText(const std::vector<Petit2D::Catalog::SpriteDef>& glyphs, float x, float y, const std::string text)
{
    stamp.x = x;
    stamp.y = y;

    int len = text.length();
    for (int i=0; i<len; i++)
    {
        auto& glyph = glyphs[static_cast<char>(text[i])];
        if (i == 0)
        {
            stamp.x += glyph.width * 0.5f;
        }

        if (text[i] != ' ')
        {
            stamp.width = glyph.width;
            stamp.height = glyph.height;
            stamp.s = glyph.s;
            stamp.t = glyph.t;
            stamp.p = glyph.p;
            stamp.q = glyph.q;
            Petit2D::Sprite::Add(stamp);
        }

        stamp.x += glyph.width;
    }
}

void TextLine::addChar(const std::vector<Petit2D::Catalog::SpriteDef>& glyphs, float x, float y, const char character)
{
    stamp.x = x;
    stamp.y = y;

    if (character != ' ')
    {
        auto& glyph = glyphs[character];
        stamp.width = glyph.width;
        stamp.height = glyph.height;
        stamp.s = glyph.s;
        stamp.t = glyph.t;
        stamp.p = glyph.p;
        stamp.q = glyph.q;
        Petit2D::Sprite::Add(stamp);
    }
}

int TextLine::getTextWidth(const std::vector<Petit2D::Catalog::SpriteDef>& glyphs, const std::string text)
{
    int size = 0;
    int len = text.length();
    for (int i=0; i<len; i++)
    {
        auto& glyph = glyphs[static_cast<char>(text[i])];
        size += glyph.width;
    }

    return size;
}

int TextLine::getCharWidth(const std::vector<Petit2D::Catalog::SpriteDef>& glyphs, const char character)
{
    return glyphs[character].width;
}

TextLineBlink::TextLineBlink(std::string text, float delay) :
TextLine(text, delay),
status(END)
{
}

TextLineBlink::~TextLineBlink()
{
}

bool TextLineBlink::update(float dt)
{
    if (textWidth == -1)
    {
        return true;
    }

    switch (status)
    {
    case END:
        timer = delay;
        status = SHOW;
    return true;

    case HIDE:
    case SHOW:
        timer -= dt;
        if (timer <= 0)
        {
            status = END;
            return false;
        }
    }

    status = ((int) (2 * timer) & 1) == 0 ? HIDE : SHOW;
    return true;
}

void TextLineBlink::render(const std::vector<Petit2D::Catalog::SpriteDef>& glyphs, float x, float y)
{
    if (textWidth == -1)
    {
        textWidth = getTextWidth(glyphs, text);
        return;
    }

    switch (status)
    {
    case SHOW:
        addText(glyphs, x - (textWidth * 0.5f), y, text);
    break;

    case HIDE:
    case END:
    break;
    }
}


TextLineScroll::TextLineScroll(std::string text, float delay, float stop, float screenWidth) :
TextLine(text, delay),
status(END),
charWidth(0),
stop(stop),
offset(0),
screenWidth(screenWidth)
{
}

TextLineScroll::~TextLineScroll()
{
}

bool TextLineScroll::update(float dt)
{
    if (textWidth == -1)
    {
        return true;
    }

    switch (status)
    {
    case END:
        offset = screenWidth;
        timer = delay;
        status = START;
    return true;

    case START:
        timer -= dt;
        if (timer <= 0)
        {
            timer = delay;
            status = SCROLL;
        }
    return true;

    case SCROLL:
        offset -= dt * 150;

        if (stop > 0)
        {
            auto textIsCentered = offset <= (screenWidth * 0.5f) - ((textWidth * 0.5f) + (charWidth * 0.5f));
            if (textIsCentered)
            {
                status = STOP;
                return true;
            }
        }

        if (offset < -textWidth)
        {
            timer -= dt;
            if (timer <= 0)
            {
                status = END;
                return false;
            }
        }
    return true;

    case STOP:
        stop -= dt;
        if (stop <= 0)
        {
            status = SCROLL;
        }
    return true;
    }

    return true;
}

void TextLineScroll::render(const std::vector<Petit2D::Catalog::SpriteDef>& glyphs, float x, float y)
{
    if (textWidth == -1)
    {
        textWidth = getTextWidth(glyphs, text);
        charWidth = getCharWidth(glyphs, ' ');
        return;
    }

    switch (status)
    {
    case STOP:
    case SCROLL:
        addText(glyphs, offset, y, text);
    break;

    case START:
    case END:
    break;
    }
}
