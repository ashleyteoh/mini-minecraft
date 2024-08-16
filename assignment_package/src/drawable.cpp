#include "drawable.h"
#include <glm_includes.h>

Drawable::Drawable(OpenGLContext* context)
    : bufHandles(),
      bufGenerated(),
      indexCounts(),
      mp_context(context)
{
    // Initialize the index count to -1 to prevent
    // attempting to draw a Drawable that has not
    // been fully initialized.
    indexCounts[INDEX] = -1;
    indexCounts[IDX_OPQ] = -1;
    indexCounts[IDX_TRNS] = -1;
}

Drawable::~Drawable() {
    destroyVBOdata();
}

void Drawable::destroyVBOdata() {
    for(auto &kvp : bufHandles) {
        mp_context->glDeleteBuffers(1, &(kvp.second));
    }
}

GLenum Drawable::drawMode() {
    // Since we want every three indices in bufIdx to be
    // read to draw our Drawable, we tell that the draw mode
    // of this Drawable is GL_TRIANGLES

    // If we wanted to draw a wireframe, we would return GL_LINES

    return GL_TRIANGLES;
}

int Drawable::elemCount(BufferType t) {
    return indexCounts[t];
}

void Drawable::generateBuffer(BufferType buf) {
    bufGenerated[buf] = true;
    mp_context->glGenBuffers(1, &bufHandles[buf]);
}

bool Drawable::bindBuffer(BufferType buf) {
    if(bufGenerated[buf]) {
        // Should you have more than one kind of index buffer,
        // make sure to update this conditional to include them.
        buf == INDEX || buf == IDX_OPQ || buf == IDX_TRNS ?
        mp_context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufHandles[buf]) :
        mp_context->glBindBuffer(GL_ARRAY_BUFFER, bufHandles[buf]);
    }
    return bufGenerated[buf];
}

InstancedDrawable::InstancedDrawable(OpenGLContext *context)
    : Drawable(context), m_numInstances(0)
{}

InstancedDrawable::~InstancedDrawable(){}

int InstancedDrawable::instanceCount() const {
    return m_numInstances;
}

void InstancedDrawable::clearOffsetBuf() {
    if(bufGenerated[INSTANCED_OFFSET]) {
        mp_context->glDeleteBuffers(1, &bufHandles[INSTANCED_OFFSET]);
        bufGenerated[INSTANCED_OFFSET] = false;
    }
}
void InstancedDrawable::clearColorBuf() {
    if(bufGenerated[COLOR]) {
        mp_context->glDeleteBuffers(1, &bufHandles[COLOR]);
        bufGenerated[COLOR] = false;
    }
}
