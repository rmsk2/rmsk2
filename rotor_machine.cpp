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

#include<rotor_machine.h>

/*! \file rotor_machine.cpp
 *  \brief Implementation of the rotor_machine class.
 */ 


unsigned int rotor_machine::encrypt(unsigned int in_char)
{
    unsigned int result = in_char;
   
    result = safe_encrypt(input_transform.get(), result);   
    
    if (is_pre_step)
    {
        stepper->step_rotors();
    }
    
    result = stepper->get_stack().encrypt(result);

    if (!is_pre_step)
    {
        stepper->step_rotors();
    }    
    
    if (stepper->get_stack().get_reflecting_flag())
    {
        result = safe_decrypt(input_transform.get(), result);
    }
    else
    {
        result = safe_encrypt(output_transform.get(), result);   
    }    
    
    return result;
}

unsigned int rotor_machine::get_enc_symbol(unsigned int symbol)
{
    unsigned int result = symbol;
   
    result = safe_encrypt(input_transform.get(), result);   
        
    result = stepper->get_stack().encrypt(result);
    
    if (stepper->get_stack().get_reflecting_flag())
    {
        result = safe_decrypt(input_transform.get(), result);
    }
    else
    {
        result = safe_encrypt(output_transform.get(), result);   
    }    
    
    return result;
}

void rotor_machine::get_current_perm(vector<unsigned int>& current_perm)
{
    current_perm.clear();
    
    for (unsigned int count = 0; count < get_size(); count++)
    {
        current_perm.push_back(get_enc_symbol(count));
    }
}

unsigned int rotor_machine::decrypt(unsigned int in_char)
{
    unsigned int result = in_char;
    
    
    if (stepper->get_stack().get_reflecting_flag())
    {
        result = safe_encrypt(input_transform.get(), result);
    }
    else
    {
        result = safe_decrypt(output_transform.get(), result);   
    }
            
    if (is_pre_step)
    {
        stepper->step_rotors();
    }
    
    result = stepper->get_stack().decrypt(result);

    if (!is_pre_step)
    {
        stepper->step_rotors();
    }    
    
    result = safe_decrypt(input_transform.get(), result);
        
    return result;
}


void rotor_machine::set_input_transform(boost::shared_ptr<encryption_transform> t) 
{
    input_transform = t; 
    
    if (stepper->get_stack().get_reflecting_flag())
    {
        output_transform = t;
    }
}

void rotor_machine::set_output_transform(boost::shared_ptr<encryption_transform> t)
{
    output_transform = t; 
    
    if (stepper->get_stack().get_reflecting_flag())
    {
        input_transform = t;
    }

}

unsigned int rotor_machine::safe_encrypt(encryption_transform *t, unsigned int in_char)
{
    unsigned int result = in_char;
    
    if (t != NULL)
    {
        result = t->encrypt(result);
    }
    
    return result;
}
    
unsigned int rotor_machine::safe_decrypt(encryption_transform *t, unsigned int in_char)
{
    unsigned int result = in_char;
    
    if (t != NULL)
    {
        result = t->decrypt(result);
    }
    
    return result;
}

bool rotor_machine::load_ini(Glib::KeyFile& ini_file)
{
    bool result = false;
    string machine_name_temp;

    if (!(result = !ini_file.has_key(MACHINE_SECTION, KEY_MACHINE_NAME)))
    {
        machine_name_temp = ini_file.get_string(MACHINE_SECTION, KEY_MACHINE_NAME);
        
        if (!(result = (machine_name_temp != machine_name)))
        {        
            machine_name = machine_name_temp;
            
            if (!(result = !ini_file.has_key(MACHINE_SECTION, "rotorsetname")))
            {
                default_rotor_set_name = ini_file.get_string(MACHINE_SECTION, "rotorsetname");
                
                if (!(result = get_stepping_gear()->load_ini(ini_file)))
                {
                    result = load_additional_components(ini_file);
                }
            }
        }        
    }
        
    return result;
}

bool rotor_machine::load(string& file_name)
{
    return rmsk::settings_file_load(file_name, sigc::mem_fun(*this, &rotor_machine::load_ini));
}

ustring rotor_machine::visualize_rotor_pos(string& rotor_identifier)
{
    ustring result;
    
    result += rmsk::std_uni_alpha()->to_val(get_stepping_gear()->get_descriptor(rotor_identifier).ring->get_pos());
    
    return result;    
}

ustring rotor_machine::visualize_rotor_pos(const char *rotor_identifier)
{
    string help(rotor_identifier); 
    
    return visualize_rotor_pos(help);
}

ustring rotor_machine::visualize_all_positions()
{
    ustring result;
    vector<string> rotor_names;
    
    get_stepping_gear()->get_rotor_identifiers(rotor_names);
    
    for (int count = get_stepping_gear()->get_num_rotors() - 1; count >= 0; count--)
    {
        if (unvisualized_rotor_names.count(rotor_names[count]) == 0)
        {
            result += visualize_rotor_pos(rotor_names[count]);
        }
    }
    
    return result;
}

