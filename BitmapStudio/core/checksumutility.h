#ifndef CHECKSUMUTILITY_H
#define CHECKSUMUTILITY_H

#include "qglobal.h"
#include <QObject>

class ChecksumUtility : public QObject
{
    Q_OBJECT
public:
    explicit ChecksumUtility(QObject *parent = nullptr);

    static quint32 SUM32(const QByteArray &data);
    static quint16 SUM16(const QByteArray &data);
    static quint8 SUM8(const QByteArray &data);
    static quint32 CRC32(const QByteArray &data);
    static quint16 CRC16(const QByteArray &data);   // CRC16-CCITT
    static quint8 CRC8(const QByteArray &data);      // CRC8
    static quint8 XOR8(const QByteArray &data);

signals:
};

#endif // CHECKSUMUTILITY_H
