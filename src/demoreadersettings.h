/*
  Copyright © 2018 Hasan Yavuz Özderya

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

#ifndef DEMOREADERSETTINGS_H
#define DEMOREADERSETTINGS_H

#include <QWidget>

namespace Ui {
class DemoReaderSettings;
}

class DemoReaderSettings : public QWidget
{
    Q_OBJECT

public:
    explicit DemoReaderSettings(QWidget *parent = 0);
    ~DemoReaderSettings();

    unsigned numChannels() const;
    /// Doesn't signal `numChannelsChanged`.
    void setNumChannels(unsigned value);

private:
    Ui::DemoReaderSettings *ui;

signals:
    void numChannelsChanged(unsigned);
};

#endif // DEMOREADERSETTINGS_H
