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
    NODE_SET_PROTOTYPE_METHOD(constructor_template, "stop", stop);
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
    string dev;
    
    HandleScope scope;
    Local<Object> obj = args.This();
    DmxMain* dmx = ObjectWrap::Unwrap<DmxMain>(obj);


    if (dmx->m_Thread != NULL) {
        fprintf(stderr, "io::open() already open\n");
        return scope.Close(Boolean::New(true));
    }                   
    
    if (args.Length() >= 1) {
        
        String::Utf8Value path(args[0]);

        if (stat(*path, &st) != 0) {
            fprintf(stderr, "io::open() no such file or directory: %s\n", *path);
            return scope.Close(Boolean::New(false));
        }
        
        dev = *path;
    }
    
    dmx->m_Thread = new DmxThread( dev );
            
    return scope.Close(Boolean::New( dmx->m_Thread->Open() ));
}

Handle<Value>
DmxMain::start(const Arguments& args) {
    HandleScope scope;
    Local<Object> obj = args.This();
    DmxMain* dmx = ObjectWrap::Unwrap<DmxMain>(obj);
    
    if (dmx->m_Thread == NULL) {
        fprintf(stderr, "io::start() not started, device not open\n");
        return scope.Close(Boolean::New(false));
    }
                    
    return scope.Close(Boolean::New( dmx->m_Thread->Start() ));
}

Handle<Value>
DmxMain::stop(const Arguments& args) {
    HandleScope scope;
    Local<Object> obj = args.This();
    DmxMain* dmx = ObjectWrap::Unwrap<DmxMain>(obj);
    
    if (dmx->m_Thread == NULL) {
        fprintf(stderr, "io::stop() not stopped, device not open\n");
        return scope.Close(Boolean::New(false));
    }
                    
    return scope.Close(Boolean::New( dmx->m_Thread->Stop() ));
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
    }
    
    dmx->m_Thread->Dispose();
    
    delete dmx->m_Thread;
    
    dmx->m_Thread = NULL;
    
    return scope.Close(Boolean::New(true));
}

Handle<Value>
DmxMain::write(const Arguments& args) {
    DmxBuffer buf;
    const char* bytes = NULL;
    int size = 0;
    
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
    
    
    size  = Buffer::Length(args[0]->ToObject());
    bytes = Buffer::Data(args[0]->ToObject());
    
    if (bytes == NULL) {
        fprintf(stderr, "io::write() unpack failed\n");
        return scope.Close(Boolean::New(false));
    }
    
    buf.Set((unsigned char*)bytes, size);
            
    return scope.Close(Boolean::New(dmx->m_Thread->WriteDmx(buf)));

}

extern "C" {
    static void init_opendmx(Handle<Object> target) {
        DmxMain::init(target);
    }
    NODE_MODULE(opendmx, init_opendmx);
}       
