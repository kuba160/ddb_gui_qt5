#ifndef COVERARTWRAPPER_H
#define COVERARTWRAPPER_H

#include <QImage>
#include <QFutureWatcher>

class CoverArtWrapper : public QObject {
    Q_OBJECT
public:
    static CoverArtWrapper *Instance(QObject *parent = 0);
    static void Destroy();

    void getCoverArt(const char *fname, const char *artist, const char *album);
    void getDefaultCoverArt();
    void openAndScaleCover(const char *fname);

    int defaultWidth;

private:
    CoverArtWrapper(QObject *parent = 0);
    static CoverArtWrapper *instance;

    QFutureWatcher<QImage *> coverLoadWatcher;

    void loadSettings();
    void saveSettings();

signals:
    void coverIsReady(const QImage &);

private slots:
    void onImageLoaded(int);
};

#endif // COVERARTWRAPPER_H
