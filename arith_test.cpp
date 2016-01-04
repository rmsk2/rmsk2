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

/*! \file arith_test.cpp
 *  \brief Implementation of some classes which allow to test the TLV infrastructure. On top of that it contains
 *         the implementation for the TLV rotor machine functionality.
 */ 

#include<arith_test.h>

unsigned int arithmetic::add_processor(tlv_entry& params, tlv_stream *out_stream)
{
    unsigned int result = ERR_OK;
    int left_summand, right_summand;
    tlv_entry calc_result;
    
    do
    {
        // Is params a SEQUENCE?
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
        
        // Do we have exactly two summands?
        if (params.children.size() != 2)
        {
            result = out_stream->write_error_tlv(47);
            break;
        }

        // Are the summands integers?
        if ((params.children[0].tag != TAG_INT) or (params.children[1].tag != TAG_INT))
        {
            result = out_stream->write_error_tlv(48);
            break;
        }

        // Can the summands be successfully converted to ints?
        if (!(params.children[0].tlv_convert(left_summand) and params.children[1].tlv_convert(right_summand)))
        {
            result = out_stream->write_error_tlv(49);
            break;
        }
        
        // Perform addition and and create result tlv_entry.
        calc_result.to_int(left_summand + right_summand);
        
        // Write result and end of result stream marker.
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
    
    // Tell client about the new handle and write end of result stream marker.
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
    
    // Try to parse the TLV structure passed in params
    if (!params.parse_all())
    {
        // Error -> Tell client
        result = out_stream->write_error_tlv(ERR_SYNTAX_INPUT);
    }
    else
    {
        // Did we get a sequence?                
        if (params.tag == TAG_SEQUENCE)
        {
            // Yes: Construct a new sequence which contains params.children as its component
            seq_return.to_sequence(params.children);
            // Send new sequence back to client write end of result stream marker.
            result = out_stream->write_success_tlv(seq_return);
        }
        else
        {
            // Send param back to client write end of result stream marker.
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

    // Tell client about the new handle and write end of result stream marker.    
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

    // Has param the proper type?
    if (params.tag != TAG_STRING)
    {
        result = out_stream->write_error_tlv(ERR_SYNTAX_INPUT);
    }
    else
    {
        Glib::ustring in((char *)params.value.c_str());
                   
        if (do_enc)
        {
            machine->get_keyboard()->symbols_typed_encrypt(in, out);
        }
        else
        {
            machine->get_keyboard()->symbols_typed_decrypt(in, out);
        }
        
        data_out.to_string(out.raw());
        // Tell client about processing result and write end of result stream marker.            
        result = out_stream->write_success_tlv(data_out);        
    }
    
    return result;

}

unsigned int rotor_machine_proxy::get_state_processor(tlv_entry& params, tlv_stream *out_stream)
{
    unsigned int result = ERR_OK;
    Glib::KeyFile ini_file;
    Glib::ustring ini_data;
    tlv_entry dumped_state;
    
    machine->save_ini(ini_file);
    ini_data = ini_file.to_data();
    dumped_state.tag = TAG_BYTE_ARRAY;
    dumped_state.value = basic_string<unsigned char>((unsigned char *)ini_data.c_str(), ini_data.length());
    
    // Tell client about processing result and write end of result stream marker.    
    result = out_stream->write_success_tlv(dumped_state);
    
    return result;
}

unsigned int rotor_machine_proxy::get_description_processor(tlv_entry& params, tlv_stream *out_stream)
{
    unsigned int result = ERR_OK;
    tlv_entry description_tlv;
    string description;
    
    description = machine->get_description();
    description_tlv.to_string(description);
    
    // Tell client about processing result and write end of result stream marker.        
    result = out_stream->write_success_tlv(description_tlv);
    
    return result;
}

unsigned int rotor_machine_proxy::get_positions_processor(tlv_entry& params, tlv_stream *out_stream)
{
    unsigned int result = ERR_OK;
    tlv_entry positions_tlv;
    ustring positions;
    
    positions = machine->visualize_all_positions();
    positions_tlv.to_string(positions.raw());    
    
    // Tell client about processing result and write end of result stream marker.
    result = out_stream->write_success_tlv(positions_tlv);
    
    return result;
}

unsigned int rotor_machine_proxy::sigaba_setup_processor(tlv_entry& params, tlv_stream *out_stream)
{
    unsigned int result = ERR_OK;
    sigaba *machine_as_sigaba;
    int rotor_num, num_steps;
    const char *rotor_names[5] = {STATOR_L, S_SLOW, S_FAST, S_MIDDLE, STATOR_R};
    ustring current_pos;
    tlv_entry current_pos_tlv;
    
    do
    {
        if (!params.parse_all())
        {
            result = out_stream->write_error_tlv(ERR_SYNTAX_INPUT);
            break;
        }
        
        // Did we receive a sequence with two children?
        if (params.children.size() != 2)
        {
            result = out_stream->write_error_tlv(ERR_SYNTAX_INPUT);
            break;
        }
        
        // Can both children be successfully converted to ints?
        if (!(params.children[0].tlv_convert(rotor_num) and params.children[1].tlv_convert(num_steps)))
        {
            result = out_stream->write_error_tlv(ERR_SYNTAX_INPUT);        
            break;
        }
        
        // Is machine actually a SIGABA?
        if ((machine_as_sigaba = dynamic_cast<sigaba *>(machine)) == NULL)
        {
            result = out_stream->write_error_tlv(ERR_SEMANTICS_INPUT);        
            break;        
        }

        // Is the number of the driver rotor in the allowed range?
        if ((rotor_num < 1) or (rotor_num > 5))
        {
            result = out_stream->write_error_tlv(ERR_SEMANTICS_INPUT);        
            break;        
        }
        
        if (num_steps <= 0)
        {
            num_steps =  1;
        }
        
        // Perform stepping
        for (int count = 0; (count < num_steps) and (result == ERR_OK); count++)
        {
            machine_as_sigaba->get_sigaba_stepper()->setup_step(rotor_names[rotor_num -1]);
            current_pos = machine->visualize_all_positions();
            current_pos_tlv.to_string(current_pos.raw());
            result = out_stream->write_tlv(current_pos_tlv);
        }

        // Write end of result stream marker.        
        (void)out_stream->write_error_tlv(result);        
        
    } while(0);
    
    return result;
}

unsigned int rotor_machine_proxy::set_state_processor(tlv_entry& params, tlv_stream *out_stream)
{
    unsigned int result = ERR_OK;
    rotor_machine *new_machine;
    
    // Did we receive a TLV byte array?
    if (params.tag != TAG_BYTE_ARRAY)
    {
        result = out_stream->write_error_tlv(ERR_SYNTAX_INPUT);
    }
    else
    {
        string state_data = string((char *)params.value.data(), params.value.length());
        // Try to restore a machine from the fiven state
        new_machine = rmsk::restore_from_data(state_data);
        
        if (new_machine == NULL)
        {
            // Restoring a machine from the given state did not work.
            result = out_stream->write_error_tlv(ERR_OBJECT_CREATE);
        }
        else
        {
            // Replace old rotor_machine object by the new one.
            set_new_machine(new_machine);
            // Write end of result stream marker.            
            result = out_stream->write_error_tlv(ERR_OK);
        }
    }
    
    return result;
}

unsigned int rotor_machine_proxy::step_processor(tlv_entry& params, tlv_stream *out_stream)
{
    unsigned int result = ERR_OK;
    int num_iterations;
    tlv_entry current_pos_tlv;
    ustring current_pos;
    
    // Did we receive a TLV integer?
    if (!params.tlv_convert(num_iterations))
    {
        out_stream->write_error_tlv(ERR_SYNTAX_INPUT);
    }
    else
    {
        if (num_iterations < 0)
        {
            num_iterations = 1;
        }
        
        // Perform steppings
        for (int count = 0; (count < num_iterations) and (result == ERR_OK); count++)
        {
            machine->step_rotors();
            current_pos = machine->visualize_all_positions();
            current_pos_tlv.to_string(current_pos.raw());
            // Transmit current rotor positions to client
            result = out_stream->write_tlv(current_pos_tlv);
        }
        
        // Write end of result stream marker.
        (void)out_stream->write_error_tlv(result);
    }
    
    return result;
}

unsigned int rotor_machine_proxy::get_permutations_processor(tlv_entry& params, tlv_stream *out_stream)
{
    unsigned int result = ERR_OK;
    int num_iterations;
    tlv_entry current_perm_tlv;
    ustring current_pos;
    vector<unsigned int> current_perm;
    
    // Did we receive a TLV integer?
    if (!params.tlv_convert(num_iterations))
    {
        out_stream->write_error_tlv(ERR_SYNTAX_INPUT);
    }
    else
    {
        if (num_iterations < 0)
        {
            num_iterations = 0;
        }
        
        machine->get_current_perm(current_perm);
        perm_to_byte_array(current_perm, current_perm_tlv);
        // Transmit current permutation to client
        result = out_stream->write_tlv(current_perm_tlv);
        
        // Perform steppings as requested by client
        for (int count = 0; (count < num_iterations) and (result == ERR_OK); count++)
        {
            machine->step_rotors();
            machine->get_current_perm(current_perm);
            perm_to_byte_array(current_perm, current_perm_tlv);
            // Transmit current permutation to client
            result = out_stream->write_tlv(current_perm_tlv);
        }

        // Write end of result stream marker.        
        (void)out_stream->write_error_tlv(result);
    }
    
    return result;
}

void rotor_machine_proxy::perm_to_byte_array(vector<unsigned int>& perm, tlv_entry& byte_array)
{
    byte_array.value.clear();
    byte_array.tag = TAG_BYTE_ARRAY;
    vector<unsigned int>::iterator iter;
    
    for (iter = perm.begin(); iter != perm.end(); ++iter)
    {
        byte_array.value.push_back((unsigned char)*iter);
    }
}

/* ---------------------------------------------------------------------------------------------- */

rotor_machine_provider::rotor_machine_provider(object_registry *obj_registry) 
    : service_provider(obj_registry) 
{
    // Fill rotor_proxy_proc
    rotor_proxy_proc["encrypt"] = &rotor_machine_proxy::encrypt_processor;
    rotor_proxy_proc["decrypt"] = &rotor_machine_proxy::decrypt_processor;    
    rotor_proxy_proc["getstate"] = &rotor_machine_proxy::get_state_processor;
    rotor_proxy_proc["setstate"] = &rotor_machine_proxy::set_state_processor;
    rotor_proxy_proc["step"] = &rotor_machine_proxy::step_processor;    
    rotor_proxy_proc["getdescription"] = &rotor_machine_proxy::get_description_processor;
    rotor_proxy_proc["sigabasetup"] = &rotor_machine_proxy::sigaba_setup_processor;
    rotor_proxy_proc["getpositions"] = &rotor_machine_proxy::get_positions_processor;
    rotor_proxy_proc["getpermutations"] = &rotor_machine_proxy::get_permutations_processor;        
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
        // Did we receive a TLV byte array?
        if (params.tag != TAG_BYTE_ARRAY)
        {
            result = out_stream->write_error_tlv(ERR_SYNTAX_INPUT);
            break;
        }
        
        string machine_state((char *)params.value.data(), params.value.length());
        
        // Attempt to restore a rotor_machine object from the state sent by the client
        machine = rmsk::restore_from_data(machine_state);
        
        // Did restoring machine state work?
        if (machine ==  NULL)
        {
            // No
            result = out_stream->write_error_tlv(ERR_OBJECT_CREATE);
            break;            
        }
        
        // Yes        
        new_object = new rotor_machine_proxy(machine);
        
        pair<void *, service_provider *> new_val(new_object, this);
    
        registry->add_object(new_object_name, new_val);
        tlv_handle.to_string(new_object_name);

        // Tell client about the new handle and write end of result stream marker.
        result = out_stream->write_success_tlv(tlv_handle);        
    
    } while(0);
    
    return result;
}

void rotor_machine_provider::delete_object(void *obj_to_delete)
{
    rotor_machine_proxy *object = static_cast<rotor_machine_proxy *>(obj_to_delete);
    delete object;
}

