#include "qgeocodereplytomtom.h"

#include <QtPositioning/QGeoCoordinate>
#include <QtPositioning/QGeoAddress>
#include <QtPositioning/QGeoLocation>
#include <QtPositioning/QGeoRectangle>

#include <QtCore/QJsonDocument>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonObject>

QGeoCodeReplyTomtom::QGeoCodeReplyTomtom(QNetworkReply *reply, QObject *parent)
:   QGeoCodeReply(parent), m_reply(reply)
{
    connect(m_reply, SIGNAL(finished()), this, SLOT(networkReplyFinished()));
    connect(m_reply, SIGNAL(error(QNetworkReply::NetworkError)),
            this, SLOT(networkReplyError(QNetworkReply::NetworkError)));

    setLimit(-1);
    setOffset(0);
}

QGeoCodeReplyTomtom::~QGeoCodeReplyTomtom()
{
    if (m_reply)
        m_reply->deleteLater();
}

void QGeoCodeReplyTomtom::abort()
{
    if (!m_reply)
        return;

    m_reply->abort();

    m_reply->deleteLater();
    m_reply = 0;
}

static QGeoCoordinate constructCoordiante(const QJsonObject &jsonCoord) {
    QGeoCoordinate coord(0,0);
    coord.setLatitude(jsonCoord.value(QStringLiteral("lat")).toDouble());
    coord.setLongitude(jsonCoord.value(QStringLiteral("lon")).toDouble());
    return coord;
}

void QGeoCodeReplyTomtom::networkReplyFinished()
{
    if (!m_reply)
        return;

    if (m_reply->error() != QNetworkReply::NoError)
        return;

    QList<QGeoLocation> locations;
//        setError(QGeoCodeReply::ParseError, QStringLiteral("Error parsing OpenRouteService xml response:") + xml.errorString() + " at line: " + xml.lineNumber());

    QJsonDocument document = QJsonDocument::fromJson(m_reply->read(m_reply->bytesAvailable()));
    if (document.isObject()) {
        QJsonObject object = document.object();

        QJsonArray jsonlocations = object.value(QStringLiteral("results")).toArray();
        qDebug() << "locations:" << jsonlocations.size();
        for(int i = 0; i < jsonlocations.size(); i++) {
            QGeoLocation location;

            QJsonObject o = jsonlocations.at(i).toObject();
            QJsonObject jaddress = o.value("address").toObject();
            QGeoAddress address;
            if (!jaddress.isEmpty()) {
                address.setCity(jaddress.value("municipality").toString());
                address.setCountryCode(jaddress.value("countryCode").toString());
                address.setCounty(jaddress.value("countrySecondarySubdivision").toString());
                address.setState(jaddress.value("countrySubdivision").toString());
                address.setDistrict(jaddress.value("municipalitySubdivision").toString());
                address.setPostalCode(jaddress.value("postalCode").toString());
                address.setStreet(jaddress.value("streetName").toString() + ", " +
                                  jaddress.value("streetNumber").toString());
                address.setText(jaddress.value("freeformAddress").toString());

            }
            location.setAddress(address);
            location.setCoordinate(constructCoordiante(o.value("position").toObject()));
            QJsonObject jaddressRanges = o.value("addressRanges").toObject();
            if (!jaddressRanges.isEmpty()) {
                QGeoRectangle r;
                r.setTopLeft(constructCoordiante(jaddressRanges.value("from").toObject()));
                r.setBottomRight(constructCoordiante(jaddressRanges.value("to").toObject()));
                location.setBoundingBox(r);
            }
            locations.append(location);
        }
    }
    setLocations(locations);
    setFinished(true);

    m_reply->deleteLater();
    m_reply = 0;
}

void QGeoCodeReplyTomtom::networkReplyError(QNetworkReply::NetworkError error)
{
    Q_UNUSED(error)

    if (!m_reply)
        return;

    setError(QGeoCodeReply::CommunicationError, m_reply->errorString());

    m_reply->deleteLater();
    m_reply = 0;
}
