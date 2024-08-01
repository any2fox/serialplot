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

#ifndef DATAFORMATPANEL_H
#define DATAFORMATPANEL_H

#include <stdint.h>
#include <QWidget>
#include <QButtonGroup>
#include <QSerialPort>
#include <QList>
#include <QSettings>
#include <QtGlobal>
#include <QButtonGroup>

#include "binarystreamreader.h"
#include "omitstreamreader.h"
#include "asciireader.h"
#include "demoreader.h"
#include "framedreader.h"
#include "datarecorder.h"

namespace Ui {
class DataFormatPanel;
}

class DataFormatPanel : public QWidget
{
    Q_OBJECT

public:
    explicit DataFormatPanel(QSerialPort* port, QWidget* parent = 0);
    ~DataFormatPanel();

    /// Returns currently selected number of channels
    unsigned numChannels() const;
    /// Returns active source (reader)
    Source* activeSource();
    /// Returns total number of bytes read
    uint64_t bytesRead();
    /// Stores data format panel settings into a `QSettings`
    void saveSettings(QSettings* settings);
    /// Loads data format panel settings from a `QSettings`.
    void loadSettings(QSettings* settings);

public slots:
    void pause(bool);
    void enableDemo(bool); // demo shouldn't be enabled when port is open

signals:
    /// Active (selected) reader has changed.
    void sourceChanged(Source* source);

private:
    Ui::DataFormatPanel *ui;
    QButtonGroup readerSelectButtons;

    QSerialPort* serialPort;

    BinaryStreamReader bsReader;
    AsciiReader asciiReader;
    FramedReader framedReader;
    OmitStreamReader osReader;

    /// Currently selected reader
    AbstractReader* currentReader;
    /// Disable current reader and enable a another one
    void selectReader(AbstractReader* reader);

    bool paused;
    uint64_t _bytesRead;

    DemoReader demoReader;
    AbstractReader* readerBeforeDemo;

    bool isDemoEnabled() const;
};

#endif // DATAFORMATPANEL_H
