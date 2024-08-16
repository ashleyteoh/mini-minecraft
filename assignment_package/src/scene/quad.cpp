#include "quad.h"
#include "chunkHelpers.h"
#include <vector>

Quad::Quad(OpenGLContext *context)
    : Drawable(context)
{}

Quad::~Quad()
{}

void Quad::createVBOdata() {
    std::vector<glm::vec4> glPos { glm::vec4(-1,-1, 1, 0),
                                   glm::vec4( 1,-1, 1, 0),
                                   glm::vec4( 1, 1, 1, 0),
                                   glm::vec4(-1, 1, 1, 0) };

    std::vector<glm::vec4> glUV { glm::vec4(0, 0, 0, 0),
                                  glm::vec4(1, 0, 0, 0),
                                  glm::vec4(1, 1, 0, 0),
                                  glm::vec4(0, 1, 0, 0) };

    std::vector<GLuint> glIndex {0,1,2,0,2,3};

    std::vector<VertexData> data;
    for(int i = 0; i < 4; ++i) {
        data.push_back({glPos[i], glm::vec4(0, 0, 1, 0), glUV[i]});
    }

    generateBuffer(IDX_TRNS);
    bindBuffer(IDX_TRNS);
    mp_context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, glIndex.size() * sizeof(GLuint), glIndex.data(), GL_STATIC_DRAW);

    indexCounts[IDX_TRNS] = glIndex.size();

    generateBuffer(INTERLEAVED_TRNS);
    bindBuffer(INTERLEAVED_TRNS);
    mp_context->glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(VertexData), data.data(), GL_STATIC_DRAW);
}
