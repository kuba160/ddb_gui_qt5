// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef SCOPESHADER_H
#define SCOPESHADER_H

#include <QtQuick/QQuickItem>
#include <QtQuick/QQuickWindow>
#if !USE_SCOPE_SHADER
class ScopeShader : public QQuickItem {
    Q_OBJECT
    QML_ELEMENT
public:
    ScopeShader(QQuickItem *parent = nullptr) : QQuickItem(parent) {}
};
#else
#include <QtCore/QFile>
#include <QtCore/QRunnable>
#include <QPainter>
#include <QGradient>
#include <QRandomGenerator>
#include <rhi/qrhi.h>
#include <deadbeef/deadbeef.h>
#include <scope/scope.h>

#include "ScopeConfig.h"


class ScopeRenderer : public QObject
{
    Q_OBJECT
public:
    void setItem(QQuickItem *it) { item = it; }
    void setWindow(QQuickWindow *window) { m_window = window; }

    ddb_scope_draw_data_t draw_data = {DDB_SCOPE_MULTICHANNEL,0,0,nullptr};
    //int fragment_dur = 200;
    QBindable<qreal> render_height;
    QBindable<qreal> render_width;
    QPointF render_bottom_left;


public slots:
    void frameStart();
    void mainPassRecordingStart();

private:
    QQuickItem *item = nullptr;
    qint8 m_swap_col;
    QQuickWindow *m_window;
    QShader m_vertexShader;
    QShader m_fragmentShader;
    std::unique_ptr<QRhiBuffer> m_vertexBuffer;
    std::unique_ptr<QRhiBuffer> m_uniformBuffer;
    std::unique_ptr<QRhiBuffer> m_colorBuffer;
    std::unique_ptr<QRhiShaderResourceBindings> m_srb;
    std::unique_ptr<QRhiGraphicsPipeline> m_pipeline;
};

//! [0]
class ScopeShader : public QQuickItem
{
    Q_OBJECT
    Q_PROPERTY(ScopeConfig *config READ getConfig CONSTANT)

    ScopeConfig* getConfig() {return &config;}

    //Q_PROPERTY(QColor fg READ getFg WRITE setFg NOTIFY fgChanged)
    //Q_PROPERTY(QColor bg READ getBg WRITE setBg NOTIFY bgChanged)
    QML_ELEMENT

public:
    ScopeShader();

    ScopeConfig config;

    //qreal t() const { return m_t; }
    //void setT(qreal t);

signals:
    void tChanged();
    void dataChanged();

public slots:
    void sync();
    void cleanup();
    void process(const ddb_audio_data_t *data);
    void onDataChanged();

private slots:
    void handleWindowChanged(QQuickWindow *win);

private:
    void releaseResources() override;

    //qreal m_t = 0;
    ScopeRenderer *m_renderer = nullptr;
    //ddb_scope_t * scope = nullptr;
};
//! [0]
#endif
#endif
