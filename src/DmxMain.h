#ifndef image_H_
#define image_H_

#include <node.h>
#include <v8.h>

#include "DmxThread.h"

using namespace v8;

class DmxMain : node::ObjectWrap {
public:
    static void init(Handle<Object> target);

    static Handle<Value> New(const Arguments& args);

    static Handle<Value> open(const Arguments& args);
    static Handle<Value> start(const Arguments& args);
    static Handle<Value> close(const Arguments& args);
    static Handle<Value> write(const Arguments& args);

protected:
    ~DmxMain();

private:
    DmxMain(Handle<Object> wrapper);
    static Persistent<FunctionTemplate> constructor_template;
    
    OpenDmxThread* m_Thread;
    
};

#endif
