#ifndef QGEOTILEDMAPPINGMANAGERENGINETOMTOM_H
#define QGEOTILEDMAPPINGMANAGERENGINETOMTOM_H

#include "QtLocation/private/qgeotiledmappingmanagerengine_p.h"
#include <QtPositioning/QGeoRectangle>
#include "QtLocation/private/qgeomaptype_p.h"

#include <QGeoServiceProvider>

#include <QList>
#include <QHash>
#include <QSet>

QT_BEGIN_NAMESPACE

class QByteArray;
class QGeoTileSpec;
class QGeoNetworkAccessManager;

class QGeoTiledMappingManagerEngineTomtom : public QGeoTiledMappingManagerEngine
{
    Q_OBJECT

public:
    QGeoTiledMappingManagerEngineTomtom(const QVariantMap &parameters,
                                       QGeoServiceProvider::Error *error,
                                       QString *errorString);
    ~QGeoTiledMappingManagerEngineTomtom();

    virtual QGeoMap *createMap();
    QString getScheme(int mapId);

private:
    void initialize();
    void populateMapSchemes();

    QHash<int, QString> m_mapSchemes;
    QString m_cacheDirectory;
};

QT_END_NAMESPACE

#endif // QGEOTILEDMAPPINGMANAGERENGINETOMTOM_H
