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

#include<boost/lexical_cast.hpp>
#include<object_registry.h>


void service_provider::make_handle(string& new_handle)
{
    new_handle = get_name() + ":" + boost::lexical_cast<string>(counter);        
    counter++;
}

/* ---------------------------------------------------------------------------------------------- */
 

void object_registry::delete_object(string& object_name)
{
    if (objects.count(object_name) > 0)
    {
        (objects[object_name].second)->delete_object(objects[object_name].first);
        objects.erase(object_name);
    }
}

void object_registry::add_object(string& object_name, pair<void *, service_provider *>& new_object)
{
    if (objects.count(object_name) > 0)
    {
        (objects[object_name].second)->delete_object(objects[object_name].first);        
    }
    
    objects[object_name] = new_object;
}

void object_registry::add_service_provider(string& class_name, service_provider *provider)
{
    func_factory[class_name] = provider;
}

void object_registry::clear()
{
    map<string, pair<void *, service_provider*> >::iterator iter;

    for (iter = objects.begin(); iter != objects.end(); ++iter)
    {
        ((iter->second).second)->delete_object((iter->second).first);
    }
    
    objects.clear();    
}

object_registry::~object_registry()
{
    map<string, service_provider *>::iterator iter;

    clear();
    
    for (iter = func_factory.begin(); iter != func_factory.end(); ++iter)
    {
        delete iter->second;
    }
    
    func_factory.clear();
}

void object_registry::delete_service_provider(string& class_name)
{
    service_provider *provider;
    map<string, pair<void *, service_provider*> >::iterator iter;
    vector<string> objs_to_erase;
    vector<string>::iterator name_iter;

    if (func_factory.count(class_name) > 0)
    {
        provider = func_factory[class_name];
    
        for (iter = objects.begin(); iter != objects.end(); ++iter)
        {
            if ((iter->second).second == provider)
            {
                provider->delete_object((iter->second).first);
                objs_to_erase.push_back(iter->first);
            }
        }
        
        for (name_iter = objs_to_erase.begin(); name_iter != objs_to_erase.end(); ++name_iter)
        {
            objects.erase(*name_iter);
        }
        
        func_factory.erase(class_name);
        delete provider;
    }
}

tlv_callback *object_registry::get_processor(string& object_name, string& method)
{   
    sigc::slot<unsigned int, tlv_entry&, tlv_stream *> *result = NULL;
    
    if (objects.count(object_name) > 0)
    {
        result = (objects[object_name].second)->make_functor(method, objects[object_name].first);
    }
    else
    {
        if (object_name == "new")
        {
            if (func_factory.count(method) > 0)
            {
                result = func_factory[method]->make_new_handler();
            }
        }
        else
        {
            if (object_name == "root")
            {
                result = manager.get_handler(method);
            }
        }                
    }
    
    return result;
}

/* ------------------------------------------------------------------------------------------- */

registry_manager::registry_manager(object_registry *rgstry) 
{
    registry = rgstry; 
    method_pointers["clear"] = &registry_manager::clear_processor;        
    method_pointers["listobjects"] = &registry_manager::list_objects_processor;            
    method_pointers["listproviders"] = &registry_manager::list_providers_processor;                
}

tlv_callback *registry_manager::get_handler(string& method_name)
{
    tlv_callback *result = NULL;
    
    if (method_pointers.count(method_name) > 0)
    {
        result = new tlv_callback(sigc::mem_fun(*this, method_pointers[method_name]));
    }
    
    return result;
}

unsigned int registry_manager::clear_processor(tlv_entry& params, tlv_stream *out_stream)
{
    unsigned int result = ERR_OK;
    tlv_entry success_code;
    
    success_code.to_int(0);    
    registry->clear();
    
    result = out_stream->write_tlv(success_code);
    
    return result;
}

unsigned int registry_manager::list_objects_processor(tlv_entry& params, tlv_stream *out_stream)
{
    unsigned int result = ERR_OK;
    tlv_entry result_code, object_handle;
    map<string, pair<void *, service_provider *> >::iterator iter;
    
    
    for (iter = registry->get_objects().begin(); (iter != registry->get_objects().end()) and (result == ERR_OK); ++iter)
    {
        object_handle.to_string(iter->first);
        result = out_stream->write_tlv(object_handle);
    }
    
    result_code.to_int((int)result);        
    (void)out_stream->write_tlv(result_code);
    
    return result;
}

unsigned int registry_manager::list_providers_processor(tlv_entry& params, tlv_stream *out_stream)
{
    unsigned int result = ERR_OK;
    tlv_entry result_code, object_handle;
    map<string, service_provider *>::iterator iter;
            
    
    for (iter = registry->get_providers().begin(); (iter != registry->get_providers().end()) and (result == ERR_OK); ++iter)
    {
        object_handle.to_string(iter->first);
        result = out_stream->write_tlv(object_handle);
    }

    result_code.to_int((int)result);    
    (void)out_stream->write_tlv(result_code);
    
    return result;
}

