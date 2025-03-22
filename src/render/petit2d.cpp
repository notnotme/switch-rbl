#include "petit2d.h"

#include <map>
#include <string>
#include <fstream>
#include <glad/glad.h>

#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_PSD
#define STBI_NO_TGA
#define STBI_NO_GIF
#define STBI_NO_HDR
#define STBI_NO_PIC
#define STBI_NO_PNM
#include "stb_image.h"

#ifdef DEBUG
#  include <cstdio>
#  define DEBUG(...) printf(__VA_ARGS__)
#else
#  define DEBUG(...)
#endif

#define MAX_SPRITES_PER_SPRITE_BATCH    512
#define MAX_INDICES_PER_SPRITE_BATCH    (MAX_SPRITES_PER_SPRITE_BATCH * 6)
#define MAX_VERTICES_PER_SPRITE_BATCH   (MAX_SPRITES_PER_SPRITE_BATCH * 4)

namespace Petit2D
{

//-----------------------------------------------------------------------------
// [SECTION] Petit2D - Forward private declarations and basic types
//-----------------------------------------------------------------------------

GLuint  compileShader   (GLenum type, const char* src);
void    checkProgram    (GLuint id);

//-----------------------------------------------------------------------------
// [SECTION] Texture
//-----------------------------------------------------------------------------

namespace Texture
{

struct Context
{
    GLuint texture[TextureUnit::UNIT_COUNT] = { 0 };
} g_context;

struct Texture
{
    GLuint  id      = 0;
    int     width   = 0;
    int     height  = 0;
};

Texture* Create()
{
    GLuint id;
    glGenTextures(1, &id);

    if (id == 0)
    {
        return nullptr;
    }

    auto texture = new Texture();
    texture->id = id;

    return texture;
}

void Destroy(Texture* texture)
{
    if (texture != nullptr)
    {
        glDeleteTextures(1, &texture->id);
        delete(texture);
        texture = nullptr;
    }
}

void Init(Texture* texture, const char* filename)
{
    int width;
    int height;
    int channels;
    int desired_channels = 4;

    auto image = stbi_load(filename, &width, &height, &channels, desired_channels);
    if(image == nullptr)
    {
        DEBUG("Error in loading the image %s\n", filename);
        return;
    }

    if (g_context.texture[TextureUnit::UNIT_0] != texture->id)
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture->id);
        g_context.texture[TextureUnit::UNIT_0] = texture->id;
    }

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
    texture->width = width;
    texture->height = height;

    stbi_image_free(image);
}

void Init(Texture* texture, int width, int height, InternalFormat internalFormat, Format format, DataType type, void* pixels)
{
    GLint glInternalFormat;
    GLenum glFormat;
    GLenum glType;
  
    switch (internalFormat)
    {
    case RGBA8: glInternalFormat = GL_RGBA8; break;
    }

    switch (format)
    {
    case RGBA: glFormat = GL_RGBA; break;
    }

    switch (type)
    {
    case UNSIGNED_BYTE: glType = GL_UNSIGNED_BYTE; break;
    }

    if (g_context.texture[TextureUnit::UNIT_0] != texture->id)
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture->id);
        g_context.texture[TextureUnit::UNIT_0] = texture->id;
    }

    glTexImage2D(GL_TEXTURE_2D, 0, glInternalFormat, width, height, 0, glFormat, glType, pixels);
    texture->width = width;
    texture->height = height;
}

void SetWrap(Texture* texture, Wrap s, Wrap t)
{
    GLint wrap_s;
    GLint wrap_t;

    switch (s)
    {
    default:
    case CLAMP:     wrap_s = GL_CLAMP_TO_EDGE; break;
    case REPEAT:    wrap_s = GL_REPEAT; break;
    }

    switch (t)
    {
    default:
    case CLAMP:     wrap_t = GL_CLAMP_TO_EDGE; break;
    case REPEAT:    wrap_t = GL_REPEAT; break;
    }

    if (g_context.texture[TextureUnit::UNIT_0] != texture->id)
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture->id);
        g_context.texture[TextureUnit::UNIT_0] = texture->id;
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap_s);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap_t);
}

void SetFilter(Texture* texture, Filter min, Filter mag)
{
    GLint min_f;
    GLint mag_f;

    switch (min)
    {
    default:
    case NEAREST:   min_f = GL_NEAREST; break;
    case LINEAR:    min_f = GL_LINEAR; break;
    }

    switch (mag)
    {
    default:
    case CLAMP:     mag_f = GL_NEAREST; break;
    case REPEAT:    mag_f = GL_LINEAR; break;
    }

    if (g_context.texture[TextureUnit::UNIT_0] != texture->id)
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture->id);
        g_context.texture[TextureUnit::UNIT_0] = texture->id;
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_f);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_f);
}

