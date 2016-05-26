#include "qplacemanagerenginetomtom.h"
#include "qplacesearchreplytomtom.h"
#include "qplacecategoriesreplytomtom.h"

#include <QtCore/QUrlQuery>
#include <QtCore/QXmlStreamReader>
#include <QtCore/QRegularExpression>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include <QtPositioning/QGeoCircle>
#include <QtLocation/private/unsupportedreplies_p.h>

#include <QtCore/QElapsedTimer>

namespace
{
QString SpecialPhrasesBaseUrl = QStringLiteral("http://wiki.openstreetmap.org/wiki/Special:Export/Nominatim/Special_Phrases/");

QString nameForTagKey(const QString &tagKey)
{
    if (tagKey == QLatin1String("aeroway"))
        return QPlaceManagerEngineTomtom::tr("Aeroway");
    else if (tagKey == QLatin1String("amenity"))
        return QPlaceManagerEngineTomtom::tr("Amenity");
    else if (tagKey == QLatin1String("building"))
        return QPlaceManagerEngineTomtom::tr("Building");
    else if (tagKey == QLatin1String("highway"))
        return QPlaceManagerEngineTomtom::tr("Highway");
    else if (tagKey == QLatin1String("historic"))
        return QPlaceManagerEngineTomtom::tr("Historic");
    else if (tagKey == QLatin1String("landuse"))
        return QPlaceManagerEngineTomtom::tr("Land use");
    else if (tagKey == QLatin1String("leisure"))
        return QPlaceManagerEngineTomtom::tr("Leisure");
    else if (tagKey == QLatin1String("man_made"))
        return QPlaceManagerEngineTomtom::tr("Man made");
    else if (tagKey == QLatin1String("natural"))
        return QPlaceManagerEngineTomtom::tr("Natural");
    else if (tagKey == QLatin1String("place"))
        return QPlaceManagerEngineTomtom::tr("Place");
    else if (tagKey == QLatin1String("railway"))
        return QPlaceManagerEngineTomtom::tr("Railway");
    else if (tagKey == QLatin1String("shop"))
        return QPlaceManagerEngineTomtom::tr("Shop");
    else if (tagKey == QLatin1String("tourism"))
        return QPlaceManagerEngineTomtom::tr("Tourism");
    else if (tagKey == QLatin1String("waterway"))
        return QPlaceManagerEngineTomtom::tr("Waterway");
    else
        return tagKey;
}

}

QPlaceManagerEngineTomtom::QPlaceManagerEngineTomtom(const QVariantMap &parameters,
                                               QGeoServiceProvider::Error *error,
                                               QString *errorString)
:   QPlaceManagerEngine(parameters), m_networkManager(new QNetworkAccessManager(this)),
    m_categoriesReply(0)
{
    if (parameters.contains(QStringLiteral("ors.useragent")))
        m_userAgent = parameters.value(QStringLiteral("ors.useragent")).toString().toLatin1();
    else
        m_userAgent = "Qt Location based application";

    if (parameters.contains(QStringLiteral("Ors.places.host")))
        m_urlPrefix = parameters.value(QStringLiteral("Ors.places.host")).toString();
    else
        m_urlPrefix = QStringLiteral("http://nominatim.openstreetmap.org/search");

    *error = QGeoServiceProvider::NoError;
    errorString->clear();
}

QPlaceManagerEngineTomtom::~QPlaceManagerEngineTomtom()
{
}

