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

#ifndef CHANNELINFOMODEL_H
#define CHANNELINFOMODEL_H

#include <QAbstractTableModel>
#include <QColor>
#include <QSettings>
#include <QStringList>

class ChannelInfoModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    enum ChannelInfoColumn
    {
        COLUMN_NAME = 0,
        COLUMN_VISIBILITY,
        COLUMN_GAIN,
        COLUMN_OFFSET,
        COLUMN_COUNT            // MUST be last
    };

    explicit ChannelInfoModel(unsigned numberOfChannels, QObject *parent = 0);
    ChannelInfoModel(const ChannelInfoModel& other);
    explicit ChannelInfoModel(const QStringList& channelNames);

    QString name     (unsigned i) const;
    QColor  color    (unsigned i) const;
    bool    isVisible(unsigned i) const;
    bool    gainEn   (unsigned i) const;
    double  gain     (unsigned i) const;
    bool    offsetEn (unsigned i) const;
    double  offset   (unsigned i) const;
    /// Returns true if any of the channels have gain or offset enabled
    bool gainOrOffsetEn() const;
    /// Returns a list of channel names
    QStringList channelNames() const;

    // implemented from QAbstractItemModel
    int           rowCount(const QModelIndex &parent = QModelIndex()) const;
    int           columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant      data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    bool          setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
    Qt::ItemFlags flags(const QModelIndex &index) const;
    QVariant      headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

    void setNumOfChannels(unsigned number);
    /// Stores all channel info into a `QSettings`
    void saveSettings(QSettings* settings) const;
    /// Loads all channel info from a `QSettings`.
    void loadSettings(QSettings* settings);

public slots:
    /// reset all channel info (names, color etc.)
    void resetInfos();
    /// reset all channel names
    void resetNames();
    /// reset all channel colors
    void resetColors();
    /// reset all channel gain values and disables gains
    void resetGains();
    /// reset all channel offset values and disables offsets
    void resetOffsets();
    /// reset visibility
    void resetVisibility(bool visible);

private:
    struct ChannelInfo
    {
        explicit ChannelInfo(unsigned index);

        QString name;
        bool visibility;
        QColor color;
        double gain, offset;
        bool gainEn, offsetEn;
    };

    unsigned _numOfChannels;     ///< @note this is not necessarily the length of `infos`

    /**
     * Channel info is added here but never removed so that we can
     * remember user entered info (names, colors etc.).
     */
    QList<ChannelInfo> infos;

    /**
     * Cache for gain and offset enabled variables of channels. If gain and/or
     * offset is not enabled for *any* of the channels this is false otherwise
     * true.
     */
    bool _gainOrOffsetEn;

    /// Updates `_gainOrOffsetEn` by scanning all channel infos.
    void updateGainOrOffsetEn();
};

#endif // CHANNELINFOMODEL_H
