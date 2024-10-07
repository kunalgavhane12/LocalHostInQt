#pragma once

#include <QObject>
#include <memory>

class QTcpServer;

class WebServer : public QObject
{
    Q_OBJECT
public:
    explicit WebServer(QObject *parent = nullptr);
    ~WebServer() override;

    void start();

private slots:
    void newConnection();

private:
    const std::unique_ptr<QTcpServer> m_server;
};
