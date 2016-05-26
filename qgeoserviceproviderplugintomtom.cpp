#include "qgeoserviceproviderplugintomtom.h"
#include "qgeocodingmanagerenginetomtom.h"
#include "qgeoroutingmanagerenginetomtom.h"
#include "qplacemanagerenginetomtom.h"
#include "qgeotiledmappingmanagerenginetomtom.h"


QGeoCodingManagerEngine *QGeoServiceProviderFactoryTomtom::createGeocodingManagerEngine(
    const QVariantMap &parameters, QGeoServiceProvider::Error *error, QString *errorString) const
{
    return new QGeoCodingManagerEngineTomtom(parameters, error, errorString);
}

QGeoRoutingManagerEngine *QGeoServiceProviderFactoryTomtom::createRoutingManagerEngine(
    const QVariantMap &parameters, QGeoServiceProvider::Error *error, QString *errorString) const
{
    return new QGeoRoutingManagerEngineTomtom(parameters, error, errorString);
}

QPlaceManagerEngine *QGeoServiceProviderFactoryTomtom::createPlaceManagerEngine(
    const QVariantMap &parameters, QGeoServiceProvider::Error *error, QString *errorString) const
{
    return new QPlaceManagerEngineTomtom(parameters, error, errorString);
}

QGeoMappingManagerEngine *QGeoServiceProviderFactoryTomtom::createMappingManagerEngine(
        const QVariantMap &parameters,
        QGeoServiceProvider::Error *error,
        QString *errorString) const
{
    return new QGeoTiledMappingManagerEngineTomtom(parameters, error, errorString);
}
