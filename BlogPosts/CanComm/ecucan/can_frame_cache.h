// Copyright (C) 2019, Burkhard Stubert (DBA Embedded Use)

#pragma once

#include <QCanBusFrame>
#include <QMap>
#include <QObject>
#include <QSet>
#include <QTimer>
#include <QVector>

#include "j1939_frame.h"

class QTimer;

class CanFrameCache : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(CanFrameCache)

public:
    explicit CanFrameCache(QObject *parent = nullptr);
    QSet<int> enqueueIncomingFrames(const QVector<QCanBusFrame> & frameColl);
    QVector<J1939Frame> takeIncomingFrames(int ecuId);

    QCanBusFrame enqueueOutgoingFrame(const QCanBusFrame &frame);
    QCanBusFrame dequeueOutgoingFrame();

signals:
    void ownFrameLost(const QCanBusFrame &lostFrame, const QCanBusFrame &nextFrame);

private:
    void handleLostOwnFrame();
    int sourceEcuId(const QCanBusFrame &frame) const;

    const QCanBusFrame c_invalidFrame{QCanBusFrame::InvalidFrame};
    QTimer m_receiptTimer;
    QMap<int, QVector<J1939Frame>> m_incomingCache;
    QVector<QCanBusFrame> m_outgoingCache;
};
