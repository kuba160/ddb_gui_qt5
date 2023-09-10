#ifndef COVERARTCACHE_H
#define COVERARTCACHE_H


class CoverArtCache : public CoverArt
{
    Q_OBJECT
public:
    explicit CoverArtCache(QObject *parent = nullptr);
};

#endif // COVERARTCACHE_H
