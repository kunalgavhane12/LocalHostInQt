#include "clientconnection.h"

#include <QRegularExpression>
#include <QTcpSocket>

ClientConnection::ClientConnection(QTcpSocket *socket, QObject *parent)
    : QObject{parent}, m_socket(socket)
{
    connect(m_socket.get(), &QAbstractSocket::disconnected, this, &QObject::deleteLater);
    connect(m_socket.get(), &QIODevice::readyRead, this, &ClientConnection::reayRead);
}

ClientConnection::~ClientConnection() = default;

void ClientConnection::reayRead()
{
    while ((m_state == ConnectionState::RequestLine || m_state == ConnectionState::RequestHeaders) &&
           m_socket->canReadLine())
    {
        QByteArray line = m_socket->readLine();

        if (line.endsWith("\r\n"))
            line.chop(2);

        if (m_state == ConnectionState::RequestLine)
        {
            const auto parts = line.split(' ');
            if (parts.size() != 3)
            {
                qWarning() << "Request line doesn't consist of 3 parts:" << line;
                deleteLater();
                return;
            }

            m_request.method = parts.at(0);
            m_request.path = parts.at(1);
            m_request.protocol = parts.at(2);

            m_state = ConnectionState::RequestHeaders;
            continue;
        }
        else
        {
            if (line.isEmpty())
            {
                if (m_request.contentLength > 0)
                {
                    m_state = ConnectionState::RequestBody;
                    continue;
                }
                else
                {
                    m_state = ConnectionState::Response;
                    sendResponse();
                }
            }

            static const QRegularExpression expr("^([^:]+): +(.*)$");
            const auto match = expr.match(line);
            if (!match.hasMatch())
            {
                qWarning() << "Could not parse header:" << line;
                return;
            }

            const auto name = match.captured(1);
            const auto value = match.captured(2);

            m_request.headers[name] = value;

            if (name.compare("Content-Length", Qt::CaseInsensitive) == 0)
            {
                bool ok;
                m_request.contentLength = value.toInt(&ok);
                if (!ok)
                {
                    qWarning() << "Could not parse Content-Length:" << line;
                    deleteLater();
                    return;
                }
            }
            continue;
        }
    }

    if (m_state == ConnectionState::RequestBody)
    {
        const qsizetype bytesToRead = m_request.contentLength - m_request.body.size();
        if (bytesToRead > 0)
        {
            QByteArray data = m_socket->read(bytesToRead);
            m_request.body.append(data);
        }

        Q_ASSERT(static_cast<qsizetype>(m_request.body.size()) <= m_request.contentLength);

        if (static_cast<qsizetype>(m_request.body.size()) == m_request.contentLength)
        {
            m_state = ConnectionState::Response;
            sendResponse();
            return;
        }
    }
}

void ClientConnection::sendResponse()
{
    QString content;
    content += "<h1>Hello!</h1>";
    content += "<h1>How Are You</h1>";
    content += "<h1>How Dumb you are?</h1>";
    // content += "<table><thead><tr><th>Name</th><th>Value</th></tr></thead><tbody>";
    // for(auto itr = m_request.headers.begin(); itr != m_request.headers.end(); itr++)
    //     content += QString("<tr><td>%0</td><td>%1</td></tr>")
    //                    .arg(itr.key().toHtmlEscaped(), itr.value().toHtmlEscaped());
    // content += "</tbody></table>";
    // content += "<form method =\"post\">"
    //            "<input type = \"text\" />"
    //            "<button type=\"submit\">SEND</button>"
    //            "</form>";

    const auto encoded = content.toUtf8();

    m_socket->write("HTTP/1.1 200 OK\r\n");
    m_socket->write("Content-Type: text/html\r\n");
    m_socket->write(QString("Content-Length: %0\r\n").arg(encoded.size()).toUtf8());
    m_socket->write("\r\n");
    m_socket->write(encoded);

    m_state = ConnectionState::RequestLine;
    m_request = {}; // Reset request
}

// void ClientConnection::sendResponse()
// {
//     m_socket->write("HTTP/1.1 200 OK\r\n");
//     m_socket->write("Content-Type: text/html\r\n");
//     m_socket->write("Content-Length: 13\r\n");
//     m_socket->write("\r\n");
//     m_socket->write("Kunal Gavhane");

//     m_state = ConnectionState::RequestLine;
//     m_request = {}; // Reset request
// }
