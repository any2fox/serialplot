/*
  Copyright © 2023 Hasan Yavuz Özderya

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

#include <algorithm>
#include <QMetaEnum>
#include <QSvgGenerator>
#include <qwt_symbol.h>
#include <qwt_plot_renderer.h>

#include "plot.h"
#include "plotmanager.h"
#include "utils.h"
#include "setting_defines.h"

PlotManager::PlotManager(QWidget* plotArea, PlotMenu* menu,
                         const Stream* stream, QObject* parent) :
    QObject(parent)
{
    _stream = stream;
    construct(plotArea, menu);
    if (_stream == nullptr) return;

    // connect to ChannelInfoModel
    infoModel = _stream->infoModel();
    connect(infoModel, &QAbstractItemModel::dataChanged,
                this, &PlotManager::onChannelInfoChanged);
    connect(infoModel, &QAbstractItemModel::modelReset,
            [this]()
            {
                onChannelInfoChanged(infoModel->index(0, 0), // start
                                     infoModel->index(infoModel->rowCount()-1, 0), // end
                                     {}); // roles ignored
            });

    connect(stream, &Stream::numChannelsChanged, this, &PlotManager::onNumChannelsChanged);
    connect(stream, &Stream::dataAdded, this, &PlotManager::replot);

    // add initial curves if any?
    for (unsigned int i = 0; i < stream->numChannels(); i++)
    {
        addCurve(stream->channel(i)->name(), stream->channel(i)->xData(), stream->channel(i)->yData());
    }
}

PlotManager::PlotManager(QWidget* plotArea, PlotMenu* menu,
                         Snapshot* snapshot, QObject *parent) :
    QObject(parent)
{
    _stream = nullptr;
    construct(plotArea, menu);

    setNumOfSamples(snapshot->numSamples());
    setPlotWidth(snapshot->numSamples());
    infoModel = snapshot->infoModel();

    for (unsigned ci = 0; ci < snapshot->numChannels(); ci++)
    {
        addCurve(snapshot->channelName(ci), snapshot->xData[ci], snapshot->yData[ci]);
    }

    connect(infoModel, &QAbstractItemModel::dataChanged,
            this, &PlotManager::onChannelInfoChanged);

    // TODO: remove when snapshot view supports multi display
    checkNoVisChannels();
}

void PlotManager::construct(QWidget* plotArea, PlotMenu* menu)
{
    _menu = menu;
    _plotArea = plotArea;
    _autoScaled = true;
    _yMin = 0;
    _yMax = 1;
    _xAxisAsIndex = true;
    isDemoShown = false;
    _numOfSamples = 1;
    _plotWidth = 1;
    showSymbols = Plot::ShowSymbolsAuto;
    emptyPlot = NULL;
    inScaleSync = false;
    lineThickness = 1;

    // initalize layout and single widget
    isMulti = false;
    scrollArea = NULL;

    // connect to  menu
    connect(menu, &PlotMenu::symbolShowChanged, this, &PlotManager::setSymbols);

    connect(&menu->showGridAction, SELECT<bool>::OVERLOAD_OF(&QAction::toggled),
            this, &PlotManager::showGrid);
    connect(&menu->showMinorGridAction, SELECT<bool>::OVERLOAD_OF(&QAction::toggled),
            this, &PlotManager::showMinorGrid);
    connect(&menu->darkBackgroundAction, SELECT<bool>::OVERLOAD_OF(&QAction::toggled),
            this, &PlotManager::darkBackground);
    connect(&menu->showMultiAction, SELECT<bool>::OVERLOAD_OF(&QAction::toggled),
            this, &PlotManager::setMulti);
    connect(&menu->unzoomAction, &QAction::triggered,
            this, &PlotManager::unzoom);

    connect(&menu->showLegendAction, SELECT<bool>::OVERLOAD_OF(&QAction::toggled),
            this, &PlotManager::showLegend);
    connect(menu, &PlotMenu::legendPosChanged, this, &PlotManager::setLegendPosition);

    // initial settings from menu actions
    showGrid(menu->showGridAction.isChecked());
    showMinorGrid(menu->showMinorGridAction.isChecked());
    darkBackground(menu->darkBackgroundAction.isChecked());
    showLegend(menu->showLegendAction.isChecked());
    setLegendPosition(menu->legendPosition());
    setMulti(menu->showMultiAction.isChecked());
}

PlotManager::~PlotManager()
{
    while (curves.size())
    {
        delete curves.takeLast();
    }

    // remove all widgets
    while (plotWidgets.size())
    {
        delete plotWidgets.takeLast();
    }

    if (scrollArea != NULL) delete scrollArea;
    if (emptyPlot != NULL) delete emptyPlot;
}

void PlotManager::onNumChannelsChanged(unsigned value)
{
    unsigned int oldNum = numOfCurves();
    unsigned numOfChannels = value;

    if (numOfChannels > oldNum)
    {
        // add new channels
        for (unsigned int i = oldNum; i < numOfChannels; i++)
        {
            addCurve(_stream->channel(i)->name(), _stream->channel(i)->xData(), _stream->channel(i)->yData());
        }
    }
    else if(numOfChannels < oldNum)
    {
        removeCurves(oldNum - numOfChannels);
    }

    replot();
}

void PlotManager::onChannelInfoChanged(const QModelIndex &topLeft,
                                       const QModelIndex &bottomRight,
                                       const QVector<int> &roles)
{
    int start = topLeft.row();
    int end = bottomRight.row();

    for (int ci = start; ci <= end; ci++)
    {
        QString name = topLeft.sibling(ci, ChannelInfoModel::COLUMN_NAME).data(Qt::EditRole).toString();
        QColor color = topLeft.sibling(ci, ChannelInfoModel::COLUMN_NAME).data(Qt::ForegroundRole).value<QColor>();
        bool visible = topLeft.sibling(ci, ChannelInfoModel::COLUMN_VISIBILITY).data(Qt::CheckStateRole).toBool();

        curves[ci]->setTitle(name);
        curves[ci]->setPen(color, lineThickness);
        curves[ci]->setVisible(visible);
        curves[ci]->setItemAttribute(QwtPlotItem::Legend, visible);

        // replot only updated widgets
        if (isMulti)
        {
            plotWidgets[ci]->updateSymbols(); // required for color change
            plotWidgets[ci]->updateLegend(curves[ci]);
            plotWidgets[ci]->setVisible(visible);
            if (visible)
            {
                plotWidgets[ci]->replot();
            }
            syncScales();
        }
    }

    checkNoVisChannels();

    // replot single widget
    if (!isMulti)
    {
        plotWidgets[0]->updateSymbols();
        plotWidgets[0]->updateLegend();
        replot();
    }
}

void PlotManager::checkNoVisChannels()
{
    // if all channels are hidden show indicator
    bool allhidden = std::none_of(curves.cbegin(), curves.cend(),
                                  [](QwtPlotCurve* c) {return c->isVisible();});

    plotWidgets[0]->showNoChannel(allhidden);
    if (isMulti)
    {
        plotWidgets[0]->showNoChannel(allhidden);
        plotWidgets[0]->setVisible(true);
    }
}

void PlotManager::setMulti(bool enabled)
{
    isMulti = enabled;

    // detach all curves
    for (auto curve : curves)
    {
        curve->detach();
    }

    // remove all widgets
    while (plotWidgets.size())
    {
        delete plotWidgets.takeLast();
    }

    // setup new layout
    setupLayout(isMulti);

    if (isMulti)
    {
        // add new widgets and attach
        int i = 0;
        for (auto curve : curves)
        {
            auto plot = addPlotWidget();
            plot->setVisible(curve->isVisible());
            if (_stream != nullptr)
            {
                plot->setDispChannels(QVector<const StreamChannel*>(1, _stream->channel(i)));
            }
            curve->attach(plot);
            i++;
        }
    }
    else
    {
        // add a single widget
        auto plot = addPlotWidget();

        if (_stream != nullptr)
        {
            plot->setDispChannels(_stream->allChannels());
        }

        // attach all curves
        for (auto curve : curves)
        {
            curve->attach(plot);
        }
    }


    // Note: direct call doesn't work presumably because widgets are not ready
    QMetaObject::invokeMethod(this, "syncScales", Qt::QueuedConnection);

    // will skip if no plot widgets exist (can happen during constructor)
    if (plotWidgets.length())
    {
        checkNoVisChannels();
    }
}

void PlotManager::setupLayout(bool multiPlot)
{
    // delete previous layout if it exists
    if (_plotArea->layout() != 0)
    {
        delete _plotArea->layout();
    }

    if (multiPlot)
    {
        // setup a scroll area
        scrollArea = new QScrollArea();
        auto scrolledPlotArea = new QWidget(scrollArea);
        scrollArea->setWidget(scrolledPlotArea);
        scrollArea->setWidgetResizable(true);

        _plotArea->setLayout(new QVBoxLayout());
        _plotArea->layout()->addWidget(scrollArea);
        _plotArea->layout()->setContentsMargins(0,0,0,0);

        layout = new QVBoxLayout(scrolledPlotArea);
    }
    else
    {
        // delete scrollArea left from multi layout
        if (scrollArea != NULL)
        {
            delete scrollArea;
            scrollArea = NULL;
        }

        layout = new QVBoxLayout(_plotArea);
    }

    layout->setContentsMargins(2,2,2,2);
    layout->setSpacing(1);
}

Plot* PlotManager::addPlotWidget()
{
    auto plot = new Plot();
    plotWidgets.append(plot);
    layout->addWidget(plot);

    plot->darkBackground(_menu->darkBackgroundAction.isChecked());
    plot->showGrid(_menu->showGridAction.isChecked());
    plot->showMinorGrid(_menu->showMinorGridAction.isChecked());
    plot->showLegend(_menu->showLegendAction.isChecked());
    plot->setLegendPosition(_menu->legendPosition());
    plot->setSymbols(_menu->showSymbols());

    plot->showDemoIndicator(isDemoShown);
    plot->setYAxis(_autoScaled, _yMin, _yMax);
    plot->setNumOfSamples(_numOfSamples);

    plot->setPlotWidth(_plotWidth);
    if (_xAxisAsIndex)
    {
        plot->setXAxis(0, _numOfSamples);
    }
    else
    {
        plot->setXAxis(_xMin, _xMax);
    }

    if (isMulti)
    {
        connect(plot->axisWidget(QwtPlot::yLeft), &QwtScaleWidget::scaleDivChanged,
                this, &PlotManager::syncScales);
    }

    return plot;
}

// Taken from Qwt "plotmatrix" playground example
void PlotManager::syncScales()
{
    if (inScaleSync) return;

    inScaleSync = true;

    // find maximum extent
    double maxExtent = 0;
    for (auto plot : plotWidgets)
    {
        if (!plot->isVisible()) continue;

        QwtScaleWidget* scaleWidget = plot->axisWidget(QwtPlot::yLeft);
        QwtScaleDraw* scaleDraw = scaleWidget->scaleDraw();
        scaleDraw->setMinimumExtent(0);

        const double extent = scaleDraw->extent(scaleWidget->font());
        if (extent > maxExtent)
            maxExtent = extent;
    }

    // apply maximum extent
    for (auto plot : plotWidgets)
    {
        QwtScaleWidget* scaleWidget = plot->axisWidget(QwtPlot::yLeft);
        scaleWidget->scaleDraw()->setMinimumExtent(maxExtent);
        scaleWidget->updateGeometry();
    }

    for (auto plot : plotWidgets)
    {
        plot->replot();
    }

    inScaleSync = false;
}

void PlotManager::addCurve(QString title, const XFrameBuffer* xBuf, const FrameBuffer* yBuf)
{
    auto curve = new QwtPlotCurve(title);
    auto series = new FrameBufferSeries(xBuf, yBuf);
    curve->setSamples(series);
    _addCurve(curve);
}

void PlotManager::_addCurve(QwtPlotCurve* curve)
{
    // store and init the curve
    curves.append(curve);

    unsigned index = curves.size()-1;
    auto color = infoModel->color(index);
    curve->setPen(color, lineThickness);

    // create the plot for the curve if we are on multi display
    Plot* plot;
    if (isMulti)
    {
        // create a new plot widget
        plot = addPlotWidget();
    }
    else
    {
        plot = plotWidgets[0];
    }

    if (_stream != nullptr)     // not displaying snapshot
    {
        QVector<const StreamChannel*> dispChannels;
        if (isMulti)
        {
            dispChannels = QVector<const StreamChannel*>(1, _stream->channel(index));
        }
        else
        {
            dispChannels = _stream->allChannels();
        }
        plot->setDispChannels(dispChannels);
    }

    // show the curve
    curve->attach(plot);
    checkNoVisChannels();
    plot->replot();
}

void PlotManager::removeCurves(unsigned number)
{
    if (_stream != nullptr)     // not displaying snapshot
    {
        if (! isMulti)
        {
            QVector<const StreamChannel*> dispChannels;
            dispChannels = _stream->allChannels();
            plotWidgets[0]->setDispChannels(dispChannels);
        }
    }

    for (unsigned i = 0; i < number; i++)
    {
        if (!curves.isEmpty())
        {
            delete curves.takeLast();
            if (isMulti) // delete corresponding widget as well
            {
                delete plotWidgets.takeLast();
            }
        }
    }
}

unsigned PlotManager::numOfCurves()
{
    return curves.size();
}

Plot* PlotManager::plotWidget(unsigned curveIndex)
{
    if (isMulti)
    {
        return plotWidgets[curveIndex];
    }
    else
    {
        return plotWidgets[0];
    }
}

void PlotManager::replot()
{
    for (auto plot : plotWidgets)
    {
        plot->replot();
    }
    if (isMulti) syncScales();
}

void PlotManager::showGrid(bool show)
{
    for (auto plot : plotWidgets)
    {
        plot->showGrid(show);
    }
}

void PlotManager::showMinorGrid(bool show)
{
    for (auto plot : plotWidgets)
    {
        plot->showMinorGrid(show);
    }
}

void PlotManager::showLegend(bool show)
{
    for (auto plot : plotWidgets)
    {
        plot->showLegend(show);
    }
}

void PlotManager::setLegendPosition(Qt::AlignmentFlag alignment)
{
    for (auto plot : plotWidgets)
    {
        plot->setLegendPosition(alignment);
    }
}

void PlotManager::showDemoIndicator(bool show)
{
    isDemoShown = show;
    for (auto plot : plotWidgets)
    {
        plot->showDemoIndicator(show);
    }
}

void PlotManager::unzoom()
{
    for (auto plot : plotWidgets)
    {
        plot->unzoom();
    }
}

void PlotManager::darkBackground(bool enabled)
{
    for (auto plot : plotWidgets)
    {
        plot->darkBackground(enabled);
    }
}

void PlotManager::setSymbols(Plot::ShowSymbols shown)
{
    showSymbols = shown;
    for (auto plot : plotWidgets)
    {
        plot->setSymbols(shown);
    }
}

void PlotManager::setYAxis(bool autoScaled, double yAxisMin, double yAxisMax)
{
    _autoScaled = autoScaled;
    _yMin = yAxisMin;
    _yMax = yAxisMax;
    for (auto plot : plotWidgets)
    {
        plot->setYAxis(autoScaled, yAxisMin, yAxisMax);
    }
}

void PlotManager::setXAxis(bool asIndex, double xMin, double xMax)
{
    _xAxisAsIndex = asIndex;
    _xMin = xMin;
    _xMax = xMax;

    int ci = 0;
    for (auto curve : curves)
    {
        FrameBufferSeries* series = static_cast<FrameBufferSeries*>(curve->data());
        series->setX(_stream->channel(ci)->xData());
        ci++;
    }
    for (auto plot : plotWidgets)
    {
        if (asIndex)
        {
            plot->setXAxis(0, _numOfSamples);
        }
        else
        {
            plot->setXAxis(xMin, xMax);
        }
    }
    replot();
}

void PlotManager::flashSnapshotOverlay()
{
    for (auto plot : plotWidgets)
    {
        plot->flashSnapshotOverlay(_menu->darkBackgroundAction.isChecked());
    }
}

void PlotManager::setNumOfSamples(unsigned value)
{
    _numOfSamples = value;
    for (auto plot : plotWidgets)
    {
        plot->setNumOfSamples(value);
        if (_xAxisAsIndex) plot->setXAxis(0, value);
    }
}

void PlotManager::setPlotWidth(double width)
{
    _plotWidth = width;
    for (auto plot : plotWidgets)
    {
        plot->setPlotWidth(width);
    }
}

void PlotManager::setLineThickness(int thickness)
{
    lineThickness = thickness;

    for (auto curve : curves)
    {
        auto pen = curve->pen();
        pen.setWidth(lineThickness);
        curve->setPen(pen);
    }

    replot();
}

void PlotManager::exportSvg(QString fileName) const
{
    QString baseName, suffix;

    baseName = fileName.section('.', 0, 0);
    suffix = "." + fileName.section('.', 1, 1);

    // if suffix is empty make sure it is svg
    if (suffix.size() == 1)
    {
        suffix = ".svg";
    }

    for (int i=0; i < plotWidgets.size(); i++)
    {
        if (plotWidgets.size() > 1)
            fileName = baseName + "-" + _stream->channel(i)->name() + suffix;

        auto plot = plotWidgets.at(i);

        QSvgGenerator gen;
        gen.setFileName(fileName);
        gen.setSize(plot->size());
        gen.setViewBox(plot->rect());

        QwtPlotRenderer renderer;
        QPainter painter;
        painter.begin(&gen);
        renderer.render(plot, &painter, plot->rect());
        painter.end();
    }
}
