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

#include "omitstreamreadersettings.h"
#include "ui_omitstreamreadersettings.h"

#include "utils.h"
#include "defines.h"
#include "setting_defines.h"

OmitStreamReaderSettings::OmitStreamReaderSettings(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::OmitStreamReaderSettings)
{
    ui->setupUi(this);

    ui->spNumOfChannels->setMaximum(MAX_NUM_CHANNELS);
    //设置忽略字符64个
    ui->spNumOfOmitByte->setMaximum(MAX_NUM_CHANNELS);
    // Note: if directly connected we get a runtime warning on incompatible signal arguments
    connect(ui->spNumOfChannels, SELECT<int>::OVERLOAD_OF(&QSpinBox::valueChanged),
            [this](int value)
            {
                emit numOfChannelsChanged(value);
            });
    connect(ui->spNumOfOmitByte, SELECT<int>::OVERLOAD_OF(&QSpinBox::valueChanged),
            [this](int value)
            {
                emit numOfOmitByteChanged(value);
            });
    connect(ui->nfBox, SIGNAL(selectionChanged(NumberFormat)),
            this, SIGNAL(numberFormatChanged(NumberFormat)));


}

OmitStreamReaderSettings::~OmitStreamReaderSettings()
{
    delete ui;
}

unsigned OmitStreamReaderSettings::numOfChannels()
{
    return ui->spNumOfChannels->value();
}
unsigned OmitStreamReaderSettings::numOfOmitByte()
{
    return ui->spNumOfOmitByte->value();
}
NumberFormat OmitStreamReaderSettings::numberFormat()
{
    return ui->nfBox->currentSelection();
}

Endianness OmitStreamReaderSettings::endianness()
{
    return ui->endiBox->currentSelection();
}

void OmitStreamReaderSettings::saveSettings(QSettings* settings)
{
    settings->beginGroup(SettingGroup_Omit);
    settings->setValue(SG_Omit_NumOfChannels, numOfChannels());
    settings->setValue(SG_Omit_NumOfOmitByte, numOfOmitByte());
    settings->setValue(SG_Omit_NumberFormat, numberFormatToStr(numberFormat()));
    settings->setValue(SG_Omit_Endianness,
                       endianness() == LittleEndian ? "little" : "big");
    settings->endGroup();
}

void OmitStreamReaderSettings::loadSettings(QSettings* settings)
{
    settings->beginGroup(SettingGroup_Omit);

    // load number of channels
    ui->spNumOfChannels->setValue(
        settings->value(SG_Omit_NumOfChannels, numOfChannels()).toInt());
    //加载忽略字节数
    ui->spNumOfOmitByte->setValue(
        settings->value(SG_Omit_NumOfOmitByte, numOfOmitByte()).toInt());
    // load number format
    NumberFormat nfSetting =
        strToNumberFormat(settings->value(SG_Omit_NumberFormat,
                                          QString()).toString());
    if (nfSetting == NumberFormat_INVALID) nfSetting = numberFormat();
    ui->nfBox->setSelection(nfSetting);

    // load endianness
    QString endiannessSetting =
        settings->value(SG_Omit_Endianness, QString()).toString();
    if (endiannessSetting == "little")
    {
        ui->endiBox->setSelection(LittleEndian);
    }
    else if (endiannessSetting == "big")
    {
        ui->endiBox->setSelection(BigEndian);
    } // else don't change

    settings->endGroup();
}
