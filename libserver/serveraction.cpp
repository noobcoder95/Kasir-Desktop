/*
 * serveraction.cpp
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
#include "serveraction.h"
#include "global_constant.h"
#include "db.h"
#include "queryhelper.h"
#include <QStringBuilder>
#include <QDateTime>
#include <QDebug>

using namespace LibServer;
using namespace LibG;
using namespace LibDB;

static QString TAG{"[SERVERACTION]"};

ServerAction::ServerAction(const QString &tableName, const QString idfield):
    mDb(Db::createInstance()),
    mTableName(tableName),
    mIdField(idfield)
{
    mFunctionMap.insert(MSG_COMMAND::INSERT, std::bind(&ServerAction::insert, this, std::placeholders::_1));
    mFunctionMap.insert(MSG_COMMAND::UPDATE, std::bind(&ServerAction::update, this, std::placeholders::_1));
    mFunctionMap.insert(MSG_COMMAND::DEL, std::bind(&ServerAction::del, this, std::placeholders::_1));
    mFunctionMap.insert(MSG_COMMAND::GET, std::bind(&ServerAction::get, this, std::placeholders::_1));
    mFunctionMap.insert(MSG_COMMAND::QUERY, std::bind(&ServerAction::query, this, std::placeholders::_1));
    mFunctionMap.insert(MSG_COMMAND::RESTORE_DELETED, std::bind(&ServerAction::restoreDelete, this, std::placeholders::_1));
}

ServerAction::~ServerAction()
{
    delete mDb;
}

Message ServerAction::exec(Message *msg)
{
    if(mFunctionMap.contains(msg->command()))
        return mFunctionMap[msg->command()](msg);
    qCritical() << TAG << "Command not available : " << msg->command() << " with type : " << msg->type();
    Message message = *msg;
    message.setError("Command not available");
    return message;
}

LibG::Message ServerAction::insert(LibG::Message *msg)
{
    LibG::Message message(msg);
    if(hasFlag(HAS_UPDATE_FIELD))
        msg->addData("updated_at", QDateTime::currentDateTime());
    if(hasFlag(USE_TRANSACTION) && mDb->isSupportTransaction())
        mDb->beginTransaction();
    if(hasFlag(BEFORE_INSERT)) {
        if(!beforeInsert(msg->data(), &message)) return message;
    }
    if(!mDb->insert(mTableName, msg->data())) {
        message.setError(mDb->lastError().text());
    } else {
        DbResult res = mDb->where("id = ", mDb->lastInsertedId())->get(mTableName);
        const QVariantMap d = res.first();
        if(hasFlag(AFTER_INSERT)) afterInsert(d);
        message.setData(d);
    }
    if(hasFlag(USE_TRANSACTION) && mDb->isSupportTransaction()) {
        if(!mDb->commit()) {
            mDb->roolback();
            message.setError(mDb->lastError().text());
        }
    }
    return message;
}

LibG::Message ServerAction::update(LibG::Message *msg)
{
    LibG::Message message(msg);
    mDb->where(mIdField % " = ", msg->data(mIdField));
    QVariantMap d = msg->data("data").toMap();
    if(hasFlag(HAS_UPDATE_FIELD))
        d.insert("updated_at", QDateTime::currentDateTime());
    if(hasFlag(USE_TRANSACTION) && mDb->isSupportTransaction())
        mDb->beginTransaction();
    DbResult oldRes = mDb->clone()->get(mTableName);
    const QVariantMap &oldData = oldRes.first();
    if(hasFlag(BEFORE_UPDATE)) {
        if(!beforeUpdate(oldData, msg, &message)) return message;
    }
    if(!mDb->clone()->update(mTableName, d)) {
        message.setError(mDb->lastError().text());
    } else {
        DbResult res = mDb->get(mTableName);
        const QVariantMap d = res.first();
        if(hasFlag(AFTER_UPDATE)) afterUpdate(oldData, d);
        message.setData(d);
    }
    if(hasFlag(USE_TRANSACTION) && mDb->isSupportTransaction()) {
        if(!mDb->commit()) {
            mDb->roolback();
            message.setError(mDb->lastError().text());
        }
    }
    return message;
}

LibG::Message ServerAction::del(LibG::Message *msg)
{
    LibG::Message message(msg);
    if(hasFlag(USE_TRANSACTION) && mDb->isSupportTransaction())
        mDb->beginTransaction();
    QVariantList ids;
    if(msg->data(mIdField).canConvert<QString>())
        ids.append(msg->data(mIdField));
    else
        ids = msg->data(mIdField).toList();
    for(int i = 0; i < ids.size(); i++) {
        DbResult res = mDb->where(mIdField % " = ", ids[i])->get(mTableName);
        if(hasFlag(BEFORE_DELETE)) {
            if(!res.isEmpty() && !beforeDelete(res.first(), &message)) return message;
        }
        mDb->where(mIdField % " = ", ids[i]);
        if(hasFlag(SOFT_DELETE)) {
            if(!mDb->update(mTableName, QVariantMap{{"deleted_at", QDateTime::currentDateTime()}})) {
                message.setError(mDb->lastError().text());
            }
        } else {
            if(!mDb->del(mTableName)) {
                message.setError(mDb->lastError().text());
            }
        }
        if(hasFlag(AFTER_DELETE)) {
            if(!res.isEmpty())
                afterDelete(res.first());
        }
    }
    if(hasFlag(USE_TRANSACTION) && mDb->isSupportTransaction()) {
        if(!mDb->commit()) {
            mDb->roolback();
            message.setError(mDb->lastError().text());
        }
    }
    return message;
}

Message ServerAction::restoreDelete(Message *msg)
{
    LibG::Message message(msg);
    if(hasFlag(USE_TRANSACTION) && mDb->isSupportTransaction())
        mDb->beginTransaction();
    DbResult res = mDb->where(mIdField % " = ", msg->data(mIdField))->get(mTableName);
    const QVariantMap &oldData = res.first();
    if(hasFlag(BEFORE_RESTORE)) {
        if(!beforeRestore(oldData, msg, &message)) return message;
    }
    mDb->where(mIdField % " = ", msg->data(mIdField));
    if(hasFlag(SOFT_DELETE)) {
        if(!mDb->update(mTableName, QVariantMap{{"deleted_at", QVariant(QVariant::DateTime)}})) {
            message.setError(mDb->lastError().text());
        }
    }
    if(hasFlag(AFTER_RESTORE)) {
        if(!res.isEmpty())
            afterRestore(res.first());
    }
    message.setData(oldData);
    if(hasFlag(USE_TRANSACTION) && mDb->isSupportTransaction()) {
        if(!mDb->commit()) {
            mDb->roolback();
            message.setError(mDb->lastError().text());
        }
    }
    return message;
}

Message ServerAction::get(Message *msg)
{
    LibG::Message message(msg);
    selectAndJoin();
    if(hasFlag(SOFT_DELETE) && !msg->hasData("withdeleted")) mDb->where(mTableName % ".deleted_at IS NULL");
    DbResult res = mDb->where(mTableName % "." % mIdField % " = ", msg->data(mIdField))->get(mTableName);
    if(res.isEmpty()) {
        message.setError("Data not found");
    } else {
        message.setData(res.first());
    }
    return message;
}

LibG::Message ServerAction::query(LibG::Message *msg)
{
    LibG::Message message(msg);
    mDb->table(mTableName);
    selectAndJoin();
    if(hasFlag(SOFT_DELETE) && !msg->hasData("withdeleted")) mDb->where(mTableName % ".deleted_at IS NULL");
    mDb = QueryHelper::filter(mDb, msg->data(), fieldMap());
    if(!(msg->data().contains(QStringLiteral("start")) && msg->data().value(QStringLiteral("start")).toInt() > 0))
        message.addData(QStringLiteral("total"), mDb->count());
    setStart(&message, msg);
    mDb = QueryHelper::limitOffset(mDb, msg->data());
    mDb = QueryHelper::sort(mDb, msg->data());
    message.addData(QStringLiteral("data"), mDb->exec().data());
    return message;
}

void ServerAction::setStart(Message *msg, Message *src)
{
    if(src->data().contains(QStringLiteral("start"))) {
        msg->addData(QStringLiteral("start"), src->data(QStringLiteral("start")).toInt());
    }
}

QMap<QString, QString> ServerAction::fieldMap() const
{
    return QMap<QString, QString>();
}

bool ServerAction::hasFlag(int flag)
{
    return (mFlag & flag) != 0;
}
