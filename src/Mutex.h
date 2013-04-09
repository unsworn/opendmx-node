/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 * OpenDMX driver from OLA. 
 * adapted for node.js with fixed darwin support
 * Copyright (C) 2010 Simon Newton              
 * Copyright (C) 2013 Nicklas Marelius
 */

#ifndef Mutex_H_
#define Mutex_H_

#include <pthread.h>

class Mutex 
{
public:
    
    Mutex();
    ~Mutex();

    void Lock();
    bool TryLock();
    void Unlock();

private:
    pthread_mutex_t m_mutex;

    Mutex(const Mutex&);
    Mutex& operator=(const Mutex&);
    
    friend class ConditionVariable;
};

class MutexLocker
{
public:
    explicit MutexLocker(Mutex *mutex);
    ~MutexLocker();

private:
    Mutex *m_mutex;

    MutexLocker(const MutexLocker&);
    MutexLocker& operator=(const MutexLocker&);
};

class ConditionVariable 
{
public:
    ConditionVariable();
    ~ConditionVariable();

    void Wait(Mutex *mutex);
    bool TimedWait(Mutex *mutex, struct timespec *wait_time);

    void Signal();
    void Broadcast();

private:
    pthread_cond_t m_condition;

    ConditionVariable(const ConditionVariable&);
    ConditionVariable& operator=(const ConditionVariable&);
};

#endif
