#include "../graphics/material.h"
#include "../graphics/texture.h"
#include "../graphics/texture2d.h"
#include "../materials/defaultmaterial.h"
#include <QFile>
#include <QTextStream>

#include <QOpenGLShader>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QColor>

#include <QOpenGLFunctions>

namespace jah3d
{

DefaultMaterial::DefaultMaterial()
{
    //diffuseTexture = nullptr;
    //textureScale = 1.0f;

    QOpenGLShader *vshader = new QOpenGLShader(QOpenGLShader::Vertex);
    vshader->compileSourceFile("app/shaders/simple.vert");
    //vshader->compileSourceFile("app/shaders/color.vert");
    //vshader->compileSourceFile("assets/advance.vert");

    QOpenGLShader *fshader = new QOpenGLShader(QOpenGLShader::Fragment);
    fshader->compileSourceFile("app/shaders/simple.frag");
    //fshader->compileSourceFile("app/shaders/color.frag");
    //fshader->compileSourceFile("assets/advance.frag");



    program = new QOpenGLShaderProgram;
    program->addShader(vshader);
    program->addShader(fshader);

    program->bindAttributeLocation("a_pos", 0);
    program->bindAttributeLocation("a_texCoord", 1);
    program->bindAttributeLocation("a_normal", 2);
    program->bindAttributeLocation("a_tangent", 3);

    program->link();

    program->bind();
    program->setUniformValue("u_useDiffuseTex",false);
    program->setUniformValue("u_useNormalTex",false);
    program->setUniformValue("u_useReflectionTex",false);
    program->setUniformValue("u_useSpecularTex",false);
    program->setUniformValue("u_material.diffuse",QVector3D(1,0,0));
    //program->release();

    textureScale = 1.0f;
    ambientColor = QColor(0,0,0);

    useNormalTex = false;
    normalIntensity = 1.0f;
    normalTexture = nullptr;

    diffuseColor = QColor(255,255,255);
    useDiffuseTex = false;
    //diffuseTexture = nullptr;

    shininess = 100;
    useSpecularTex = false;
    specularColor = QColor(200,200,200);
    specularTexture = nullptr;

    float reflectionInfluence;
    bool useReflectonTex;
    QOpenGLTexture* reflectionTexture;

    this->setDiffuseColor(diffuseColor);
    this->setSpecularColor(specularColor);
    this->setShininess(0.01f);

    //program->release();
}

void DefaultMaterial::begin(QOpenGLFunctions* gl)
{
    program->bind();

    //program->setUniformValue("u_material.diffuse",false);
    //program->setUniformValue("u_useNormalTex",false);

    //set textures
    for(int i=0;i<textures.size();i++)
    {
        auto tex = textures[i];
        gl->glActiveTexture(GL_TEXTURE0+i);

        if(tex->texture!=nullptr)
        {
            tex->texture->bind(i);
            program->setUniformValue(tex->name.toStdString().c_str(), i);
        }
        else
        {
            //gl->glActiveTexture(GL_TEXTURE0+i);
            gl->glBindTexture(GL_TEXTURE_2D,0);
        }
    }

    //set params
    program->setUniformValue("u_material.diffuse",QVector3D(diffuseColor.redF(),diffuseColor.greenF(),diffuseColor.blueF()));

}

void DefaultMaterial::end()
{
    //unset textures
}

void DefaultMaterial::setDiffuseTexture(QSharedPointer<Texture2D> tex)
{
    diffuseTexture=tex;
    auto matTex = new MaterialTexture();
    matTex->texture = tex->texture;//bad! fix!
    matTex->name = "u_diffuseTexture";
    textures.append(matTex);

    //useDiffuseTex
    program->bind();
    program->setUniformValue("u_useDiffuseTex",true);
    //program->release();
}

/*
QOpenGLShader* DefaultMaterial::loadShader(QOpenGLShader::ShaderType type,QString filePath)
{
    //http://stackoverflow.com/questions/17149454/reading-entire-file-to-qstring
    auto shader = new QOpenGLShader(type,this);
    shader->compileSourceFile(filePat);

}
*/

void DefaultMaterial::setAmbientColor(QColor col)
{
    ambientColor = col;

    program->bind();
    program->setUniformValue("u_material.ambient",QVector3D(col.redF(),col.greenF(),col.blueF()));
    //program->release();
}

void DefaultMaterial::setDiffuseColor(QColor col)
{
    diffuseColor = col;

    program->bind();
    program->setUniformValue("u_material.diffuse",QVector3D(col.redF(),col.greenF(),col.blueF()));
    //program->setUniformValue("u_material.diffuse",QVector3D(1,1,1));
    //program->release();
}

QColor DefaultMaterial::getDiffuseColor()
{
    return QColor();
}

void DefaultMaterial::setNormalTexture(QOpenGLTexture* tex)
{
    normalTexture=tex;
    auto matTex = new MaterialTexture();
    matTex->texture = tex;
    matTex->name = "u_normalTexture";
    textures.append(matTex);

    //useDiffuseTex
    program->bind();
    program->setUniformValue("u_useNormalTex",true);
    //program->release();
}

void DefaultMaterial::setNormalIntensity(float intensity)
{
    normalIntensity = intensity;

    program->bind();
    program->setUniformValue("u_normalIntensity",normalIntensity);
    //program->release();
}

float DefaultMaterial::getNormalIntensity()
{
    return normalIntensity;
}


void DefaultMaterial::setSpecularTexture(QOpenGLTexture* tex)
{
    specularTexture=tex;
    auto matTex = new MaterialTexture();
    matTex->texture = tex;
    matTex->name = "u_specularTexture";
    textures.append(matTex);

    //useDiffuseTex
    program->bind();
    program->setUniformValue("u_useSpecularTex",true);
    //program->release();
}

void DefaultMaterial::setSpecularColor(QColor col)
{
    specularColor = col;
    program->bind();
    auto spec = QVector3D(col.redF(),col.greenF(),col.blueF());
    program->setUniformValue("u_material.specular",spec);
    //program->release();
}

QColor DefaultMaterial::getSpecularColor()
{
    return specularColor;
}

void DefaultMaterial::setShininess(float shininess)
{
    this->shininess = shininess;
    program->bind();
    program->setUniformValue("u_material.shininess",shininess);
    //program->release();
}

float DefaultMaterial::getShininess()
{
    return 0.0f;
}


void DefaultMaterial::setReflectionTexture(QOpenGLTexture* tex)
{
    reflectionTexture=tex;
    auto matTex = new MaterialTexture();
    matTex->texture = tex;
    matTex->name = "u_reflectionTexture";
    textures.append(matTex);

    program->bind();
    program->setUniformValue("u_useReflectionTex",true);
    //program->release();
}

void DefaultMaterial::setReflectionInfluence(float intensity)
{
    reflectionInfluence = intensity;

    program->bind();
    program->setUniformValue("u_reflectionInfluence",intensity);
    //program->release();
}

void DefaultMaterial::setTextureScale(float scale)
{
    this->textureScale = scale;
    //qDebug()<<scale<<endl;
    //textureScaleParam->setValue(scale);

    program->bind();
    program->setUniformValue("u_textureScale",scale);
    //program->release();
}

}
