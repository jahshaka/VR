/**************************************************************************
This file is part of JahshakaVR, VR Authoring Toolkit
http://www.jahshaka.com
Copyright (c) 2016  GPLv3 Jahshaka LLC <coders@jahshaka.com>

This is free software: you may copy, redistribute
and/or modify it under the terms of the GPLv3 License

For more information see the LICENSE file
*************************************************************************/

#ifndef TRANSFORMEDITOR_H
#define TRANSFORMEDITOR_H

#include <QWidget>
#include <QSharedPointer>

namespace iris
{
    class SceneNode;
}

namespace Ui {
class TransformEditor;
}

class TransformEditor : public QWidget
{
    Q_OBJECT

public:
    explicit TransformEditor(QWidget *parent = 0);
    ~TransformEditor();

    /**
     *  sects active scene node
     * @param sceneNode
     */
    void setSceneNode(QSharedPointer<iris::SceneNode> sceneNode);

protected slots:
    /**
     * triggered when active scene node's properties gets updated externally (from gizmo, scripts, etc)
     */
    //void sceneNodeUpdated();

    /**
     * translation change callbacks
     */
    void xPosChanged(double value);
    void yPosChanged(double value);
    void zPosChanged(double value);

    /**
     * rotation change callbacks
     */
    void xRotChanged(double value);
    void yRotChanged(double value);
    void zRotChanged(double value);

    /**
     * scale change callbacks
     */
    void xScaleChanged(double value);
    void yScaleChanged(double value);
    void zScaleChanged(double value);

private:
    Ui::TransformEditor *ui;

    QSharedPointer<iris::SceneNode> sceneNode;
};

#endif // TRANSFORMEDITOR_H
