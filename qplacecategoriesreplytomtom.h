#ifndef QPLACECATEGORIESREPLYTOMTOM_H
#define QPLACECATEGORIESREPLYTOMTOM_H

#include <QtLocation/QPlaceReply>

class QPlaceCategoriesReplyTomtom : public QPlaceReply
{
    Q_OBJECT

public:
    explicit QPlaceCategoriesReplyTomtom(QObject *parent = 0);
    ~QPlaceCategoriesReplyTomtom();

    void emitFinished();
    void setError(QPlaceReply::Error errorCode, const QString &errorString);
};

#endif // QPLACECATEGORIESREPLYTOMTOM_H
