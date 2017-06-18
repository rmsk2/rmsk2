/***************************************************************************
 * Copyright 2017 Martin Grap
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

/*! \file object_registry.cpp
 *  \brief Contains the implementation for the service_provider, registry_manager and object_registry classes.
 */ 

#include<boost/lexical_cast.hpp>
#include<boost/scoped_ptr.hpp>
#include<object_registry.h>
#include<rmsk_globals.h>
#include<configurator.h>
#include<rotor_machine.h>
#include<tlv_data_struct.h>


void service_provider::make_handle(string& new_handle)
{
    new_handle = get_name() + ":" + boost::lexical_cast<string>(counter);        
    counter++;
}

/* ---------------------------------------------------------------------------------------------- */

object_registry::object_registry() 
    : manager(this) 
{ 
    num_calls = 0; 
} 

void object_registry::delete_object(string& object_name)
{
    if (objects.count(object_name) > 0)
    {
        (objects[object_name].second)->delete_object(objects[object_name].first);
        objects.erase(object_name);
    }
}

/*! If object_name specifies an existing handle, the object currently represented by that handle
 *  is deleted and replaced by the newly created object.
 */
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
    map<string, pseudo_object *>::iterator iter2;    

    // Delete all objects
    clear();
    
    // Delete all service providers
    for (iter = func_factory.begin(); iter != func_factory.end(); ++iter)
    {
        delete iter->second;
    }

    func_factory.clear();

    // Delete all pseudo objcets
    for (iter2 = pseudo_objects.begin(); iter2 != pseudo_objects.end(); ++iter2)
    {
        delete iter2->second;
    }
    
    pseudo_objects.clear();
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
        
        // Delete objects managed by the provider which is to be deleted
        for (iter = objects.begin(); iter != objects.end(); ++iter)
        {
            if ((iter->second).second == provider)
            {
                provider->delete_object((iter->second).first);
                // Do not remove the current entry while iterating the data structure
                // that contains it.
                objs_to_erase.push_back(iter->first);
            }
        }
        
        // Now remove the marked entries from the objects map.
        for (name_iter = objs_to_erase.begin(); name_iter != objs_to_erase.end(); ++name_iter)
        {
            objects.erase(*name_iter);
        }
        
        // Finally delete the service provider object and remove it from the func_factory map.
        func_factory.erase(class_name);
        delete provider;
    }
}

void object_registry::add_pseudo_object(string& pseudo_name, pseudo_object *pseudo_obj)
{
    pseudo_objects[pseudo_name] = pseudo_obj;
}

void object_registry::delete_pseudo_object(string& pseudo_name)
{
    if (pseudo_objects.count(pseudo_name) != 0)
    {
        pseudo_objects.erase(pseudo_name);
    }
}

tlv_callback *object_registry::get_processor(string& object_name, string& method)
{   
    sigc::slot<unsigned int, tlv_entry&, tlv_stream *> *result = NULL;
    
    // Call to a "real" object.
    if (objects.count(object_name) > 0)
    {
        result = (objects[object_name].second)->make_functor(method, objects[object_name].first);
    }
    else
    {
        // Call to the "new" pseudo object.
        if (object_name == "new")
        {
            if (func_factory.count(method) > 0)
            {
                // Forward the call to new appropriate service_provider.
                result = func_factory[method]->make_new_handler();
            }
        }
        else
        {
            if (pseudo_objects.count(object_name) != 0)
            {
                result = pseudo_objects[object_name]->get_handler(method);
            }
        }                
    }
    
    return result;
}

/* ------------------------------------------------------------------------------------------- */

