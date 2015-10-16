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

#ifndef __object_registry__
#define __object_registry__

#include<string>
#include<utility>
#include<map>
#include<sigc++/functors/slot.h>
#include<tlv_stream.h>

const unsigned int ERR_METHOD_NOT_FOUND = 100;
const unsigned int ERR_CLASS_NOT_FOUND = 101;
const unsigned int ERR_NOT_IMPLEMENTED = 102;
const unsigned int ERR_SYNTAX_INPUT = 103;
const unsigned int ERR_OBJECT_CREATE = 104;

using namespace std;

class object_registry;
typedef sigc::slot<unsigned int, tlv_entry&, tlv_stream *> tlv_callback;

class service_provider {
public:
    service_provider(object_registry *obj_registry) { registry = obj_registry; counter = 0; }

    virtual tlv_callback *make_new_handler() = 0;    
    virtual unsigned int new_object(tlv_entry& params, tlv_stream *out_stream) = 0;
    virtual tlv_callback *make_functor(string& method_name, void *object) = 0;
    virtual void delete_object(void *obj_to_delete) = 0;
    virtual void make_handle(string& new_handle);
    virtual string get_name() = 0;
    
    virtual ~service_provider() { ; }

protected:
    object_registry *registry;
    unsigned long int counter;
};

class registry_manager;
typedef unsigned int (registry_manager::*manager_fun)(tlv_entry& params, tlv_stream *out_stream); 

class registry_manager {
public:
    registry_manager(object_registry *rgstry);
    
    tlv_callback *get_handler(string& method_name);
    virtual unsigned int clear_processor(tlv_entry& params, tlv_stream *out_stream);
    virtual unsigned int list_objects_processor(tlv_entry& params, tlv_stream *out_stream);
    virtual unsigned int list_providers_processor(tlv_entry& params, tlv_stream *out_stream);    
    
    virtual ~registry_manager() { ; }
    
protected:
    object_registry *registry;
    map<string, manager_fun> method_pointers;
};

class object_registry {
public:
    object_registry() : manager(this) { ; }
    virtual tlv_callback *get_processor(string& object_name, string& method);
    
    virtual void add_object(string& name, pair<void *, service_provider *>& new_object);
    virtual void delete_object(string& object_name);
    virtual void clear();
    virtual map<string, pair<void *, service_provider *> >& get_objects() { return objects; } 
    virtual map<string, service_provider *>& get_providers() { return func_factory; } 
    
    /*! \brief Adds the service_provider object referenced through parameter provider under the name given in parameter class_name to
     *         the object registry.
     *
     *  Important: The object_registry instance takes ownership of the object referenced through the parameter provider and deletes
     *  it when this becomes necessary.
     */
    virtual void add_service_provider(string& class_name, service_provider *provider);
    virtual void delete_service_provider(string& class_name);
    
    virtual ~object_registry();
    
protected:
    // object_name -> <object, service_provider>
    map<string, pair<void *, service_provider *> > objects;
    // class_name -> service_provider
    map<string, service_provider *> func_factory;
    registry_manager manager;
};


#endif /* __object_registry__ */
