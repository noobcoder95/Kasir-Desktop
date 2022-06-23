/*
 * solditemreturnwidget.cpp
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
#include "solditemreturnwidget.h"
#include "ui_normalwidget.h"
#include "tablewidget.h"
#include "tablemodel.h"
#include "global_constant.h"
#include "guiutil.h"
#include "tableview.h"
#include "tableitem.h"
#include "dbutil.h"
#include "headerwidget.h"
#include "db_constant.h"
#include "message.h"
#include "flashmessagemanager.h"
#include "tilewidget.h"
#include "preference.h"
#include "addsoldreturndialog.h"
#include <QMessageBox>
#include <QPushButton>

using namespace LibGUI;
using namespace LibG;

SoldItemReturnWidget::SoldItemReturnWidget(LibG::MessageBus *bus, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::NormalWidget),
    mTableWidget(new TableWidget(this)),
    mTotalDebt(new TileWidget(this))
{
    ui->setupUi(this);
    setMessageBus(bus);
    ui->labelTitle->setText(tr("Sold return"));
    mTotalDebt->setTitleValue(tr("Total Debt"), tr("loading..."));
    ui->verticalLayout->addWidget(mTotalDebt);
    ui->verticalLayout->addWidget(mTableWidget);
    mTableWidget->initCrudButton();
    auto model = mTableWidget->getModel();
    model->setMessageBus(bus);
    model->addColumn("created_at", tr("Date"), Qt::AlignLeft, [](TableItem *item, const QString &key) {
        return LibDB::DBUtil::sqlDateToDateTime(item->data(key).toString()).toString("dd-MM-yyyy hh:mm");
    });
    model->addColumn("barcode", tr("Barcode"));
    model->addColumn("name", tr("Name"));
    model->addColumn("number", tr("Transaction<br>Number"));
    model->addColumn("sold_created_at", tr("Transaction<br>Date"), Qt::AlignLeft, [](TableItem *item, const QString &key) {
        return LibDB::DBUtil::sqlDateToDateTime(item->data(key).toString()).toString("dd-MM-yyyy hh:mm");
    });
    model->addColumn("status", tr("Status"), Qt::AlignLeft, [](TableItem *item, const QString &key) {
        return item->data(key).toInt() == PURCHASE_RETURN_STATUS::UNRETURN ? tr("Unreturn") : tr("Returned");
    });
    model->addColumn("count", tr("Count"), Qt::AlignHCenter);
    model->addColumnMoney("total", tr("Total"));
    model->addColumn("return_date", tr("Return Date"), Qt::AlignLeft, [](TableItem *item, const QString &key) {
        if(item->data("status").toInt() == PURCHASE_RETURN_STATUS::UNRETURN) return tr("-");
        return LibDB::DBUtil::sqlDateToDateTime(item->data(key).toString()).toString("dd-MM-yyyy");
    });
    model->addColumn("item_returned", tr("Item<br>Returned"), Qt::AlignHCenter);
    model->addColumnMoney("money_returned", tr("Money<br>Returned"));
    model->addColumn("note", tr("Note"));
    model->addHeaderFilter("barcode", HeaderFilter{HeaderWidget::LineEdit, TableModel::FilterLike, QVariant()});
    model->addHeaderFilter("name", HeaderFilter{HeaderWidget::LineEdit, TableModel::FilterLike, QVariant()});
    model->addHeaderFilter("suplier", HeaderFilter{HeaderWidget::LineEdit, TableModel::FilterLike, QVariant()});
    model->addHeaderFilter("status", HeaderFilter{HeaderWidget::Combo, TableModel::FilterEQ, QVariant()});
    model->addHeaderFilter("return_type", HeaderFilter{HeaderWidget::Combo, TableModel::FilterEQ, QVariant()});
    model->setTypeCommand(MSG_TYPE::SOLDRETURN, MSG_COMMAND::QUERY);
    model->setTypeCommandOne(MSG_TYPE::SOLDRETURN, MSG_COMMAND::GET);
    mTableWidget->setupTable();
    GuiUtil::setColumnWidth(mTableWidget->getTableView(), QList<int>() << 150 << 150 << 200 << 100 << 150 << 100 << 100 << 100 << 100 << 100);
    mTableWidget->getTableView()->horizontalHeader()->setStretchLastSection(true);
    model->setSort("created_at DESC");
    model->refresh();

    connect(mTableWidget, SIGNAL(addClicked()), SLOT(addClicked()));
    connect(mTableWidget, SIGNAL(updateClicked(QModelIndex)), SLOT(updateClicked(QModelIndex)));
    connect(mTableWidget, SIGNAL(deleteClicked(QModelIndexList)), SLOT(deleteClicked(QModelIndexList)));
    connect(model, SIGNAL(firstDataLoaded()), SLOT(getSummary()));
}

void SoldItemReturnWidget::messageReceived(Message *msg)
{
    if(msg->isTypeCommand(MSG_TYPE::SOLDRETURN, MSG_COMMAND::SUMMARY)) {
        if(msg->isSuccess()) {
            mTotalDebt->setValue(Preference::formatMoney(msg->data("total").toDouble()));
        }
    } else if(msg->isTypeCommand(MSG_TYPE::SOLDRETURN, MSG_COMMAND::DEL)) {
        mTableWidget->getModel()->refresh();
    }
}

void SoldItemReturnWidget::addClicked()
{
    AddSoldReturnDialog dialog(mMessageBus, this);
    dialog.exec();
    mTableWidget->getModel()->refresh();
}

void SoldItemReturnWidget::updateClicked(const QModelIndex &index)
{
    auto item = static_cast<TableItem*>(index.internalPointer());
    AddSoldReturnDialog dialog(mMessageBus, this);
    dialog.fill(item->data());
    dialog.exec();
    mTableWidget->getModel()->refresh();
}

void SoldItemReturnWidget::deleteClicked(const QModelIndexList &index)
{
    if(index.empty()) return;
    int res = QMessageBox::question(this, tr("Confirmation remove"), tr("Are you sure to delete the item?"));
    if(res == QMessageBox::Yes) {
        QList<QVariant> ids;
        for(int i = 0; i < index.size(); i++) {
            auto item = static_cast<TableItem*>(index[i].internalPointer());
            ids.append(item->id);
        }
        Message msg(MSG_TYPE::SOLDRETURN, MSG_COMMAND::DEL);
        msg.addData("id", ids);
        sendMessage(&msg);
    }
}

void SoldItemReturnWidget::getSummary()
{
    Message msg(MSG_TYPE::SOLDRETURN, MSG_COMMAND::SUMMARY);
    sendMessage(&msg);
}
