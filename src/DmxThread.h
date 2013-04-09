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
 
#ifndef DmxThread_H_
#define DmxThread_H_

#include <string>

#include <ftd2xx.h>

#include "DmxBuffer.h"
#include "Thread.h"


class DmxThread: public Thread {
  public:
    explicit DmxThread(const string &path);
    ~DmxThread() {}
    bool Open();
    void Dispose();
    bool Stop();
    bool WriteDmx(const DmxBuffer &buffer);
    void *Run();
  protected:
    void InitFt();
  private:
    int m_fd;
    string m_path;
    DmxBuffer m_buffer;
    bool m_term;
    Mutex m_mutex;
    Mutex m_term_mutex;
    ConditionVariable m_term_cond;
    FT_HANDLE m_handle;
    static const int INVALID_FD = -1;
};

#endif
