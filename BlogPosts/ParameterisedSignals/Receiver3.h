#pragma once

#include <QObject>
#include <QtGlobal>
class QCanBusFrame;

class Receiver3 : public QObject
{
    Q_OBJECT
public:
    Receiver3(quint8 deviceId, QObject *parent = nullptr);

    int relevantMessageCount() const;

public slots:
    void onNewMessage(const QCanBusFrame &frame);

private:
    const quint8 c_deviceId;
    int m_relevantMessageCount;
};
