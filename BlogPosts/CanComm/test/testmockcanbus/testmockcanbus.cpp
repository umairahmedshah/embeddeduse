// Copyright (C) 2019, Burkhard Stubert (DBA Embedded Use)

#include <algorithm>
#include <memory>

#include <QByteArray>
#include <QCanBus>
#include <QCanBusDevice>
#include <QCanBusDeviceInfo>
#include <QCanBusFrame>
#include <QCoreApplication>
#include <QList>
#include <QObject>
#include <QSignalSpy>
#include <QString>
#include <QStringList>
#include <QtDebug>
#include <QtTest>

#include "canbusext.h"
#include "mockcanutils.h"

class TestMockCanBus : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void testReceiveOwnConfigurationKey();
    void testReceiveOwnWrittenFrames_data();
    void testReceiveOwnWrittenFrames();
    void testWriteBufferOverflow_data();
    void testWriteBufferOverflow();

private:
    QCanBusDevice *createAndConnectDevice(const QString &interface);
};

void TestMockCanBus::initTestCase()
{
    // The loader for the CAN bus plugins adds /canbus to each library path and looks for
    // plugins in /library/path/canbus. Hence, the directory containing the mockcan plugin
    // is called "canbus".
    QCoreApplication::addLibraryPath("../../");
}
void TestMockCanBus::testReceiveOwnConfigurationKey()
{
    const auto receiveOwnConfKey = QCanBusDevice::ConfigurationKey::ReceiveOwnKey;
    std::unique_ptr<QCanBusDevice> device{createAndConnectDevice("mcan0")};
    QVERIFY(!device->configurationParameter(receiveOwnConfKey).toBool());
    device->setConfigurationParameter(receiveOwnConfKey, true);
    QVERIFY(device->configurationParameter(receiveOwnConfKey).toBool());
    device->setConfigurationParameter(receiveOwnConfKey, false);
    QVERIFY(!device->configurationParameter(receiveOwnConfKey).toBool());
}

void TestMockCanBus::testReceiveOwnWrittenFrames_data()
{
    QTest::addColumn<bool>("receiveOwn");
    QTest::addColumn<CanBusFrameCollection>("outgoingFrames");
    QTest::addColumn<MockCanFrameCollection>("expectedCanIo");

    auto out1 = MockCanFrame{MockCanFrame::Type::Outgoing, 0x18ef0201U, "018A010000000000"};
    auto ownOut1 = MockCanFrame{MockCanFrame::Type::OwnIncoming, out1};
    auto in1 = MockCanFrame{MockCanFrame::Type::Incoming, 0x18ef0102U, "018A0103A4000000"};

    QTest::newRow("own on: out1-ownOut1")
            << true
            << CanBusFrameCollection{out1}
            << MockCanFrameCollection{out1, ownOut1};
    QTest::newRow("own on: out1-ownOut1-in1")
            << true
            << CanBusFrameCollection{out1}
            << MockCanFrameCollection{out1, ownOut1, in1};
    QTest::newRow("own on: out1-in1-ownOut1")
            << true
            << CanBusFrameCollection{out1}
            << MockCanFrameCollection{out1, in1, ownOut1};
    QTest::newRow("own off, but own expected")
            << false
            << CanBusFrameCollection{out1}
            << MockCanFrameCollection{out1, ownOut1};
}

void TestMockCanBus::testReceiveOwnWrittenFrames()
{
    QFETCH(bool, receiveOwn);
    QFETCH(CanBusFrameCollection, outgoingFrames);
    QFETCH(MockCanFrameCollection, expectedCanIo);

    std::unique_ptr<QCanBusDevice> device{createAndConnectDevice("mcan0")};
    device->setConfigurationParameter(QCanBusDevice::ConfigurationKey::ReceiveOwnKey,
                                      receiveOwn);
    setExpectedCanIo(device.get(), expectedCanIo);

    for (const auto &frame : outgoingFrames) {
        device->writeFrame(frame);
    }
    auto goldenCanIo = MockCanFrameCollection{};
    std::copy_if(expectedCanIo.cbegin(), expectedCanIo.cend(), std::back_inserter(goldenCanIo),
                 [receiveOwn](const MockCanFrame &f) {
                     return receiveOwn || !f.isOwnIncoming();
                 });
    QCOMPARE(actualCanIo(device.get()), goldenCanIo);
}

void TestMockCanBus::testWriteBufferOverflow_data()
{
    auto req1 = MockCanFrame{MockCanFrame::Type::Outgoing, 0x18ef0201U, "0101000000000000"};
    auto rsp1 = MockCanFrame{MockCanFrame::Type::Incoming, 0x18ef0102U, "0101001200000000"};
    auto req2 = MockCanFrame{MockCanFrame::Type::Outgoing, 0x18ef0201U, "0102000000000000"};
    auto rsp2 = MockCanFrame{MockCanFrame::Type::Incoming, 0x18ef0102U, "0102002300000000"};

}

void TestMockCanBus::testWriteBufferOverflow()
{

}

QCanBusDevice *TestMockCanBus::createAndConnectDevice(const QString &interface)
{
    QString errorStr;
    auto device = QCanBus::instance()->createDevice("mockcan", interface, &errorStr);
    device->connectDevice();
    return device;
}

QTEST_GUILESS_MAIN(TestMockCanBus)

#include "testmockcanbus.moc"
