#include "qgeoroutereplytomtom.h"
#include <QDebug>
#include <QGeoRectangle>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonObject>
#include <QtLocation/QGeoRouteSegment>
#include <QtLocation/QGeoManeuver>

static QList<QGeoCoordinate> parsePolyline(const QByteArray &data)
{
    QList<QGeoCoordinate> path;

    bool parsingLatitude = true;

    int shift = 0;
    int value = 0;

    QGeoCoordinate coord(0, 0);

    for (int i = 0; i < data.length(); ++i) {
        unsigned char c = data.at(i) - 63;

        value |= (c & 0x1f) << shift;
        shift += 5;

        // another chunk
        if (c & 0x20)
            continue;

        int diff = (value & 1) ? ~(value >> 1) : (value >> 1);

        if (parsingLatitude) {
            coord.setLatitude(coord.latitude() + (double)diff/1e5);
        } else {
            coord.setLongitude(coord.longitude() + (double)diff/1e5);
            path.append(coord);
        }

        parsingLatitude = !parsingLatitude;

        value = 0;
        shift = 0;
    }

    return path;
}

static QGeoCoordinate constructCoordiante(const QJsonObject &jsonCoord) {
    QGeoCoordinate coord(0,0);
    coord.setLatitude(jsonCoord.value(QStringLiteral("latitude")).toDouble());
    coord.setLongitude(jsonCoord.value(QStringLiteral("longitude")).toDouble());
    return coord;
}

/*
ferry-train
ferry
*/

static QGeoManeuver::InstructionDirection tomtomInstructionDirection(const QString &instructionCode)
{
    if (instructionCode == "STRAIGHT")
        return QGeoManeuver::DirectionForward;
    else if (instructionCode == "KEEP_RIGHT")
        return QGeoManeuver::DirectionLightRight;
    else if (instructionCode == "KEEP_LEFT")
        return QGeoManeuver::DirectionLightLeft;
    else if (instructionCode == "TURN_RIGHT" ||
             instructionCode == "ROUNDABOUT_RIGHT")
        return QGeoManeuver::DirectionRight;
    else if (instructionCode == "TURN_LEFT" ||
             instructionCode == "ROUNDABOUT_LEFT")
        return QGeoManeuver::DirectionLeft;
    else if (instructionCode == "SHARP_RIGHT")
        return QGeoManeuver::DirectionHardRight;
    else if (instructionCode == "SHARP_LEFT")
        return QGeoManeuver::DirectionHardLeft;
    else if (instructionCode == "uturn-right")
        return QGeoManeuver::DirectionUTurnRight;
    else if (instructionCode == "MAKE_UTURN")
        return QGeoManeuver::DirectionUTurnLeft;
    else if (instructionCode == "BEAR_RIGHT" ||
             instructionCode == "MOTORWAY_EXIT_RIGHT")
        return QGeoManeuver::DirectionBearRight;
    else if (instructionCode == "BEAR_LEFT" ||
             instructionCode == "MOTORWAY_EXIT_LEFT")
        return QGeoManeuver::DirectionBearLeft;
    else
        return QGeoManeuver::DirectionForward;
}


QGeoRouteReplyTomtom::QGeoRouteReplyTomtom(QNetworkReply *reply, const QGeoRouteRequest &request,
                                     QObject *parent)
    :   QGeoRouteReply(request, parent), m_reply(reply)
{
    connect(m_reply, SIGNAL(finished()), this, SLOT(networkReplyFinished()));
    connect(m_reply, SIGNAL(error(QNetworkReply::NetworkError)),
            this, SLOT(networkReplyError(QNetworkReply::NetworkError)));
}

QGeoRouteReplyTomtom::~QGeoRouteReplyTomtom()
{
    if (m_reply)
        m_reply->deleteLater();
}

void QGeoRouteReplyTomtom::abort()
{
    if (!m_reply)
        return;

    m_reply->abort();

    m_reply->deleteLater();
    m_reply = 0;
}

