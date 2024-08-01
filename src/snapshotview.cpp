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

#include "snapshotview.h"
#include "ui_snapshotview.h"

SnapshotView::SnapshotView(MainWindow* parent, Snapshot* snapshot) :
    QMainWindow(parent),
    ui(new Ui::SnapshotView),
    renameDialog(this),
    plotMenu(parent->viewSettings())
{
    _snapshot = snapshot;

    ui->setupUi(this);

    plotMan = new PlotManager(ui->plotArea, &plotMenu, snapshot, this);

    ui->menuSnapshot->insertAction(ui->actionClose, snapshot->deleteAction());
    this->setWindowTitle(snapshot->displayName());

    // initialize curves
    // unsigned numOfChannels = snapshot->data.size();
    // unsigned numOfSamples = snapshot->data[0].size();
    // for (unsigned ci = 0; ci < numOfChannels; ci++)
    // {
    //     plotMan->addCurve(snapshot->channelName(ci), snapshot->data[ci]);
    // }
    // plotMan->setNumOfSamples(numOfSamples);
    // plotMan->setPlotWidth(numOfSamples);

    renameDialog.setWindowTitle("Rename Snapshot");
    renameDialog.setLabelText("Enter new name:");
    connect(ui->actionRename, &QAction::triggered,
            this, &SnapshotView::showRenameDialog);

    connect(ui->actionSave, &QAction::triggered,
            this, &SnapshotView::save);

    connect(ui->actionExportSvg, &QAction::triggered,
            this, &SnapshotView::exportSvg);

    // add "View" menu
    menuBar()->insertMenu(NULL, &plotMenu);
}

SnapshotView::~SnapshotView()
{
    for (auto curve : curves)
    {
        delete curve;
    }
    delete plotMan;
    delete ui;
}

void SnapshotView::closeEvent(QCloseEvent *event)
{
    QMainWindow::closeEvent(event);
    emit closed();
}

void SnapshotView::showRenameDialog()
{
    renameDialog.setTextValue(_snapshot->name());
    renameDialog.open(this, SLOT(renameSnapshot(QString)));
}

void SnapshotView::renameSnapshot(QString name)
{
    _snapshot->setName(name);
    setWindowTitle(_snapshot->displayName());
}

void SnapshotView::save()
{
    QString fileName = QFileDialog::getSaveFileName(
        this, tr("Export CSV File"), _snapshot->name() + ".csv", "CSV (*.csv)");

    if (fileName.isNull()) return; // user canceled

    _snapshot->save(fileName);

    // saving changes snapshots display name (* is no more)
    setWindowTitle(_snapshot->displayName());
}

void SnapshotView::exportSvg()
{
    QString fileName = QFileDialog::getSaveFileName(
        this, tr("Export SVG File(s)"), _snapshot->name() + ".svg", "Images (*.svg)");

    if (fileName.isNull()) return; // user canceled

    plotMan->exportSvg(fileName);
}
