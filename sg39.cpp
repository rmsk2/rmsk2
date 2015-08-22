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

/*! \file sg39.cpp
 *  \brief This file contains the implementation of a simulator for the Schluesselgeraet 39
 */ 

#include<rmsk_globals.h>
#include<alphabet.h>
#include<sg39.h>

rotor_set sg39_rotor_factory::sg39_set(rmsk::std_alpha()->get_size());

/*! The document describing the Schlüsselgerät 39 contained no info about the wiring of the rotors or
 *  how many rotors were provided. Below you find 10 random fix point free permutations.
 */
rotor_set& sg39_rotor_factory::get_rotor_set()
{
    if (sg39_set.get_num_rotors() == 0)
    {
        // Normal rotors
        sg39_set.add_rotor(SG39_ROTOR_0, rmsk::std_alpha()->to_vector(string("iymhkeqgbdtuosajvzlwrfpcxn")));
        sg39_set.add_rotor(SG39_ROTOR_1, rmsk::std_alpha()->to_vector(string("nxlpymdvrzieohsafjqctkbguw")));
        sg39_set.add_rotor(SG39_ROTOR_2, rmsk::std_alpha()->to_vector(string("xomuvpktbyswalfhjndercizgq")));
        sg39_set.add_rotor(SG39_ROTOR_3, rmsk::std_alpha()->to_vector(string("ptrbhkwyqaeozumflgxvdijcns")));
        sg39_set.add_rotor(SG39_ROTOR_4, rmsk::std_alpha()->to_vector(string("kuzxvrmqycghftbwanldpoeisj")));
        sg39_set.add_rotor(SG39_ROTOR_5, rmsk::std_alpha()->to_vector(string("fslczopdmqeruhxkywiagntbjv")));
        sg39_set.add_rotor(SG39_ROTOR_6, rmsk::std_alpha()->to_vector(string("nljapsiukmogfvetzwxchqydbr")));
        sg39_set.add_rotor(SG39_ROTOR_7, rmsk::std_alpha()->to_vector(string("wklogxuzrheqbvcmfdjaynpist")));
        sg39_set.add_rotor(SG39_ROTOR_8, rmsk::std_alpha()->to_vector(string("jeoimprbavxqcsgnkywldtzuhf")));
        sg39_set.add_rotor(SG39_ROTOR_9, rmsk::std_alpha()->to_vector(string("xaryumpscfijzwktdgvonqbelh")));
        
        // Reflector
        sg39_set.add_rotor(ID_SG39_UKW,  rmsk::std_alpha()->to_vector(string("ugvhpmbdolyjfqienwxzacrskt")));
    }
    
    return sg39_set;
}

bool sg39_stepping_gear::wheel_is_at_notch(const char *identifier)
{
    return ((get_descriptor(identifier).binary_vals["wheeldata"])[get_descriptor(identifier).mod_int_vals["wheelpos"]] != 0);
}

void sg39_stepping_gear::advance_wheel(const char *identifier)
{
    get_descriptor(identifier).mod_int_vals["wheelpos"].increment();
}

void sg39_stepping_gear::step_rotors()
{
    stepping_gear::step_rotors();
    bool one_steps = false, two_steps = false, three_steps = false;
            
    one_steps = wheel_is_at_notch(ROTOR_1) or rotor_is_at_notch(ROTOR_3);

    two_steps = wheel_is_at_notch(ROTOR_2) or rotor_is_at_notch(ROTOR_1) or rotor_is_at_notch(ROTOR_2);

    three_steps = wheel_is_at_notch(ROTOR_3) or rotor_is_at_notch(ROTOR_2);

    if (one_steps)
    {
        advance_rotor(ROTOR_1);
    }

    if (two_steps)
    {
        advance_rotor(ROTOR_2);
    }

    if (three_steps)
    {
        advance_rotor(ROTOR_3);
    }
    
    advance_wheel(ROTOR_1);    
    advance_wheel(ROTOR_2);    
    advance_wheel(ROTOR_3);        
}

void sg39_stepping_gear::reset()
{
    unsigned int count = 0;
    
    stepping_gear::reset();
    
    for (count = 0; count < num_rotors; count++)
    {
        set_rotor_displacement(count, 0);        
        get_descriptor(count).ring->set_offset(0);
        get_descriptor(count).mod_int_vals["wheelpos"].set_val(0);
    }    
}