void rotor_machine::prepare_rotor(const char *rotor_set_name, rotor_id r_id, const char *rotor_name, bool reverse)
{ 
    string help1(rotor_name); 
    string help2(rotor_set_name); 
    
    prepare_rotor(help2, r_id, help1, reverse); 
}

void rotor_machine::prepare_rotor(rotor_set& r_set, rotor_id r_id, string& rotor_name, bool reverse)
{
    pair<boost::shared_ptr<rotor>, boost::shared_ptr<rotor_ring> > help;
    
    help = r_set.make_rotor(r_id);
    
    if (reverse)
    {
        help.first->get_perm()->switch_to_inverse();
    }
        
    get_stepping_gear()->insert_rotor_and_ring(rotor_name, help);
    get_stepping_gear()->get_descriptor(rotor_name).id.r_id = r_id.r_id;
    get_stepping_gear()->get_descriptor(rotor_name).id.ring_id = r_id.ring_id;    
    get_stepping_gear()->get_descriptor(rotor_name).id.insert_inverse = r_id.insert_inverse;    
}

vector<string> rotor_machine::get_rotor_set_names()
{
    map<string, rotor_set>::iterator iter;
    vector<string> result;
    
    for (iter = rotor_sets.begin(); iter != rotor_sets.end(); ++iter)
    {
        result.push_back(iter->first);
    }
    
    return result;
}

void rotor_machine::add_rotor_set(string& name, rotor_set& set)
{
    rotor_sets[name] = set;
}

rotor_set& rotor_machine::get_rotor_set(string& name)
{
    rmsk::simple_assert(rotor_sets.count(name) == 0, "programmer error: rotor set name unknown");
    
    return rotor_sets[name];   
}

void rotor_machine::delete_rotor_set(string& name)
{
    rmsk::simple_assert(rotor_sets.count(name) == 0, "programmer error: rotor set name unknown");
    
    rotor_sets.erase(name);
}

void rotor_machine::prepare_rotor(string& rotor_set_name, rotor_id r_id, string& rotor_name, bool reverse)
{
    rotor_set& r_set = get_rotor_set(rotor_set_name);
    
    prepare_rotor(r_set, r_id, rotor_name, reverse);
}

void rotor_machine::save_ini(Glib::KeyFile& ini_file)
{
    get_stepping_gear()->save_ini(ini_file);
    ini_file.set_string(MACHINE_SECTION, KEY_MACHINE_NAME, machine_name);
    ini_file.set_string(MACHINE_SECTION, "rotorsetname", default_rotor_set_name);
    save_additional_components(ini_file);
}

pair<boost::shared_ptr<rotor>, boost::shared_ptr<rotor_ring> > rotor_machine::make_rotor(const unsigned int id, bool insert_inverse)
{
    return get_rotor_set(default_rotor_set_name).make_rotor(rotor_id(id, insert_inverse));
}

pair<boost::shared_ptr<rotor>, boost::shared_ptr<rotor_ring> > rotor_machine::make_rotor_rid(rotor_id r_id)
{
    return get_rotor_set(default_rotor_set_name).make_rotor(r_id);
}

pair<boost::shared_ptr<rotor>, boost::shared_ptr<rotor_ring> > rotor_machine::make_rotor(const char *rotor_set_name, const unsigned int id, bool insert_inverse)
{
    return get_rotor_set(rotor_set_name).make_rotor(rotor_id(id, insert_inverse));
}

pair<boost::shared_ptr<rotor>, boost::shared_ptr<rotor_ring> > rotor_machine::make_rotor(string& rotor_set_name, const unsigned int id, bool insert_inverse)
{
    return get_rotor_set(rotor_set_name).make_rotor(rotor_id(id, insert_inverse));
}

pair<boost::shared_ptr<rotor>, boost::shared_ptr<rotor_ring> > rotor_machine::make_rotor_rid(string& rotor_set_name, rotor_id r_id)
{
    return get_rotor_set(rotor_set_name).make_rotor(r_id);
}

pair<boost::shared_ptr<rotor>, boost::shared_ptr<rotor_ring> > rotor_machine::make_rotor_rid(const char *rotor_set_name, rotor_id r_id)
{
    return get_rotor_set(rotor_set_name).make_rotor(r_id);
}

void rotor_machine::set_printer(boost::shared_ptr<printing_device> new_printer) 
{ 
    printer = new_printer;
    printer->set_machine(this);
}

void rotor_machine::set_keyboard(boost::shared_ptr<rotor_keyboard> new_keyboard)
{
    keyboard = new_keyboard;
    keyboard->set_machine(this);
}

bool rotor_machine::save(string& file_name)
{   
    return rmsk::settings_file_save(file_name, sigc::mem_fun(*this, &rotor_machine::save_ini));
}


