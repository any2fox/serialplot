/*
  Copyright © 2019 Hasan Yavuz Özderya

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

#ifndef OMITSTREAMREADER_H
#define OMITSTREAMREADER_H

#include <QSettings>

#include "abstractreader.h"
#include "omitstreamreadersettings.h"
#include <QTimer>
/**
 * Reads a simple stream of samples in omit form from the
 * device. There is no means of synchronization other than a button
 * that should be manually triggered by user.
 */
class OmitStreamReader : public AbstractReader
{
    Q_OBJECT
public:
    explicit OmitStreamReader(QIODevice* device, QObject *parent = 0);
    QWidget* settingsWidget();
    unsigned numChannels() const;
    unsigned numOmitByte() const;
    /// Stores settings into a `QSettings`
    void saveSettings(QSettings* settings);
    /// Loads settings from a `QSettings`.
    void loadSettings(QSettings* settings);

private:
    OmitStreamReaderSettings _settingsWidget;
    QTimer _datastreamtimer;
    //获取数据数据
    QByteArray Read_Buff;

    unsigned _numChannels;
    unsigned _numOmitByte;
    unsigned sampleSize;
    bool skipByteRequested;
    bool skipSampleRequested;
    //数据头标记
    bool data_head_flag;
    //上次数据长度
    unsigned preBytesAvailable;

    /// points to the readSampleAs function for currently selected number format
    double (OmitStreamReader::*readSample)();

    /**
     * Reads 1 sample from the device in given format.
     *
     * @note Device should already have enough bytes present before
     * calling this function.
     */
    template<typename T> double readSampleAs();
    unsigned readData() override;
    //数据流的头
    void into_data_head();
    //清除数据流的头
    void clean_data_head();
private slots:
    void onNumberFormatChanged(NumberFormat numberFormat);
    void onNumOfChannelsChanged(unsigned value);
    void onNumOfOmitByteChanged(unsigned value);
};

#endif // OmitSTREAMREADER_H
