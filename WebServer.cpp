#include "WebServer.h"

WebServer::WebServer(QObject *parent)
    : QObject{parent}
{
    _webSocketServer = new QWebSocketServer(QStringLiteral("MyServer"), QWebSocketServer::NonSecureMode, parent);
    if (_webSocketServer->listen(QHostAddress::Any, WEB_PORT)) { connect(_webSocketServer, &QWebSocketServer::newConnection, this, &WebServer::onNewConnection); }
}

WebServer::~WebServer()
{
    _webSocketServer->close();
    qDeleteAll(_clients.begin(), _clients.end());
}

void WebServer::sendData(QString data)
{
    for (QWebSocket* client : _clients) {
        if (client->isValid()) {
            client->sendTextMessage(data);
            qDebug() << "WEBSERVER FRAME SENT: " <<  data;
        }
    }
}

void WebServer::onNewConnection()
{
    QWebSocket* webSocket = _webSocketServer->nextPendingConnection();
    if (webSocket) {
        qDebug() << "Web server connected";
        connect(webSocket, &QWebSocket::textMessageReceived, this, &WebServer::processMessage);
        connect(webSocket, &QWebSocket::disconnected, this, &WebServer::onSocketDisconnected);
        _clients.append(webSocket);
    }
}

void WebServer::processMessage(QString message)
{
    QWebSocket* client = qobject_cast<QWebSocket*>(sender());
    if (client) { emit messageReceived(message); }
}

void WebServer::onSocketDisconnected()
{
    QWebSocket* webSocket = qobject_cast<QWebSocket*>(sender());
    if (webSocket) {
        _clients.removeAll(webSocket);
        webSocket->deleteLater();
    }
}
