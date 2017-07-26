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

/*! \file enigma_sim.cpp
 *  \brief Implementation of the simulators of all Enigma variants known to this software.
 */ 

#include<stdexcept>
#include<set>
#include<rmsk_globals.h>
#include<enigma_sim.h>
#include<rotor_set.h>
#include<enigma_rotor_set.cpp>
#include<machine_config.h>

#define ETW "eintrittswalze"

rotor_set enigma_rotor_factory::enigma_set(rmsk::std_alpha()->get_size());

rotor_set& enigma_rotor_factory::get_rotor_set()
{
    if (enigma_set.get_num_rotors() == 0)
    {
        // Use rotor set data included from enigma_rotor_set.cpp created by enigrotorset.py
        Glib::ustring data(enigma_rotor_set);
        Glib::KeyFile ini;
        
        (void)ini.load_from_data(data);
        (void)enigma_set.load_ini(ini);
    }
    
    return enigma_set;
}

enigma_family_base::enigma_family_base() 
    : rotor_machine()
{ 
    is_pre_step = true;

    add_rotor_set(DEFAULT_SET, enigma_rotor_factory::get_rotor_set());    
}

bool enigma_family_base::move_all_rotors(ustring& new_positions)
{
    bool result = false;
    stepping_gear *s = get_stepping_gear();
    vector<string> ids, visible_ids;
    vector<string>::iterator iter;
    unsigned int count;
    
    s->get_rotor_identifiers(ids);    
    
    // Determine ids which belong to visible rotors
    for (iter = ids.begin(); iter != ids.end(); ++iter)
    {
        if (unvisualized_rotor_names.count(*iter) == 0)
        {
            visible_ids.push_back(*iter);
        }
    }
    
    // Check if new_positions has correct length
    result = (visible_ids.size() != new_positions.length());
    
    for (count = visible_ids.size(), iter = visible_ids.begin(); (iter != visible_ids.end()) && (!result); count--, ++iter)
    {
        // Does new_positions[count - 1] belong to rmsk::std_uni_alpha()?
        result = !rmsk::std_uni_alpha()->contains_symbol(new_positions[count - 1]);        
    }
    
    // Everything is checked. Now do actual modifications
    for (count = visible_ids.size(), iter = visible_ids.begin(); (iter != visible_ids.end()) && (!result); count--, ++iter)
    {
        s->set_ring_pos(*iter, rmsk::std_uni_alpha()->from_val(new_positions[count - 1]));
    }
    
    return result;
}

void enigma_base::save_additional_components(Glib::KeyFile& ini_file)
{
    rotor_descriptor& desc = get_enigma_stepper()->get_descriptor(UMKEHRWALZE);
    vector<unsigned int> perm_data_raw;
    vector<int> perm_data;
    int std_ukwd_perm[26] = {5, 14, 22, 20, 11, 0, 16, 24, 18, 17, 19, 4, 25, 21, 1, 23, 6, 9, 8, 10, 3, 13, 2, 15, 7, 12};
    
    
    ini_file.set_string("machine", "machinetype", machine_type);
    
    if (desc.id.r_id == UKW_D)
    {
        // Machine uses UKW_D. Save the permutation in use
        desc.r->get_perm()->to_vec(perm_data_raw);
        
        for (unsigned int count = 0; count < 26; count++)
        {
            perm_data.push_back((int)perm_data_raw[count]);
        }
    }
    else
    {
        // Machine does not use UKW_D. Save default value.
        for (unsigned int count = 0; count < 26; count++)
        {
            perm_data.push_back(std_ukwd_perm[count]);
        }        
    }
    
    ini_file.set_integer_list("machine", "ukwdwiring", perm_data);
}

bool enigma_base::load_additional_components(Glib::KeyFile& ini_file)
{
    bool result = false;
    
    result = !ini_file.has_key("machine", "machinetype");
    
    if (!result)
    {
        result = !(machine_type == ini_file.get_string("machine", "machinetype"));
    }
    
    // Ignore ukwdwiring key in section machine. We have no real way to store it anyway.
    
    return result;
}

