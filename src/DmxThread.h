#ifndef DmxThread_H_
#define DmxThread_H_

#include <string>
#include "DmxBuffer.h"
#include "Thread.h"


class OpenDmxThread: public Thread {
  public:
    explicit OpenDmxThread(const string &path);
    ~OpenDmxThread() {}

    bool Stop();
    bool WriteDmx(const DmxBuffer &buffer);
    void *Run();

  private:
    int m_fd;
    string m_path;
    DmxBuffer m_buffer;
    bool m_term;
    Mutex m_mutex;
    Mutex m_term_mutex;
    ConditionVariable m_term_cond;

    static const int INVALID_FD = -1;
};

#endif
