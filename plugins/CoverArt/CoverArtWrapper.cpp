#include "CoverArtWrapper.h"

#include <QtConcurrentRun>

#include "QtGuiSettings.h"
#include "QtGui.h"
#include <include/callbacks.h>

CoverArtWrapper *CoverArtWrapper::instance;

CoverArtWrapper::CoverArtWrapper(QObject *parent) : coverLoadWatcher(this) {
    connect(&coverLoadWatcher, SIGNAL(resultReadyAt(int)), this, SLOT(onImageLoaded(int)));
    loadSettings();
}

void CoverArtWrapper::loadSettings() {
    QSettings settings;
    settings.beginGroup("CoverArt");
    defaultWidth = settings.value("maximum_width", 100).toInt();
    settings.endGroup();
}

void CoverArtWrapper::saveSettings() {
    QSettings settings;
    settings.beginGroup("CoverArt");
    settings.setValue("maximum_width", defaultWidth);
    settings.endGroup();
}

void CoverArtWrapper::Destroy() {
    CoverArtWrapper::Instance()->saveSettings();
    if (COVERART)
        COVERART->reset(1);
    delete instance;
    instance = NULL;
}

CoverArtWrapper *CoverArtWrapper::Instance(QObject *parent) {
    if (instance == NULL) {
        instance = new CoverArtWrapper(parent);
    }
    return instance;
}

void CoverArtWrapper::onImageLoaded(int num) {
    emit coverIsReady(*coverLoadWatcher.resultAt(num));
}

void CoverArtWrapper::getCoverArt(const char *fname, const char *artist, const char *album) {
    if (!COVERART)
        return;
    char *image_fname = COVERART->get_album_art(fname, artist, album, -1, CALLBACK(&cover_avail_callback), NULL);
    if (image_fname) {
        coverLoadWatcher.setFuture(QtConcurrent::run(scale, image_fname));
    }
}

void CoverArtWrapper::getDefaultCoverArt() {
    if (!COVERART) {
        return;
    }
    const char *image_fname = COVERART->get_default_cover();
    if (image_fname) {
        coverLoadWatcher.setFuture(QtConcurrent::run(scale, image_fname));
    }
}

void CoverArtWrapper::openAndScaleCover(const char *fname) {
    coverLoadWatcher.setFuture(QtConcurrent::run(scale, fname));
}