bool enigma_base::randomize(string& param)
{
    bool result = false;    
    
    machine_config rand_conf;
    Glib::ustring config_name = machine_type;
    rand_conf.make_config(config_name);
    bool suitable_config_found = true;

    steckered_enigma *e = dynamic_cast<steckered_enigma *>(this);
    
    // Do we have a Steckerbrett?
    if (e != NULL)
    {
        // Is Uhr in use?
        rand_conf.get_uses_uhr() = e->uses_uhr();        
    }
    
    if (rand_conf.get_uhr_capable())
    {
        if ((param == "uhr") || (param == "uhronly") || (param == "fancy"))
        {
            rand_conf.get_uses_uhr() = true;
        }

        if ((param == "nouhr") || (param == "basic") || (param == "ukwdonly"))
        {
            rand_conf.get_uses_uhr() = false;
        }
    }
    
    do
    {               
        if ((result = rand_conf.randomize()))
        {
            continue;
        }       

        if ((result = rand_conf.configure_machine(this)))
        {
            continue;
        }
        
        if (rand_conf.is_ukw_d_capable() && (machine_type != "KD") && ((param == "basic") || (param == "uhronly") || (param == "ukwdonly") || (param == "fancy")))        
        {
            if (machine_type == "Services")
            {            
                if ((param == "basic") || (param == "uhronly"))
                {
                    suitable_config_found = get_stepping_gear()->get_descriptor(UMKEHRWALZE).id.r_id != UKW_D;
                }
                else
                {
                    suitable_config_found = get_stepping_gear()->get_descriptor(UMKEHRWALZE).id.r_id == UKW_D;
                }
            }

            if (machine_type == "M3")
            {            
                if (param == "basic")
                {
                    suitable_config_found = get_stepping_gear()->get_descriptor(UMKEHRWALZE).id.r_id != UKW_D;
                }
                else
                {
                    if (param == "ukwdonly")
                    {
                        suitable_config_found = get_stepping_gear()->get_descriptor(UMKEHRWALZE).id.r_id == UKW_D;
                    }
                }
            }
        }            
        
    } while((!suitable_config_found) && (!result));
    
    return result;
}

enigma_stepper_base::enigma_stepper_base(vector<string>& rotor_identifiers) 
    : stepping_gear(rotor_identifiers, rmsk::std_alpha()->get_size()) 
{
    ;
}

/*! The default used is: Ringestellung ist set to all 'a' and rotor displacement is set to the value that makes
 *  'a' appear in the rotor window of each rotor. When using Ringstellung 'a' this is equivalent to setting
 *  the rotor displacement to 0.
*/
void enigma_stepper_base::reset()
{
    stepping_gear::reset();
        
    set_ringstellung(FAST, 'a');
    set_ringstellung(MIDDLE, 'a');
    set_ringstellung(SLOW, 'a');
    set_ringstellung(UMKEHRWALZE, 'a');
    set_rotor_pos(FAST, 'a');
    set_rotor_pos(MIDDLE, 'a');
    set_rotor_pos(SLOW, 'a');
    set_rotor_pos(UMKEHRWALZE, 'a');
    
    // Only M4 has a "Griechenwalze"
    if (rotors.count(GRIECHENWALZE) != 0)
    {
        set_ringstellung(GRIECHENWALZE, 'a');
        set_rotor_pos(GRIECHENWALZE, 'a');
    }
    
    // Services, M3 and M4 Enigma do not need an explicit ETW
    if (rotors.count(ETW) != 0)
    {
        set_ringstellung(ETW, 'a');
        set_rotor_pos(ETW, 'a');
    } 
    
}

void enigma_stepper_base::set_ringstellung(string& identifier, char new_pos)
{
    rmsk::simple_assert(rotors.count(identifier) == 0, "programmer error: rotor identifier unknown");

    get_descriptor(identifier).ring->set_offset(rmsk::std_alpha()->from_val(new_pos));
}

char enigma_stepper_base::get_ringstellung(string& identifier)
{
    rmsk::simple_assert(rotors.count(identifier) == 0, "programmer error: rotor identifier unknown");
    
    return (rmsk::std_alpha()->to_val(get_descriptor(identifier).ring->get_offset()));
}

