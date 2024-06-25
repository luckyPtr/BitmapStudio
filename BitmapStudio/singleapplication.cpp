#include "singleapplication.h"
#include <QLocalSocket>
#include <QTimer>


SingleApplication::SingleApplication(int &argc, char **argv)
    :QApplication(argc, argv)
{
    if (argc > 1)
    {
        filePath = QString::fromUtf8(argv[1]);
        QTimer::singleShot(10, this, [=]() {
            emit importFile(filePath);
        });
    }

    initConnect();
}

bool SingleApplication::isRunning()
{
    return m_IsRunning;
}

void SingleApplication::initConnect()
{
    m_IsRunning = false;
    QLocalSocket socket;
    socket.connectToServer(serverName);
    bool ret = socket.waitForConnected(500);
    if (ret)
    {
        m_IsRunning = true;
        socket.write(filePath.toUtf8());
        socket.waitForBytesWritten();
        socket.disconnectFromServer();
        return;
    }

    newLocalServer();
}

void SingleApplication::newLocalServer()
{
    m_server = new QLocalServer(this);
    connect(m_server, &QLocalServer::newConnection, this, &SingleApplication::newLocalConnect);
    if (!m_server->listen(serverName)) {
        if (m_server->serverError() == QAbstractSocket::AddressInUseError) {
            QLocalServer::removeServer(serverName);
            m_server->listen(serverName);
        }
    }
}

void SingleApplication::newLocalConnect()
{
    QLocalSocket *socket = m_server->nextPendingConnection();
    if (socket)
    {
        socket->waitForReadyRead(1000);
        emit importFile(QString(socket->readAll()));

        delete socket;
    }
}
