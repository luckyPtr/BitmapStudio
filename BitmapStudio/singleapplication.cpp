#include "singleapplication.h"
#include <QLocalSocket>
#include <QDebug>

SingleApplication::SingleApplication(int &argc, char **argv)
    :QApplication(argc, argv)
{
    if (argc > 1)
    {
        filePath = QString::fromUtf8(argv[1]);
    }

    // m_server_name = QFileInfo(QCoreApplication::applicationFilePath()).fileName();
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
    socket.connectToServer("BitmapStudio");
    bool ret = socket.waitForConnected(500);
    if (ret)
    {
        m_IsRunning = true;
        socket.write(filePath.toUtf8());
        socket.waitForBytesWritten();
        socket.disconnectFromServer();
        qDebug() << "running...";
        return;
    }

    newLocalServer();
}

void SingleApplication::newLocalServer()
{
    qDebug() << "newLocalServer";

    m_server = new QLocalServer(this);
    connect(m_server, &QLocalServer::newConnection, this, &SingleApplication::newLocalConnect);
    if (!m_server->listen("BitmapStudio")) {
        if (m_server->serverError() == QAbstractSocket::AddressInUseError) {
            QLocalServer::removeServer("BitmapStudio");
            m_server->listen("BitmapStudio");
        }
    }
}

void SingleApplication::newLocalConnect()
{
    qDebug() << "newLocalConnect";

    QLocalSocket *socket = m_server->nextPendingConnection();
    if (socket)
    {
        socket->waitForReadyRead(1000);
        emit importFile(QString(socket->readAll()));

        delete socket;
    }
}
