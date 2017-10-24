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
#include "base_reports.h"

/**
 * @brief Reporting::Base::Base
 * @param config
 * @param parent
 */
Reporting::CreateBase::CreateBase(TConfiguration *config, QObject *parent) : QObject(parent)
{
    MyConfiguration = config;

}

/**
 * @brief Reporting::Base::init
 */
void Reporting::CreateBase::init()
{
    document        = QDomDocument();
    document.appendChild(document.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"UTF-8\""));
    root            = document.createElementNS("http://schemas.garlic-player.com/gapi-1.0", "report");
    document.appendChild(root);

    root.appendChild(createTagWithTextValue("date", QDateTime::currentDateTimeUtc().toString(Qt::ISODate)));
    root.appendChild(createTagWithTextValue("version", "1.0"));

    player = document.createElement("player");
    root.appendChild(player);
    player.setAttribute("id", MyConfiguration->getUuid());

}

/**
 * @brief Reporting::Base::createTagWithTextValue
 * @param tag_name
 * @param tag_value
 * @return
 */
QDomElement Reporting::CreateBase::createTagWithTextValue(QString tag_name, QString tag_value)
{
    QDomElement tag = document.createElement(tag_name);
    tag.appendChild(document.createTextNode(tag_value));
    return tag;
}

/**
 * @brief Reporting::Base::createPropTag
 * @param name
 * @param value
 * @return
 */
QDomElement Reporting::CreateBase::createPropTag(QString name, QString value)
{
    QDomElement tag = document.createElement("prop");
    tag.setAttribute("name", name);
    tag.setAttribute("value", value);
    return tag;
}
