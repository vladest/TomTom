#include "qplacecategoriesreplytomtom.h"

QPlaceCategoriesReplyTomtom::QPlaceCategoriesReplyTomtom(QObject *parent)
:   QPlaceReply(parent)
{
}

QPlaceCategoriesReplyTomtom::~QPlaceCategoriesReplyTomtom()
{
}

void QPlaceCategoriesReplyTomtom::emitFinished()
{
    setFinished(true);
    emit finished();
}

void QPlaceCategoriesReplyTomtom::setError(QPlaceReply::Error errorCode, const QString &errorString)
{
    QPlaceReply::setError(errorCode, errorString);
    emit error(errorCode, errorString);
}
