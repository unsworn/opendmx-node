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
 * Thread.h
 * A thread object.
 * Copyright (C) 2010 Simon Newton
 */

#ifndef Thread_H_
#define Thread_H_

#include <pthread.h>
#include "Mutex.h"

typedef pthread_t ThreadId;

/**
 * A thread object to be subclassed.
 */
class Thread {
  public:
    Thread(): m_thread_id(), m_running(false) {}
    virtual ~Thread() {}

    virtual bool Start();
    virtual bool FastStart();
    virtual bool Join(void *ptr = NULL);
    bool IsRunning();

    ThreadId Id() const { return m_thread_id; }

    // Called by pthread_create
    void *_InternalRun();

    static inline ThreadId Self() { return pthread_self(); }

  protected:
    // Sub classes implement this.
    virtual void *Run() = 0;

  private:
    pthread_t m_thread_id;
    bool m_running;
    Mutex m_mutex;  // protects m_running
    ConditionVariable m_condition;  // use to wait for the thread to start
};

#endif