char enigma_stepper_base::get_rotor_pos(string& identifier)
{
    rmsk::simple_assert(rotors.count(identifier) == 0, "programmer error: rotor identifier unknown");
    
    return (rmsk::std_alpha()->to_val(get_descriptor(identifier).ring->get_pos()));
}

void enigma_stepper_base::set_rotor_pos(string& identifier, char new_pos)
{
    rmsk::simple_assert(rotors.count(identifier) == 0, "programmer error: rotor identifier unknown");

    get_descriptor(identifier).ring->set_pos(rmsk::std_alpha()->from_val(new_pos));
}


void enigma_stepper::step_rotors()
{
    stepping_gear::step_rotors();
    bool middle_steps, slow_steps;
    
    middle_steps = (rotor_is_at_notch(FAST) || rotor_is_at_notch(MIDDLE));
    slow_steps = rotor_is_at_notch(MIDDLE);
    
    advance_rotor(FAST);
    
    if (middle_steps)
    {
        advance_rotor(MIDDLE);
    }
    
    if (slow_steps)
    {
        advance_rotor(SLOW);
    }
}

void abwehr_stepper::step_rotors()
{
    stepping_gear::step_rotors();
    bool middle_steps, slow_steps, ukw_steps;
    
    middle_steps = rotor_is_at_notch(FAST);
    slow_steps = middle_steps && rotor_is_at_notch(MIDDLE);
    ukw_steps = slow_steps && rotor_is_at_notch(SLOW);    
    
    advance_rotor(FAST);
    
    if (middle_steps)
    {
        advance_rotor(MIDDLE);
    }
    
    if (slow_steps)
    {
        advance_rotor(SLOW);
    }
    
    if (ukw_steps)
    {
        advance_rotor(UMKEHRWALZE);
    }
}

enigma_uhr *steckered_enigma::get_uhr()
{
    return dynamic_cast<enigma_uhr *>(input_transform.get());
}

void steckered_enigma::set_stecker_brett(vector<pair<char, char> >& stecker, bool force_uhr)
{
    boost::shared_ptr<encryption_transform> stecker_perm = boost::shared_ptr<encryption_transform>(rmsk::std_alpha()->make_involution_ptr(stecker));
    
    if (force_uhr)
    {
        if (uses_uhr())
        {
            get_uhr()->set_cabling(stecker);
        }
        else
        {
            enigma_uhr *uhr = new enigma_uhr();
            
            uhr->set_cabling(stecker);
            boost::shared_ptr<encryption_transform> uhr_transform(uhr);
            set_input_transform(uhr_transform);
        }
    }
    else
    {    
        set_input_transform(stecker_perm);
    }
}

void steckered_enigma::get_stecker_brett(vector<pair<char, char> >& stecker)
{
    enigma_uhr *uhr;
    string help;
    set<pair<unsigned int, unsigned int> > conv_help;
    unsigned int help_cip;
    set<pair<unsigned int, unsigned int> >::iterator iter;

    stecker.clear();
    
    if (uses_uhr())
    {
        uhr = get_uhr();
        uhr->get_cabling(help);
        
        // When using the Uhr exactly 10 plugs are inserted
        for (unsigned int count = 0; count < 10; count++)
        {
            pair<char, char> plug_help(help[2 * count], help[(2 * count) + 1]);
             
            stecker.push_back(plug_help);
        }
    }
    else
    {
        if (input_transform.get() != NULL)
        {   
            // The plugboard creates a mapping that consists of up to 13 two element cycles. The below code
            // determines these two element cycles and leaves out the one element cycles.
            for (unsigned int count = 0; count < get_size(); count++)
            {
                help_cip = input_transform->encrypt(count);
                
                // Two element cycle?
                if (help_cip != count)
                {
                    // Placing the smaller value of the cycle first makes sure that the cycle (k, n) is
                    // mapped to the same value as the cycle (n, k)
                    if (count < help_cip)
                    {
                        conv_help.insert(pair<unsigned int, unsigned int>(count, help_cip));
                    }
                    else
                    {
                        conv_help.insert(pair<unsigned int, unsigned int>(help_cip, count));
                    }
                }
            }
            
            // Transform the cycles to letter pairs
            for (iter = conv_help.begin(); iter != conv_help.end(); ++iter)
            {
                stecker.push_back(pair<char, char>(rmsk::std_alpha()->to_val(iter->first), rmsk::std_alpha()->to_val(iter->second)));
            }
        }
    }    
}