void QGeoRouteReplyTomtom::networkReplyFinished()
{
    if (!m_reply)
        return;

    if (m_reply->error() != QNetworkReply::NoError) {
        setError(QGeoRouteReply::CommunicationError, m_reply->errorString());
        m_reply->deleteLater();
        m_reply = 0;
        return;
    }
    QList<QGeoRoute> routes;

    QJsonDocument document = QJsonDocument::fromJson(m_reply->read(m_reply->bytesAvailable()));
    if (document.isObject()) {
        QJsonObject object = document.object();

        QString status = object.value(QStringLiteral("statusCode")).toString();
        QJsonObject errorObject = object.value(QStringLiteral("error")).toObject();

        // status code is OK in case of success
        // an error occurred when trying to find a route
        if (!errorObject.isEmpty()) {
            setError(QGeoRouteReply::UnknownError, errorObject.value("description").toString());
            m_reply->deleteLater();
            m_reply = 0;
            return;
        }

        QJsonArray jsonroutes = object.value(QStringLiteral("routes")).toArray();
        //qDebug() << "routes:" << jsonroutes.size();
        for(int i = 0; i < jsonroutes.size(); i++) {
            QGeoRoute route;

            QJsonObject o = jsonroutes.at(i).toObject();

            QJsonObject summary = o.value(QStringLiteral("summary")).toObject();

            route.setDistance(summary.value("lengthInMeters").toDouble());
            route.setTravelTime(summary.value("travelTimeInSeconds").toDouble());
            QList<QGeoCoordinate> path;

            QJsonArray legs = o.value(QStringLiteral("legs")).toArray();
            for(int l = 0; l < legs.size(); l++) {
                QJsonObject lego = legs.at(l).toObject();
                QJsonObject legsummary = lego.value(QStringLiteral("summary")).toObject();
                QJsonArray legpoints = lego.value(QStringLiteral("points")).toArray();
                for(int lp = 0; lp < legpoints.size(); lp++) {
                    path.append(constructCoordiante(legpoints.at(lp).toObject()));
                }
            }

            QList<QGeoRouteSegment> segments;
            QJsonObject guidanceo = o.value(QStringLiteral("guidance")).toObject();
            QJsonArray instructions = guidanceo.value(QStringLiteral("instructions")).toArray();
            for(int ins = 0; ins < instructions.count(); ins++) {
                QJsonObject instruction = instructions.at(ins).toObject();
                QGeoRouteSegment segment;
                QGeoManeuver maneuver;
                double distance = instruction.value("routeOffsetInMeters").toDouble();
                double segmentTime = instruction.value("travelTimeInSeconds").toDouble();

                QGeoCoordinate maneurPos = constructCoordiante(instruction.value("point").toObject());
                QString maneuverCode = instruction.value("maneuver").toString();
                QString instructionText = instruction.value("message").toString();
                maneuver.setPosition(maneurPos);
                maneuver.setWaypoint(maneurPos);
                maneuver.setDirection(tomtomInstructionDirection(maneuverCode));
                maneuver.setInstructionText(instructionText);

                maneuver.setDistanceToNextInstruction(distance);
                maneuver.setTimeToNextInstruction(segmentTime);

                segment.setManeuver(maneuver);
                segments.append(segment);
            }

            for (int segcnt = 0; segcnt < segments.count(); segcnt++) {
                QList<QGeoCoordinate> steppath;
                QGeoRouteSegment segment = segments.at(segcnt);

                if (segcnt < segments.count() - 1) {
                    QGeoRouteSegment nextsegment = segments.at(segcnt + 1);
                    segment.setNextRouteSegment(nextsegment);
                    //fillup segment path
                    bool foundFirst = false;
                    for (int pathcount = 0; pathcount < path.count(); pathcount++) {
                        if (path.at(pathcount) == segment.maneuver().position())
                            foundFirst = true;
                        if (path.at(pathcount) == nextsegment.maneuver().position()) {
                            //found last
                            steppath.append(path.at(pathcount));
                            break;
                        }
                        if (foundFirst)
                            steppath.append(path.at(pathcount));
                    }
                    segment.setPath(steppath);
                    //correct time and distance
                    segment.setDistance(nextsegment.maneuver().distanceToNextInstruction() - segment.maneuver().distanceToNextInstruction());
                    segment.maneuver().setDistanceToNextInstruction(segment.distance());
                    segment.setTravelTime(nextsegment.maneuver().timeToNextInstruction() - segment.maneuver().timeToNextInstruction());
                    segment.maneuver().setTimeToNextInstruction(segment.travelTime());
                    //qDebug() << "set path to seg" << segcnt << "path steps" << steppath.count() << "total path" << path.count();
                }
                if (segcnt == 0) {
                    route.setFirstRouteSegment(segment);
                }
            }

            QGeoRectangle r(path.first(),path.last()); //TODO: check for sides of the world
            route.setBounds(r);
            route.setPath(path);
            routes.append(route);
        }

        setRoutes(routes);
        setFinished(true);
    }
    else {
        setError(QGeoRouteReply::ParseError, QStringLiteral("Error parsing  Tomtom JSON response:"));
    }

    m_reply->deleteLater();
    m_reply = 0;
}

void QGeoRouteReplyTomtom::networkReplyError(QNetworkReply::NetworkError error)
{
    Q_UNUSED(error)

    if (!m_reply)
        return;

    setError(QGeoRouteReply::CommunicationError, m_reply->errorString());

    m_reply->deleteLater();
    m_reply = 0;
}