int GetWidth(const Texture* texture)
{
    return texture->width;
}

int GetHeight(const Texture* texture)
{
    return texture->height;
}

} // namespace Texture

//-----------------------------------------------------------------------------
// [SECTION] FrameBuffer
//-----------------------------------------------------------------------------

namespace FrameBuffer
{

struct Context
{
    GLuint frameBuffer = 0;
} g_context;

struct FrameBuffer
{
    GLuint frameBufferId;
    GLuint renderBufferId;
};

FrameBuffer* Create()
{
    GLuint frameBufferId;
    glGenFramebuffers(1, &frameBufferId);

    if (frameBufferId == 0)
    {
        return nullptr;
    }

    GLuint renderBufferId;
    glGenRenderbuffers(1, &renderBufferId);

    if (renderBufferId == 0)
    {
        glDeleteFramebuffers(1, &frameBufferId);
        return nullptr;
    }

    auto frameBuffer = new FrameBuffer();
    frameBuffer->frameBufferId = frameBufferId;
    frameBuffer->renderBufferId = renderBufferId;

    return frameBuffer;
}

void Destroy(FrameBuffer* frameBuffer)
{
    if (frameBuffer != nullptr)
    {
        glDeleteFramebuffers(1, &frameBuffer->frameBufferId);
        glDeleteRenderbuffers(1, &frameBuffer->renderBufferId);
        delete(frameBuffer);
        frameBuffer = nullptr;
    }
}

void Init(FrameBuffer* frameBuffer, const Texture::Texture* texture)
{
    glBindRenderbuffer(GL_RENDERBUFFER, frameBuffer->renderBufferId);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, texture->width, texture->height);

    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer->frameBufferId);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, frameBuffer->renderBufferId);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture->id, 0);

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        DEBUG("glCheckFramebufferStatus\n");
    }

    g_context.frameBuffer = frameBuffer->frameBufferId;
}

} // namespace FrameBuffer

//-----------------------------------------------------------------------------
// [SECTION] Sprites
//-----------------------------------------------------------------------------

namespace Sprite
{

const char* VERTEX_SRC = R"text(
    #version 330 core
    precision mediump float;

    layout (location = 0) in vec2 position;
    layout (location = 1) in vec2 texCoord;
    layout (location = 2) in vec4 color;
    layout (location = 3) in float angle;
    layout (location = 4) in vec2 translation;
    layout (location = 5) in vec2 scale;

    out vec4 inColor;
    out vec2 inTexCoord;

    uniform mat4 projection;

    void main() {
        mat3 rotate_mat = mat3
            (cos(angle), -sin(angle), 0.0,
             sin(angle), cos(angle), 0.0,
             0.0, 0.0, 1.0);

        mat3 scale_mat = mat3
            (scale.x, 0.0, 0.0,
             0.0, scale.y, 0.0,
             0.0, 0.0, 1.0);

        mat3 translate_mat = mat3
            (1.0, 0.0, 0.0,
             0.0, 1.0, 0.0,
             translation.x, translation.y, 0.0);

        vec3 transformed = translate_mat * rotate_mat * scale_mat * vec3(position, 1.0);

        gl_Position = projection * vec4(transformed, 1.0);
        inColor = color;
        inTexCoord = texCoord;
    }
)text";

const char* FRAGMENT_SRC = R"text(
    #version 330 core
    precision mediump float;

    in vec4 inColor;
    in vec2 inTexCoord;

    out vec4 fragColor;

    uniform sampler2D tex2D;

    void main() {
        fragColor = inColor * texture(tex2D, inTexCoord);
    }
)text";

struct Context
{
    GLuint  vertexBufferId          = 0;
    GLuint  indiceBufferId          = 0;
    GLuint  vertexArrayId           = 0;
    int     spriteCount             = 0;
    void*   storage                 = nullptr;

    GLuint  programShaderId         = 0;
    GLint   matrixUniform           = 0;
    GLint   textureUniform          = 0;
    GLint   vertexLocation          = 0;
    GLint   colorLocation           = 0;
    GLint   textureLocation         = 0;
    GLint   translationLocation     = 0;
    GLint   scaleLocation           = 0;
    GLint   angleLocation           = 0;
} g_context;

struct SpriteVertex
{
    float x                 = 0.0f;
    float y                 = 0.0f;
    float s                 = 0.0f;
    float t                 = 0.0f;
    float r                 = 0.0f;
    float g                 = 0.0f;
    float b                 = 0.0f;
    float a                 = 0.0f;
    float translation_x     = 0.0f;
    float translation_y     = 0.0f;
    float rotation          = 0.0f;
    float scale_x           = 0.0f;
    float scale_y           = 0.0f;
};

