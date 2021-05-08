#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTextEdit>
#include <QTcpServer>
#include <QTcpSocket>
#include <QJsonParseError>
#include <QJsonObject>
#include "JsonFileHandle.h"
#include "JsonTools.h"
#include "capturethread.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QString configFile, QWidget *parent = 0);
    ~MainWindow();

signals:

private:
    QTcpServer *    m_pTcpServer;
    QTcpSocket *    m_pTcpClient;
    QTextEdit*      m_pText;
    bool            m_bValid;
    CaptureThread*  m_pCapturer;
    QJsonObject     m_configJsonObj;
    int             m_nFrameIndex;

    bool loadConfig(QString);
    void displayMessage(QString);

private slots:
    void newConnection();
    void readSocket();
    void discardSocket();
    void sendPackage(QByteArray);
};

#endif // MAINWINDOW_H
