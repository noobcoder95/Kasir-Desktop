/*
 * categoryaddialog.h
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
#ifndef CATEGOTYADDDIALOG_H
#define CATEGOTYADDDIALOG_H

#include <QDialog>

class QComboBox;

namespace Ui {
class CategoryAddDialog;
}

namespace LibGUI {

class CategoryAddDialog : public QDialog
{
    Q_OBJECT

public:
    CategoryAddDialog(QWidget *parent = nullptr);
    ~CategoryAddDialog();
    void reset();
    void fill(int id, int parent, const QString &name, const QString &code);
    void enableSaveButton(bool enable);
    QComboBox *getComboParent();

private:
    Ui::CategoryAddDialog *ui;
    int mId = 0;

signals:
    void saveRequest(const QVariantMap &data, int id);

private slots:
    void saveClicked();
};

}
#endif // CATEGOTYADDDIALOG_H
