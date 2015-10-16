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

#include<arith_test.h>

unsigned int arithmetic::add_processor(tlv_entry& params, tlv_stream *out_stream)
{
    unsigned int result = ERR_OK;
    int left_summand, right_summand;
    tlv_entry calc_result;
    
    do
    {
        if (params.tag != TAG_SEQUENCE)
        {
            result = out_stream->write_error_tlv(45);
            break;
        }
        
        if (!params.parse_all())
        {
            result = out_stream->write_error_tlv(46);
            break;
        }
        
        if (params.children.size() != 2)
        {
            result = out_stream->write_error_tlv(47);
            break;
        }

        if ((params.children[0].tag != TAG_INT) or (params.children[1].tag != TAG_INT))
        {
            result = out_stream->write_error_tlv(48);
            break;
        }

        if (!(params.children[0].tlv_convert(left_summand) and params.children[1].tlv_convert(right_summand)))
        {
            result = out_stream->write_error_tlv(49);
            break;
        }
        
        calc_result.to_int(left_summand + right_summand);
        
        result = out_stream->write_success_tlv(calc_result);
            
    } while(0);
    
    return result;    
}

/* ---------------------------------------------------------------------------------------------- */


unsigned int arithmetic_provider::new_object(tlv_entry& params, tlv_stream *out_stream)
{
    unsigned int result = ERR_OK;
    string new_object_name;
    arithmetic *new_object = new arithmetic();
    tlv_entry tlv_handle;
    
    make_handle(new_object_name);
    pair<void *, service_provider *> new_val(new_object, this);
    
    registry->add_object(new_object_name, new_val);
    tlv_handle.to_string(new_object_name);
    
    result = out_stream->write_success_tlv(tlv_handle);

    return result;
}

tlv_callback *arithmetic_provider::make_new_handler()
{
    tlv_callback *result = new tlv_callback(sigc::mem_fun(*this, &arithmetic_provider::new_object));
    
    return result;
}

tlv_callback *arithmetic_provider::make_functor(string& method_name, void *object)
{
    tlv_callback *result = NULL;
    arithmetic *self = static_cast<arithmetic *>(object);
    
    if (method_name == "add")
    {
        result = new tlv_callback(sigc::mem_fun(*self, add_proc));
    }
    
    return result;
}

void arithmetic_provider::delete_object(void *obj_to_delete)
{
    arithmetic *object = static_cast<arithmetic *>(obj_to_delete);
    delete object;
}

/* ---------------------------------------------------------------------------------------------- */

unsigned int echo::echo_processor(tlv_entry& params, tlv_stream *out_stream)
{
    unsigned int result = ERR_OK;
    tlv_entry seq_return;
    
    if (!params.parse_all())
    {
        result = out_stream->write_error_tlv((int)ERR_SYNTAX_INPUT);
    }
    else
    {        
        params.print();
        
        if (params.tag == TAG_SEQUENCE)
        {
            seq_return.to_sequence(params.children);
            result = out_stream->write_success_tlv(seq_return);
        }
        else
        {
            result = out_stream->write_success_tlv(params);
        }                
    }
                
    return result;
}

/* ---------------------------------------------------------------------------------------------- */

unsigned int echo_provider::new_object(tlv_entry& params, tlv_stream *out_stream)
{
    unsigned int result = ERR_OK;
    string new_object_name;
    echo *new_object = new echo();
    tlv_entry tlv_handle;
    
    make_handle(new_object_name);
    pair<void *, service_provider *> new_val(new_object, this);
    
    registry->add_object(new_object_name, new_val);
    tlv_handle.to_string(new_object_name);
    
    result = out_stream->write_success_tlv(tlv_handle);
    
    return result;
}

tlv_callback *echo_provider::make_new_handler()
{
    tlv_callback *result = new tlv_callback(sigc::mem_fun(*this, &echo_provider::new_object));
    
    return result;
}

tlv_callback *echo_provider::make_functor(string& method_name, void *object)
{
    tlv_callback *result = NULL;
    echo *self = static_cast<echo *>(object);
    
    if (method_name == "echo")
    {
        result = new tlv_callback(sigc::mem_fun(*self, echo_proc));
    }
    
    return result;
}

