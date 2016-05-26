#include "QtLocation/private/qgeocameracapabilities_p.h"
#include "qgeotiledmappingmanagerenginetomtom.h"
#include "qgeotiledmaptomtom.h"
#include "qgeotilefetchertomtom.h"
#include "QtLocation/private/qgeotilespec_p.h"
#include "QtLocation/private/qgeofiletilecache_p.h"

#include <QDebug>
#include <QDir>
#include <QVariant>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonObject>
#include <QtCore/QJsonDocument>
#include <QtCore/qmath.h>
#include <QtCore/qstandardpaths.h>

QT_BEGIN_NAMESPACE

QGeoTiledMappingManagerEngineTomtom::QGeoTiledMappingManagerEngineTomtom(const QVariantMap &parameters,
    QGeoServiceProvider::Error *error,
    QString *errorString)
    : QGeoTiledMappingManagerEngine()
{
    Q_UNUSED(error);
    Q_UNUSED(errorString);

    QGeoCameraCapabilities capabilities;

    capabilities.setMinimumZoomLevel(0.0);
    capabilities.setMaximumZoomLevel(18.0);

    setCameraCapabilities(capabilities);

    int tile = parameters.value(QStringLiteral("tomtom.maps.tilesize"), 256).toInt();

    setTileSize(QSize(tile, tile));

    QList<QGeoMapType> types;
    types << QGeoMapType(QGeoMapType::StreetMap, tr("Basic Map"), tr("Normal map view in daylight mode"), false, false, 1);
    types << QGeoMapType(QGeoMapType::HybridMap, tr("Hybrid Map"), tr("Hybrid map view in daylight mode"), false, false, 2);
    types << QGeoMapType(QGeoMapType::TransitMap, tr("Labels Map"), tr("Labels map view in daylight mode"), false, false, 3);
    types << QGeoMapType(QGeoMapType::StreetMap, tr("Basic Night Map"), tr("Normal map view in night mode"), false, true, 4);
    types << QGeoMapType(QGeoMapType::HybridMap, tr("Hybrid Night Map"), tr("Hybrid map view in night mode"), false, true, 5);
    types << QGeoMapType(QGeoMapType::TransitMap, tr("Labels Night Map"), tr("Labels map view in night mode"), false, true, 6);
    setSupportedMapTypes(types);

    QGeoTileFetcherTomtom *fetcher = new QGeoTileFetcherTomtom(parameters, this, tileSize());
    setTileFetcher(fetcher);

    m_cacheDirectory = QAbstractGeoTileCache::baseCacheDirectory() + QLatin1String("tomtom");

    QAbstractGeoTileCache *tileCache = new QGeoFileTileCache(m_cacheDirectory);
    setTileCache(tileCache);

    populateMapSchemes();
}

QGeoTiledMappingManagerEngineTomtom::~QGeoTiledMappingManagerEngineTomtom()
{
}

void QGeoTiledMappingManagerEngineTomtom::populateMapSchemes()
{
    m_mapSchemes[0] = QStringLiteral("basic.main");
    m_mapSchemes[1] = QStringLiteral("basic.main");
    m_mapSchemes[2] = QStringLiteral("hybrid.main");
    m_mapSchemes[3] = QStringLiteral("labels.main");
    m_mapSchemes[4] = QStringLiteral("basic.night");
    m_mapSchemes[5] = QStringLiteral("hybrid.night");
    m_mapSchemes[6] = QStringLiteral("labels.night");

}

QString QGeoTiledMappingManagerEngineTomtom::getScheme(int mapId)
{
    return m_mapSchemes[mapId];
}

QGeoMap *QGeoTiledMappingManagerEngineTomtom::createMap()
{
    return new QGeoTiledMapTomtom(this);
}

QT_END_NAMESPACE

