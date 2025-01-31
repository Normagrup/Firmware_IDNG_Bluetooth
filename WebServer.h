#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <QObject>
#include <QWebSocketServer>
#include <QWebSocket>

#define WEB_PORT        4322

class WebServer : public QObject
{
    Q_OBJECT
public:
    explicit WebServer(QObject *parent = nullptr);
    ~WebServer();

    void sendData(QString data);

signals:
    bool messageReceived(QString message);

private slots:
    void onNewConnection(void);
    void processMessage(QString message);
    void onSocketDisconnected(void);

private:
    QWebSocketServer* _webSocketServer;
    QList<QWebSocket*> _clients;
};

#endif // WEBSERVER_H
