#ifndef QGEOCODEREPLYTOMTOM_H
#define QGEOCODEREPLYTOMTOM_H

#include <QtNetwork/QNetworkReply>
#include <QtLocation/QGeoCodeReply>

class QGeoCodeReplyTomtom : public QGeoCodeReply
{
    Q_OBJECT

public:
    explicit QGeoCodeReplyTomtom(QNetworkReply *reply, QObject *parent = 0);
    ~QGeoCodeReplyTomtom();

    void abort();

private Q_SLOTS:
    void networkReplyFinished();
    void networkReplyError(QNetworkReply::NetworkError error);

private:
    QNetworkReply *m_reply;
};

#endif // QGEOCODEREPLYORS_H
