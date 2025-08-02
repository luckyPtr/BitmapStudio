#ifndef CHECKSUMUTILITY_H
#define CHECKSUMUTILITY_H

#include <QObject>

class ChecksumUtility : public QObject
{
    Q_OBJECT
public:
    explicit ChecksumUtility(QObject *parent = nullptr);

    static quint16 SUM16(const QByteArray &data);
    static quint32 CRC32(const QByteArray &data);
    static quint16 CRC16(const QByteArray &data);   // CRC16-CCITT
signals:
};

#endif // CHECKSUMUTILITY_H
