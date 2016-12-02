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

#include <QOpenGLFunctions_3_2_Core>

namespace jah3d
{

DefaultMaterial::DefaultMaterial()
{
    setTextureCount(4);

    QOpenGLShader *vshader = new QOpenGLShader(QOpenGLShader::Vertex);
    vshader->compileSourceFile("app/shaders/simple.vert");

    QOpenGLShader *fshader = new QOpenGLShader(QOpenGLShader::Fragment);
    fshader->compileSourceFile("app/shaders/simple.frag");



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
    //normalTexture = nullptr;

    diffuseColor = QColor(255,255,255);
    useDiffuseTex = false;
    //diffuseTexture = nullptr;

    shininess = 20.0f;
    useSpecularTex = false;
    specularColor = QColor(20,20,20);
    //specularTexture = nullptr;

    reflectionInfluence = 0.0f;
    useReflectionTex = false;
    //QOpenGLTexture* reflectionTexture;

    //this->setDiffuseColor(diffuseColor);
    //this->setSpecularColor(specularColor);
    //this->setShininess(10);

    //program->release();
}

void DefaultMaterial::begin(QOpenGLFunctions_3_2_Core* gl)
{
    program->bind();

    //program->setUniformValue("u_material.diffuse",false);
    //program->setUniformValue("u_useNormalTex",false);

    //set textures
    /*
    for(int i=0;i<textures.size();i++)
    {
        auto tex = textures[i];
        gl->glActiveTexture(GL_TEXTURE0+i);

        if(tex->texture!=nullptr)
        {
            tex->texture->bind();
            program->setUniformValue(tex->name.toStdString().c_str(), i);
        }
        else
        {
            //gl->glActiveTexture(GL_TEXTURE0+i);
            gl->glBindTexture(GL_TEXTURE_2D,0);
        }
    }
    */
    bindTextures(gl);

    //set params
    program->setUniformValue("u_material.diffuse",QVector3D(diffuseColor.redF(),diffuseColor.greenF(),diffuseColor.blueF()));
    program->setUniformValue("u_material.ambient",QVector3D(ambientColor.redF(),ambientColor.greenF(),ambientColor.blueF()));
    program->setUniformValue("u_material.specular",QVector3D(specularColor.redF(),specularColor.greenF(),specularColor.blueF()));
    program->setUniformValue("u_material.shininess",shininess);

    program->setUniformValue("u_textureScale", this->textureScale);

    program->setUniformValue("u_normalIntensity",normalIntensity);
    program->setUniformValue("u_reflectionInfluence",reflectionInfluence);

    program->setUniformValue("u_useDiffuseTex",useDiffuseTex);
    program->setUniformValue("u_useNormalTex",useNormalTex);
    program->setUniformValue("u_useReflectionTexture",useReflectionTex);

}

void DefaultMaterial::end(QOpenGLFunctions_3_2_Core* gl)
{
    //unset textures
}

void DefaultMaterial::setDiffuseTexture(QSharedPointer<Texture2D> tex)
{
    if(!!tex)
    {
        diffuseTexture=tex;
        useDiffuseTex = true;
        addTexture("u_diffuseTexture",tex);
    }
    else
    {
        useDiffuseTex = false;
        removeTexture("u_diffuseTexture");
    }
}

QString DefaultMaterial::getDiffuseTextureSource()
{
    if(!!diffuseTexture)
    {
        return diffuseTexture->source;
    }

    return QString::null;
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
}

void DefaultMaterial::setDiffuseColor(QColor col)
{
    diffuseColor = col;
}

QColor DefaultMaterial::getDiffuseColor()
{
    return diffuseColor;
}

void DefaultMaterial::setNormalTexture(QSharedPointer<Texture2D> tex)
{
    if(!!tex)
    {
        normalTexture=tex;
        useNormalTex = true;
        addTexture("u_normalTexture",tex);
    }
    else
    {
        useNormalTex = false;
        removeTexture("u_normalTexture");
    }
}

QString DefaultMaterial::getNormalTextureSource()
{
    if(!!normalTexture)
    {
        return normalTexture->source;
    }

    return QString::null;
}

void DefaultMaterial::setNormalIntensity(float intensity)
{
    normalIntensity = intensity;
}

float DefaultMaterial::getNormalIntensity()
{
    return normalIntensity;
}


void DefaultMaterial::setSpecularTexture(QSharedPointer<Texture2D> tex)
{
    if(!!tex)
    {
        specularTexture=tex;
        useSpecularTex = true;
        addTexture("u_specularTexture",tex);
    }
    else
    {
        useSpecularTex = false;
        removeTexture("u_specularTexture");
    }
}

QString DefaultMaterial::getSpecularTextureSource()
{
    if(!!specularTexture)
    {
        return specularTexture->source;
    }

    return QString::null;
}

void DefaultMaterial::setSpecularColor(QColor col)
{
    specularColor = col;
    //program->bind();
    //auto spec = QVector3D(col.redF(),col.greenF(),col.blueF());
    //program->setUniformValue("u_material.specular",spec);
    //program->release();
}

QColor DefaultMaterial::getSpecularColor()
{
    return specularColor;
}

void DefaultMaterial::setShininess(float shininess)
{
    this->shininess = shininess;
    //program->bind();
    //program->setUniformValue("u_material.shininess",shininess);
    //program->release();
}

float DefaultMaterial::getShininess()
{
    return shininess;
}


void DefaultMaterial::setReflectionTexture(QSharedPointer<Texture2D> tex)
{
    if(!!tex)
    {
        reflectionTexture=tex;
        useReflectionTex = true;
        addTexture("u_reflectionTexture",tex);
    }
    else
    {
        useReflectionTex = false;
        removeTexture("u_reflectionTexture");
    }
}

QString DefaultMaterial::getReflectionTextureSource()
{
    if(!!reflectionTexture)
    {
        return reflectionTexture->source;
    }

    return QString::null;
}

void DefaultMaterial::setReflectionInfluence(float intensity)
{
    reflectionInfluence = intensity;
}

float DefaultMaterial::getReflectionInfluence()
{
    return reflectionInfluence;
}

void DefaultMaterial::setTextureScale(float scale)
{
    this->textureScale = scale;
}

float DefaultMaterial::getTextureScale()
{
    return textureScale;
}

}
