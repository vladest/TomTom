#include "qgeomapreplytomtom.h"
#include <QNetworkAccessManager>
#include <QNetworkCacheMetaData>
#include <QDateTime>

QT_BEGIN_NAMESPACE

QGeoMapReplyTomtom::QGeoMapReplyTomtom(QNetworkReply *reply, const QGeoTileSpec &spec, QObject *parent)
        : QGeoTiledMapReply(spec, parent),
        m_reply(reply)
{
    connect(m_reply,
            SIGNAL(finished()),
            this,
            SLOT(networkFinished()));

    connect(m_reply,
            SIGNAL(error(QNetworkReply::NetworkError)),
            this,
            SLOT(networkError(QNetworkReply::NetworkError)));
}

QGeoMapReplyTomtom::~QGeoMapReplyTomtom()
{
}

QNetworkReply *QGeoMapReplyTomtom::networkReply() const
{
    return m_reply;
}

void QGeoMapReplyTomtom::abort()
{
    if (!m_reply)
        return;

    m_reply->abort();
}

void QGeoMapReplyTomtom::networkFinished()
{
    if (!m_reply)
        return;

    if (m_reply->error() != QNetworkReply::NoError)
        return;

    setMapImageData(m_reply->readAll());
    setMapImageFormat("png");
    setFinished(true);

    m_reply->deleteLater();
    m_reply = 0;
}

void QGeoMapReplyTomtom::networkError(QNetworkReply::NetworkError error)
{
    if (!m_reply)
        return;

    if (error != QNetworkReply::OperationCanceledError)
        setError(QGeoTiledMapReply::CommunicationError, m_reply->errorString());
    setFinished(true);
    m_reply->deleteLater();
    m_reply = 0;
}

QT_END_NAMESPACE
