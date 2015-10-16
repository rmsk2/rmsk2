/***************************************************************************
 * Copyright 2015 Martin Grap
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ***************************************************************************/

#ifndef __tlv_server_h__
#define __tlv_server_h__

#include<string>
#include<sigc++/functors/slot.h>
#include<tlv_stream.h>
#include<object_registry.h>

using namespace std;

const int ERR_READ_OBJECT_NAME = 42;
const int ERR_READ_METHOD_NAME = 43;
const int ERR_DETERMINE_PROCESSOR = 51;

class tlv_server {
public:
    virtual unsigned int start(sigc::slot<unsigned int, tlv_stream *, object_registry *> processor, object_registry *registry) = 0;
    virtual unsigned int stop() = 0;
    
    virtual ~tlv_server() { ; }
};

class uxdomain_socket_server : public tlv_server {
public:
    uxdomain_socket_server(string& path);
    
    virtual unsigned int start(sigc::slot<unsigned int, tlv_stream *, object_registry *> processor, object_registry *registry);
    virtual unsigned int stop();    
    virtual unsigned int on_connect(tlv_stream *client_stream, object_registry *registry);

    virtual ~uxdomain_socket_server() { ; }   
protected:
    string address_path;   
    bool keep_running;
    int sock_fd, new_sock_fd; 
    object_registry *reg_help;
};

#endif /* __tlv_server_h__ */