void Create()
{
    auto vertexShader = compileShader(GL_VERTEX_SHADER, VERTEX_SRC);
    auto fragmentShader = compileShader(GL_FRAGMENT_SHADER, FRAGMENT_SRC);

    g_context.programShaderId = glCreateProgram();
    glAttachShader(g_context.programShaderId, vertexShader);
    glAttachShader(g_context.programShaderId, fragmentShader);
    glLinkProgram(g_context.programShaderId);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    checkProgram(g_context.programShaderId);

    g_context.matrixUniform = glGetUniformLocation(g_context.programShaderId, "projection");
    g_context.textureUniform = glGetUniformLocation(g_context.programShaderId, "tex2D");
    g_context.vertexLocation = glGetAttribLocation(g_context.programShaderId, "position");
    g_context.colorLocation = glGetAttribLocation(g_context.programShaderId, "color");
    g_context.textureLocation = glGetAttribLocation(g_context.programShaderId, "texCoord");
    g_context.translationLocation = glGetAttribLocation(g_context.programShaderId, "translation");
    g_context.scaleLocation = glGetAttribLocation(g_context.programShaderId, "scale");
    g_context.angleLocation = glGetAttribLocation(g_context.programShaderId, "angle");

    glGenVertexArrays(1, &g_context.vertexArrayId);
    glBindVertexArray(g_context.vertexArrayId);

    glGenBuffers(1, &g_context.vertexBufferId);
    glBindBuffer(GL_ARRAY_BUFFER, g_context.vertexBufferId);
    glBufferData(GL_ARRAY_BUFFER, sizeof(SpriteVertex) * MAX_VERTICES_PER_SPRITE_BATCH, nullptr, GL_STREAM_DRAW);

    glGenBuffers(1, &g_context.indiceBufferId);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_context.indiceBufferId);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(short) * MAX_INDICES_PER_SPRITE_BATCH, nullptr, GL_STATIC_DRAW);

    auto indiceRawPointer = glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY);
    auto indice = static_cast<short*> (indiceRawPointer);
    auto j = 0;
    for (auto i=0; i<MAX_INDICES_PER_SPRITE_BATCH; i+=6, j+=4) {
        indice[i] = j + 3;
        indice[i+1] = j + 2;
        indice[i+2] = j;
        indice[i+3] = j;
        indice[i+4] = j + 2;
        indice[i+5] = j + 1;
    }

    glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);

    glVertexAttribPointer(g_context.vertexLocation,      2, GL_FLOAT, false, sizeof(SpriteVertex), (void*) offsetof(SpriteVertex, x));
    glVertexAttribPointer(g_context.textureLocation,     2, GL_FLOAT, false, sizeof(SpriteVertex), (void*) offsetof(SpriteVertex, s));
    glVertexAttribPointer(g_context.colorLocation,       4, GL_FLOAT, false, sizeof(SpriteVertex), (void*) offsetof(SpriteVertex, r));
    glVertexAttribPointer(g_context.angleLocation,       1, GL_FLOAT, false, sizeof(SpriteVertex), (void*) offsetof(SpriteVertex, rotation));
    glVertexAttribPointer(g_context.translationLocation, 2, GL_FLOAT, false, sizeof(SpriteVertex), (void*) offsetof(SpriteVertex, translation_x));
    glVertexAttribPointer(g_context.scaleLocation,       2, GL_FLOAT, false, sizeof(SpriteVertex), (void*) offsetof(SpriteVertex, scale_x));

    glEnableVertexAttribArray(g_context.vertexLocation);
    glEnableVertexAttribArray(g_context.textureLocation);
    glEnableVertexAttribArray(g_context.colorLocation);
    glEnableVertexAttribArray(g_context.angleLocation);
    glEnableVertexAttribArray(g_context.translationLocation);
    glEnableVertexAttribArray(g_context.scaleLocation);

    glBindVertexArray(0);
}

void Destroy()
{
    glDeleteProgram(g_context.programShaderId);
    glDeleteBuffers(1, &g_context.vertexBufferId);
    glDeleteBuffers(1, &g_context.indiceBufferId);
    glDeleteVertexArrays(1, &g_context.vertexArrayId);
}

void Use()
{
    glUseProgram(g_context.programShaderId);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_context.indiceBufferId);
    glBindBuffer(GL_ARRAY_BUFFER, g_context.vertexBufferId);
    glBindVertexArray(g_context.vertexArrayId);
}

