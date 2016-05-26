#ifndef QGEOTILEFETCHERTOMTOM_H
#define QGEOTILEFETCHERTOMTOM_H

#include "qgeoserviceproviderplugintomtom.h"

#include <QtLocation/private/qgeotilefetcher_p.h>
#include <QMutex>
#include <QNetworkReply>

QT_BEGIN_NAMESPACE

class QGeoTiledMapReply;
class QGeoTileSpec;
class QGeoTiledMappingManagerEngine;
class QGeoTiledMappingManagerEngineTomtom;
class QNetworkAccessManager;

class QGeoTileFetcherTomtom : public QGeoTileFetcher
{
    Q_OBJECT

public:
    QGeoTileFetcherTomtom(const QVariantMap &parameters,
                         QGeoTiledMappingManagerEngineTomtom *engine, const QSize &tileSize);
    ~QGeoTileFetcherTomtom();

    QGeoTiledMapReply *getTileImage(const QGeoTileSpec &spec);

private:
    QString _getURL(int mapId, int x, int y, int zoom);

private slots:
    void _networkReplyError(QNetworkReply::NetworkError error);
    void _replyDestroyed();

private:
    Q_DISABLE_COPY(QGeoTileFetcherTomtom)

    QNetworkAccessManager *m_networkManager;

    QNetworkRequest netRequest;
    QPointer<QGeoTiledMappingManagerEngineTomtom> m_engineTomtom;
    QSize m_tileSize;
    QString m_apiKey;
    QString m_baseUri;

    int             _timeout;
    QNetworkReply*  _mapReply;
    QByteArray      _userAgent;
    QString         _language;

    // TomTom version strings
    QString         _versionTomtomMap;
};

QT_END_NAMESPACE

#endif
