#ifndef QGEOMAPTOMTOM_H
#define QGEOMAPTOMTOM_H

#include "QtLocation/private/qgeotiledmap_p.h"
#include <QtGui/QImage>
#include <QtCore/QPointer>

QT_BEGIN_NAMESPACE

class QGeoTiledMappingManagerEngineTomtom;

class QGeoTiledMapTomtom: public QGeoTiledMap
{
Q_OBJECT
public:
    QGeoTiledMapTomtom(QGeoTiledMappingManagerEngineTomtom *engine, QObject *parent = 0);
    ~QGeoTiledMapTomtom();

    QString getViewCopyright();
    void evaluateCopyrights(const QSet<QGeoTileSpec> &visibleTiles);

private:
    QImage m_copyrightsSlab;
    QString m_lastCopyrightsString;
    QPointer<QGeoTiledMappingManagerEngineTomtom> m_engine;

    Q_DISABLE_COPY(QGeoTiledMapTomtom)
};

QT_END_NAMESPACE

#endif // QGEOMAPTOMTOM_H