enigma_I::enigma_I(unsigned int ukw_id, unsigned int slow_id, unsigned int middle_id, unsigned int fast_id, bool type_m3)
    : steckered_enigma()
{
    vector<string> rotor_names;
    
    stepper = NULL;
    machine_name = MNAME_ENIGMA_I;
    
    if (type_m3)
    {
        machine_type = "M3";
        randomizer_params.push_back(randomizer_descriptor("ukwdonly", "Force use of UKW D"));  
        randomizer_params.push_back(randomizer_descriptor("basic", "UKW D is forbidden"));
    }
    else
    {
        machine_type = "Services";
        randomizer_params.push_back(randomizer_descriptor("uhr", "Force use of Uhr and allow for UKW D"));
        randomizer_params.push_back(randomizer_descriptor("nouhr", "Uhr not used but allow for UKW D"));    
        randomizer_params.push_back(randomizer_descriptor("uhronly", "Force use of Uhr but UKW D is forbidden"));
        randomizer_params.push_back(randomizer_descriptor("ukwdonly", "Force use of UKW D but Uhr is forbidden"));    
        randomizer_params.push_back(randomizer_descriptor("basic", "Both Uhr and UKW D are forbidden"));
        randomizer_params.push_back(randomizer_descriptor("fancy", "Force use of Uhr and UKW D"));    
    }
    
    
    // Set names of rotor slots
    rotor_names.push_back(FAST);
    rotor_names.push_back(MIDDLE);
    rotor_names.push_back(SLOW);
    rotor_names.push_back(UMKEHRWALZE);
    
    set_stepping_gear(new enigma_stepper(rotor_names));
    get_stepping_gear()->get_stack().set_reflecting_flag(true);
    
    // Put rotors in machine
    prepare_rotor(fast_id, FAST);
    prepare_rotor(middle_id, MIDDLE);
    prepare_rotor(slow_id, SLOW);    
    prepare_rotor(ukw_id, UMKEHRWALZE);    
        
    get_stepping_gear()->reset();
}

unsteckered_enigma::unsteckered_enigma(unsigned int ukw_id, unsigned int slow_id, unsigned int middle_id, unsigned int fast_id, unsigned int etw_id)
    : enigma_base()
{
    vector<string> rotor_names;
    
    stepper = NULL;
    
    // Set names of rotor slots
    rotor_names.push_back(ETW);
    rotor_names.push_back(FAST);
    rotor_names.push_back(MIDDLE);
    rotor_names.push_back(SLOW);
    rotor_names.push_back(UMKEHRWALZE);
    
    set_stepping_gear(new enigma_stepper(rotor_names));
    get_stepping_gear()->get_stack().set_reflecting_flag(true);
    
    // Put rotors in machine
    // The Eintrittswalze (ETW) permutation has to be placed in reverse in the machine    
    prepare_rotor(etw_id, ETW, true);
    prepare_rotor(fast_id, FAST);
    prepare_rotor(middle_id, MIDDLE);
    prepare_rotor(slow_id, SLOW);    
    prepare_rotor(ukw_id, UMKEHRWALZE);
    
    // No fancy character translations on output side
    boost::shared_ptr<printing_device> prt(new symmetric_printing_device(ustring("abcdefghijklmnopqrstuvwxyz")));
    set_printer(prt);
    
    // No fancy character translations on input side    
    boost::shared_ptr<rotor_keyboard> kbd(new symmetric_keyboard(ustring("abcdefghijklmnopqrstuvwxyz")));
    set_keyboard(kbd);
    
    unvisualized_rotor_names.insert(ETW);            
        
    get_stepping_gear()->reset();    
}

