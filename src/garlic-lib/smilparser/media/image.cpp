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

#include "image.h"

TImage::TImage(TContainer *pc, Files::MediaManager *mm, MainConfiguration *config, QObject *parent) : BaseMedia(mm, config, parent)
{
    parent_container = pc;
    setParentTag(pc->getRootElement().nodeName());
    setObjectName("TImage");
    is_media = true;
}

TImage::~TImage()
{
}

void TImage::prepareDurationTimerBeforePlay()
{
    if (!MyExpr.executeQuery() || getLoadablePath().isEmpty())
    {
        skipElement();
        return;
    }

    if (status == _playing && !isRestartable())
        return;

    if (startDurTimer() || isEndTimerActive())
    {
        emitStartElementSignal(this);
    }
    else
    {
        skipElement();
    }
}

// ====================  protected methods =================================

void TImage::setAttributes()
{
    parseBaseMediaAttributes();
    src        = getAttributeFromRootElement("src", "");
}

// ====================  private methods =================================
