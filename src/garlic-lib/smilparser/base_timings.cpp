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

#include "base_timings.h"
// remarks ... delete after refactoring
// set end here cause begin and end depending on start of the par/excl parent. With seq it depends on last element
// end attribute has priority over dur (simple duration

BaseTimings::BaseTimings(QObject * parent) : TBase(parent)
{
}

BaseTimings::~BaseTimings()
{
    if (BeginTimer != Q_NULLPTR)
    {
        BeginTimer->stop();
        delete BeginTimer;
    }
    if (DurTimer != Q_NULLPTR)
    {
       DurTimer->stop();
       delete DurTimer;
    }
    if (EndTimer != Q_NULLPTR)
    {
        EndTimer->stop();
        delete EndTimer;
    }
    if (InternalTimer != Q_NULLPTR)
    {
        InternalTimer->stop();
        delete InternalTimer;
    }
}

void BaseTimings::startTimers()
{
    qDebug() << "prepare Play: " + getID() + " parent: " + getParentContainer()->getID();
    if (EndTimer != Q_NULLPTR)
    {
        EndTimer->start();
    }
    // must before start otherwise when begin = 0s
    // elements starts play method immediately get the played status and will be overwriten here
    status = _waiting;
    BeginTimer->start();
}

void BaseTimings::pauseTimers()
{
    qDebug() << "prepare Plause: " + getID() + " parent: " + getParentContainer()->getID();
    if (BeginTimer != Q_NULLPTR)
    {
        BeginTimer->pause();
    }
    if (DurTimer != Q_NULLPTR)
    {
       DurTimer->pause();
    }
    if (EndTimer != Q_NULLPTR)
    {
        EndTimer->pause();
    }
    return;
}

void BaseTimings::stopTimers()
{
    qDebug() << "prepare Stop: " + getID() + " parent: " + getParentContainer()->getID();
    if (BeginTimer != Q_NULLPTR)
    {
        BeginTimer->stop();
    }
    if (EndTimer != Q_NULLPTR)
    {
        EndTimer->stop();
    }
    if (DurTimer != Q_NULLPTR)
    {
        DurTimer->stop();
    }
}

void BaseTimings::resumeTimers()
{
    qDebug() << "prepare Resume: " + getID() + " parent: " + getParentContainer()->getID();
    if (EndTimer != Q_NULLPTR && !EndTimer->resume())
    {
        finishedActiveDuration();
        return; // not go further when ended
    }

    if (BeginTimer != Q_NULLPTR && BeginTimer->resume())
    {
        status          = _waiting;
        return; // not go further when a begin trigger is set
    }

    if (DurTimer != Q_NULLPTR)
    {
        DurTimer->resume();
    }
}

bool BaseTimings::hasActiveTimers()
{
    if (isEndTimerActive())
        return true;
    if (isDurTimerActive())
        return true;
    if (isBeginTimerActive())
        return true;

    return false;
}

void BaseTimings::finishedNotFound()
{
    // to fix https://github.com/sagiadinos/garlic-player/issues/12
    // when a lonely element (e.g video without dur) in a repeatCount=indefinite playlist not found,
    // a finishedActiveDuration will cause segmentation fault (heap crash) after some some hundreds runs
    // so we need to wait some milliseconds when timer is set.
    if (!BeginTimer->isActive() && isEndTimerActive() && isDurTimerActive())
    {
        skipElement();
    }
}

/**
 * @brief TBaseTiming::skipElement
 * this is needed to prevent a recursion stack overflow when download is not complete
 */
void BaseTimings::skipElement()
{
    if (InternalTimer == Q_NULLPTR)
    {
        InternalTimer = new QTimer(this);
        connect(InternalTimer, SIGNAL(timeout()), this, SLOT(emitfinishedActiveDuration()));
        InternalTimer->setSingleShot(true);
        InternalTimer->setTimerType(Qt::PreciseTimer);
    }
    InternalTimer->start(100);
}

// ========================= protected methods ======================================================

