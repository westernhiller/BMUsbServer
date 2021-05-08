#include "capturethread.h"
#include <QDateTime>
#include <QDebug>

#include "v4l2capture.h"
//#include "turbojpeg.h"

CaptureThread::CaptureThread(QString camera, int width, int height, QObject* parent)
    : QThread(parent)
    , m_camera(camera)
    , m_nWidth(width)
    , m_nHeight(height)
    , m_pVideoCapturer(nullptr)
{
}

CaptureThread::~CaptureThread()
{
    requestInterruption();
    m_mutex.lock();
    if(m_pVideoCapturer)
    {
        m_pVideoCapturer->stopCapture();
        m_pVideoCapturer->closeDevice();
        delete m_pVideoCapturer;
        m_pVideoCapturer = nullptr;
    }
    m_mutex.unlock();
    quit();
    wait();
}

bool CaptureThread::initCamera()
{
    m_pVideoCapturer = new V4L2Capture((char *)m_camera.toStdString().c_str(), m_nWidth, m_nHeight);
    m_pVideoCapturer->openDevice();
    if(-1 == m_pVideoCapturer->initDevice(m_nWidth, m_nHeight))
    {
        QString msg = "Failed initializing camera " + m_camera;
        qDebug() << msg;
        return false;
    }
    return true;
}

void CaptureThread::run()
{
    if(!initCamera())
        return;

    m_pVideoCapturer->startCapture();
    char *yuv422frame = nullptr;
    int yuvframeSize = 0;

    while (!isInterruptionRequested())
    {
        m_mutex.lock();
        if(-1 != m_pVideoCapturer->getFrame((void **) &yuv422frame, (int *)&yuvframeSize))
        {
            QByteArray data((char*)yuv422frame, yuvframeSize);
            emit sendPackage(data);
            m_pVideoCapturer->backFrame();
        }
        m_mutex.unlock();
        usleep(10000);
    }
}


