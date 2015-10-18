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

#include<tlv_server.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<sys/time.h>
#include<unistd.h>
#include<stdlib.h>
#include<sys/un.h>
#include<iostream>
#include<memory>

uxdomain_socket_server::uxdomain_socket_server(string& path)
{
    address_path = path;
    keep_running = true;
}

unsigned int uxdomain_socket_server::stop()
{
    keep_running = false;
    
    return ERR_OK;
}

unsigned int uxdomain_socket_server::start(sigc::slot<unsigned int, tlv_stream *, object_registry *> processor, object_registry *registry)
{
    unsigned int result = ERR_OK;        
    int serv_len;
    socklen_t client_address_len;
    struct sockaddr_un  client_address, server_address;
    socket_tlv_stream *tlv_stream;
    struct timeval timeout;

    (void)unlink(address_path.c_str());
    timeout.tv_sec = 8;
    timeout.tv_usec = 0;

    reg_help = registry;

    do
    {       
        if ((sock_fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
        {
            result = ERR_SOCK_CREATE;
            break;
        }
        
        if (address_path.length() >= sizeof(server_address.sun_path))
        {
            result = ERR_SOCK_PATH_LEN;
            break;
        }        
            
        bzero((char *)&server_address, sizeof(server_address));
        server_address.sun_family = AF_UNIX;
        strcpy(server_address.sun_path, address_path.c_str());
        serv_len = strlen(server_address.sun_path) + sizeof(server_address.sun_family);
    
        if(bind(sock_fd, (struct sockaddr *)&server_address, serv_len) < 0)
        {
            close(sock_fd);
            result = SOCK_ERR_BIND;    
            break; 
        }

        listen(sock_fd, 5);
        client_address_len = sizeof(client_address);
       
        while(keep_running and (result == ERR_OK))
        {
            new_sock_fd = accept(sock_fd, (struct sockaddr *)&client_address, &client_address_len);
           
            if (new_sock_fd < 0) 
            {
                result = SOCK_ERR_ACCEPT;
                continue;
            }
            
            setsockopt(new_sock_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
            tlv_stream = new socket_tlv_stream(new_sock_fd);            
            result = processor(tlv_stream, registry);            
            delete tlv_stream;
        }
       
        close(sock_fd); 
       
        if (unlink(address_path.c_str()) != 0)
        {
            // Preserve original error code
            if (result == ERR_OK)
            {
                result = ERR_REMOVE_PATH;
            }
        }
        
    } while(0);
    
    return result;
}

unsigned int uxdomain_socket_server::on_connect(tlv_stream *client_stream, object_registry *registry)
{
    unsigned int result = ERR_OK;
    string string_val;
    vector<tlv_entry> entries;
    tlv_entry end_marker;
    tlv_entry object_name_read, method_name_read, parameters_read;
    string object_name, method_name;
    tlv_entry error_code, success_code;
    
    do
    {
        if ((result = client_stream->read_tlv(object_name_read)) != ERR_OK)
        {
            break;
        }
        
        if (!object_name_read.tlv_convert(object_name))
        {
            result = client_stream->write_error_tlv(ERR_READ_OBJECT_NAME);
            break;
        }
        
        if ((result = client_stream->read_tlv(method_name_read)) != ERR_OK)
        {
            break;
        }
        
        if (!method_name_read.tlv_convert(method_name))
        {
            result = client_stream->write_error_tlv(ERR_READ_METHOD_NAME);
            break;
        }

        if ((result = client_stream->read_tlv(parameters_read)) != ERR_OK)
        {
            break;
        }        

        if ((object_name == "root") and (method_name == "close"))
        {
            (void)stop();
            result = client_stream->write_error_tlv(0);
            break;
        }

        if (method_name == "delete")
        {
            registry->delete_object(object_name);
            result = client_stream->write_error_tlv(0);
            break;
        }
        
        unique_ptr<sigc::slot<unsigned int, tlv_entry&, tlv_stream *> > processor(registry->get_processor(object_name, method_name));
            
        if (processor.get() == NULL)
        {
            result = client_stream->write_error_tlv(ERR_DETERMINE_PROCESSOR);
            break;
        }        
        
        if ((result = (*processor)(parameters_read, client_stream)) != ERR_OK)
        {         
            //(void)client_stream->write_error_tlv(result);
            break;        
        }
                    
    } while(0);
            
    return result;
}