QPlaceSearchReply *QPlaceManagerEngineTomtom::search(const QPlaceSearchRequest &request)
{
    bool unsupported = false;

    // Only public visibility supported
    unsupported |= request.visibilityScope() != QLocation::UnspecifiedVisibility &&
                   request.visibilityScope() != QLocation::PublicVisibility;
    unsupported |= request.searchTerm().isEmpty() && request.categories().isEmpty();

    if (unsupported)
        return QPlaceManagerEngine::search(request);

    QUrlQuery queryItems;

    queryItems.addQueryItem(QStringLiteral("format"), QStringLiteral("jsonv2"));

    //queryItems.addQueryItem(QStringLiteral("accept-language"), QStringLiteral("en"));

    QGeoRectangle boundingBox;
    QGeoShape searchArea = request.searchArea();
    switch (searchArea.type()) {
    case QGeoShape::CircleType: {
        QGeoCircle c(searchArea);
        qreal radius = c.radius();
        if (radius < 0)
            radius = 50000;

        boundingBox = QGeoRectangle(c.center().atDistanceAndAzimuth(radius, -45),
                                    c.center().atDistanceAndAzimuth(radius, 135));
        break;
    }
    case QGeoShape::RectangleType:
        boundingBox = searchArea;
        break;
    default:
        ;
    }

    if (!boundingBox.isEmpty()) {
        queryItems.addQueryItem(QStringLiteral("bounded"), QStringLiteral("1"));
        QString coordinates;
        coordinates = QString::number(boundingBox.topLeft().longitude()) + QLatin1Char(',') +
                      QString::number(boundingBox.topLeft().latitude()) + QLatin1Char(',') +
                      QString::number(boundingBox.bottomRight().longitude()) + QLatin1Char(',') +
                      QString::number(boundingBox.bottomRight().latitude());
        queryItems.addQueryItem(QStringLiteral("viewbox"), coordinates);
    }

    QStringList queryParts;
    if (!request.searchTerm().isEmpty())
        queryParts.append(request.searchTerm());

    foreach (const QPlaceCategory &category, request.categories()) {
        QString id = category.categoryId();
        int index = id.indexOf(QLatin1Char('='));
        if (index != -1)
            id = id.mid(index+1);
        queryParts.append(QLatin1Char('[') + id + QLatin1Char(']'));
    }

    queryItems.addQueryItem(QStringLiteral("q"), queryParts.join(QLatin1Char('+')));

    QVariantMap parameters = request.searchContext().toMap();

    QStringList placeIds = parameters.value(QStringLiteral("ExcludePlaceIds")).toStringList();
    if (!placeIds.isEmpty())
        queryItems.addQueryItem(QStringLiteral("exclude_place_ids"), placeIds.join(QLatin1Char(',')));

    queryItems.addQueryItem(QStringLiteral("addressdetails"), QStringLiteral("1"));

    QUrl requestUrl(m_urlPrefix);
    requestUrl.setQuery(queryItems);

    QNetworkReply *networkReply = m_networkManager->get(QNetworkRequest(requestUrl));

    QPlaceSearchReplyTomtom *reply = new QPlaceSearchReplyTomtom(request, networkReply, this);
    connect(reply, SIGNAL(finished()), this, SLOT(replyFinished()));
    connect(reply, SIGNAL(error(QPlaceReply::Error,QString)),
            this, SLOT(replyError(QPlaceReply::Error,QString)));

    return reply;
}

QPlaceReply *QPlaceManagerEngineTomtom::initializeCategories()
{
    // Only fetch categories once
    if (m_categories.isEmpty() && !m_categoriesReply) {
        m_categoryLocales = m_locales;
        m_categoryLocales.append(QLocale(QLocale::English));
        fetchNextCategoryLocale();
    }

    QPlaceCategoriesReplyTomtom *reply = new QPlaceCategoriesReplyTomtom(this);
    connect(reply, SIGNAL(finished()), this, SLOT(replyFinished()));
    connect(reply, SIGNAL(error(QPlaceReply::Error,QString)),
            this, SLOT(replyError(QPlaceReply::Error,QString)));

    // TODO delayed finished() emission
    if (!m_categories.isEmpty())
        reply->emitFinished();

    m_pendingCategoriesReply.append(reply);
    return reply;
}

QString QPlaceManagerEngineTomtom::parentCategoryId(const QString &categoryId) const
{
    Q_UNUSED(categoryId)

    // Only a two category levels
    return QString();
}

QStringList QPlaceManagerEngineTomtom::childCategoryIds(const QString &categoryId) const
{
    return m_subcategories.value(categoryId);
}

QPlaceCategory QPlaceManagerEngineTomtom::category(const QString &categoryId) const
{
    return m_categories.value(categoryId);
}

QList<QPlaceCategory> QPlaceManagerEngineTomtom::childCategories(const QString &parentId) const
{
    QList<QPlaceCategory> categories;
    foreach (const QString &id, m_subcategories.value(parentId))
        categories.append(m_categories.value(id));
    return categories;
}