void SetTexture(Texture::TextureUnit unit)
{
    glUniform1i(g_context.textureUniform, unit);
}

void SetMatrix(const float* value)
{
    glUniformMatrix4fv(g_context.matrixUniform, 1, GL_FALSE, value);
}

void Begin()
{
    g_context.spriteCount = 0;
    g_context.storage = glMapBufferRange
    (
        GL_ARRAY_BUFFER,
        0,
        sizeof(SpriteVertex) * MAX_VERTICES_PER_SPRITE_BATCH,
        GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT
    );
}

void Add(const Sprite& sprite)
{
    if (g_context.storage == nullptr)
    {
        DEBUG("storage nullptr\n");
        return;
    }

    if (g_context.spriteCount >= MAX_SPRITES_PER_SPRITE_BATCH)
    {
        DEBUG("spriteCount >= MAX_SPRITES_PER_SPRITE_BATCH\n");
        return;
    }

    auto storage = static_cast<SpriteVertex*>(g_context.storage);
    auto index = g_context.spriteCount * 4;
    auto width = sprite.width * 0.5f;
    auto height = sprite.height * 0.5f;
    auto angle = sprite.rotation * (3.14f / 180.0f);
        
    storage[index].x = -width;
    storage[index].y = -height;
    storage[index].s = sprite.s;
    storage[index].t = sprite.t;
    storage[index].r = sprite.r;
    storage[index].g = sprite.g;
    storage[index].b = sprite.b;
    storage[index].a = sprite.a;
    storage[index].translation_x = sprite.x;
    storage[index].translation_y = sprite.y;
    storage[index].rotation = angle;
    storage[index].scale_x = sprite.scale_x;
    storage[index].scale_y = sprite.scale_y;
    index++;

    storage[index].x = width;
    storage[index].y = -height;
    storage[index].s = sprite.p;
    storage[index].t = sprite.t;
    storage[index].r = sprite.r;
    storage[index].g = sprite.g;
    storage[index].b = sprite.b;
    storage[index].a = sprite.a;
    storage[index].translation_x = sprite.x;
    storage[index].translation_y = sprite.y;
    storage[index].rotation = angle;
    storage[index].scale_x = sprite.scale_x;
    storage[index].scale_y = sprite.scale_y;
    index++;
    
    storage[index].x = width;
    storage[index].y = height;
    storage[index].s = sprite.p;
    storage[index].t = sprite.q;
    storage[index].r = sprite.r;
    storage[index].g = sprite.g;
    storage[index].b = sprite.b;
    storage[index].a = sprite.a;
    storage[index].translation_x = sprite.x;
    storage[index].translation_y = sprite.y;
    storage[index].rotation = angle;
    storage[index].scale_x = sprite.scale_x;
    storage[index].scale_y = sprite.scale_y;
    index++;

    storage[index].x = -width;
    storage[index].y = height;
    storage[index].s = sprite.s;
    storage[index].t = sprite.q;
    storage[index].r = sprite.r;
    storage[index].g = sprite.g;
    storage[index].b = sprite.b;
    storage[index].a = sprite.a;
    storage[index].translation_x = sprite.x;
    storage[index].translation_y = sprite.y;
    storage[index].rotation = angle;
    storage[index].scale_x = sprite.scale_x;
    storage[index].scale_y = sprite.scale_y;

    g_context.spriteCount++;
}

void End()
{
    glUnmapBuffer(GL_ARRAY_BUFFER);
}

void Render()
{
    if (g_context.spriteCount > 0)
    {
        glDrawElements(GL_TRIANGLES, g_context.spriteCount * 6, GL_UNSIGNED_SHORT, 0);
    }
}

} // namespace Sprite

//-----------------------------------------------------------------------------
// [SECTION] Catalog
//-----------------------------------------------------------------------------