registry_manager::registry_manager(object_registry *rgstry) 
    : pseudo_object("root")
{
    registry = rgstry; 
    method_pointers["clear"] = &registry_manager::clear_processor;        
    method_pointers["listobjects"] = &registry_manager::list_objects_processor; 
    method_pointers["listpseudoobjects"] = &registry_manager::list_pseudo_objects_processor;     
    method_pointers["listproviders"] = &registry_manager::list_providers_processor;
    method_pointers["numcalls"] = &registry_manager::get_num_calls;                
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

unsigned int registry_manager::get_num_calls(tlv_entry& params, tlv_stream *out_stream)
{
    unsigned int result = ERR_OK;
    tlv_entry num_calls_as_string;
    string num_calls_raw = boost::lexical_cast<string>(registry->get_num_calls());
    
    num_calls_as_string.to_string(num_calls_raw);
    result = out_stream->write_tlv(num_calls_as_string);
    
    // Write end of result stream marker, i.e. the result code
    (void)out_stream->write_error_tlv(result);
    
    return result;
}

unsigned int registry_manager::clear_processor(tlv_entry& params, tlv_stream *out_stream)
{
    unsigned int result = ERR_OK;
    
    registry->clear();
    
    // Write end of result stream marker, i.e. the result code
    result = out_stream->write_error_tlv(ERR_OK);
    
    return result;
}

unsigned int registry_manager::list_pseudo_objects_processor(tlv_entry& params, tlv_stream *out_stream)
{
    unsigned int result = ERR_OK;
    tlv_entry result_code, object_name;
    map<string, pseudo_object *>::iterator iter;
        
    for (iter = registry->get_pseudo_objects().begin(); (iter != registry->get_pseudo_objects().end()) and (result == ERR_OK); ++iter)
    {
        object_name.to_string(iter->first);
        result = out_stream->write_tlv(object_name);
    }

    // Write end of result stream marker, i.e. the result code    
    (void)out_stream->write_error_tlv(result);
    
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

    // Write end of result stream marker, i.e. the result code    
    (void)out_stream->write_error_tlv(result);
    
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
    
    // Write end of result stream marker, i.e. the result code    
    (void)out_stream->write_error_tlv(result);
    
    return result;
}

/* ------------------------------------------------------------------------------------------- */

rmsk_pseudo_object::rmsk_pseudo_object()
    : pseudo_object("rmsk2")
{
    ;
}

tlv_callback *rmsk_pseudo_object::get_handler(string& method)
{
    tlv_callback *result = NULL;
    
    if (method == "getdefaultstate")
    {
        result = new tlv_callback(sigc::mem_fun(*this, &rmsk_pseudo_object::get_default_state_processor));
    }
    else
    {
        if (method == "makestate")
        {
            result = new tlv_callback(sigc::mem_fun(*this, &rmsk_pseudo_object::get_state_processor));
        }
    }
    
    return result;
}
    
unsigned int rmsk_pseudo_object::get_default_state_processor(tlv_entry& params, tlv_stream *out_stream)
{
    unsigned int result = ERR_OK;
    Glib::KeyFile ini_file;
    Glib::ustring ini_data;
    tlv_entry dumped_state;
    
    if (params.tag != TAG_STRING)
    {
        result = out_stream->write_error_tlv(ERR_SYNTAX_INPUT);
    }
    else
    {    
        string machine_name((char *)params.value.c_str());
        boost::scoped_ptr<rotor_machine> machine(rmsk::make_default_machine(machine_name));
        
        if (machine.get() != NULL)
        {        
            machine->save_ini(ini_file);
            ini_data = ini_file.to_data();
            dumped_state.tag = TAG_BYTE_ARRAY;
            dumped_state.value = basic_string<unsigned char>((unsigned char *)ini_data.c_str(), ini_data.length());
            
            // Tell client about processing result and write end of result stream marker.    
            result = out_stream->write_success_tlv(dumped_state);
        }
        else
        {
            result = out_stream->write_error_tlv(ERR_OBJECT_CREATE);
        }
    }
    
    return result;
}

unsigned int rmsk_pseudo_object::get_state_processor(tlv_entry& params, tlv_stream *out_stream)
{
    unsigned int result = ERR_OK;
    string machine_name, rotor_positions;
    ustring rotor_pos_unicode;
    string config_name;
    map<string, string> config_dict;
    tlv_map tlv_config_dict;
    Glib::KeyFile ini_file;
    Glib::ustring ini_data;
    tlv_entry dumped_state;    
    
    do
    {
        // Verify number and type of parameters
        if (params.tag != TAG_SEQUENCE)
        {
            result = out_stream->write_error_tlv(ERR_SYNTAX_INPUT);
            break;
        }

        if (!params.parse_all())
        {
            result = out_stream->write_error_tlv(ERR_SYNTAX_INPUT);
            break;
        }

        if (params.children.size() != 3)
        {
            result = out_stream->write_error_tlv(ERR_SYNTAX_INPUT);
            break;        
        }        
        
        // Parse and convert elements of parameter vector        
        if (tlv_config_dict.set_elements(params.children[1]))
        {
            result = out_stream->write_error_tlv(ERR_SYNTAX_INPUT);
            break;        
        }
        
        if (!params.children[0].tlv_convert(machine_name))
        {
            result = out_stream->write_error_tlv(ERR_SYNTAX_INPUT);
            break;        
        }

        if (!params.children[2].tlv_convert(rotor_positions))
        {
            result = out_stream->write_error_tlv(ERR_SYNTAX_INPUT);
            break;        
        }
        
        // Parameter verified correctly. Now do the real work. First create a default machine state.
        boost::scoped_ptr<rotor_machine> machine(rmsk::make_default_machine(machine_name));
        
        if (machine.get() == NULL)
        {
            result = out_stream->write_error_tlv(ERR_OBJECT_CREATE);
            break;        
        }
        
        // Change machine state to desired configurataion
        config_name = rmsk::get_config_name(machine.get());
        
        boost::scoped_ptr<configurator> c(configurator_factory::get_configurator(config_name));
        
        if (c.get() == NULL)
        {
            result = out_stream->write_error_tlv(ERR_CALL_FAILED);
            break;
        }
        
        tlv_config_dict.tlv_convert(config_dict);
                
        if (c->configure_machine(config_dict, machine.get()) != ERR_OK)
        {
            result = out_stream->write_error_tlv(ERR_CALL_FAILED);
            break;        
        }
        
        // Set rotor positions to desired value       
        Glib::ustring rotor_pos_unicode((char *)params.children[2].value.c_str());
        
        if (rotor_pos_unicode != "")
        {
            if (machine->move_all_rotors(rotor_pos_unicode))
            {
                result = out_stream->write_error_tlv(ERR_CALL_FAILED);
                break;
            }
        }
        
        // Determine machine state
        machine->save_ini(ini_file);
        ini_data = ini_file.to_data();
        dumped_state.tag = TAG_BYTE_ARRAY;
        dumped_state.value = basic_string<unsigned char>((unsigned char *)ini_data.c_str(), ini_data.length());
        
        // Tell client about processing result and write end of result stream marker.    
        result = out_stream->write_success_tlv(dumped_state);                                
    
    } while(0);
    
    return result;
}

