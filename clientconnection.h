#pragma once

#include <QObject>
#include <memory>
#include <QMap>
#include <QByteArray>
#include <QScreen>
#include <QGuiApplication>
#include <QImage>
#include <QBuffer>
#include <QPixmap>

class QTcpSocket;

class ClientConnection : public QObject
{
    Q_OBJECT
public:
    explicit ClientConnection(QTcpSocket *socket, QObject *parent = nullptr);
    ~ClientConnection() override;

private slots:
    void reayRead();

private:
    void sendResponse();

    const std::unique_ptr<QTcpSocket> m_socket;

    struct Request {
        std::size_t contentLength{};
        QString method;
        QString path;
        QString protocol;

        QMap<QString, QString> headers;
        QByteArray body;
    };

    Request m_request;

    enum ConnectionState {
        RequestLine,
        RequestHeaders,
        RequestBody,
        Response
    };

    ConnectionState m_state{ConnectionState::RequestLine};
};
