#include "mainwindow.h"
#include <QLayout>
#include <QDateTime>

MainWindow::MainWindow(QString configFile, QWidget *parent)
    : QMainWindow(parent)
    , m_pTcpServer(nullptr)
    , m_pTcpClient(nullptr)
    , m_bValid(false)
    , m_pCapturer(nullptr)
    , m_nFrameIndex(0)
{
    QHBoxLayout* playout = new QHBoxLayout();
    m_pText = new QTextEdit();
    m_pText->setReadOnly(true);
    playout->addWidget(m_pText);
    QWidget* pWidget = new QWidget();
    pWidget->setLayout(playout);
    setCentralWidget(pWidget);

    loadConfig(configFile);
    QJsonObject networkObj = m_configJsonObj["network"].toObject();
    int port = networkObj["port"].toInt();

    m_pTcpServer = new QTcpServer();
    if(!m_pTcpServer->listen(QHostAddress::Any, port))
    {
        QString message = QString("Unable to start the server: %1.").arg(m_pTcpServer->errorString());
        displayMessage(message);
        qCritical() << "QTCPServer " << message;
        return;
    }

    connect(m_pTcpServer, SIGNAL(newConnection()), this, SLOT(newConnection()));
    QString message = QString("QTCPServer listening at port %1").arg(port);
    displayMessage(message);
    qDebug() << message;

    m_bValid = true;
}

MainWindow::~MainWindow()
{
    if(m_pTcpClient)
    {
        m_pTcpClient->close();
        m_pTcpClient->deleteLater();
    }

    m_pTcpServer->close();
    m_pTcpServer->deleteLater();
}

void MainWindow::displayMessage(QString message)
{
    QDateTime timeNow = QDateTime::currentDateTime();
    QString time = timeNow.toString("yyyy-MM-dd HH:mm:ss:\t");
    m_pText->append(time + message);
}

void MainWindow::newConnection()
{
    if(m_pTcpClient)
        m_pTcpClient->abort();

    m_pTcpClient = m_pTcpServer->nextPendingConnection();
//    connect(m_pTcpClient, SIGNAL(readyRead()), this, SLOT(readSocket()));
    connect(m_pTcpClient, SIGNAL(disconnected()), this, SLOT(discardSocket()));

    displayMessage(QString("INFO :: Client with sockd:%1 has just connected").arg(m_pTcpClient->socketDescriptor()));

    if(!m_pCapturer)
    {
        QJsonObject cameraObj = m_configJsonObj["camera"].toObject();
        QString cam = cameraObj["device"].toString();
        int width = cameraObj["width"].toInt();
        int height = cameraObj["height"].toInt();

        m_pCapturer = new CaptureThread(cam, width, height);
        connect(m_pCapturer, SIGNAL(sendPackage(QByteArray)), this, SLOT(sendPackage(QByteArray)));
        m_pCapturer->start();
        displayMessage(QString("Capturing image from %1").arg(cam));
    }
}

bool MainWindow::loadConfig(QString file)
{
    JsonFileHandle fileHandle;
    QString configJson = fileHandle.readObjectFromFile(file);
    m_configJsonObj = JsonTools::getObjectFromString(configJson);
    if(m_configJsonObj.empty())
    {
        qDebug() << QStringLiteral("配置文件解析失败, 请检查配置文件json格式");

        return false;
    }

    return true;
}

void MainWindow::readSocket()
{
//    QTcpSocket* socket = reinterpret_cast<QTcpSocket*>(sender());
}

void MainWindow::discardSocket()
{
    QTcpSocket* socket = reinterpret_cast<QTcpSocket*>(sender());
    QString message = QString("INFO :: A client %1 has disconnected.").arg(socket->socketDescriptor());
    qDebug() << message;
    displayMessage(message);
    socket->deleteLater();
    m_pTcpClient = nullptr;
    m_nFrameIndex = 0;
}


void MainWindow::sendPackage(QByteArray data)
{
    if(m_pTcpClient && m_pTcpClient->isOpen())
    {
        QString head = QString("FRAME%1#%2#").arg(m_nFrameIndex++).arg(data.size());

        m_pTcpClient->write(head.toUtf8());
        m_pTcpClient->write(data);
    }
}
