/*************************************************************************************
    garlic-player: SMIL Player for Digital Signage
    Copyright (C) 2016 Nikolaos Saghiadinos <ns@smil-control.com>
    This file is part of the garlic-player source code

    This program is free software: you can redistribute it and/or  modify
    it under the terms of the GNU Affero General Public License, version 3,
    as published by the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Affero General Public License for more details.

    You should have received a copy of the GNU Affero General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*************************************************************************************/
#ifndef SUBSCRIPTION_H
#define SUBSCRIPTION_H

#include <QObject>
#include <QDomElement>

class SubScription : public QObject
{
        Q_OBJECT
    public:
        explicit SubScription(QObject *parent = nullptr);
        void parse(QDomElement element);
        QString getType() const;
        void setType(const QString &value);
        QString getAction() const;
        void setAction(const QString &value);
        QString getMethod() const;
        void setMethod(const QString &value);

        int getRefreshInterval() const;
        void setRefreshInterval(int value);
        int getRetryInterval() const;
        void setRetryInterval(int value);


    protected:
        QString   type, action, method = "";
        int       refresh_interval, retry_interval = 3600;
};

#endif // SUBSCRIPTION_LIST_H