QList<QLocale> QPlaceManagerEngineTomtom::locales() const
{
    return m_locales;
}

void QPlaceManagerEngineTomtom::setLocales(const QList<QLocale> &locales)
{
    m_locales = locales;
}

void QPlaceManagerEngineTomtom::categoryReplyFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    if (!reply)
        return;

    reply->deleteLater();

    QXmlStreamReader parser(reply);
    while (!parser.atEnd() && parser.readNextStartElement()) {
        if (parser.name() == QLatin1String("mediawiki"))
            continue;
        if (parser.name() == QLatin1String("page"))
            continue;
        if (parser.name() == QLatin1String("revision"))
            continue;
        if (parser.name() == QLatin1String("text")) {
            // parse
            QString page = parser.readElementText();
            QRegularExpression regex(QStringLiteral("\\| ([^|]+) \\|\\| ([^|]+) \\|\\| ([^|]+) \\|\\| ([^|]+) \\|\\| ([\\-YN])"));
            QRegularExpressionMatchIterator i = regex.globalMatch(page);
            while (i.hasNext()) {
                QRegularExpressionMatch match = i.next();
                QString name = match.capturedRef(1).toString();
                QString tagKey = match.capturedRef(2).toString();
                QString tagValue = match.capturedRef(3).toString();
                QString op = match.capturedRef(4).toString();
                QString plural = match.capturedRef(5).toString();

                // Only interested in any operator plural forms
                if (op != QLatin1String("-") || plural != QLatin1String("Y"))
                    continue;

                if (!m_categories.contains(tagKey)) {
                    QPlaceCategory category;
                    category.setCategoryId(tagKey);
                    category.setName(nameForTagKey(tagKey));
                    m_categories.insert(category.categoryId(), category);
                    m_subcategories[QString()].append(tagKey);
                    emit categoryAdded(category, QString());
                }

                QPlaceCategory category;
                category.setCategoryId(tagKey + QLatin1Char('=') + tagValue);
                category.setName(name);

                if (!m_categories.contains(category.categoryId())) {
                    m_categories.insert(category.categoryId(), category);
                    m_subcategories[tagKey].append(category.categoryId());
                    emit categoryAdded(category, tagKey);
                }
            }
        }

        parser.skipCurrentElement();
    }

    if (m_categories.isEmpty() && !m_categoryLocales.isEmpty()) {
        fetchNextCategoryLocale();
        return;
    } else {
        m_categoryLocales.clear();
    }

    foreach (QPlaceCategoriesReplyTomtom *reply, m_pendingCategoriesReply)
        reply->emitFinished();
    m_pendingCategoriesReply.clear();
}

void QPlaceManagerEngineTomtom::categoryReplyError()
{
    foreach (QPlaceCategoriesReplyTomtom *reply, m_pendingCategoriesReply)
        reply->setError(QPlaceReply::CommunicationError, tr("Network request error"));
}

void QPlaceManagerEngineTomtom::replyFinished()
{
    QPlaceReply *reply = qobject_cast<QPlaceReply *>(sender());
    if (reply)
        emit finished(reply);
}

void QPlaceManagerEngineTomtom::replyError(QPlaceReply::Error errorCode, const QString &errorString)
{
    QPlaceReply *reply = qobject_cast<QPlaceReply *>(sender());
    if (reply)
        emit error(reply, errorCode, errorString);
}

void QPlaceManagerEngineTomtom::fetchNextCategoryLocale()
{
    if (m_categoryLocales.isEmpty()) {
        qWarning("No locales specified to fetch categories for");
        return;
    }

    QLocale locale = m_categoryLocales.takeFirst();

    // FIXME: Categories should be cached.
    QUrl requestUrl = QUrl(SpecialPhrasesBaseUrl + locale.name().left(2).toUpper());

    m_categoriesReply = m_networkManager->get(QNetworkRequest(requestUrl));
    connect(m_categoriesReply, SIGNAL(finished()), this, SLOT(categoryReplyFinished()));
    connect(m_categoriesReply, SIGNAL(error(QNetworkReply::NetworkError)),
            this, SLOT(categoryReplyError()));
}
