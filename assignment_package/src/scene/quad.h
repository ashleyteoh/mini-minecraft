#ifndef QUAD_H
#define QUAD_H
#include <QOpenGLFunctions_3_3_Core>
#include "drawable.h"

class Quad : public Drawable {
public:
    Quad(OpenGLContext* context);
    ~Quad();

    // A function that initializes the vertex
    // coordinates for a quadrangle that covers
    // the entire screen in screen space,
    // then buffers that data to the GPU.
    void createVBOdata() override;
};

#endif // QUAD_H
