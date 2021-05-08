#ifndef CAPTURETHREAD_H
#define CAPTURETHREAD_H

#include <QThread>
#include <QMutex>
#include "v4l2capture.h"

class CaptureThread : public QThread
{
    Q_OBJECT
public:
    explicit CaptureThread(QString camera, int width, int height, QObject* parent = nullptr);
    ~CaptureThread();

signals:
    void            sendPackage(QByteArray);

protected:
    void            run();

private:
    QMutex          m_mutex;
    QString         m_camera;
    int             m_nWidth;
    int             m_nHeight;
    V4L2Capture *   m_pVideoCapturer;

    bool            initCamera();
};

#endif // CAPTURETHREAD_H