railway_enigma::railway_enigma(unsigned int slow_id, unsigned int middle_id, unsigned int fast_id) 
    : unsteckered_enigma(UKW_RB, slow_id, middle_id, fast_id, WALZE_RB_ETW) 
{ 
    machine_name = MNAME_RAILWAY_ENIGMA; 
    machine_type = "Railway";
}

tirpitz_enigma::tirpitz_enigma(unsigned int slow_id, unsigned int middle_id, unsigned int fast_id) 
    : unsteckered_enigma(UKW_T, slow_id, middle_id, fast_id, WALZE_T_ETW) 
{ 
    machine_name = MNAME_TIRPITZ_ENIGMA; 
    machine_type = "Tirpitz";
}

kd_enigma::kd_enigma(unsigned int slow_id, unsigned int middle_id, unsigned int fast_id) 
    : unsteckered_enigma(UKW_D, slow_id, middle_id, fast_id, WALZE_KD_ETW) 
{ 
    machine_name = MNAME_KD_ENIGMA; 
    machine_type = "KD";
    unvisualized_rotor_names.insert(UMKEHRWALZE);
}

abwehr_enigma::abwehr_enigma(unsigned int slow_id, unsigned int middle_id, unsigned int fast_id)
    : enigma_base()
{
    vector<string> rotor_names;
    
    stepper = NULL;
    machine_name = MNAME_ABWEHR_ENIGMA;
    machine_type = "Abwehr";
    
    // Set names of rotor slots
    rotor_names.push_back(ETW);
    rotor_names.push_back(FAST);
    rotor_names.push_back(MIDDLE);
    rotor_names.push_back(SLOW);
    rotor_names.push_back(UMKEHRWALZE);
    
    set_stepping_gear(new abwehr_stepper(rotor_names));
    get_stepping_gear()->get_stack().set_reflecting_flag(true);

    // Place rotors in machine
    // The QWERTZU in the Eintrittswalze (ETW) has to be placed in reverse in the machine
    prepare_rotor(WALZE_ABW_ETW, ETW, true);
    prepare_rotor(fast_id, FAST);
    prepare_rotor(middle_id, MIDDLE);
    prepare_rotor(slow_id, SLOW);    
    prepare_rotor(UKW_ABW, UMKEHRWALZE);  
    
    // No special character transforms on the output side
    boost::shared_ptr<printing_device> prt(new symmetric_printing_device(ustring("abcdefghijklmnopqrstuvwxyz")));
    set_printer(prt);
    
    // No special character transforms on the input side
    boost::shared_ptr<rotor_keyboard> kbd(new symmetric_keyboard(ustring("abcdefghijklmnopqrstuvwxyz")));
    set_keyboard(kbd);
    
    unvisualized_rotor_names.insert(ETW);
            
    get_stepping_gear()->reset();
}


enigma_M4::enigma_M4(unsigned int ukw_id, unsigned int griechen_id, unsigned int slow_id, unsigned int middle_id, unsigned int fast_id)
    : steckered_enigma()
{
    vector<string> rotor_names;
    
    stepper = NULL;
    machine_name = MNAME_M4_ENIGMA;
    machine_type = "M4";
    
    // Set rotor slot names
    rotor_names.push_back(FAST);
    rotor_names.push_back(MIDDLE);
    rotor_names.push_back(SLOW);
    rotor_names.push_back(GRIECHENWALZE);
    rotor_names.push_back(UMKEHRWALZE);
    
    set_stepping_gear(new enigma_stepper(rotor_names));
    get_stepping_gear()->get_stack().set_reflecting_flag(true);    

    // Place rotors in machine
    prepare_rotor(fast_id, FAST);
    prepare_rotor(middle_id, MIDDLE);
    prepare_rotor(slow_id, SLOW);
    prepare_rotor(griechen_id, GRIECHENWALZE);            
    prepare_rotor(ukw_id, UMKEHRWALZE);    
        
    get_stepping_gear()->reset();
}

