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

/*! \file tlv_server.h
 *  \brief Header file for the tlv_server and uxdomain_socket_server classes.
 */ 


#include<string>
#include<sigc++/functors/slot.h>
#include<tlv_stream.h>
#include<object_registry.h>

using namespace std;

const int ERR_READ_OBJECT_NAME = 42;
const int ERR_READ_METHOD_NAME = 43;
const int ERR_DETERMINE_PROCESSOR = 51;

/*! \brief An abstract TLV class which defines an interface for managing a TLV server.
 */
class tlv_server {
public:
    /*! \brief This method is expected to start a TLV server instance. The parameter processor has to specify a callback which is called
     *         each time a request is received by the server. The parameter registry has to point to the object registry which is to be
     *         used by this server instance.
     *
     *  Implementations of this method have to return ERR_OK (i.e. 0) if no error ocurred.
     */
    virtual unsigned int start(sigc::slot<unsigned int, tlv_stream *, object_registry *> processor, object_registry *registry) = 0;

    /*! \brief This method is expected to stop a TLV server instance.
     *
     *  Implementations of this method have to return ERR_OK (i.e. 0) if no error ocurred.
     */
    virtual unsigned int stop() = 0;

    /*! \brief Destructor.
     */    
    virtual ~tlv_server() { ; }
};

/*! \brief This TLV class implements the tlv_server interface where the communication is done via UNIX domain sockets. When the start
 *         method is called the server starts to listen on the given address.
 */
class uxdomain_socket_server : public tlv_server {
public:
    /*! \brief Constructor. The parameter path specifies the server's address, i.e. a file name.
     */
    uxdomain_socket_server(string& path);

    /*! \brief This method starts a uxdomain_socket_server instance. The parameter processor has to specify a callback which is called
     *         each time a request is received by the server. The parameter registry has to point to the object registry which is
     *         used by this server instance.
     *
     *  This method reuturns ERR_OK (i.e. 0) if no error ocurred.
     */    
    virtual unsigned int start(sigc::slot<unsigned int, tlv_stream *, object_registry *> processor, object_registry *registry);

    /*! \brief This method stops a uxdomain_socket_server instance.
     *
     *  This method returns ERR_OK (i.e. 0) if no error ocurred.
     */
    virtual unsigned int stop();    

    /*! \brief This method is intended to be called each time a client request is received. The parameter client_stream points to 
     *         a tlv_stream implementation which allows to talk to the connected client. The parameter registry has to point to an
     *         object registry which allows this method to determine an object which knows how to handle the request.
     *
     *  This method returns ERR_OK (i.e. 0) if no error ocurred.
     */
    virtual unsigned int on_connect(tlv_stream *client_stream, object_registry *registry);

    /*! \brief Destructor.
     */
    virtual ~uxdomain_socket_server() { ; }   
protected:
    /*! \brief Holds the file name which serves as the server's address. */ 
    string address_path;   
    /*! \brief While this variable is true the server continues to handle requests. */     
    bool keep_running;
    /*! \brief Holds the file handle of the server socket. */         
    int sock_fd;
    /*! \brief Holds the file handle of the socket which allows to talk to the newly connected client. */             
    int new_sock_fd; 
    /*! \brief Holds the object regsitry which is to be used by this uxdomain_socket_server instance. */             
    object_registry *reg_help;
};

#endif /* __tlv_server_h__ */
