/***************************************************************************
 * Copyright 2018 Martin Grap
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

#include<boost/scoped_ptr.hpp>
#include<configurator.h>
#include<arith_test.h>
#include<tlv_data_struct.h>
#include<algorithm>

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

unsigned int echo::echo_dict_processor(tlv_entry& params, tlv_stream *out_stream)
{
    unsigned int result = ERR_OK;
    tlv_map test_map;
    map<string, string> res_map, new_map;
    map<string, string>::iterator iter;
    
    // Try to parse the TLV structure passed in params
    if (test_map.set_elements(params))
    {
        // Error -> Tell client
        result = out_stream->write_error_tlv(ERR_SYNTAX_INPUT);
    }
    else
    {
        test_map.tlv_convert(res_map);

        for (iter = res_map.begin(); iter != res_map.end(); ++iter)
        {
            new_map[iter->first] = iter->second + " echo";
        }
        
        test_map.to_tlv_entry(new_map);

        result = out_stream->write_success_tlv(test_map.get_elements());
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
    else
    {
        if (method_name == "echodict")
        {
            result = new tlv_callback(sigc::mem_fun(*self, echo_dict_proc));
        }    
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

unsigned int rotor_machine_proxy::get_config_processor(tlv_entry& params, tlv_stream *out_stream)
{
    unsigned int result = ERR_OK;
    tlv_map config_map;
    string config_name = rmsk::get_config_name(machine);
    map<string, string> current_config;
    
    try
    {
        do
        {
            boost::scoped_ptr<configurator> c(configurator_factory::get_configurator(config_name));
            
            if (c.get() == NULL)
            {
                result = out_stream->write_error_tlv(ERR_CALL_FAILED);
                break;
            }
            
            c->get_config(current_config, machine);
            
            config_map.to_tlv_entry(current_config);

            // Tell client about processing result and write end of result stream marker.        
            result = out_stream->write_success_tlv(config_map.get_elements());
        } while(0);
    }
    catch(...)
    {
        result = out_stream->write_error_tlv(ERR_CALL_FAILED);
    }
    
    return result;
}

unsigned int rotor_machine_proxy::get_rotor_set_names_processor(tlv_entry& params, tlv_stream *out_stream)
{
    unsigned int result = ERR_OK, err_temp;
    vector<string> all_names = machine->get_rotor_set_names();
    
    for (const string& iter: all_names)
    {
        tlv_entry tlv_set_name;
        
        tlv_set_name.to_string(iter);
        result = out_stream->write_tlv(tlv_set_name);
        
        // Exit loop in case of error
        if (result != ERR_OK)
        {
            break;
        }        
    }    
    
    err_temp = out_stream->write_error_tlv(result);
    
    // A previously generated error takes precedence. Set result to value of last write_error_tlv() call
    // only if all previous calls were successfull. 
    if (result == ERR_OK)
    {
        result = err_temp;
    }
    
    return result;
}

unsigned int rotor_machine_proxy::get_rotor_set_state_processor(tlv_entry& params, tlv_stream *out_stream)
{
    unsigned int result = ERR_OK;
    Glib::KeyFile ini_file;
    Glib::ustring ini_data;
    tlv_entry dumped_state;
    string rotor_set_name;
    
    if (!params.tlv_convert(rotor_set_name))
    {
        result = out_stream->write_error_tlv(ERR_SYNTAX_INPUT);
    }
    else
    {
        vector<string> rotor_set_names = machine->get_rotor_set_names();
        
        if (std::count(rotor_set_names.begin(), rotor_set_names.end(), rotor_set_name) != 0)
        {
            machine->get_rotor_set(rotor_set_name)->save_ini(ini_file);
            ini_data = ini_file.to_data();
            dumped_state.tag = TAG_BYTE_ARRAY;
            dumped_state.value = basic_string<unsigned char>((unsigned char *)ini_data.c_str(), ini_data.length());
            
            result = out_stream->write_success_tlv(dumped_state);
        }
        else
        {
            result = out_stream->write_error_tlv(ERR_ROTOR_SET_UNKNOWN);        
        }
    }    
    
    return result;
}

unsigned int rotor_machine_proxy::randomize_rotor_set_state_processor(tlv_entry& params, tlv_stream *out_stream)
{
    unsigned int result = ERR_OK;
    string rotor_set_name;
    
    if (!params.tlv_convert(rotor_set_name))
    {
        result = out_stream->write_error_tlv(ERR_SYNTAX_INPUT);
    }
    else
    {
        vector<string> rotor_set_names = machine->get_rotor_set_names();
        
        if (std::count(rotor_set_names.begin(), rotor_set_names.end(), rotor_set_name) != 0)
        {
            machine->get_rotor_set(rotor_set_name)->replace_permutations();
            result = out_stream->write_error_tlv(ERR_OK);
        }
        else
        {
            result = out_stream->write_error_tlv(ERR_ROTOR_SET_UNKNOWN);        
        }
    }    
    
    return result;
}

unsigned int rotor_machine_proxy::set_rotor_set_state_processor(tlv_entry& params, tlv_stream *out_stream)
{
    unsigned int result = ERR_OK;
    vector<tlv_entry> children;
    string rotor_set_name;
    basic_string<unsigned char> rotor_set_data;
    
    if (params.tlv_convert(children))
    {
        if ((children.size() == 2) && (children[0].tlv_convert(rotor_set_name)) && (children[1].tlv_convert(rotor_set_data)))
        {
            vector<string> rotor_set_names = machine->get_rotor_set_names();
        
            // Check if the named rotor set exists in the current machine
            if (std::count(rotor_set_names.begin(), rotor_set_names.end(), rotor_set_name) != 0)
            {
                try
                {
                    string set_state((char *)rotor_set_data.c_str(), rotor_set_data.length());
                    Glib::KeyFile ini_file;
                    // load rotor set ini file
                    bool load_success = ini_file.load_from_data(set_state);
                
                    // Check if ini file could be loaded successfully
                    if (load_success)
                    {
                        // load ini file into a test rotor set object
                        rotor_set test_load_set(machine->get_rotor_set(rotor_set_name)->get_rotor_size());
                        bool test_load_fail = test_load_set.load_ini(ini_file);
                        
                        // Check that loading the rotor set file was successfull and that the name of the original and
                        // the loaded set match
                        if ((!test_load_fail) && (test_load_set.get_name() == rotor_set_name))
                        {                        
                            vector<unsigned int> rotor_ids, ring_ids, rotor_ids_test, ring_ids_test;
                            
                            // Check that the loaded set and the current set contain the same rotor and ring ids.
                            machine->get_rotor_set(rotor_set_name)->get_ids(rotor_ids);
                            machine->get_rotor_set(rotor_set_name)->get_ring_ids(ring_ids);
                            test_load_set.get_ids(rotor_ids_test);
                            test_load_set.get_ring_ids(ring_ids_test);
                                                        
                            if ((rotor_ids == rotor_ids_test) && (ring_ids == ring_ids_test))
                            {
                                // Replace current set with ini file contents. Can not fail after the previous checks.
                                machine->get_rotor_set(rotor_set_name)->load_ini(ini_file);
                                result = out_stream->write_error_tlv(ERR_OK);
                            }
                            else
                            {
                                result = out_stream->write_error_tlv(ERR_CALL_FAILED);
                            }
                        }
                        else
                        {
                            result = out_stream->write_error_tlv(ERR_CALL_FAILED);
                        }                        
                    }
                    else
                    {
                        result = out_stream->write_error_tlv(ERR_CALL_FAILED);
                    }
                }
                catch(...)
                {
                    // Loading the ini file did throw an ecxeption
                    result = out_stream->write_error_tlv(ERR_CALL_FAILED);
                }                
            }
            else
            {
                result = out_stream->write_error_tlv(ERR_ROTOR_SET_UNKNOWN);        
            }            
        }
        else
        {
            result = out_stream->write_error_tlv(ERR_SYNTAX_INPUT);
        }
    }
    else
    {
        result = out_stream->write_error_tlv(ERR_SYNTAX_INPUT);
    }
    
    return result;
}

unsigned int rotor_machine_proxy::set_config_processor(tlv_entry& params, tlv_stream *out_stream)
{
    unsigned int result = ERR_OK;
    tlv_map config_map;
    string config_name = rmsk::get_config_name(machine);
    map<string, string> current_config;
    
    do
    {
        if (config_map.set_elements(params))
        {
            result = out_stream->write_error_tlv(ERR_SYNTAX_INPUT);
            break;
        }
    
        boost::scoped_ptr<configurator> c(configurator_factory::get_configurator(config_name));
        
        if (c.get() == NULL)
        {
            result = out_stream->write_error_tlv(ERR_CALL_FAILED);
            break;
        }
        
        config_map.tlv_convert(current_config);
        
        if (c->configure_machine(current_config, machine) != ERR_OK)
        {
            result = out_stream->write_error_tlv(ERR_CALL_FAILED);
            break;        
        }

        result = out_stream->write_error_tlv(ERR_OK);
        
    } while(0);

    
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

unsigned int rotor_machine_proxy::go_to_letter_state_processor(tlv_entry& params, tlv_stream *out_stream)
{
    unsigned int result = ERR_OK;
    
    machine->go_to_letter_state();
    result = out_stream->write_error_tlv(ERR_OK);
    
    return result;
}

unsigned int rotor_machine_proxy::set_positions_processor(tlv_entry& params, tlv_stream *out_stream)
{
    unsigned int result = ERR_OK;
    
    if (params.tag == TAG_STRING)
    {
        Glib::ustring desired_positions((char *)params.value.c_str());
        
        if (machine->move_all_rotors(desired_positions))
        {
            result = out_stream->write_error_tlv(ERR_CALL_FAILED);
        }
        else
        {
            result = out_stream->write_error_tlv(ERR_OK);        
        }
    }
    else
    {
        result = out_stream->write_error_tlv(ERR_SYNTAX_INPUT);    
    }    
    
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

unsigned int rotor_machine_proxy::randomize_state_processor(tlv_entry& params, tlv_stream *out_stream)
{
    unsigned int result = ERR_OK;
    
    // Did we receive a TLV string?
    if (params.tag != TAG_STRING)
    {
        result = out_stream->write_error_tlv(ERR_SYNTAX_INPUT);
    }
    else
    {        
        string randomize_param = string((char *)params.value.data(), params.value.length());
        bool rand_result = machine->randomize(randomize_param);
        
        if (rand_result)
        {
            // Randomization did not work
            result = out_stream->write_error_tlv(ERR_RANDOMIZATION_FAILED);
        }
        else
        {
            // Write end of result stream marker.            
            result = out_stream->write_error_tlv(ERR_OK);
        }
    }
    
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

unsigned int rotor_machine_proxy::get_randparm_processor(tlv_entry& params, tlv_stream *out_stream)
{
    unsigned int result = ERR_OK;
    tlv_entry current_param_tlv;
    ustring current_param;
    vector<string> randomizer_params = machine->get_randomizer_params();
    vector<string>::iterator iter;
    
    if (randomizer_params.size() != 0)
    {        
        for (iter = randomizer_params.begin(); (iter != randomizer_params.end()) and (result == ERR_OK); ++iter)
        {
            current_param = *iter;
            current_param_tlv.to_string(current_param.raw());
            // Transmit parameter to client
            result = out_stream->write_tlv(current_param_tlv);
        }
    }
    else
    {
        current_param = "";
        current_param_tlv.to_string(current_param.raw());
        // Transmit parameter to client
        result = out_stream->write_tlv(current_param_tlv);        
    }
    
    // Write end of result stream marker.
    (void)out_stream->write_error_tlv(result);
    
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
    
    auto append_lambda = [&byte_array] (unsigned int iter) { byte_array.value.push_back((unsigned char)iter); };
    for_each(perm.begin(), perm.end(), append_lambda);
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
    rotor_proxy_proc["randomizestate"] = &rotor_machine_proxy::randomize_state_processor;
    rotor_proxy_proc["setpositions"] = &rotor_machine_proxy::set_positions_processor;
    rotor_proxy_proc["getconfig"] = &rotor_machine_proxy::get_config_processor;
    rotor_proxy_proc["setconfig"] = &rotor_machine_proxy::set_config_processor;
    rotor_proxy_proc["getrandparm"] = &rotor_machine_proxy::get_randparm_processor;    
    rotor_proxy_proc["gotoletterstate"] = &rotor_machine_proxy::go_to_letter_state_processor;
    rotor_proxy_proc["getrotorsetnames"] = &rotor_machine_proxy::get_rotor_set_names_processor;
    rotor_proxy_proc["getrotorsetstate"] = &rotor_machine_proxy::get_rotor_set_state_processor;
    rotor_proxy_proc["randomizerotorsetstate"] = &rotor_machine_proxy::randomize_rotor_set_state_processor;
    rotor_proxy_proc["setrotorsetstate"] = &rotor_machine_proxy::set_rotor_set_state_processor;    
    
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

/* ---------------------------------------------------------------------------------------------- */