void sg39_stepping_gear::set_wheel_pos(const char *rotor_name, unsigned int new_pos)
{
    rmsk::simple_assert(rotors.count(rotor_name) == 0, "programmer error: rotor identifier unknown");
    
    get_descriptor(rotor_name).mod_int_vals["wheelpos"] = new_pos;
}

unsigned int sg39_stepping_gear::get_wheel_pos(const char *rotor_name)
{
    rmsk::simple_assert(rotors.count(rotor_name) == 0, "programmer error: rotor identifier unknown");
    
    return get_descriptor(rotor_name).mod_int_vals["wheelpos"];
}

void sg39_stepping_gear::set_wheel_data(const char *rotor_name, vector<unsigned int>& new_data)
{
    rmsk::simple_assert(rotors.count(rotor_name) == 0, "programmer error: rotor identifier unknown");
    rmsk::simple_assert(get_descriptor(rotor_name).mod_int_vals["wheelpos"].get_mod() > new_data.size(), "programmer error: wheel data too short");    
    
    get_descriptor(rotor_name).binary_vals["wheeldata"] = new_data;
}

void sg39_stepping_gear::get_wheel_data(const char *rotor_name, vector<unsigned int>& data)
{
    rmsk::simple_assert(rotors.count(rotor_name) == 0, "programmer error: rotor identifier unknown");
    
    data = get_descriptor(rotor_name).binary_vals["wheeldata"];    
}

bool sg39_stepping_gear::load_additional_components(string& identifier, Glib::KeyFile& ini_file)
{
    bool result = false;
    string section_name = "rotor_" + identifier;
    vector<int> temp_wheel_data; 
    vector<unsigned int> wheel_data, current_wheel_data;
    int temp_wheelpos;

    // load wheel data (only for the first three rotors)
    if ((identifier == ROTOR_1) or (identifier == ROTOR_2) or (identifier == ROTOR_3))
    {    
        do
        {
            if ((result = !ini_file.has_key(section_name, "wheelpos")))
            {
                break;
            }
            
            temp_wheelpos = ini_file.get_integer(section_name, "wheelpos");       
            
            if ((result = !ini_file.has_key(section_name, "wheeldata")))
            {
                break;
            }
            
            temp_wheel_data = ini_file.get_integer_list(section_name, "wheeldata");
            // Get current wheel data in order to check whether data from ini file has correct size
            get_wheel_data(identifier.c_str(), current_wheel_data);
            
            if ((result = (temp_wheel_data.size() != current_wheel_data.size())))
            {
                break;
            }
            
            // From this point on no error can occur. Make the necessary changes to this object.
            set_wheel_pos(identifier.c_str(), (unsigned int)temp_wheelpos);
            
            // Cast wheel data to unsigned int
            for (unsigned int count = 0; count < temp_wheel_data.size(); count++)
            {
                wheel_data.push_back((unsigned int)temp_wheel_data[count]);
            }
            
            set_wheel_data(identifier.c_str(), wheel_data);
        
        } while(0);    
    }
    
    return result;
}

void sg39_stepping_gear::save_additional_components(string& identifier, Glib::KeyFile& ini_file)
{
    string section_name = "rotor_" + identifier;
    vector<int> temp_wheel_data; 
    vector<unsigned int> wheel_data;
    
    // Save wheeldata and wheelpos currently in use (only for the first three rotors)
    if ((identifier == ROTOR_1) or (identifier == ROTOR_2) or (identifier == ROTOR_3))
    {
        ini_file.set_integer(section_name, "wheelpos", (int)get_wheel_pos(identifier.c_str()));
     
        get_wheel_data(identifier.c_str(), wheel_data); 
        // Cast wheel data to int  
        for (unsigned int count = 0; count < wheel_data.size(); count++)
        {
            temp_wheel_data.push_back((int)wheel_data[count]);
        }
        
        ini_file.set_integer_list(section_name, "wheeldata", temp_wheel_data);
    }
}

/*! The wheels are implemented by storing the wheeldata as well as the wheelpos in key/value pairs of the corresponding 
 *  ::rotor_descriptor object.
 */
