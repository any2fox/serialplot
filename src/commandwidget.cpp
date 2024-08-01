/*
  Copyright © 2020 Hasan Yavuz Özderya

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

#include "commandwidget.h"
#include "ui_commandwidget.h"
#include "utils.h"
#include <QRegExp>
#include <QRegExpValidator>
#include <QtDebug>
#include <QIcon>

CommandWidget::CommandWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CommandWidget),
    _sendAction(this)
{
    ui->setupUi(this);

    //定时器单发模式
    _autoSendtimer.setSingleShot(true);

    //定时器1毫秒
    _autoSendtimer.stop();

    connect(&_autoSendtimer,&QTimer::timeout,this,&CommandWidget::onSendClicked);

    connect(ui->spNumOfAutoSendTimer_ms, SELECT<int>::OVERLOAD_OF(&QSpinBox::valueChanged),
            [this](int value)
            {
                emit spNumOfAutoSendTimer_ms(value);
            });

    connect(ui->pbDelete, &QPushButton::clicked, this, &CommandWidget::onDeleteClicked);
    connect(ui->pbSend, &QPushButton::clicked, this, &CommandWidget::onSendClicked);
    connect(ui->pbASCII, &QPushButton::toggled, this, &CommandWidget::onASCIIToggled);
    connect(ui->leName, &QLineEdit::textChanged, [this](QString text)
            {
                this->_sendAction.setText(text);
            });
    connect(&_sendAction, &QAction::triggered, this, &CommandWidget::onSendClicked);
    connect(ui->rbAutoSend, &QRadioButton::toggled, [this](bool checked)
            {
                if (checked) this->onSendClicked();
            });
}

CommandWidget::~CommandWidget()
{
    delete ui;
}
//void CommandWidget::spNumOfAutoSendTimer_ms(unsigned value){
//    _numOfAutoSendTimer_ms = value;
//}
void CommandWidget::onDeleteClicked()
{
    this->deleteLater();
}

void CommandWidget::onSendClicked()
{
    auto command = ui->leCommand->text();
    _autoSendtimer.setInterval(ui->spNumOfAutoSendTimer_ms->value());
    if (command.isEmpty())
    {
        qWarning() << "Enter a command to send!";
        ui->leCommand->setFocus(Qt::OtherFocusReason);
        emit focusRequested();
        _autoSendtimer.stop();
        return;
    }

    if (isASCIIMode())
    {
        qDebug() << "Sending " << name() << ":" << command;
        emit sendCommand(ui->leCommand->unEscapedText().toLatin1());
    }
    else // hex mode
    {
        command = command.remove(' ');
        // check if nibbles are missing
        if (command.size() % 2 == 1)
        {
            qWarning() << "HEX command is missing a nibble at the end!";
            ui->leCommand->setFocus(Qt::OtherFocusReason);
            // highlight the byte that is missing a nibble (last byte obviously)
            int textSize = ui->leCommand->text().size();
            ui->leCommand->setSelection(textSize-1, textSize);
            _autoSendtimer.stop();
            return;
        }
//        qDebug() << "Sending HEX:" << command;
        emit sendCommand(QByteArray::fromHex(command.toLatin1()));
    }
    if(ui->rbAutoSend->isChecked()){
        _autoSendtimer.start();
    }
}

void CommandWidget::onASCIIToggled(bool checked)
{
    ui->leCommand->setMode(checked);
}

bool CommandWidget::isASCIIMode()
{
    return ui->pbASCII->isChecked();
}

void CommandWidget::setASCIIMode(bool enabled)
{
    if (enabled)
    {
        ui->pbASCII->setChecked(true);
    }
    else
    {
        ui->pbHEX->setChecked(true);
    }
}

void CommandWidget::setName(QString name)
{
    ui->leName->setText(name);
}

QString CommandWidget::name()
{
    return ui->leName->text();
}

void CommandWidget::setFocusToEdit()
{
    ui->leCommand->setFocus(Qt::OtherFocusReason);
}

QAction* CommandWidget::sendAction()
{
    return &_sendAction;
}

QString CommandWidget::commandText()
{
    return ui->leCommand->text();
}

void CommandWidget::setCommandText(QString str)
{
    ui->leCommand->selectAll();
    ui->leCommand->insert(str);
}