void steckered_enigma::save_additional_components(Glib::KeyFile& ini_file)
{
    // Permutation currently used as input_transform 
    vector<int> perm_data;
    enigma_uhr *uhr;
    string cable_spec;

    enigma_base::save_additional_components(ini_file);

    if (input_transform.get() != NULL)
    {
        // Determine the permutation currently used as input_transform 
        for (unsigned int count = 0; count < get_size(); count++)
        {
            perm_data.push_back(input_transform->encrypt(count));
        }
        
        // Check if Uhr is in use
        if ((uhr = dynamic_cast<enigma_uhr *>(input_transform.get())) != NULL)
        {
            // Uhr is used
            ini_file.set_boolean("plugboard", "usesuhr", true);
            uhr->get_cabling(cable_spec);
            ini_file.set_string("plugboard", "uhrcabling", cable_spec);
            ini_file.set_integer("plugboard", "uhrdialpos", (int)uhr->get_dial_pos());
            
        }
        else
        {
            // No Uhr
            ini_file.set_boolean("plugboard", "usesuhr", false);
        }        
    }
    else
    {
        // No input_transformation in use
        
        ini_file.set_boolean("plugboard", "usesuhr", false);
        
        // Set perm_data to the identity permutation
        for (int count = 0; count < (int)get_size(); count++)
        {
            perm_data.push_back(count);
        }            
    }
    
    // Store permutation data
    ini_file.set_integer_list("plugboard", "entry", perm_data);    
}

steckered_enigma::steckered_enigma() 
    : enigma_base() 
{
    // No special character transformations (shifting, X as space ...) on output      
    boost::shared_ptr<printing_device> prt(new symmetric_printing_device(ustring("abcdefghijklmnopqrstuvwxyz")));
    set_printer(prt);    
    
    // No special character transformations on input
    boost::shared_ptr<rotor_keyboard> kbd(new symmetric_keyboard(ustring("abcdefghijklmnopqrstuvwxyz")));
    set_keyboard(kbd);
    
    unvisualized_rotor_names.insert(UMKEHRWALZE);    
}

bool steckered_enigma::uses_uhr()
{
    return (NULL != dynamic_cast<enigma_uhr *>(input_transform.get()));
}

/*! This method returns false when no errors occured and true otherwise.
*/
bool steckered_enigma::load_additional_components(Glib::KeyFile& ini_file)
{
    bool result = false;
    vector<int> temp_permdata;
    vector<unsigned int> perm_data;
    enigma_uhr *uhr;
    string uhr_cabling;
    
    result = enigma_base::load_additional_components(ini_file);
    
    // Plugboard section in ini_file?
    if (!(result = result || (!ini_file.has_key("plugboard", "entry"))))
    {
        temp_permdata = ini_file.get_integer_list("plugboard", "entry");
        
        if (!(result = (temp_permdata.size() != get_size())))
        {
            // cast int values in temp_permdata to unsigned int
            for (unsigned int count = 0; count < get_size(); count++)
            {
                perm_data.push_back((unsigned int)temp_permdata[count]);
            }
            
            if (!(result = !ini_file.has_key("plugboard", "usesuhr")))
            {
                if (!ini_file.get_boolean("plugboard", "usesuhr"))
                {
                    // No Uhr
                    boost::shared_ptr<encryption_transform> input_trans(new permutation(perm_data));
                    set_input_transform(input_trans);
                }
                else
                {
                    // Uhr is used
                    if (!(result = !(ini_file.has_key("plugboard", "uhrcabling") and ini_file.has_key("plugboard", "uhrdialpos"))))
                    {
                        uhr_cabling = ini_file.get_string("plugboard", "uhrcabling");
                        
                        // verify that cabling contains only values from a-z and has correct length
                        if (!(result = !(rmsk::std_alpha()->contains_symbols(uhr_cabling) && (uhr_cabling.length() == 20))))
                        {                            
                            uhr = new enigma_uhr;
                            uhr->set_cabling(uhr_cabling);
                            uhr->set_dial_pos((unsigned int)ini_file.get_integer("plugboard", "uhrdialpos"));
                            boost::shared_ptr<encryption_transform> input_trans(uhr);
                            set_input_transform(input_trans);
                        }
                    }
                }
            }
        }
    }
    
    return result;
}


