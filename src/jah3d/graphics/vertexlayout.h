#ifndef VERTEXLAYOUT_H
#define VERTEXLAYOUT_H

#include <QList>
#include <QOpenGLShaderProgram>

namespace jah3d
{

//todo: use VAO
struct VertexAttribute
{
    //determined by shader
    int loc;
    QString name;

    int type;//GL_FLOAT,GL_INT, etc
    int count;//2 for vec2, 3 for vec3, etc
    int sizeInBytes;
};

class VertexLayout
{
    QList<VertexAttribute> attribs;
    int stride;

public:
    VertexLayout()
    {
        stride = 0;
    }

    void addAttrib(QString name,int type,int count,int sizeInBytes)
    {
        VertexAttribute attrib = {-1,name,type,count,sizeInBytes};
        attribs.append(attrib);

        stride += sizeInBytes;
    }

    //todo: make this more efficient
    void bind(QOpenGLShaderProgram* program)
    {
        int offset = 0;
        int loc = 0;
        for(auto attrib: attribs)
        {
            program->enableAttributeArray(loc);
            program->bindAttributeLocation(attrib.name,loc);
            program->setAttributeBuffer(attrib.loc, attrib.type, offset, attrib.count,stride);
            offset += attrib.sizeInBytes;
            loc++;
        }
    }

    void unbind(QOpenGLShaderProgram* program)
    {
        int loc = 0;
        for(auto attrib: attribs)
        {
            program->disableAttributeArray(loc);
            loc++;
        }
    }

    //default vertex layout for meshes
    static VertexLayout* createMeshDefault()
    {
        auto layout = new VertexLayout();

        layout->addAttrib("a_pos",GL_FLOAT,3,sizeof(GLfloat)*3);
        layout->addAttrib("a_normal",GL_FLOAT,3,sizeof(GLfloat)*3);
        layout->addAttrib("a_tangent",GL_FLOAT,3,sizeof(GLfloat)*3);
        layout->addAttrib("a_texCoord",GL_FLOAT,2,sizeof(GLfloat)*2);

        return layout;
    }
};

}

#endif // VERTEXLAYOUT_H
