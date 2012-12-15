#include "webcontact.h"

/* THIS CLASS IS A SINGLETON: */
WebContact* WebContact::singleton = NULL;

WebContact::WebContact()
{
    /* Make sure the Engine is running: */
    if (!Engine::isRunning()) {
        qCritical() << "[ERROR] Engine class called, but Engine was not started!";
        return;
    }

    if (singleton != NULL) {
        qWarning() << "WARNING: WebContact is a SINGLETON that was already instantiated! Please, use WebContact::getInstance() instead.";
        return;
    }
    isWaitingRoomsArrive = false;
    requestedInfo = false;
    singleton = this;
    contactURL = "";
    manager = new QNetworkAccessManager(this);
    connect(manager,SIGNAL(finished(QNetworkReply*)),this,SLOT(replyFinished(QNetworkReply*)), Qt::QueuedConnection);
    this->start();
}

WebContact *WebContact::getInstance()
{
    if (singleton == NULL)
        return new WebContact();
    else
        return singleton;

}

void WebContact::setURL(QString url)
{
    urlLock.lock();
    contactURL = url + "/" + URL_CONNECT_FILENAME;
    urlLock.unlock();
}

void WebContact::run()
{
    exec();
}

void WebContact::requestPublishedRooms(qint32 ipsToFetch)
{
    if (isWaitingRoomsArrive) return;

    if ( ipsToFetch < SQL_GET_IPS )
        ipsToFetch = SQL_GET_IPS;

    isWaitingRoomsArrive = true;
    /* Set a timeout to restore the break given: */
    QTimer::singleShot(REFRESH_ROOMS_BREAK_MILLISECONDS, this, SLOT(timeExpiredRoomRefresh()));

    QUrl params;
    params.addQueryItem(SQL_TYPE_PARAMETER, QString::number(ipsToFetch));
    params.addQueryItem(SQL_KEY_PARAMETER, generateHashKey(ipsToFetch));
    makeListRequest(params);
}

void WebContact::publishRoom(qint32 port)
{
    QUrl params;
    params.addQueryItem(SQL_TYPE_PARAMETER, QString::number(SQL_ADD_IP));
    params.addQueryItem(SQL_PORT_PARAMETER, QString::number(port));
    params.addQueryItem(SQL_KEY_PARAMETER, generateHashKey(port));

    makeListRequest(params);
}

void WebContact::removePublishedRoom(qint32 port)
{
    QUrl params;
    params.addQueryItem(SQL_TYPE_PARAMETER, QString::number(SQL_REMOVE_IP));
    params.addQueryItem(SQL_PORT_PARAMETER, QString::number(port));
    params.addQueryItem(SQL_KEY_PARAMETER, generateHashKey(port));
    makeListRequest(params);
}

void WebContact::requestWebInfo()
{
    if (requestedInfo) return;
    requestedInfo = true;
    QUrl params;
    params.addQueryItem(SQL_TYPE_PARAMETER, QString::number(SQL_INFO));
    makeListRequest(params);

    /* Set a timeout to restore the break given: */
    QTimer::singleShot(REFRESH_INFO_REQUEST_BREAK_MILLISECONDS, this, SLOT(timeExpiredInfoRefresh()));
}

QNetworkRequest WebContact::getRequest()
{
    QNetworkRequest request;
    request.setUrl(QUrl(contactURL));
    request.setHeader(QNetworkRequest::ContentTypeHeader,QVariant("application/x-www-form-urlencoded"));
    return request;
}

void WebContact::replyFinished(QNetworkReply *reply)
{
    QString all = reply->readAll();
    QStringList list = all.split("\n{*}");

    switch(list.at(0).toInt()) {

    case SQL_ANSWER_INFO:
        list.removeAt(0);
        qDebug() << "[WebContact] Received info: ";
        if (list.length() < LINE_TOTAL_LINES) return;
        emit infoRequested(list);
        emit locationReceived(list.at(LINE_LOCATION));
        emit ipReceived(list.at(LINE_IP));
        break;

    case SQL_ANSWER_ROOM_CLOSED:
        qDebug() << "[WebContact] The room was closed.";
        break;

    case SQL_ANSWER_FAIL:
        qDebug() << "[WebContact] Error: " << list.at(1);
        isWaitingRoomsArrive = false;
        break;

    case SQL_ANWER_LIST_FETCHED:
        list.removeAt(0);
        qDebug() << "[WebContact] Received list." << list;
        emit roomListRequested(list);
        isWaitingRoomsArrive = false;
        break;

    case SQL_ANSWER_ROOM_PUBLISHED:
        emit roomPublished();
        break;

    default :
       qDebug() << "[Webcontact] Received unexpected answer: " << all;

    }

    reply->deleteLater();
}

void WebContact::timeExpiredRoomRefresh()
{
    isWaitingRoomsArrive = false;
}

void WebContact::timeExpiredInfoRefresh()
{
     requestedInfo = false;
}


void WebContact::makeListRequest(QUrl params)
{
    /* Schedule the message to be sent: */
    manager->post(getRequest(), params.encodedQuery());
}

QString WebContact::generateHashKey(int value)
{
    QString key = QString::number(value) + Engine::getAppSecret();
    return QCryptographicHash::hash((key.toUtf8()),QCryptographicHash::Md5).toHex();
}

