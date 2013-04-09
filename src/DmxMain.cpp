#include "DmxMain.h"
#include <node_buffer.h>

#include <string>
#include <string.h>

using namespace std;
using namespace node;
using namespace v8;

Persistent<FunctionTemplate> DmxMain::constructor_template;

DmxMain::DmxMain(Handle<Object> wrapper) :
m_Thread(NULL)
{
}

DmxMain::~DmxMain()
{
    if (this->m_Thread != NULL) {
        fprintf(stderr, "Warning opendmx.io not closed properly\n");
        if (this->m_Thread->IsRunning()) {
            this->m_Thread->Stop();
            this->m_Thread->Join();
        }
        delete this->m_Thread;
        this->m_Thread = NULL;
    }
}

void
DmxMain::init(Handle<Object> target) {
    HandleScope scope;

    Local<FunctionTemplate> t = FunctionTemplate::New(DmxMain::New);

    constructor_template = Persistent<FunctionTemplate>::New(t);

    constructor_template->InstanceTemplate()->SetInternalFieldCount(1);

    constructor_template->SetClassName(String::New("io"));

    NODE_SET_PROTOTYPE_METHOD(constructor_template, "open", open);
    NODE_SET_PROTOTYPE_METHOD(constructor_template, "start", start);
    NODE_SET_PROTOTYPE_METHOD(constructor_template, "close", close);
    NODE_SET_PROTOTYPE_METHOD(constructor_template, "write", write);

    target->Set(String::New("io"), constructor_template->GetFunction());
}

Handle<Value>
DmxMain::New(const Arguments& args) {
    HandleScope scope;
    DmxMain* dmx = new DmxMain(args.This());
    dmx->Wrap(args.This());
    return scope.Close(args.This());
}

Handle<Value>
DmxMain::open(const Arguments& args) {
    struct stat st;
    
    HandleScope scope;
    Local<Object> obj = args.This();
    DmxMain* dmx = ObjectWrap::Unwrap<DmxMain>(obj);

    if (args.Length() < 1) {
        fprintf(stderr, "io::open() needs atleast 1 argument\n");
        return scope.Close(Boolean::New(false));
    }
    
    if (dmx->m_Thread != NULL) {
        fprintf(stderr, "io::open() already open\n");
        return scope.Close(Boolean::New(true));
    }
        
    String::Utf8Value path(args[0]);

    if (stat(*path, &st) != 0) {
        fprintf(stderr, "io::open() no such file or directory: %s\n", *path);
        return scope.Close(Boolean::New(false));
    }
    
    dmx->m_Thread = new OpenDmxThread( *path );
            
    return scope.Close(Boolean::New(true));
}

Handle<Value>
DmxMain::start(const Arguments& args) {
    HandleScope scope;
    Local<Object> obj = args.This();
    DmxMain* dmx = ObjectWrap::Unwrap<DmxMain>(obj);
    
    if (dmx->m_Thread == NULL) {
        fprintf(stderr, "io::start() not started\n");
        return scope.Close(Boolean::New(false));
    }
                    
    return scope.Close(Boolean::New( dmx->m_Thread->Start() ));
}

Handle<Value>
DmxMain::close(const Arguments& args) {   
    HandleScope scope;
    Local<Object> obj = args.This();
    DmxMain* dmx = ObjectWrap::Unwrap<DmxMain>(obj);

    if (dmx->m_Thread == NULL) {
        fprintf(stderr, "io::close() already closed\n");
        return scope.Close(Boolean::New(false));
    }
    
    fprintf(stderr, "io::close() shutdown dmx thread\n");
    if (dmx->m_Thread->IsRunning()) {
        dmx->m_Thread->Stop();
        dmx->m_Thread->Join();
    }
        
    delete dmx->m_Thread;
    
    dmx->m_Thread = NULL;
    
    return scope.Close(Boolean::New(true));
}

Handle<Value>
DmxMain::write(const Arguments& args) {
    DmxBuffer buf;
    Buffer* bytes;
    
    HandleScope scope;    
    Local<Object> obj = args.This();
    DmxMain* dmx = ObjectWrap::Unwrap<DmxMain>(obj);
    
    if (args.Length() < 1) {
        fprintf(stderr, "io::write() needs atleast 1 argument\n");
        return scope.Close(Boolean::New(false));
    }

    if (!Buffer::HasInstance(args[0])) {
        fprintf(stderr, "io::write() first argument needs to be of type buffer\n");
        return scope.Close(Boolean::New(false));
    }
    
    if (dmx->m_Thread == NULL) {
        fprintf(stderr, "io::write() not started\n");
        return scope.Close(Boolean::New(false));
    }
    
    obj = args[0]->ToObject();
    
    //bytes = ObjectWrap::Unwrap<Buffer>();
    /*
    if (bytes == NULL) {
        fprintf(stderr, "io::write() unpack failed\n");
        return scope.Close(Boolean::New(false));
    }
    
    buf.Set((unsigned char*)Buffer::Data(bytes), Buffer::Length(bytes));
            
    return scope.Close(Boolean::New(dmx->m_Thread->WriteDmx(buf)));
    */
    return scope.Close(Boolean::New(true));
}

extern "C" {
    static void init_opendmx(Handle<Object> target) {
        DmxMain::init(target);
    }
    NODE_MODULE(opendmx, init_opendmx);
}       