schluesselgeraet39::schluesselgeraet39(unsigned int rotor_1_id, unsigned int rotor_2_id, unsigned int rotor_3_id, unsigned int rotor_4_id) 
    : rotor_machine() 
{ 
    vector<string> rotor_names;
    // Set default all zero wheeldata    
    vector<unsigned int> dat_1(21, 0), dat_2(23, 0), dat_3(25, 0);
    simple_mod_int mod_1(21), mod_2(23), mod_3(25);
    
    add_rotor_set(DEFAULT_SET, sg39_rotor_factory::get_rotor_set());  
    
    is_pre_step = true;
    machine_name = "SG39";
    
    // Set names of rotor slots
    rotor_names.push_back(ROTOR_1);
    rotor_names.push_back(ROTOR_2);
    rotor_names.push_back(ROTOR_3);
    rotor_names.push_back(ROTOR_4);
    rotor_names.push_back(UKW_SG39);
    
    sg39_stepping_gear *s = new sg39_stepping_gear(rotor_names);    
    
    set_stepping_gear(s);

    // Place rotors in machine and initialize wheeldata and wheelpos
    prepare_rotor(rotor_1_id, ROTOR_1);
    s->get_descriptor(ROTOR_1).binary_vals["wheeldata"] = dat_1;
    s->get_descriptor(ROTOR_1).mod_int_vals["wheelpos"] = mod_1;        

    prepare_rotor(rotor_2_id, ROTOR_2);
    s->get_descriptor(ROTOR_2).binary_vals["wheeldata"] = dat_2;
    s->get_descriptor(ROTOR_2).mod_int_vals["wheelpos"] = mod_2;        

    prepare_rotor(rotor_3_id, ROTOR_3);    
    s->get_descriptor(ROTOR_3).binary_vals["wheeldata"] = dat_3;
    s->get_descriptor(ROTOR_3).mod_int_vals["wheelpos"] = mod_3;        

    // Place rotor 4 and reflector in machine. There is no wheel controlling the 4th rotor or the reflector
    prepare_rotor(rotor_4_id, ROTOR_4);             
    prepare_rotor(ID_SG39_UKW, UKW_SG39);
    
    // Use standard printing device
    boost::shared_ptr<printing_device> prt(new symmetric_printing_device(ustring("abcdefghijklmnopqrstuvwxyz")));
    set_printer(prt);                   

    // Use standard keyboard
    boost::shared_ptr<rotor_keyboard> kbd(new symmetric_keyboard(ustring("abcdefghijklmnopqrstuvwxyz")));
    set_keyboard(kbd);                   

    get_stepping_gear()->reset();    
}

void schluesselgeraet39::save_additional_components(Glib::KeyFile& ini_file)
{
    vector<int> perm_data;

    // Is there an input transform ?
    if (input_transform.get() != NULL)
    {
        // Yes
        // Determine the current input transform
        for (unsigned int count = 0; count < get_size(); count++)
        {
            perm_data.push_back(input_transform->encrypt(count));
        }        
    }
    else
    {
        // No
        // Use identity as input transform
        for (int count = 0; count < (int)get_size(); count++)
        {
            perm_data.push_back(count);
        }            
    }
    
    ini_file.set_integer_list("plugboard", "entry", perm_data);    
}

bool schluesselgeraet39::load_additional_components(Glib::KeyFile& ini_file)
{
    bool result = false;
    vector<int> temp_permdata;
    vector<unsigned int> perm_data;
    
    if (!(result = !ini_file.has_key("plugboard", "entry")))
    {
        temp_permdata = ini_file.get_integer_list("plugboard", "entry");
        
        if (!(result = (temp_permdata.size() != get_size())))
        {
            // Cast temp_permdata to unisgned int
            for (unsigned int count = 0; count < get_size(); count++)
            {
                perm_data.push_back((unsigned int)temp_permdata[count]);
            }
            
            // Create new input transform as a permutation initialized by perm_data
            boost::shared_ptr<encryption_transform> input_trans(new permutation(perm_data));
            set_input_transform(input_trans);
        }
    }
    
    return result;
}


void schluesselgeraet39::set_reflector(vector<pair<char, char> >& data)
{
    boost::shared_ptr<permutation> new_reflector = boost::shared_ptr<permutation>(rmsk::std_alpha()->make_involution_ptr(data));
    
    get_stepping_gear()->get_descriptor(UKW_SG39).r->set_perm(new_reflector);
}

void schluesselgeraet39::set_reflector(boost::shared_ptr<permutation> involution)
{
    get_stepping_gear()->get_descriptor(UKW_SG39).r->set_perm(involution);
}

