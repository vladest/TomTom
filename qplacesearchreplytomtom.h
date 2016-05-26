#ifndef QPLACESEARCHREPLYTOMTOM_H
#define QPLACESEARCHREPLYTOMTOM_H

#include <QtLocation/QPlaceSearchReply>

QT_BEGIN_NAMESPACE

class QNetworkReply;
class QPlaceManagerEngineTomtom;
class QPlaceResult;

class QPlaceSearchReplyTomtom : public QPlaceSearchReply
{
    Q_OBJECT

public:
    QPlaceSearchReplyTomtom(const QPlaceSearchRequest &request, QNetworkReply *reply,
                          QPlaceManagerEngineTomtom *parent);
    ~QPlaceSearchReplyTomtom();

    void abort();

private slots:
    void setError(QPlaceReply::Error errorCode, const QString &errorString);
    void replyFinished();

private:
    QPlaceResult parsePlaceResult(const QJsonObject &item) const;

    QNetworkReply *m_reply;
};

QT_END_NAMESPACE

#endif // QPLACESEARCHREPLYORS_H