void BaseTimings::parseTimingAttributes()
{
    setBaseAttributes();
    handleBeginTimer();
    if (root_element.hasAttribute("end"))
    {
        EndTimer = new Timings::EnhancedTimer(this);
        connect(EndTimer, SIGNAL(timeout()), this, SLOT(finishedActiveDuration()));
        EndTimer->parse(root_element.attribute("end"));

        // todo: Check for sync und event trigger

    }
    if (root_element.hasAttribute("dur"))
    {
        DurTimer = new Timings::SimpleTimer(this);
        connect(DurTimer, SIGNAL(timeout()), this, SLOT(finishedSimpleDuration()));
        DurTimer->parse(root_element.attribute("dur"));
    }
    if (root_element.hasAttribute("repeatCount"))
    {
        setRepeatCount(root_element.attribute("repeatCount"));
    }
    if (root_element.hasAttribute("restart"))
    {
        setRestart(root_element.attribute("restart"));
    }
}

void BaseTimings::resetInternalRepeatCount()
{
    internal_count = 1;
}

bool BaseTimings::startDurTimer()
{
    if (DurTimer == Q_NULLPTR)
    {
        return false;
    }

    if (DurTimer->getType() == DurTimer->TYPE_CLOCKVALUE)
    {
        DurTimer->start();
        return true;
    }

    // todo what happens if end !="indefinite"
    if (DurTimer->getType() == DurTimer->TYPE_INDEFINITE)
    {
        return true; // no timer needed on indefinite
    }

    return false;
}

/**
 * @brief  TBaseTiming::checkRepeatCountStatus returns true, when playlist can be repeated
 * @return bool
 */
bool BaseTimings::handleRepeatCountStatus()
{
    bool ret = false;
    if (indefinite)
    {
        ret = true;
    }
    else if (repeatCount > 0 && internal_count < repeatCount)
    {
        internal_count++;
        ret = true;
    }
    return ret;
}

bool BaseTimings::isBeginTimerActive()
{
    if (BeginTimer == Q_NULLPTR)
    {
        return false;
    }
    else
    {
        return BeginTimer->isActive();
    }
}

bool BaseTimings::isEndTimerActive()
{
    if (EndTimer == Q_NULLPTR)
    {
        return false;
    }
    else
    {
        return EndTimer->isActive();
    }
}

bool BaseTimings::isDurTimerActive()
{
    if (DurTimer == Q_NULLPTR)
    {
        return false;
    }
    else
    {
        return DurTimer->isActive();
    }
}

bool BaseTimings::isRestartable()
{
    switch (restart)
    {
        case WHEN_NOT_ACTIVE:
            if (status == _pending)
                return true;
            else
                return false;
        case NEVER:
             stopTimers(); // important to prevent any kind of restart in this activity cycle
             return false;
        case ALWAYS:
        default:
            return true;
    }

}

void BaseTimings::finishedActiveDuration()
{
    qDebug() << getID() <<  " end active duration";
    emitfinishedActiveDuration();
}

/**
 * The intrinsic Duration can be also the implicit duration
 * of a seq, par, or excl time container
 * https://www.w3.org/TR/SMIL3/smil-timing.html#q69
 *
 * If we need more differentation one day we should add an
 * finishImplicitDuration method. At the moment only dur is supported
 *
 */
void BaseTimings::finishIntrinsicDuration()
{
    if (isDurTimerActive()) // video/audio can can have a dur-value > media duration
        return;

    finishedSimpleDuration();
}

void BaseTimings::finishedSimpleDuration()
{
    qDebug() << getID() <<  " end simple duration";
    if (handleRepeatCountStatus())
    {
        start();
        return;
    }

    if (BeginTimer->isActive() || isEndTimerActive())
    {
        status = _pending;
        return;
    }

    finishedActiveDuration();
}

// ========================= private methods ======================================================

void BaseTimings::setRepeatCount(QString rC)
{
    if (rC == "indefinite")
        indefinite = true;
    else
       indefinite = false;
    repeatCount = rC.toInt();
}

void BaseTimings::handleBeginTimer()
{
    BeginTimer = new Timings::EnhancedTimer(this);
    connect(BeginTimer, SIGNAL(timeout()), this, SLOT(prepareDurationTimerBeforePlay()));
    QString begin_value = "";

    if (getParentTag() == SMIL_TAG::excl)
        begin_value = getAttributeFromRootElement("begin", "indefinite");
    else
        begin_value = getAttributeFromRootElement("begin", "0s");

   // Todo: Check for event oder sync trigger

    BeginTimer->parse(begin_value);
}

void BaseTimings::setRestart(QString attr_value)
{
    if (attr_value == "never")
        restart = NEVER;
    else if (attr_value == "always")
        restart = ALWAYS;
    else
        restart = WHEN_NOT_ACTIVE;
}
