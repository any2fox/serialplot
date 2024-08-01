/*
  Copyright © 2021 Hasan Yavuz Özderya

  This file is part of serialplot.

  serialplot is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  serialplot is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with serialplot.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <QtEndian>
#include <QtDebug>
#include <QTimer>
#include "omitstreamreader.h"
#include "byteswap.h"
#include "portcontrol.h"

OmitStreamReader::OmitStreamReader(QIODevice* device, QObject* parent) :
    AbstractReader(device, parent)
{
    paused = false;
    skipByteRequested = false;
    skipSampleRequested = false;

    _numChannels = _settingsWidget.numOfChannels();
    //定时器单发模式
    _datastreamtimer.setSingleShot(true);

    //定时器1毫秒
    _datastreamtimer.setInterval(1);
    _datastreamtimer.stop();

    connect(&_datastreamtimer,&QTimer::timeout,this,&OmitStreamReader::into_data_head);

    connect(&_settingsWidget, &OmitStreamReaderSettings::numOfChannelsChanged,
                     this, &OmitStreamReader::onNumOfChannelsChanged);

    _numOmitByte = _settingsWidget.numOfOmitByte();
    connect(&_settingsWidget, &OmitStreamReaderSettings::numOfOmitByteChanged,
                     this, &OmitStreamReader::onNumOfOmitByteChanged);
    // initial number format selection
    onNumberFormatChanged(_settingsWidget.numberFormat());
    connect(&_settingsWidget, &OmitStreamReaderSettings::numberFormatChanged,
            this, &OmitStreamReader::onNumberFormatChanged);

    // enable skip byte and sample buttons
    connect(&_settingsWidget, &OmitStreamReaderSettings::skipByteRequested,
            [this]()
            {
                skipByteRequested = true;
            });
    connect(&_settingsWidget, &OmitStreamReaderSettings::skipSampleRequested,
            [this]()
            {
                skipSampleRequested = true;
            });
}

QWidget* OmitStreamReader::settingsWidget()
{
    return &_settingsWidget;
}

unsigned OmitStreamReader::numChannels() const
{
    return _numChannels;
}
unsigned OmitStreamReader::numOmitByte() const
{
    return _numOmitByte;
}
void OmitStreamReader::onNumberFormatChanged(NumberFormat numberFormat)
{
    switch(numberFormat)
    {
        case NumberFormat_uint8:
            sampleSize = sizeof(quint8);
            readSample = &OmitStreamReader::readSampleAs<quint8>;
            break;
        case NumberFormat_int8:
            sampleSize = sizeof(qint8);
            readSample = &OmitStreamReader::readSampleAs<qint8>;
            break;
        case NumberFormat_uint16:
            sampleSize = sizeof(quint16);
            readSample = &OmitStreamReader::readSampleAs<quint16>;
            break;
        case NumberFormat_int16:
            sampleSize = sizeof(qint16);
            readSample = &OmitStreamReader::readSampleAs<qint16>;
            break;
        case NumberFormat_uint32:
            sampleSize = sizeof(quint32);
            readSample = &OmitStreamReader::readSampleAs<quint32>;
            break;
        case NumberFormat_int32:
            sampleSize = sizeof(qint32);
            readSample = &OmitStreamReader::readSampleAs<qint32>;
            break;
        case NumberFormat_float:
            sampleSize = sizeof(float);
            readSample = &OmitStreamReader::readSampleAs<float>;
            break;
        case NumberFormat_double:
            sampleSize = sizeof(double);
            readSample = &OmitStreamReader::readSampleAs<double>;
            break;
        case NumberFormat_INVALID:
            Q_ASSERT(1); // never
            break;
    }
}

void OmitStreamReader::onNumOfChannelsChanged(unsigned value)
{
    _numChannels = value;
    updateNumChannels();
    emit numOfChannelsChanged(value);
}
void OmitStreamReader::onNumOfOmitByteChanged(unsigned value)
{
    _numOmitByte = value;
    updateNumOmitByte();
    emit numOfChannelsChanged(value);
}
unsigned OmitStreamReader::readData()
{
    // a package is a set of channel data like {CHAN0_SAMPLE, CHAN1_SAMPLE...}
    unsigned bytesAvailable;
    //获取当前数据长度
    bytesAvailable = _device->bytesAvailable();
    //加入数据
    Read_Buff.append(_device->read(bytesAvailable));
    clean_data_head();
    //返回数据长度
    return bytesAvailable;
}

template<typename T> double OmitStreamReader::readSampleAs()
{
    T data;
    QByteArray data2num;
    switch (_settingsWidget.numberFormat()) {
        case NumberFormat_uint8:
            data2num = Read_Buff.left(1);
            Read_Buff = Read_Buff.right(Read_Buff.length() - 1);
            break;
        case NumberFormat_int8:
            data2num = Read_Buff.left(1);
            Read_Buff = Read_Buff.right(Read_Buff.length() - 1);
            break;
        case NumberFormat_uint16:
            data2num = Read_Buff.left(2);
            Read_Buff = Read_Buff.right(Read_Buff.length() - 2);
            break;
        case NumberFormat_int16:
            data2num = Read_Buff.left(2);
            Read_Buff = Read_Buff.right(Read_Buff.length() - 2);
            break;
        case NumberFormat_uint32:
            data2num = Read_Buff.left(4);
            Read_Buff = Read_Buff.right(Read_Buff.length() - 4);
            break;
        case NumberFormat_int32:
            data2num = Read_Buff.left(4);
            Read_Buff = Read_Buff.right(Read_Buff.length() - 4);
            break;
        case NumberFormat_float:
            data2num = Read_Buff.left(4);
            Read_Buff = Read_Buff.right(Read_Buff.length() - 4);
            break;
        case NumberFormat_double:
            data2num = Read_Buff.left(8);
            Read_Buff = Read_Buff.right(Read_Buff.length() - 8);
            break;
        case NumberFormat_INVALID:
            Q_ASSERT(1); // never
            break;
    }
    memcpy(&data,data2num,data2num.size());
    if (_settingsWidget.endianness() == LittleEndian)
    {
        data = qFromLittleEndian(data);
    }
    else
    {
        data = qFromBigEndian(data);
    }

    return double(data);
}

void OmitStreamReader::saveSettings(QSettings* settings)
{
    _settingsWidget.saveSettings(settings);
}

void OmitStreamReader::loadSettings(QSettings* settings)
{
    _settingsWidget.loadSettings(settings);
}
void OmitStreamReader::into_data_head(){
    unsigned packageSize = sampleSize * _numChannels;
    unsigned dataLenght;
    dataLenght = Read_Buff.length();
    if(dataLenght == 0) return;

    if(_numOmitByte <= dataLenght - packageSize){
        Read_Buff = Read_Buff.right(dataLenght - _numOmitByte);
        dataLenght -= _numOmitByte;
    }else{
        qCritical() << "Omit Byte Number" << _numOmitByte << "Too Large!";
        Read_Buff.clear();
        return;
    }
    unsigned numOfPackagesToRead =
        (dataLenght - (dataLenght % packageSize)) / packageSize;
    // actual reading
    SamplePack samples(1, _numChannels);
    if(numOfPackagesToRead > 0){
        for (unsigned ci = 0; ci < _numChannels; ci++)
        {
            samples.data(ci)[0] = (this->*readSample)();
        }
    }else{
        qCritical() << "Err Data Is:" << Read_Buff;
        Read_Buff.clear();
        return;
    }
    feedOut(samples);
    data_head_flag = true;
    Read_Buff.clear();
}
void OmitStreamReader::clean_data_head(){
    data_head_flag = false;
    _datastreamtimer.setInterval(1);
    _datastreamtimer.start();
//    qDebug() << "timer is start!";
}