void echo_provider::delete_object(void *obj_to_delete)
{
    echo *object = static_cast<echo *>(obj_to_delete);
    delete object;
}

/* ---------------------------------------------------------------------------------------------- */

rotor_machine_proxy::rotor_machine_proxy(rotor_machine *m)
{
    machine = m;
}

void rotor_machine_proxy::set_new_machine(rotor_machine *new_machine)
{
    delete machine;
    machine = new_machine;
}

unsigned int rotor_machine_proxy::encrypt_processor(tlv_entry& params, tlv_stream *out_stream)
{
    return encdec_processor(params, out_stream, true);    
}

unsigned int rotor_machine_proxy::decrypt_processor(tlv_entry& params, tlv_stream *out_stream)
{
    return encdec_processor(params, out_stream, false);    
}

unsigned int rotor_machine_proxy::encdec_processor(tlv_entry& params, tlv_stream *out_stream, bool do_enc)
{
    unsigned int result = ERR_OK;
    string data_to_process;
    tlv_entry data_out;
    Glib::ustring out;

    if (params.tag != TAG_STRING)
    {
        result = out_stream->write_error_tlv((int)ERR_SYNTAX_INPUT);
    }
    else
    {
        Glib::ustring in((char *)params.value.data(), params.value.length());
                   
        if (do_enc)
        {
            machine->get_keyboard()->symbols_typed_encrypt(in, out);
        }
        else
        {
            machine->get_keyboard()->symbols_typed_decrypt(in, out);
        }
        
        data_out.to_string(out.raw());
        result = out_stream->write_success_tlv(data_out);        
    }
    
    return result;

}

rotor_machine_provider::rotor_machine_provider(object_registry *obj_registry) 
    : service_provider(obj_registry) 
{ 
    rotor_proxy_proc["encrypt"] = &rotor_machine_proxy::encrypt_processor;
    rotor_proxy_proc["decrypt"] = &rotor_machine_proxy::decrypt_processor;    
}

tlv_callback *rotor_machine_provider::make_new_handler()
{
    tlv_callback *result = new tlv_callback(sigc::mem_fun(*this, &rotor_machine_provider::new_object));
    
    return result;
}

tlv_callback *rotor_machine_provider::make_functor(string& method_name, void *object)
{
    tlv_callback *result = NULL;
    rotor_machine_proxy *self = static_cast<rotor_machine_proxy *>(object);
    
    if (rotor_proxy_proc.count(method_name) > 0)
    {
        result = new tlv_callback(sigc::mem_fun(*self, rotor_proxy_proc[method_name]));
    }
    
    return result;
}

unsigned int rotor_machine_provider::new_object(tlv_entry& params, tlv_stream *out_stream)
{
    unsigned int result = ERR_OK;
    string new_object_name;
    tlv_entry tlv_handle;
    rotor_machine *machine;
    rotor_machine_proxy *new_object;
    
    make_handle(new_object_name);
    
    do
    {
        if (params.tag != TAG_BYTE_ARRAY)
        {
            result = out_stream->write_error_tlv((int)ERR_SYNTAX_INPUT);
            break;
        }
        
        string machine_state((char *)params.value.data(), params.value.length());
        
        machine = rmsk::restore_from_data(machine_state);
        
        if (machine ==  NULL)
        {
            result = out_stream->write_error_tlv((int)ERR_OBJECT_CREATE);
            break;            
        }
        
        new_object = new rotor_machine_proxy(machine);
        
        pair<void *, service_provider *> new_val(new_object, this);
    
        registry->add_object(new_object_name, new_val);
        tlv_handle.to_string(new_object_name);
    
        result = out_stream->write_success_tlv(tlv_handle);        
    
    } while(0);
    
    return result;
}

void rotor_machine_provider::delete_object(void *obj_to_delete)
{
    rotor_machine_proxy *object = static_cast<rotor_machine_proxy *>(obj_to_delete);
    delete object;
}

