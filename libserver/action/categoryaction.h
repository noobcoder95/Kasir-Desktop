/*
 * categoryaction.h
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
#ifndef CATEGORYACTION_H
#define CATEGORYACTION_H

#include "serveraction.h"

namespace LibServer {

class CategoryAction : public ServerAction
{
public:
    CategoryAction();

protected:
    void afterInsert(const QVariantMap &data) override;
    void afterDelete(const QVariantMap &oldData) override;
    void parentAddChild(int parent, int child);
};

}
#endif // CATEGORYACTION_H
