/*
  Copyright © 2016 Hasan Yavuz Özderya

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

#include "endiannessbox.h"
#include "ui_endiannessbox.h"

EndiannessBox::EndiannessBox(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::EndiannessBox)
{
    ui->setupUi(this);

    connect(ui->rbLittleE, &QRadioButton::toggled, [this](bool checked)
            {
                emit selectionChanged(currentSelection());
            });
}

EndiannessBox::~EndiannessBox()
{
    delete ui;
}

Endianness EndiannessBox::currentSelection()
{
    if (ui->rbLittleE->isChecked())
    {
        return LittleEndian;
    }
    else
    {
        return BigEndian;
    }
}

void EndiannessBox::setSelection(Endianness endianness)
{
    if (endianness == LittleEndian)
    {
        ui->rbLittleE->setChecked(true);
    }
    else // big endian
    {
        ui->rbBigE->setChecked(true);
    }
}
