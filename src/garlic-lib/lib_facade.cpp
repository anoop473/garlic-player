﻿/*************************************************************************************
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
#include "lib_facade.h"

LibFacade::LibFacade(QObject *parent) : QObject(parent)
{

}



LibFacade::~LibFacade()
{
    killTimer(resource_monitor_timer_id);
}

/**
 * This is needed because qmlRegisterType in player-c2qml did not work
 * with constructor params except QObject
 * So we need to make the dependencyinjections here
 *
 * @brief LibFacade::init
 * @param config
 */
void LibFacade::init(MainConfiguration *config)
{
    MyConfiguration.reset(config);

    MyIndexManager.reset(new Files::IndexManager(MyConfiguration.data(), this));
    connect(MyIndexManager.data(), SIGNAL(readyForLoading()), this, SLOT(loadIndex()));

    resource_monitor_timer_id = startTimer(300000); // every 300s for ressource monitor

    MyTaskScheduler.reset(new SmilHead::TaskScheduler(MyConfiguration.data(), this));
    connect(MyTaskScheduler.data(), SIGNAL(applyConfiguration()), this, SLOT(changeConfig()));
    connect(MyTaskScheduler.data(), SIGNAL(installSoftware(QString)), this, SLOT(emitInstallSoftware(QString)));
    connect(MyTaskScheduler.data(), SIGNAL(reboot()), this, SLOT(reboot()));

    MyInventoryTable.reset(new DB::InventoryTable(this));
    MyInventoryTable.data()->init(MyConfiguration.data()->getPaths("logs"));
}

void LibFacade::shutDownParsing()
{
    MyBodyParser.data()->endPlaying();
    MyIndexManager.data()->deactivateRefresh();
}


void LibFacade::initParser()
{
    MyIndexManager.data()->init(MyConfiguration.data()->getIndexUri());
    MyIndexManager.data()->lookUpForUpdatedIndex();
    loadIndex();
}


void LibFacade::setConfigFromExternal(QString config_path, bool restart_smil_parsing)
{
    MyXMLConfiguration.reset(new SmilHead::XMLConfiguration(MyConfiguration.data(), this));
    if (restart_smil_parsing)
        connect(MyXMLConfiguration.data(), SIGNAL(finishedConfiguration()), this, SLOT(reboot()));
    MyXMLConfiguration.data()->processFromLocalFile(config_path);
}

/**
 * When new index/content_url came from an external source like a launcher at runtime
 *
 * @brief LibFacade::reloadWithNewIndex
 * @param index_path
 */
void LibFacade::reloadWithNewIndex(QString index_path)
{
    MyConfiguration->setIndexUri(index_path);
    initParser();
}


void LibFacade::beginSmilPlaying()
{
    MyBodyParser.data()->beginPlaying();
}

QString LibFacade::requestLoaddableMediaPath(QString path)
{
    return MyMediaManager.data()->requestLoadablePath(path);
}

void LibFacade::nextSmilMedia(int zone)
{
    Q_UNUSED(zone);
}

void LibFacade::previousSmilMedia(int zone)
{
    Q_UNUSED(zone);
}

void LibFacade::jumpToSmilMedia(int position, int zone)
{
    Q_UNUSED(position);
    Q_UNUSED(zone);
}


void LibFacade::loadIndex()
{
    MyIndexManager.data()->init(MyConfiguration.data()->getIndexUri());
    if (!MyBodyParser.isNull())
    {
        MyBodyParser.data()->endPlaying();
        MyIndexManager.data()->deactivateRefresh();
    }

    // Start with this only when it is absolutly sure that in the player component is no activity anymore.
    if (!MyIndexManager.data()->load())
        return;

    initFileManager();

    processHeadParsing();

}

void LibFacade::changeConfig()
{
    if (has_launcher)
    {
        emit newConfig();
    }
    else
    {
        loadIndex();
    }
}



void LibFacade::initFileManager()
{
    MyMediaModel.reset(new MediaModel(this));
    MyDownloadQueue.reset(new DownloadQueue(MyConfiguration.data(), this));
    MyDownloadQueue.data()->setInventoryTable(MyInventoryTable.data());
    MyMediaManager.reset(new Files::MediaManager(MyMediaModel.data(), MyDownloadQueue.data(), MyConfiguration.data(), this));
    qDebug(Develop) <<  " end initFileManager" ;
}

void LibFacade::processHeadParsing()
{
    MyHeadParser.reset(new HeadParser(MyConfiguration.data(), MyMediaManager.data(), MyInventoryTable.data(), this));
    connect(MyHeadParser.data(), SIGNAL(parsingCompleted()), this, SLOT(processBodyParsing()));

    MyHeadParser.data()->parse(MyIndexManager->getHead(), MyTaskScheduler.data());
    qDebug(Develop) <<  " end processHeadParsing" ;
}

void LibFacade::processBodyParsing()
{
    MyElementsContainer.reset(new ElementsContainer(MyHeadParser.data(), this)); // must be setted, when Layout is known

    MyBodyParser.reset(new BodyParser(MyMediaManager.data(), MyElementsContainer.data(), this));

    connect(MyBodyParser.data(), SIGNAL(preloadingBodyCompleted()), this, SLOT(preparedForPlaying()));
    connect(MyBodyParser.data(), SIGNAL(startShowMedia(BaseMedia *)), this, SLOT(emitStartShowMedia(BaseMedia *)));
    connect(MyBodyParser.data(), SIGNAL(stopShowMedia(BaseMedia *)), this, SLOT(emitStopShowMedia(BaseMedia *)));
    MyBodyParser->beginPreloading(MyIndexManager->getBody());
    qDebug(Develop) <<  " end processBodyParsing" ;
}

void LibFacade::preparedForPlaying()
{
    MyIndexManager.data()->activateRefresh(MyHeadParser->getRefreshTime());

    emit readyForPlaying();
    qDebug(Develop) <<  " emit readyForPlaying" ;
}

void LibFacade::timerEvent(QTimerEvent *event)
{
    Q_UNUSED(event);
    MyResourceMonitor.refresh();

    qInfo(Develop) << MyResourceMonitor.getTotalMemorySystem() << MyResourceMonitor.getFreeMemorySystem();
    qInfo(Develop) << MyResourceMonitor.getMemoryAppUse() << MyResourceMonitor.getMaxMemoryAppUsed();
    qInfo(Develop) << MyResourceMonitor.getThreadsNumber() << MyResourceMonitor.getMaxThreadsNumber();
}

void LibFacade::emitInstallSoftware(QString file_path)
{
    emit installSoftware(file_path);
}

void LibFacade::reboot()
{
    emit rebootOS();
}

void LibFacade::emitStartShowMedia(BaseMedia *media)
{
    emit startShowMedia(media);
    qDebug(Develop) << "emitStartShowMedia " << media->getID();
}

void LibFacade::emitStopShowMedia(BaseMedia *media)
{
    emit stopShowMedia(media);
    qDebug(Develop) << "emitStopShowMedia " << media->getID();
}

