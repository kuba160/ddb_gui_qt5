// Copyright (C) 2024 Jakub Wasylków
// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: BSD-3-Clause

#include "ScopeShader.h"

#include <QDebug>
#include <QMutex>

#if USE_SCOPESHADER
QMutex mm;

ScopeShader::ScopeShader() : config(this) {
    connect(this, &QQuickItem::windowChanged, this, &ScopeShader::handleWindowChanged);
    //scope = ddb_scope_alloc();
    //ddb_scope_init(scope);
    qDebug() << "SCOPE SHADER CREATED";

    config.bindableDrawDataPoints().setBinding([&](){qDebug() << bindableWidth().value(); return (int) bindableWidth().value();});
    connect(this, &ScopeShader::dataChanged, this, &ScopeShader::onDataChanged);
}

void ScopeShader::handleWindowChanged(QQuickWindow *win) {
    if (win) {
        connect(win, &QQuickWindow::afterSynchronizing, this, &ScopeShader::sync, Qt::DirectConnection);
        connect(win, &QQuickWindow::sceneGraphInvalidated, this, &ScopeShader::cleanup, Qt::DirectConnection);
        // Ensure we start with cleared to black. The squircle's blend mode relies on this.
        win->setColor(Qt::black);
    }
}

void ScopeShader::cleanup() {
    // This function is invoked on the render thread, if there is one.
    ddb_scope_draw_data_dealloc(&m_renderer->draw_data);
    delete m_renderer;
    //ddb_scope_dealloc(scope);
    m_renderer = nullptr;
}

class CleanupJob : public QRunnable {
public:
    CleanupJob(ScopeRenderer *renderer) : m_renderer(renderer) { }
    void run() override { delete m_renderer; }
private:
    ScopeRenderer *m_renderer;
};

void ScopeShader::releaseResources() {
    window()->scheduleRenderJob(new CleanupJob(m_renderer), QQuickWindow::BeforeSynchronizingStage);
    m_renderer = nullptr;
}

// void ScopeShader::setT(qreal t) {
//     if (t == m_t)
//         return;
//     m_t = t;
//     emit tChanged();
//     if (window())
//         window()->update();
// }

void ScopeShader::process(const ddb_audio_data_t *data) {
    //qDebug() << "SCOPE SHADER PROCESS";
    //scope->mode = DDB_SCOPE_MULTICHANNEL;
    //scope->fragment_duration = 100;
    //if (m_renderer) m_renderer->fragment_dur = scope->fragment_duration;
    //scope->channels = 2;
    //ddb_scope_process(scope, data->fmt->samplerate, data->fmt->channels, data->data, data->nframes);
    //mm.lock();
    // polar
    if (m_renderer)
        config.process_draw_data(data, &m_renderer->draw_data);
        //ddb_scope_get_draw_data (scope, this->width(), 2, 0, &m_renderer->draw_data);
    //m_renderer->last_frame++;
    //ddb_scope_get_draw_data (scope, this->width(), 2, 0, &m_renderer->draw_data);
    //mm.unlock();

    // debug
    // for (int i = 0; i < m_renderer->draw_data.point_count*m_renderer->draw_data.channels; i++) {
    //     qDebug() << "i:" << i << m_renderer->draw_data.points[i].ymin << m_renderer->draw_data.points[i].ymax;
    // }

    emit dataChanged();
    //if (window()) {
    //    window()->update();
    //}
}

void ScopeShader::onDataChanged() {
    if (window()) {
        window()->update();
        //window()->requestUpdate();
        //window()->setColor(Qt::black);
        //sync();
    }
}

//! [sync]
void ScopeShader::sync()
{
    // This function is invoked on the render thread, if there is one.

    if (!m_renderer) {
        m_renderer = new ScopeRenderer;
        // Initializing resources is done after starting to record the
        // renderpass, regardless of wanting an underlay or overlay.
        connect(window(), &QQuickWindow::beforeRendering, m_renderer, &ScopeRenderer::frameStart, Qt::DirectConnection);
        // Here we want an underlay and therefore connect to
        // afterRenderPassRecording. Changing to afterRenderPassRecording
        // would render the squircle on top (overlay).
        connect(window(), &QQuickWindow::afterRenderPassRecording, m_renderer, &ScopeRenderer::mainPassRecordingStart, Qt::DirectConnection);

        m_renderer->render_height = this->bindableHeight();
        m_renderer->render_width = this->bindableWidth();
        //m_renderer->render_bottom_left.setValue(QPointF(this->x(), this->y()));//.setBinding([&]() {return QPo );

    }
    m_renderer->setItem(this);
    m_renderer->render_bottom_left = mapToScene(QPointF(0, this->height()));
    m_renderer->setWindow(window());

}
//! [sync]