random_provider::random_provider(object_registry *obj_registry) 
    : service_provider(obj_registry) 
{
    // Fill random_proxy_proc
    random_proxy_proc["randstring"] = &random_proxy::random_string_processor;
    random_proxy_proc["randpermutation"] = &random_proxy::random_permutation_processor;    
}

tlv_callback *random_provider::make_new_handler()
{
    tlv_callback *result = new tlv_callback(sigc::mem_fun(*this, &random_provider::new_object));
    
    return result;
}

tlv_callback *random_provider::make_functor(string& method_name, void *object)
{
    tlv_callback *result = NULL;
    random_proxy *self = static_cast<random_proxy *>(object);
    
    if (random_proxy_proc.count(method_name) > 0)
    {
        result = new tlv_callback(sigc::mem_fun(*self, random_proxy_proc[method_name]));
    }
    
    return result;
}

unsigned int random_provider::new_object(tlv_entry& params, tlv_stream *out_stream)
{
    unsigned int result = ERR_OK;
    string new_object_name;
    tlv_entry tlv_handle;
    alphabet<char> *new_alpha;
    random_proxy *new_object;
    string alpha_spec;
    
    make_handle(new_object_name);
    
    do
    {
        // Did we receive a TLV string?
        if (params.tag != TAG_STRING)
        {
            result = out_stream->write_error_tlv(ERR_SYNTAX_INPUT);
            break;
        }
        
        if (!params.tlv_convert(alpha_spec))
        {
            result = out_stream->write_error_tlv(ERR_SYNTAX_INPUT);
            break;
        }
        
        new_alpha = new alphabet<char>(alpha_spec.c_str(), alpha_spec.length());
        new_object = new random_proxy(new_alpha);
                
        pair<void *, service_provider *> new_val(new_object, this);
    
        registry->add_object(new_object_name, new_val);
        tlv_handle.to_string(new_object_name);

        // Tell client about the new handle and write end of result stream marker.
        result = out_stream->write_success_tlv(tlv_handle);        
    
    } while(0);
    
    return result;
}

