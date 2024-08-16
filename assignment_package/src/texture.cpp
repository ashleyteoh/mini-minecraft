#include "texture.h"

Texture::Texture(OpenGLContext* context)
    : context(context), m_textureHandle(-1)
{}

Texture::~Texture()
{
    destroy();
}

#if 0
void Texture::create(const char *texturePath,
                     GLenum internalFormat,
                     GLenum format)
#endif
void Texture::create(const char *texturePath)
{
    // Create a texture object on the GPU
    context->glGenTextures(1, &m_textureHandle);
    // Make that texture the "active texture" so that
    // any functions we call that operate on textures
    // operate on this one.
    context->glActiveTexture(GL_TEXTURE0);
    context->glBindTexture(GL_TEXTURE_2D, m_textureHandle);



    // Set the image filtering and UV wrapping options for the texture.
    // These parameters need to be set for EVERY texture you create.
    // They don't always have to be set to the values given here,
    // but they do need to be set.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);



    // Only bother to set up a QImage to read the texture file
    // if there *is* a texture file. If there's not, then this
    // Texture is meant to be used as the write target for something
    // like a FrameBuffer.
    if(texturePath) {
        // Create a QImage to load the image data
        // from the file indicated by `texturePath`
        QImage img(texturePath);
        img.convertTo(QImage::Format_ARGB32);
        img = img.mirrored();

        m_textureImage = img;
        // Take the data stored in our QImage and send it to the GPU,
        // where it will be stored in the texture object we created with
        // glGenTextures.
        bufferPixelData(img.width(), img.height(), img.bits());
    }
    context->printGLErrorLog();
}


void Texture::loadTexture(GLuint texSlot = 0) {
    // Loads image into the GPU
    context->printGLErrorLog();

    context->glActiveTexture(GL_TEXTURE0 + texSlot);
    context->glBindTexture(GL_TEXTURE_2D, m_textureHandle);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    context->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                                m_textureImage.width(), m_textureImage.height(),
                                0, GL_BGRA, GL_UNSIGNED_BYTE, m_textureImage.bits());

    context->printGLErrorLog();
}

#if 0
void Texture::bufferPixelData(unsigned int width, unsigned int height,
                              GLenum internalFormat, GLenum format,
                              GLvoid *pixels) {
    context->glActiveTexture(GL_TEXTURE0);
    context->glBindTexture(GL_TEXTURE_2D, m_textureHandle);
    context->glTexImage2D(GL_TEXTURE_2D, 0, internalFormat,
                            width, height,
                            0, format, GL_UNSIGNED_BYTE, pixels);
}

#endif

void Texture::bufferPixelData(unsigned int width, unsigned int height,
                              GLvoid *pixels) {
    context->glActiveTexture(GL_TEXTURE0);
    context->glBindTexture(GL_TEXTURE_2D, m_textureHandle);
    context->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,
                            width, height,
                            0, GL_RGB, GL_UNSIGNED_BYTE, pixels);
}

void Texture::bind(int texSlot = 0)
{
    context->glActiveTexture(GL_TEXTURE0 + texSlot);
    context->glBindTexture(GL_TEXTURE_2D, m_textureHandle);
}



void Texture::destroy() {
    context->glDeleteTextures(1, &m_textureHandle);
}