namespace Catalog
{

//-----------------------------------------------------------------------------
// [SECTION] Catalog - Forward declarations and basic types
//-----------------------------------------------------------------------------

struct  Catalog
{
    int imageWidth      = 0;
    int imageHeight     = 0;
    int spriteCount     = 0;
    std::map<std::string, SpriteDef> sprites;
};

//-----------------------------------------------------------------------------
// [SECTION] Catalog - End-user API functions
//-----------------------------------------------------------------------------

Catalog* Create()
{
    return new Catalog();
}

void Init(Catalog* catalog, const char* filename)
{
    auto file = std::ifstream(filename, std::ios::in | std::ios::binary | std::ios::ate);
    if (!file.is_open())
    {
        DEBUG("Catalog not found %s\n", filename);
        return;
    }

    if (file.tellg() <= 6)
    {
        DEBUG("Not a catalog file %s\n", filename);
        return;
    }

    char check[6] = { 0 };
    file.seekg(std::ios::beg);
    file.read(check, 6);

    if (std::string(check) != "SPRCAT")
    {
        file.close();
        DEBUG("SPRCAT signature not present.\n");
        return;
    }

    file.read(reinterpret_cast<char*>(&catalog->imageWidth), sizeof(catalog->imageWidth));
    file.read(reinterpret_cast<char*>(&catalog->imageHeight), sizeof(catalog->imageHeight));
    file.read(reinterpret_cast<char*>(&catalog->spriteCount), sizeof(catalog->spriteCount));

    catalog->sprites.clear();
    for (int i=0; i<catalog->spriteCount; i++)
    {
        char name[32] = { 0 };
        file.read(name, 32);

        SpriteDef spriteDef;
        file.read(reinterpret_cast<char*>(&spriteDef), sizeof(SpriteDef));
        
        catalog->sprites.insert(
            std::pair<std::string, SpriteDef>(std::string(name), spriteDef)
        );
    }

    file.close();
}

void Destroy(Catalog* catalog)
{
    if (catalog != nullptr)
    {
        catalog->sprites.clear();
        delete(catalog);
        catalog = nullptr;
    }
}

void Set(Catalog* catalog, const char* name, Sprite::Sprite& sprite)
{
    if (catalog->sprites.find(name) == catalog->sprites.end())
    {
        DEBUG("Sprite not found: %s\n", name);
        return;
    }

    auto spriteDef = catalog->sprites[name];
    sprite.width = spriteDef.width;
    sprite.height = spriteDef.height;
    sprite.s = spriteDef.s;
    sprite.t = spriteDef.t;
    sprite.p = spriteDef.p;
    sprite.q = spriteDef.q;
}

} // namespace Catalog

//-----------------------------------------------------------------------------
// [SECTION] Petit2D
//-----------------------------------------------------------------------------

void Create()
{
    glCullFace(GL_BACK);
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glEnable(GL_TEXTURE_2D);
    glDisable(GL_DEPTH_WRITEMASK);
    glDisable(GL_DEPTH_TEST);

    Sprite::Create();
}

void Destroy()
{
    Sprite::Destroy();
}

void SetClearColor(float r, float g, float b, float a)
{
    glClearColor(r, g, b, a);
}

void Clear()
{
    glClear(GL_COLOR_BUFFER_BIT);
}

void SetViewport(int x, int y, int width, int height)
{
    glViewport(x, y, width, height);
}

void SetBlending(BlendMode mode)
{
    switch (mode)
    {
    case NONE:      glDisable(GL_BLEND); break;
    case ALPHA:     glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); break;
    case ADDITIVE:  glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE); break;
    }
}

void SetFrameBuffer(const FrameBuffer::FrameBuffer* frameBuffer)
{
    if (frameBuffer == nullptr)
    {
        if (FrameBuffer::g_context.frameBuffer != 0)
        {
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            FrameBuffer::g_context.frameBuffer = 0;
        }
    }
    else
    {
        if (FrameBuffer::g_context.frameBuffer != frameBuffer->frameBufferId)
        {
            glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer->frameBufferId);
            FrameBuffer::g_context.frameBuffer = frameBuffer->frameBufferId;
        }
    }
}

void SetTexture(const Texture::Texture* texture, Texture::TextureUnit unit)
{
    if (Texture::g_context.texture[unit] != texture->id)
    {
        glActiveTexture(GL_TEXTURE0 + unit);
        glBindTexture(GL_TEXTURE_2D, texture->id);
        Texture::g_context.texture[unit] = texture->id;
    }
}

//-----------------------------------------------------------------------------
// [SECTION] Petit2D - Private declarations and basic types
//-----------------------------------------------------------------------------

GLuint compileShader(GLenum type, const char* src)
{
    GLint success;
    auto id = glCreateShader(type);

    glShaderSource(id, 1, &src, nullptr);
    glCompileShader(id);

    glGetShaderiv(id, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        GLchar buf[512] = { 0 };
        glGetShaderInfoLog(id, sizeof(buf)-1, nullptr, buf);
        DEBUG("Could not compile shader: %s\n", buf);
        return 0;
    }

    return id;
}

void checkProgram(GLuint id)
{
    GLint success;
    glGetProgramiv(id, GL_LINK_STATUS, &success);
    if (!success)
    {
        char buf[512] = { 0 };
        glGetProgramInfoLog(id, sizeof(buf)-1, nullptr, buf);
        DEBUG("Could not link program: %s\n", buf);
    }
}

} // namespace Petit2D
