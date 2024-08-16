#pragma once

#include <openglcontext.h>
#include <glm_includes.h>

class Texture
{
private:
    OpenGLContext* context;
    GLuint m_textureHandle;


public:
    QImage m_textureImage;

    Texture(OpenGLContext* context);
    ~Texture();

//    void create(const char *texturePath,
//                 GLenum internalFormat,
//                 GLenum format);

    void create(const char *texturePath);
//    void bufferPixelData(unsigned int width, unsigned int height,
//                          GLenum internalFormat, GLenum format,
//                          GLvoid *pixels);
    void bufferPixelData(unsigned int width, unsigned int height,
                          GLvoid *pixels);

    void loadTexture(GLuint texSlot);
    void bind(int texSLot);
    void destroy();
};