void random_provider::delete_object(void *obj_to_delete)
{
    random_proxy *object = static_cast<random_proxy *>(obj_to_delete);
    delete object;
}

/* ---------------------------------------------------------------------------------------------- */

unsigned int random_proxy::random_string_processor(tlv_entry& params, tlv_stream *out_stream)
{
    unsigned int result = ERR_OK;
    int string_size;
    tlv_entry rand_string;
    string rand_string_raw;
    
    // Did we receive a TLV integer?
    if (!params.tlv_convert(string_size))
    {
        out_stream->write_error_tlv(ERR_SYNTAX_INPUT);
    }
    else
    {
        if (string_size < 1)
        {
            string_size = 1;
        }
        
        rand_string_raw = alpha->get_random_string((unsigned int)string_size);
        rand_string.to_string(rand_string_raw);
        
        result = out_stream->write_success_tlv(rand_string);
    }
    
    return result;
}

unsigned int random_proxy::random_permutation_processor(tlv_entry& params, tlv_stream *out_stream)
{
    unsigned int result = ERR_OK;
    tlv_entry new_perm_tlv;
    basic_string<unsigned char> new_perm_out;        
    permutation new_perm = alpha->get_random_permutation();
    vector<unsigned int> new_perm_raw;
    
    new_perm.to_vec(new_perm_raw);
    
    for (unsigned int count = 0; count < alpha->get_size(); count++)
    {
        new_perm_out.push_back((unsigned char)(new_perm_raw[count]));
    }

    new_perm_tlv.to_byte_array(new_perm_out);

    result = out_stream->write_success_tlv(new_perm_tlv);
    
    return result;
}

