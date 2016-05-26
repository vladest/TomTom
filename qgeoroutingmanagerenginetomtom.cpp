#include "qgeoroutingmanagerenginetomtom.h"
#include "qgeoroutereplytomtom.h"

#include <QtCore/QUrlQuery>

#include <QtCore/QDebug>

QGeoRoutingManagerEngineTomtom::QGeoRoutingManagerEngineTomtom(const QVariantMap &parameters,
                                                         QGeoServiceProvider::Error *error,
                                                         QString *errorString)
:   QGeoRoutingManagerEngine(parameters), m_networkManager(new QNetworkAccessManager(this))
{
    if (parameters.contains(QStringLiteral("tomtom.useragent")))
        m_userAgent = parameters.value(QStringLiteral("tomtom.useragent")).toString().toLatin1();
    else
        m_userAgent = "Qt Location based application";

    m_urlPrefix = QStringLiteral("https://api.tomtom.com/");
    m_apiKey = parameters.value(QStringLiteral("tomtom.route.apikey")).toString();

    *error = QGeoServiceProvider::NoError;
    errorString->clear();
    m_tomtomVersion = "1";
}

QGeoRoutingManagerEngineTomtom::~QGeoRoutingManagerEngineTomtom()
{
}

QGeoRouteReply* QGeoRoutingManagerEngineTomtom::calculateRoute(const QGeoRouteRequest &request)
{
    QNetworkRequest networkRequest;
    networkRequest.setRawHeader("User-Agent", m_userAgent);

    if (m_apiKey.isEmpty()) {
        QGeoRouteReply *reply = new QGeoRouteReply(QGeoRouteReply::UnsupportedOptionError, "Set tomtom.route.apikey with TomTom Route key", this);
        emit error(reply, reply->error(), reply->errorString());
        return reply;
    }

    QString _url = m_urlPrefix + "routing/" + m_tomtomVersion + "/calculateRoute/";

    QStringList waypoints;
    foreach (const QGeoCoordinate &c, request.waypoints()) {
        QString scoord = QString::number(c.latitude()) + QLatin1Char(',') + QString::number(c.longitude());
        waypoints.append(scoord);
    }
    if (waypoints.size() > 0)
        _url += waypoints.join(":");

    _url += "/json";

    QUrl url(_url);
    QUrlQuery query;

    query.addQueryItem(QStringLiteral("maxAlternatives"), QString::number(request.numberAlternativeRoutes()));

    if (request.travelModes() & QGeoRouteRequest::CarTravel)
        query.addQueryItem(QStringLiteral("travelMode"), QStringLiteral("car"));
    else if (request.travelModes() & QGeoRouteRequest::TruckTravel)
        query.addQueryItem(QStringLiteral("travelMode"), QStringLiteral("truck"));
    else if (request.travelModes() & QGeoRouteRequest::PedestrianTravel)
        query.addQueryItem(QStringLiteral("travelMode"), QStringLiteral("pedestrian"));
    else if (request.travelModes() & QGeoRouteRequest::BicycleTravel)
        query.addQueryItem(QStringLiteral("travelMode"), QStringLiteral("bicycle"));
    else if (request.travelModes() & QGeoRouteRequest::PublicTransitTravel)
        query.addQueryItem(QStringLiteral("travelMode"), QStringLiteral("bus"));

    foreach (QGeoRouteRequest::FeatureType routeFeature, request.featureTypes()) {
        QGeoRouteRequest::FeatureWeight weigth = request.featureWeight(routeFeature);
        if (weigth == QGeoRouteRequest::AvoidFeatureWeight
                || weigth == QGeoRouteRequest::DisallowFeatureWeight) {
            if (routeFeature == QGeoRouteRequest::TollFeature)
                query.addQueryItem(QStringLiteral("avoid"), QStringLiteral("tollRoads"));
            if (routeFeature == QGeoRouteRequest::HighwayFeature)
                query.addQueryItem(QStringLiteral("avoid"), QStringLiteral("motorways"));
            if (routeFeature == QGeoRouteRequest::FerryFeature)
                query.addQueryItem(QStringLiteral("avoid"), QStringLiteral("ferries"));
            if (routeFeature == QGeoRouteRequest::MotorPoolLaneFeature)
                query.addQueryItem(QStringLiteral("avoid"), QStringLiteral("carpools"));
            if (routeFeature == QGeoRouteRequest::DirtRoadFeature)
                query.addQueryItem(QStringLiteral("avoid"), QStringLiteral("unpavedRoads"));
        }
    }

    if (request.routeOptimization() & QGeoRouteRequest::FastestRoute)
        query.addQueryItem(QStringLiteral("routeType"), QStringLiteral("fastest"));
    else if (request.routeOptimization() & QGeoRouteRequest::ShortestRoute)
        query.addQueryItem(QStringLiteral("routeType"), QStringLiteral("shortest"));
    else if (request.routeOptimization() & QGeoRouteRequest::MostEconomicRoute)
        query.addQueryItem(QStringLiteral("routeType"), QStringLiteral("eco"));
    else if (request.routeOptimization() & QGeoRouteRequest::MostScenicRoute)
        query.addQueryItem(QStringLiteral("routeType"), QStringLiteral("thrilling"));

    query.addQueryItem(QStringLiteral("routeRepresentation"),
                       QStringLiteral("polyline"));

    query.addQueryItem(QStringLiteral("instructionsType"),
                       QStringLiteral("text"));

    const QLocale loc(locale());

    if (QLocale::C != loc.language() && QLocale::AnyLanguage != loc.language()) {
        query.addQueryItem(QStringLiteral("language"), loc.name());
    }

    query.addQueryItem(QStringLiteral("key"), m_apiKey);


    url.setQuery(query);
    //qDebug() << url;
    networkRequest.setUrl(url);

    QNetworkReply *reply = m_networkManager->get(networkRequest);

    QGeoRouteReplyTomtom *routeReply = new QGeoRouteReplyTomtom(reply, request, this);

    connect(routeReply, SIGNAL(finished()), this, SLOT(replyFinished()));
    connect(routeReply, SIGNAL(error(QGeoRouteReply::Error,QString)),
            this, SLOT(replyError(QGeoRouteReply::Error,QString)));

    return routeReply;
}

void QGeoRoutingManagerEngineTomtom::replyFinished()
{
    QGeoRouteReply *reply = qobject_cast<QGeoRouteReply *>(sender());
    if (reply)
        emit finished(reply);
}

void QGeoRoutingManagerEngineTomtom::replyError(QGeoRouteReply::Error errorCode,
                                             const QString &errorString)
{
    QGeoRouteReply *reply = qobject_cast<QGeoRouteReply *>(sender());
    if (reply)
        emit error(reply, errorCode, errorString);
}
