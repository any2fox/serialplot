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

#ifndef ZOOMER_H
#define ZOOMER_H

#include <QVector>
#include <QList>
#include <QRect>

#include "scrollzoomer.h"
#include "streamchannel.h"

class Zoomer : public ScrollZoomer
{
    Q_OBJECT

public:
    Zoomer(QWidget*, bool doReplot=true);
    void zoom(int up);
    void zoom(const QRectF&);
    /// Set displayed channels for value tracking (can be null)
    void setDispChannels(QVector<const StreamChannel*> channels);

signals:
    void unzoomed();

protected:
    /// Re-implemented to display selection size in the tracker text.
    QwtText trackerTextF(const QPointF &pos) const override;
    /// Re-implemented for sample value tracker
    QRect trackerRect(const QFont&) const override;
    /// Re-implemented for alpha background
    void drawRubberBand(QPainter* painter) const override;
    /// Re-implemented to draw sample values
    void drawTracker(QPainter* painter) const override;
    /// Re-implemented for alpha background (masking is basically disabled)
    QRegion rubberBandMask() const;
    /// Overloaded for panning
    void widgetMousePressEvent(QMouseEvent* mouseEvent);
    /// Overloaded for panning
    void widgetMouseReleaseEvent(QMouseEvent* mouseEvent);
    /// Overloaded for panning
    void widgetMouseMoveEvent(QMouseEvent* mouseEvent);

private:
    bool is_panning;
    QPointF pan_point;
    /// displayed channels for value tracking
    QVector<const StreamChannel*> dispChannels;

    /// Get a list of visible channels
    QList<const StreamChannel*> visChannels() const;
    /// Draw sample values
    void drawValues(QPainter* painter) const;
    /// Returns trackerRect for value tracker
    QRect valueTrackerRect(const QFont& font) const;
};

#endif // ZOOMER_H
