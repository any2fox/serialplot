/*
  Copyright © 2015 Hasan Yavuz Özderya

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

#include <QRectF>

#include "scalezoomer.h"

ScaleZoomer::ScaleZoomer(QwtPlot* plot, QwtPlotZoomer* zoomer) :
    QObject(plot),
    bottomPicker(plot->axisWidget(QwtPlot::xBottom), plot->canvas()),
    leftPicker(plot->axisWidget(QwtPlot::yLeft), plot->canvas())
{
    _plot = plot;
    _zoomer = zoomer;
    connect(&bottomPicker, &ScalePicker::picked, this, &ScaleZoomer::bottomPicked);
    connect(&leftPicker, &ScalePicker::picked, this, &ScaleZoomer::leftPicked);
}

void ScaleZoomer::setPickerPen(QPen pen)
{
    bottomPicker.setPen(pen);
    leftPicker.setPen(pen);
}

void ScaleZoomer::bottomPicked(double firstPos, double lastPos)
{
    QRectF zRect;
    if (lastPos > firstPos)
    {
        zRect.setLeft(firstPos);
        zRect.setRight(lastPos);
    }
    else
    {
        zRect.setLeft(lastPos);
        zRect.setRight(firstPos);
    }

    zRect.setBottom(_plot->axisScaleDiv(QwtPlot::yLeft).lowerBound());
    zRect.setTop(_plot->axisScaleDiv(QwtPlot::yLeft).upperBound());
    _zoomer->zoom(zRect);
}

void ScaleZoomer::leftPicked(double firstPos, double lastPos)
{
    QRectF zRect;
    if (lastPos > firstPos)
    {
        zRect.setBottom(firstPos);
        zRect.setTop(lastPos);
    }
    else
    {
        zRect.setBottom(lastPos);
        zRect.setTop(firstPos);
    }

    zRect.setLeft(_plot->axisScaleDiv(QwtPlot::xBottom).lowerBound());
    zRect.setRight(_plot->axisScaleDiv(QwtPlot::xBottom).upperBound());
    _zoomer->zoom(zRect);
}
