#include "qgeocodingmanagerenginetomtom.h"
#include "qgeocodereplytomtom.h"

#include <QtCore/QVariantMap>
#include <QtCore/QUrl>
#include <QtCore/QUrlQuery>
#include <QtCore/QLocale>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtPositioning/QGeoCoordinate>
#include <QtPositioning/QGeoAddress>
#include <QtPositioning/QGeoShape>
#include <QtPositioning/QGeoCircle>
#include <QtPositioning/QGeoRectangle>

static QString addressToQuery(const QGeoAddress &address)
{
    return address.street() + QStringLiteral(", ") +
           address.district() + QStringLiteral(", ") +
           address.city() + QStringLiteral(", ") +
           address.state() + QStringLiteral(", ") +
           address.country();
}

static QString coordinateToQuery(const QGeoCoordinate &coordinate)
{
    return QString::number(coordinate.latitude()) + QStringLiteral(", ") +
           QString::number(coordinate.longitude());
}


QGeoCodingManagerEngineTomtom::QGeoCodingManagerEngineTomtom(const QVariantMap &parameters,
                                                       QGeoServiceProvider::Error *error,
                                                       QString *errorString)
:   QGeoCodingManagerEngine(parameters), m_networkManager(new QNetworkAccessManager(this))
{
    if (parameters.contains(QStringLiteral("tomtom.useragent")))
        m_userAgent = parameters.value(QStringLiteral("tomtom.useragent")).toString().toLatin1();
    else
        m_userAgent = "Qt Location based application";

    m_urlPrefix = QStringLiteral("https://api.tomtom.com");
    m_apiKey = parameters.value(QStringLiteral("tomtom.search.apikey")).toString();

    *error = QGeoServiceProvider::NoError;
    errorString->clear();
    m_versionNumber = "1";
}

QGeoCodingManagerEngineTomtom::~QGeoCodingManagerEngineTomtom()
{
}

QGeoCodeReply *QGeoCodingManagerEngineTomtom::geocode(const QGeoAddress &address, const QGeoShape &bounds)
{
    return geocode(addressToQuery(address), -1, -1, bounds);
}

QGeoCodeReply *QGeoCodingManagerEngineTomtom::geocode(const QString &address, int limit, int offset, const QGeoShape &bounds)
{

    QNetworkRequest request;
    request.setRawHeader("User-Agent", m_userAgent);

    QUrl url(QString("%1/search/%2/geocode/%3.json")
             .arg(m_urlPrefix)
             .arg(m_versionNumber)
             .arg(address));
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("key"), m_apiKey);
    if (limit > 0)
        query.addQueryItem(QStringLiteral("limit"), QString::number(limit));
    if (offset > 0)
        query.addQueryItem(QStringLiteral("ofs"), QString::number(offset));

    if (bounds.isValid() && !bounds.isEmpty() && bounds.type() != QGeoShape::UnknownType) {
        if (bounds.type() == QGeoShape::CircleType) {
            const QGeoCircle &c = static_cast<const QGeoCircle&>(bounds);
            query.addQueryItem(QStringLiteral("radius"), QString::number(c.radius()));
        } else if (bounds.type() == QGeoShape::RectangleType) {
            const QGeoRectangle &r = static_cast<const QGeoRectangle&>(bounds);
            query.addQueryItem(QStringLiteral("topLeft"), coordinateToQuery(r.topLeft()));
            query.addQueryItem(QStringLiteral("btmRight"), coordinateToQuery(r.bottomRight()));
        }
    }

    url.setQuery(query);
    request.setUrl(url);

    QNetworkReply *reply = m_networkManager->get(request);

    QGeoCodeReplyTomtom *geocodeReply = new QGeoCodeReplyTomtom(reply, this);

    connect(geocodeReply, SIGNAL(finished()), this, SLOT(replyFinished()));
    connect(geocodeReply, SIGNAL(error(QGeoCodeReply::Error,QString)),
            this, SLOT(replyError(QGeoCodeReply::Error,QString)));

    return geocodeReply;
}

QGeoCodeReply *QGeoCodingManagerEngineTomtom::reverseGeocode(const QGeoCoordinate &coordinate,
                                                          const QGeoShape &bounds) {
    QNetworkRequest request;
    request.setRawHeader("User-Agent", m_userAgent);

    QUrl url(QString("%1/search/%2/reverseGeocode/%3.json")
             .arg(m_urlPrefix)
             .arg(m_versionNumber)
             .arg(coordinateToQuery(coordinate)));
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("key"), m_apiKey);
    query.addQueryItem(QStringLiteral("limit"), QString::number(100)); //TODO: any parameter for this?

    if (bounds.isValid() && !bounds.isEmpty() && bounds.type() != QGeoShape::UnknownType) {
        if (bounds.type() == QGeoShape::CircleType) {
            const QGeoCircle &c = static_cast<const QGeoCircle&>(bounds);
            query.addQueryItem(QStringLiteral("radius"), QString::number(c.radius()));
        } else if (bounds.type() == QGeoShape::RectangleType) {
            const QGeoRectangle &r = static_cast<const QGeoRectangle&>(bounds);
            double w = r.topLeft().distanceTo(r.topRight());
            double h = r.topLeft().distanceTo(r.bottomLeft());
            query.addQueryItem(QStringLiteral("radius"), QString::number(qMin(w,h)/2.0));
        }
    }
    url.setQuery(query);
    request.setUrl(url);

    QNetworkReply *reply = m_networkManager->get(request);

    QGeoCodeReplyTomtom *geocodeReply = new QGeoCodeReplyTomtom(reply, this);

    connect(geocodeReply, SIGNAL(finished()), this, SLOT(replyFinished()));
    connect(geocodeReply, SIGNAL(error(QGeoCodeReply::Error,QString)),
            this, SLOT(replyError(QGeoCodeReply::Error,QString)));

    return geocodeReply;
}

void QGeoCodingManagerEngineTomtom::replyFinished()
{
    QGeoCodeReply *reply = qobject_cast<QGeoCodeReply *>(sender());
    if (reply)
        emit finished(reply);
}

void QGeoCodingManagerEngineTomtom::replyError(QGeoCodeReply::Error errorCode, const QString &errorString)
{
    QGeoCodeReply *reply = qobject_cast<QGeoCodeReply *>(sender());
    if (reply)
        emit error(reply, errorCode, errorString);
}
