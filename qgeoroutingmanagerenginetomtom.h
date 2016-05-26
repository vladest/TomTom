#ifndef QGEOROUTINGMANAGERENGINETOMTOM_H
#define QGEOROUTINGMANAGERENGINETOMTOM_H

#include <QtLocation/QGeoServiceProvider>
#include <QtLocation/QGeoRoutingManagerEngine>

class QNetworkAccessManager;

class QGeoRoutingManagerEngineTomtom : public QGeoRoutingManagerEngine
{
    Q_OBJECT

public:
    QGeoRoutingManagerEngineTomtom(const QVariantMap &parameters,
                                QGeoServiceProvider::Error *error,
                                QString *errorString);
    ~QGeoRoutingManagerEngineTomtom();

    QGeoRouteReply *calculateRoute(const QGeoRouteRequest &request);

private Q_SLOTS:
    void replyFinished();
    void replyError(QGeoRouteReply::Error errorCode, const QString &errorString);

private:
    QNetworkAccessManager *m_networkManager;
    QByteArray m_userAgent;
    QString m_urlPrefix;
    QString m_apiKey;
    QString m_tomtomVersion;
};

#endif // QGEOROUTINGMANAGERENGINETOMTOM_H

