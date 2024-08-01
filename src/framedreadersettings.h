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

#ifndef FRAMEDREADERSETTINGS_H
#define FRAMEDREADERSETTINGS_H

#include <QWidget>
#include <QByteArray>
#include <QSettings>
#include <QButtonGroup>

#include "numberformatbox.h"
#include "endiannessbox.h"

namespace Ui {
class FramedReaderSettings;
}

class FramedReaderSettings : public QWidget
{
    Q_OBJECT

public:
    enum class SizeFieldType
    {
        Fixed, Field1Byte, Field2Byte
    };

    explicit FramedReaderSettings(QWidget *parent = 0);
    ~FramedReaderSettings();

    void showMessage(QString message, bool error = false);

    unsigned numOfChannels();
    NumberFormat numberFormat();
    Endianness endianness();
    QByteArray syncWord();
    SizeFieldType sizeFieldType() const;
    unsigned fixedFrameSize() const;
    bool isChecksumEnabled();
    bool isDebugModeEnabled();
    /// Save settings into a `QSettings`
    void saveSettings(QSettings* settings);
    /// Loads settings from a `QSettings`.
    void loadSettings(QSettings* settings);

signals:
    /// If sync word is invalid (empty or 1 nibble missing at the end)
    /// signaled with an empty array
    void syncWordChanged(QByteArray);
    /// 'size' field is only valid with 'Fixed' type
    void sizeFieldChanged(SizeFieldType type, unsigned size);
    /// `0` indicates frame size byte is enabled
    void fixedFrameSizeChanged(unsigned);
    void checksumChanged(bool);
    void numOfChannelsChanged(unsigned);
    void numberFormatChanged(NumberFormat);
    void debugModeChanged(bool);

private:
    Ui::FramedReaderSettings *ui;
    QButtonGroup fbGroup;

private slots:
    void onSyncWordEdited();
};

#endif // FRAMEDREADERSETTINGS_H
