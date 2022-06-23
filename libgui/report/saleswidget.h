/*
 * saleswidget.h
 * Copyright 2017 - ~, Apin <apin.klas@gmail.com>
 *
 * This file is part of Sultan.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef SALESWIDGET_H
#define SALESWIDGET_H

#include "messagehandler.h"
#include <QWidget>

namespace Ui {
class SalesWidget;
}

namespace LibGUI {

class TableWidget;

class SalesWidget : public QWidget, public LibG::MessageHandler
{
    Q_OBJECT
public:
    SalesWidget(LibG::MessageBus *bus, QWidget *parent = nullptr);

protected:
    void messageReceived(LibG::Message *msg) override;

private:
    Ui::SalesWidget *ui;
    TableWidget *mTableWidget;

private slots:
    void refreshSummary();
    void exportClicked();
};

}

#endif // SALESWIDGET_H
