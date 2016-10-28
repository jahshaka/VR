#ifndef TEXTURE_H
#define TEXTURE_H

#include <QSharedPointer>

class QOpenGLTexture;

namespace jah3d
{

typedef QSharedPointer<Texture> TexturePtr;

class Texture
{
public:
    QOpenGLTexture* texture;
    QString source;
};

}

#endif // TEXTURE_H