static QShader getShader(const QString &name)
{
    QFile f(name);
    if (f.open(QIODevice::ReadOnly))
        return QShader::fromSerialized(f.readAll());

    return QShader();
}

/// SCOPE RENDERER

static const float vertices[] = {
    -1, -1,
     1, -1,
    -1,  1,
     1,  1
};

int counter = 0;

void ScopeRenderer::frameStart()
{
    //qDebug() << "FRAME_START" << count++ ;
    // This function is invoked on the render thread, if there is one.

    QRhi *rhi = m_window->rhi();
    if (!rhi) {
        qWarning("QQuickWindow is not using QRhi for rendering");
        return;
    }
    QRhiSwapChain *swapChain = m_window->swapChain();
    if (!swapChain) {
        qWarning("No QRhiSwapChain?");
        return;
    }
    QRhiResourceUpdateBatch *resourceUpdates = rhi->nextResourceUpdateBatch();


    if (!m_pipeline) {
        m_vertexShader = getShader(QLatin1String(":/ScopeShader.vert.qsb"));
        if (!m_vertexShader.isValid())
            qWarning("Failed to load vertex shader; rendering will be incorrect");

        m_fragmentShader = getShader(QLatin1String(":/ScopeShader.frag.qsb"));
        if (!m_fragmentShader.isValid())
            qWarning("Failed to load fragment shader; rendering will be incorrect");

        m_vertexBuffer.reset(rhi->newBuffer(QRhiBuffer::Immutable, QRhiBuffer::VertexBuffer, sizeof(vertices)));
        m_vertexBuffer->create();
        resourceUpdates->uploadStaticBuffer(m_vertexBuffer.get(), vertices);


        // default draw data
        if (!m_uniformBuffer) {
            const quint32 WIDTH = swapChain->currentPixelSize().width();
            const quint32 HEIGHT = swapChain->currentPixelSize().height();
            const quint32 CHANNELS = 2;
            const quint32 POINT_COUNT = 1; //swapChain->currentPixelSize().width();
            const quint32 UBUF_SIZE = 4 + 4 + 4  + 4 +
                                      POINT_COUNT * sizeof(ddb_scope_point_t) * CHANNELS;

            m_uniformBuffer.reset(rhi->newBuffer(QRhiBuffer::Immutable, QRhiBuffer::StorageBuffer, UBUF_SIZE));
            m_uniformBuffer->create();

            qfloat16 empty[4] = {0};
            resourceUpdates->updateDynamicBuffer(m_uniformBuffer.get(), 0, UBUF_SIZE, &empty);
            resourceUpdates->updateDynamicBuffer(m_uniformBuffer.get(), 4, 4, &POINT_COUNT);
            resourceUpdates->updateDynamicBuffer(m_uniformBuffer.get(), 8, 4, &WIDTH);
            resourceUpdates->updateDynamicBuffer(m_uniformBuffer.get(), 12, 4, &HEIGHT);
            resourceUpdates->updateDynamicBuffer(m_uniformBuffer.get(), 16, UBUF_SIZE-16, empty);
        }
        if (!m_colorBuffer) {
        // color
            const float foreground[4] = {.1568,.4667,0.6824,1.0};
            const float background[4] = {0.0,0.0,0.0,1.0};
            m_colorBuffer.reset(rhi->newBuffer(QRhiBuffer::Static, QRhiBuffer::UniformBuffer, sizeof(foreground)+sizeof(background)));
            m_colorBuffer->create();
            resourceUpdates->updateDynamicBuffer(m_colorBuffer.get(), 0, sizeof(foreground), foreground);
            resourceUpdates->updateDynamicBuffer(m_colorBuffer.get(), sizeof(foreground), sizeof(background), background);
        }



        m_srb.reset(rhi->newShaderResourceBindings());
        const auto visibleToAll = QRhiShaderResourceBinding::VertexStage | QRhiShaderResourceBinding::FragmentStage;
        m_srb->setBindings({
            QRhiShaderResourceBinding::uniformBuffer(0, visibleToAll, m_colorBuffer.get()),
            QRhiShaderResourceBinding::bufferLoad(1, visibleToAll, m_uniformBuffer.get()),

//            QRhiShaderResourceBinding::sampledTexture(1, QRhiShaderResourceBinding::FragmentStage,
//                                                      m_texture.get(), m_sampler.get())
        });
        m_srb->create();

        QRhiVertexInputLayout inputLayout;
        inputLayout.setBindings({
            { 2 * sizeof(float) }
        });
        inputLayout.setAttributes({
            { 0, 0, QRhiVertexInputAttribute::Float2, 0 }
        });

        m_pipeline.reset(rhi->newGraphicsPipeline());
        m_pipeline->setTopology(QRhiGraphicsPipeline::TriangleStrip);
        QRhiGraphicsPipeline::TargetBlend blend;
        blend.enable = true;
        blend.srcColor = QRhiGraphicsPipeline::SrcAlpha;
        blend.srcAlpha = QRhiGraphicsPipeline::SrcAlpha;
        blend.dstColor = QRhiGraphicsPipeline::One;
        blend.dstAlpha = QRhiGraphicsPipeline::One;
        m_pipeline->setTargetBlends({ blend });
        m_pipeline->setShaderStages({
            { QRhiShaderStage::Vertex, m_vertexShader },
            { QRhiShaderStage::Fragment, m_fragmentShader }
        });
        m_pipeline->setVertexInputLayout(inputLayout);
        m_pipeline->setShaderResourceBindings(m_srb.get());
        m_pipeline->setRenderPassDescriptor(swapChain->currentFrameRenderTarget()->renderPassDescriptor());
        m_pipeline->create();
    }
    else {
        qDebug() << "SHADER UPDATE";
    //if (drawn_frame < last_frame) {
    //    drawn_frame = last_frame;
        qobject_cast<ScopeShader*>(item)->config.draw_data_mut.lock();
        const quint32 WIDTH = swapChain->currentPixelSize().width();
        const quint32 HEIGHT = swapChain->currentPixelSize().height();
        const quint32 CHANNELS = draw_data.channels;
        const quint32 POINT_COUNT = draw_data.point_count; //swapChain->currentPixelSize().width();
        const quint32 UBUF_SIZE = 4 + 4 + 4  + 4 +
                                  POINT_COUNT * sizeof(ddb_scope_point_t) * CHANNELS;

        if (m_uniformBuffer->size() != UBUF_SIZE) {
            m_uniformBuffer.reset(rhi->newBuffer(QRhiBuffer::Immutable, QRhiBuffer::StorageBuffer, UBUF_SIZE));
            m_uniformBuffer->create();
        }

        resourceUpdates->updateDynamicBuffer(m_uniformBuffer.get(), 0, 4, &CHANNELS);
        resourceUpdates->updateDynamicBuffer(m_uniformBuffer.get(), 4, 4, &POINT_COUNT);
        resourceUpdates->updateDynamicBuffer(m_uniformBuffer.get(), 8, 4, &WIDTH);
        resourceUpdates->updateDynamicBuffer(m_uniformBuffer.get(), 12, 4, &HEIGHT);

            resourceUpdates->updateDynamicBuffer(m_uniformBuffer.get(), 16, UBUF_SIZE-16, draw_data.points);
        qobject_cast<ScopeShader*>(item)->config.draw_data_mut.unlock();


        // TODO handle color
        if (!m_colorBuffer) {
            // color
            const float foreground[4] = {.1568,.4667,0.6824,1.0};
            const float background[4] = {0.0,0.0,0.0,1.0};
            //m_colorBuffer.reset(rhi->newBuffer(QRhiBuffer::Static, QRhiBuffer::UniformBuffer, sizeof(foreground)+sizeof(background)));
            //m_colorBuffer->create();
            resourceUpdates->updateDynamicBuffer(m_colorBuffer.get(), 0, sizeof(foreground), foreground);
            resourceUpdates->updateDynamicBuffer(m_colorBuffer.get(), sizeof(foreground), sizeof(background), background);
        }
        //m_window->update();
    }


    //float t = m_t;
    //resourceUpdates->updateDynamicBuffer(m_uniformBuffer.get(), 0, 4, &t);

    swapChain->currentFrameCommandBuffer()->resourceUpdate(resourceUpdates);

    //m_window->requestUpdate();
    //rhi->endFrame(swapChain);

    //count = draw_data.point_count % fragment_dur;
}

void ScopeRenderer::mainPassRecordingStart()
{
    //qDebug() << "MAIN_PASS";
    // This function is invoked on the render thread, if there is one.

    QRhi *rhi = m_window->rhi();
    QRhiSwapChain *swapChain = m_window->swapChain();
    if (!rhi || !swapChain || !m_pipeline.get())
        return;


    //
    QRhiCommandBuffer *cb = m_window->swapChain()->currentFrameCommandBuffer();


    // calculate item position in window
    // also y axis has to be inverted

    float window_h = swapChain->currentFrameRenderTarget()->pixelSize().height();
    float item_h = render_height.value();
    float item_w = render_width.value();


    //cb->setViewport({0,0, 200,200});
    // bottom left
    QPointF bl = render_bottom_left; // item->mapToScene({0, item_h});
    // // invert y
    bl = {bl.x(), window_h - bl.y()};

    cb->setViewport({ (float) bl.x(), (float) bl.y(), item_w, item_h });


    cb->setGraphicsPipeline(m_pipeline.get());
    cb->setShaderResources();
    const QRhiCommandBuffer::VertexInput vbufBinding(m_vertexBuffer.get(), 0);
    cb->setVertexInput(0, 1, &vbufBinding);
    cb->draw(4);
}

#endif
