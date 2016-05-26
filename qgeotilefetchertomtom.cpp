#include "qgeotilefetchertomtom.h"
#include "qgeomapreplytomtom.h"
#include "qgeotiledmaptomtom.h"
#include "qgeotiledmappingmanagerenginetomtom.h"
#include <QtLocation/private/qgeotilespec_p.h>

#include <QDebug>
#include <QSize>
#include <QDir>
#include <QUrl>
#include <QUrlQuery>
#include <QTime>
#include <QNetworkProxy>

#include <map>

QT_BEGIN_NAMESPACE

namespace
{
    char hostPrefix()
    {
        static char _hostPrefix = 'a' - 1;
        if (++_hostPrefix > 'd')
            _hostPrefix = 'a';
        return _hostPrefix;
    }
}

QGeoTileFetcherTomtom::QGeoTileFetcherTomtom(const QVariantMap &parameters,
                                           QGeoTiledMappingManagerEngineTomtom *engine,
                                           const QSize &tileSize)
:   QGeoTileFetcher(engine),
  m_networkManager(new QNetworkAccessManager(this)),
  m_engineTomtom(engine),
  m_tileSize(tileSize)
{
    m_apiKey = parameters.value(QStringLiteral("tomtom.maps.apikey")).toString();
    m_baseUri = QStringLiteral("api.tomtom.com/");
    if (parameters.contains(QStringLiteral("tomtom.useragent")))
        _userAgent = parameters.value(QStringLiteral("tomtom.useragent")).toString().toLatin1();
    else
        _userAgent = "Mozilla/5.0 (X11; Linux i586; rv:31.0) Gecko/20100101 Firefox/31.0";

    QStringList langs = QLocale::system().uiLanguages();
    if (langs.length() > 0) {
        _language = langs[0];
    }
    _versionTomtomMap            = "1";
    netRequest.setAttribute(QNetworkRequest::HttpPipeliningAllowedAttribute, true);
    netRequest.setRawHeader("Accept", "*/*");
    netRequest.setRawHeader("User-Agent", _userAgent);
}

QGeoTileFetcherTomtom::~QGeoTileFetcherTomtom()
{
}

QGeoTiledMapReply *QGeoTileFetcherTomtom::getTileImage(const QGeoTileSpec &spec)
{
    if (m_apiKey.isEmpty()) {
        QGeoTiledMapReply *reply = new QGeoTiledMapReply(QGeoTiledMapReply::UnknownError, "Set tomtom.maps.apikey with TomTom Online Maps key", this);
        emit tileError(spec, reply->errorString());
        return reply;
    }
    //qDebug() << "tomtom zoom:" << spec.zoom();
    QString surl = _getURL(spec.mapId(), spec.x(), spec.y(), spec.zoom());
    QUrl url(surl);
    //qDebug() << "maps url:" << url;
    netRequest.setUrl(url); // The extra pair of parens disambiguates this from a function declaration
    QNetworkReply *netReply = m_networkManager->get(netRequest);
    QGeoTiledMapReply *mapReply = new QGeoMapReplyTomtom(netReply, spec);

    return mapReply;
}

QString QGeoTileFetcherTomtom::_getURL(int mapId, int x, int y, int zoom)
{
    QStringList scheme = m_engineTomtom->getScheme(mapId).split(".");
    QString layer = "basic";
    QString style = "main";
    if (scheme.count() == 2) {
        layer = scheme.at(0);
        style = scheme.at(1);
    }
    return QString("http://%1.api.tomtom.com/map/%2/tile/%3/%4/%5/%6/%7.png?key=%8&tileSize=%9")
            .arg(hostPrefix()).arg(_versionTomtomMap)
            .arg(layer).arg(style)
            .arg(zoom).arg(x).arg(y)
            .arg(m_apiKey)
            .arg(m_tileSize.width());
}

void QGeoTileFetcherTomtom::_networkReplyError(QNetworkReply::NetworkError error)
{
    qWarning() << "Could not connect to tomtom maps. Error:" << error;
    if(_mapReply)
    {
        _mapReply->deleteLater();
        _mapReply = NULL;
    }
}

void QGeoTileFetcherTomtom::_replyDestroyed()
{
    _mapReply = NULL;
}


QT_END_NAMESPACE
