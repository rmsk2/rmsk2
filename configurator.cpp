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

/*! \file configurator.cpp
 *  \brief Contains the implementation of ::configurator and its subclasses.
 */ 


#include<configurator.h>
#include<sigaba.h>
#include<sg39.h>
#include<typex.h>
#include<kl7.h>
#include<nema.h>
#include<boost/lexical_cast.hpp>


configurator *configurator_factory::get_configurator(string& machine_name)
{
    configurator *result = NULL;
    
    if (machine_name == MNAME_SIGABA)
    {
        result = new sigaba_configurator;
    }

    if (machine_name == MNAME_SG39)
    {
        result = new sg39_configurator;
    }

    if (machine_name == MNAME_TYPEX)
    {
        result = new typex_configurator;
    }

    if (machine_name == MNAME_KL7)
    {
        result = new kl7_configurator;
    }

    if (machine_name == MNAME_NEMA)
    {
        result = new nema_configurator;
    }
    
    return result;
}

/* ----------------------------------------------------------- */

string configurator::get_entry_plugboard(encryption_transform *t)
{
    string result;
        
    for (unsigned int count = 0; count < 26; count++)
    {
        result += t->encrypt(count) + 'a';
    }
    
    return result;
}

string configurator::get_reflector(encryption_transform *t)
{
    string result;
    set<char> already_used;
    unsigned int candidate = 0;
    
    while (already_used.size() != 26)
    {
        if (already_used.count(candidate + 'a') == 0)
        {
            // construct the two element cycles that make up the reflector's involution
            result += candidate + 'a';
            result += t->encrypt(candidate) + 'a';
            already_used.insert(t->encrypt(candidate) + 'a'); 
            already_used.insert(candidate + 'a'); 
        }
        
        candidate++;
    }
        
    return result;
}

string configurator::bool_to_string(vector<unsigned int>& vec)
{
    string result;
    unsigned int max_pos = ((vec.size() > 26) ? 26 : vec.size()); 
    
    for (unsigned int count = 0; count < max_pos; count++)
    {
        if (vec[count] != 0)
        {
            result += rmsk::std_alpha()->to_val(count);
        }
    }   
    
    return result;
}

void configurator::string_to_bool(vector<unsigned int>& vec, string& pin_spec)
{
    unsigned int pos;

    for (unsigned int count = 0; count < pin_spec.length(); count++)
    {
        if (rmsk::std_alpha()->contains_symbol(pin_spec[count]))
        {
            pos = rmsk::std_alpha()->from_val(pin_spec[count]);
            
            if (pos < vec.size())
            {
                vec[pos] = 1;
            }
        }
    }
}

string configurator::vec_to_bool(vector<unsigned int>& vec)
{
    string result;
    vector<unsigned int>::iterator iter;
    
    for (iter = vec.begin(); iter != vec.end(); ++iter)
    {
        if (*iter == 0)
        {
            result += '0';
        }
        else
        {
            result += '1';
        }        
    }
    
    return result;
}

bool configurator::check_bool(string& to_test, unsigned int desired_length, vector<unsigned int>& data)
{
    bool result = true;
    
    result = (to_test.length() == desired_length);
    
    if (result)
    {
        data.clear();
        
        for (unsigned int count = 0; count < desired_length; count++)
        {
            if (to_test[count] == '0')
            {
                data.push_back(0);
            }
            else
            {
                data.push_back(1);
            }
        }
    }
    
    return result;
}

bool configurator::check_for_completeness(map<string, string>& config_data)
{
    bool result = true;    
    vector<key_word_info> infos;
    vector<key_word_info>::iterator iter;
    
    get_keywords(infos);
    
    for (iter = infos.begin(); (iter != infos.end()) && (result); ++iter)
    {
        result = (config_data.count(iter->keyword) != 0);
    }
    
    return result;
}

bool configurator::check_for_perm(string& to_test)
{
    set<char> chars_seen;
    
    for (unsigned int count = 0; count < to_test.length(); count++)
    {
        chars_seen.insert(to_test[count]);
    }
    
    return (to_test.length() == 26) && (chars_seen.size() == 26);   
}

/*!
 *   Return value answers the question "Was parsing data into a numeric vector successful?" 
 */
bool configurator::parse_numeric_vector(string& data, vector<unsigned int>& parse_result)
{
    bool result = true, numeric_part;
    string help, current_number;
    unsigned first_char, current_char;
    const unsigned int NUM_MAX_LEN = 5;
    
    
    // Trim the input string to exclude leading spaces. In python this would have been a 
    // single line of code
    do
    {
        if (!(result = (data.length() != 0)))
        {
            break;
        }
        
        // we know now that data is not empty, i.e. data.length() >= 1
        
        for (unsigned int count = 0; (count < data.length()) && result; count++)
        {
            result = ((data[count] >= '0') && (data[count] <= '9')) || (data[count] == ' ');
        }
        
        if (!result)
        {
            break;
        }
        
        // we know now that data contains only numeric characters and spaces
        
        // search the first numeric character from the left of the string
        for (first_char = 0; (first_char < data.length()) && (data[first_char] == ' '); first_char++)
            ;
            
        if (!(result = (first_char < data.length())))
        {
            // no numeric character in the whole string
            break;
        }
        
        // we know now that data contains at least one numeric character and it is at position first_char
        
        help = data.substr(first_char, data.length() - first_char);
        
        // help now consists of numerical characters interspersed with blanks. Trailing spaces are left intact
        
    } while(0);
    
    parse_result.clear();
    
    current_number = "";
    numeric_part = true;
    // Make sure there is at least one blank at the end of help, so we can find it later.
    // The below algorithm depends on it.
    help += ' ';
    
    for (current_char = 0; (current_char < help.length()) && result; current_char++)
    {
        if (numeric_part)
        {
            // piece togther current number
            if (help[current_char] != ' ')
            {
                current_number += help[current_char];
            }
            else
            {
                // current number finished
                numeric_part = false;                
                result = (current_number.length() <= NUM_MAX_LEN);
                
                if (result)
                {
                    // convert current number
                    parse_result.push_back(boost::lexical_cast<unsigned int>(current_number));
                    current_number = "";
                }
            }
        }
        else
        {
            // look for end of whitespace that separates numbers
            if (help[current_char] != ' ')
            {
                numeric_part = true;
                current_number += help[current_char];
            }
        }
    }
    
    return result;
}

bool configurator::check_vector(vector<unsigned int>& to_test, unsigned int range_start, unsigned int range_end, unsigned int desired_length, bool require_unique)
{   
    set<unsigned int> reference;
    
    // create reference set with numbers between range_start and range_end
    for (unsigned int count = range_start; count <= range_end; count++)
    {
        reference.insert(count);
    }
    
    // DRY: delegate work to appropriate method
    return check_vector(to_test, reference, desired_length, require_unique);
}

bool configurator::check_vector(vector<unsigned int>& to_test, set<unsigned int>& ref_values, unsigned int desired_length, bool require_unique)
{
    bool result = (to_test.size() == desired_length);
    set<unsigned int> uniqueness_test;
    
    for (unsigned int count = 0; (count < to_test.size()) && result; count++)
    {
        // to_test[count] contained in set of reference values?
        result = (ref_values.count(to_test[count]) != 0);
        uniqueness_test.insert(to_test[count]);
    }
    
    if (require_unique && result)
    {
        // all numbers are unique if the set uniqueness_test contains the same number of numbers
        // as the vector to_test
        result = (uniqueness_test.size() == desired_length);
    }
    
    return result;
}

bool configurator::check_rotor_spec(string& rotor_spec, char start_char, char end_char, unsigned int desired_length, bool require_unique)
{
    bool result = (rotor_spec.length() == desired_length);
    set<char> uniqueness_test;
    
    for (unsigned int count = 0; (count < rotor_spec.length()) && result; count++)
    {
        result = (rotor_spec[count] >= start_char) && ((rotor_spec[count] <= end_char));
        uniqueness_test.insert(rotor_spec[count]);
    }
    
    if (require_unique && result)
    {
        // all characters are unique if the set uniqueness_test contains the same number of characters
        // as the string rotor_spec
        result = (uniqueness_test.size() == desired_length);
    }
    
    return result;
}

bool configurator::check_pin_spec(string& pin_spec, char start_char, char end_char, unsigned int max_length)
{
    bool result = (pin_spec.length() <= max_length);
    set<char> uniqueness_test;
    
    for (unsigned int count = 0; (count < pin_spec.length()) && result; count++)
    {
        result = (pin_spec[count] >= start_char) && ((pin_spec[count] <= end_char));
        uniqueness_test.insert(pin_spec[count]);
    }
    
    if (result)
    {
        // all characters are unique if the set uniqueness_test contains the same number of characters
        // as the string rotor_spec
        result = (uniqueness_test.size() == pin_spec.length());
    }
    
    return result;
}


/* ----------------------------------------------------------- */

void sigaba_configurator::get_keywords(vector<key_word_info>& infos)
{
    infos.clear();

   /*
    * Determines which cipher and control rotors are placed in the machine and in what sequence. There are ten rotors (0-9). 
    * Each one of them can be placed in Normal or in Reverse orientation in the machine. Therefore for each rotor its designation
    * and orientation (N or R) have to be specified. All of the ten available rotors have to be placed in the machine.
    */    
    infos.push_back(key_word_info(KW_CIPHER_ROTORS, KEY_STRING));    
    infos.push_back(key_word_info(KW_CONTROL_ROTORS, KEY_STRING));        

   /*
    * Determines which index rotors are placed in the machine and in what sequence. There are five rotors (0-4). Each one of 
    * them can be placed in Normal or in Reverse orientation in the machine. Therefore for each rotor its designation and orientation
    * (N or R) have to be specified. All of the five available rotors have to be placed in the machine.
    */
    infos.push_back(key_word_info(KW_INDEX_ROTORS, KEY_STRING));        
    
   /*
    * Determines whether the simulated machine is of type CSP 2900 (CONF_TRUE) or CSP889 (CONF_FALSE)
    */
    infos.push_back(key_word_info(KW_CSP_2900_FLAG, KEY_BOOL));            
}

unsigned int sigaba_configurator::configure_machine(map<string, string>& config_data, rotor_machine *machine_to_configure)
{
    unsigned int result = CONFIGURATOR_OK;
    sigaba *machine = dynamic_cast<sigaba *>(machine_to_configure);
    
    if (machine == NULL)
    {
        result = CONFIGURATOR_ERROR;
    }
    else
    {
        // Parse and verify configuration
        // Fill parsed_config and csp_2900_flag        
        result = parse_config(config_data);
        
        if (result == CONFIGURATOR_OK)
        {
            // Set mode to CSP 889 or CSP 2900
            machine->get_sigaba_stepper()->prepare_machine_type(csp_2900_flag);
        
            // Insert cipher rotors
            machine->prepare_rotor(rotor_set_name.c_str(), parsed_config[0], R_ZERO);
            machine->prepare_rotor(rotor_set_name.c_str(), parsed_config[1], R_ONE);            
            machine->prepare_rotor(rotor_set_name.c_str(), parsed_config[2], R_TWO);                        
            machine->prepare_rotor(rotor_set_name.c_str(), parsed_config[3], R_THREE);                        
            machine->prepare_rotor(rotor_set_name.c_str(), parsed_config[4], R_FOUR);           
                         
            // Insert driver machine/cotrol rotors                         
            machine->get_sigaba_stepper()->get_driver_machine()->prepare_rotor(rotor_set_name.c_str(), parsed_config[9], STATOR_R);
            machine->get_sigaba_stepper()->get_driver_machine()->prepare_rotor(rotor_set_name.c_str(), parsed_config[8], S_MIDDLE);    
            machine->get_sigaba_stepper()->get_driver_machine()->prepare_rotor(rotor_set_name.c_str(), parsed_config[7], S_FAST);    
            machine->get_sigaba_stepper()->get_driver_machine()->prepare_rotor(rotor_set_name.c_str(), parsed_config[6], S_SLOW);
            machine->get_sigaba_stepper()->get_driver_machine()->prepare_rotor(rotor_set_name.c_str(), parsed_config[5], STATOR_L);    
            
            // Insert index machine rotors                                     
            machine->get_sigaba_stepper()->get_index_bank()->prepare_rotor(rotor_set_name.c_str(), parsed_config[10], I_ZERO);
            machine->get_sigaba_stepper()->get_index_bank()->prepare_rotor(rotor_set_name.c_str(), parsed_config[11], I_ONE);    
            machine->get_sigaba_stepper()->get_index_bank()->prepare_rotor(rotor_set_name.c_str(), parsed_config[12], I_TWO);    
            machine->get_sigaba_stepper()->get_index_bank()->prepare_rotor(rotor_set_name.c_str(), parsed_config[13], I_THREE);
            machine->get_sigaba_stepper()->get_index_bank()->prepare_rotor(rotor_set_name.c_str(), parsed_config[14], I_FOUR);                
            
            // Set rotor positions to default values
            machine->get_stepping_gear()->reset();              
        }
    }
    
    return result;
}

rotor_machine *sigaba_configurator::make_machine(map<string, string>& config_data)
{
    rotor_machine *result = NULL;
    
    if (parse_config(config_data) == CONFIGURATOR_OK)
    {
        // Throws exception upon failure
        // Beware: machine is constructed using the default rotor set!
        result = new sigaba(parsed_config, csp_2900_flag);
        // Also call configure_machine in order to make sure that the correct rotor_set as specified in rotor_set_name is used.
        // This call does not change the machine's configuration as long as rotor_set_name == DEFAULT_SET.
        // Result casted to void. Call can not fail.
        (void)configure_machine(config_data, result);
    }
    
    return result;
}

unsigned int sigaba_configurator::parse_rotor_bank(string& rotor_spec, vector<rotor_id>& parsed_ids, bool index_rotors)
{
    unsigned int result = CONFIGURATOR_OK;
    string help;
    // use when index_rotors = false
    char upper_limit = '9';
    
    if (index_rotors)
    {
        // use when index_rotors = true
        upper_limit = '4';
    }
    
    // A rotor bank has 5 rotors, each rotor needs a two character specification
    if (rotor_spec.length() != 10)
    {
        result = CONFIGURATOR_ERROR;
    }
    else
    {    
        for (unsigned int count = 0; (count < 5) && (result == CONFIGURATOR_OK); count++)
        {
            help = rotor_spec.substr(count * 2, 2);
            
            // Check that first character is a digit and second character is either 'R' or 'N'
            if (((help[0] >= '0') && (help[0] <= upper_limit)) && ((help[1] == 'N' || (help[1] == 'R'))))
            {
                parsed_ids.push_back(rotor_id(help[0] - '0', help[1] == 'R'));
            }
            else
            {
                result = CONFIGURATOR_ERROR;
            }
        }
    }
    
    return result;
}

unsigned int sigaba_configurator::parse_config(map<string, string>& config_data)
{
    unsigned int result = CONFIGURATOR_OK;
    
    parsed_config.clear();
    csp_2900_flag = false;
    vector<unsigned int> rotor_ids;
    
    do
    {
        // Check that a value has been specified for each keyword
        if (!check_for_completeness(config_data))
        {
            result = CONFIGURATOR_INCONSISTENT;
            break;
        }

        // Verify and parse config for cipher rotors
        if ((result = parse_rotor_bank(config_data[KW_CIPHER_ROTORS], parsed_config)) != CONFIGURATOR_OK)
        {
            break;
        }

        // Verify and parse config for driver/control rotors        
        if ((result = parse_rotor_bank(config_data[KW_CONTROL_ROTORS], parsed_config)) != CONFIGURATOR_OK)
        {
            break;
        }
        
        // Verify and parse config for index rotors        
        if ((result = parse_rotor_bank(config_data[KW_INDEX_ROTORS], parsed_config, true)) != CONFIGURATOR_OK)
        {
            break;
        }
        
        // Check that no cipher or control rotor id is duplicated
        for (unsigned int count = 0; count < 10; count++)
        {
            rotor_ids.push_back(parsed_config[count].r_id);
        }
            
        if (!check_vector(rotor_ids, 0, 9, 10, true))
        {
            result = CONFIGURATOR_INCONSISTENT;
            break;
        }
        
        rotor_ids.clear();

        // Check that no index rotor id is duplicated
        for (unsigned int count = 10; count < 15; count++)
        {
            rotor_ids.push_back(parsed_config[count].r_id);
        }
            
        if (!check_vector(rotor_ids, 0, 4, 5, true))
        {
            result = CONFIGURATOR_INCONSISTENT;
            break;
        }
        
        // Retrieve CSP 2900 flag
        csp_2900_flag = (config_data[KW_CSP_2900_FLAG] == CONF_TRUE);            
        
    } while(0);
    
    if (result != CONFIGURATOR_OK)
    {
        parsed_config.clear();
        csp_2900_flag = false;        
    }
    
    return result;
}

string sigaba_configurator::transform_rotor_info(rotor_id& id)
{
    string help = "N";
    
    if (id.insert_inverse)
    {
        help = "R";
    }
    
    return (boost::lexical_cast<string>(id.r_id) + help);
}

void sigaba_configurator::get_config(map<string, string>& config_data, rotor_machine *configured_machine)
{   
    sigaba *machine = dynamic_cast<sigaba *>(configured_machine);    
    rmsk::simple_assert(machine == NULL, "programmer error: machine is not SIGABA");    
    string help;
    
    config_data.clear();
    
    // Retrieve cipher rotor information
    help += transform_rotor_info(machine->get_stepping_gear()->get_descriptor(R_ZERO).id);
    help += transform_rotor_info(machine->get_stepping_gear()->get_descriptor(R_ONE).id);    
    help += transform_rotor_info(machine->get_stepping_gear()->get_descriptor(R_TWO).id);    
    help += transform_rotor_info(machine->get_stepping_gear()->get_descriptor(R_THREE).id);        
    help += transform_rotor_info(machine->get_stepping_gear()->get_descriptor(R_FOUR).id);        
    
    config_data[KW_CIPHER_ROTORS] = help;
    help = "";

    // Retrieve driver/control rotor information
    help += transform_rotor_info(machine->get_sigaba_stepper()->get_driver_machine()->get_stepping_gear()->get_descriptor(STATOR_L).id);
    help += transform_rotor_info(machine->get_sigaba_stepper()->get_driver_machine()->get_stepping_gear()->get_descriptor(S_SLOW).id);    
    help += transform_rotor_info(machine->get_sigaba_stepper()->get_driver_machine()->get_stepping_gear()->get_descriptor(S_FAST).id);    
    help += transform_rotor_info(machine->get_sigaba_stepper()->get_driver_machine()->get_stepping_gear()->get_descriptor(S_MIDDLE).id);        
    help += transform_rotor_info(machine->get_sigaba_stepper()->get_driver_machine()->get_stepping_gear()->get_descriptor(STATOR_R).id);        

    config_data[KW_CONTROL_ROTORS] = help;
    help = "";

    // Retrieve index rotor information
    help += transform_rotor_info(machine->get_sigaba_stepper()->get_index_bank()->get_stepping_gear()->get_descriptor(I_ZERO).id);
    help += transform_rotor_info(machine->get_sigaba_stepper()->get_index_bank()->get_stepping_gear()->get_descriptor(I_ONE).id);    
    help += transform_rotor_info(machine->get_sigaba_stepper()->get_index_bank()->get_stepping_gear()->get_descriptor(I_TWO).id);    
    help += transform_rotor_info(machine->get_sigaba_stepper()->get_index_bank()->get_stepping_gear()->get_descriptor(I_THREE).id);        
    help += transform_rotor_info(machine->get_sigaba_stepper()->get_index_bank()->get_stepping_gear()->get_descriptor(I_FOUR).id);        

    config_data[KW_INDEX_ROTORS] = help;
    help = CONF_FALSE;
    
    // Retrieve mode information (CSP 889 or CSP 2900)
    if (machine->get_sigaba_stepper()->is_2900())
    {
        help = CONF_TRUE;
    }
    
    config_data[KW_CSP_2900_FLAG] = help;    
}

/* ----------------------------------------------------------- */

void sg39_configurator::get_keywords(vector<key_word_info>& infos)
{
    infos.clear();

   /*
    * Determines which rotors are placed in the machine and in what sequence. There are 10 rotors (0-9). For each 
    * rotor that is to be placed in the machine a designation has to be specified. Each designation may only appear
    * once. The number of the leftmost (stationary) rotor has to be specified as the first character.
    */    
    infos.push_back(key_word_info(KW_SG39_ROTORS, KEY_STRING));


   /*
    * Determines the ring position of each of the inserted rotors. So this has to be a string of four characters
    * each of which has to be in the range a-z. The first character specifies the ring position of the leftmost
    * (stationary) rotor.
    */    
    infos.push_back(key_word_info(KW_SG39_RING_POS, KEY_STRING));    
        
   /*
    * As with some Enigma variants the reflector of the SG39 can be set in the field. The reflector is specified 
    * by 13 pairs of letters. Example the pairs are: aw bi cv dk et fm gn hz ju lo pq ry sx specify a valid reflector 
    * setting. Each letter can and must occur once in this seting.   
    */    
    infos.push_back(key_word_info(KW_SG39_REFLECTOR_PLUGS, KEY_STRING));
    
   /*
    * As with some Enigma variants the SG39 had a plugboard that permuted the input before entering and again after
    * leaving the rotor stack. In this setting a permutation of a-z has to be specified. As an example the value 
    * ldtrmihoncpwjkbyevsaxgfzuq is a valid plugboard setting.   
    */        
    infos.push_back(key_word_info(KW_SG39_ENTRY_PLUGS, KEY_STRING)); 
    
   /*
    * This configuration element has to contain at most 21 characters which can be from the range a-u. Each letter
    * corresponds to a set pin on the position specified by the letter.   
    */
    infos.push_back(key_word_info(KW_SG39_PINS_WHEEL_1, KEY_STRING));     

   /*
    * This configuration element has to contain at most 23 characters which can be from the range a-w. Each letter
    * corresponds to a set pin on the position specified by the letter.   
    */
    infos.push_back(key_word_info(KW_SG39_PINS_WHEEL_2, KEY_STRING));         
    
   /*
    * This configuration element has to contain at most 25 characters which can be from the range a-y. Each letter
    * corresponds to a set pin on the position specified by the letter.   
    */
    infos.push_back(key_word_info(KW_SG39_PINS_WHEEL_3, KEY_STRING));         

   /*
    * Each of these configuration elements has to contain at most 26 characters which can be from the range a-z.
    * Each letter corresponds to a set pin on the position specified by the letter.   
    */
    infos.push_back(key_word_info(KW_SG39_PINS_ROTOR_1, KEY_STRING));     
    infos.push_back(key_word_info(KW_SG39_PINS_ROTOR_2, KEY_STRING));         
    infos.push_back(key_word_info(KW_SG39_PINS_ROTOR_3, KEY_STRING));         
}

/*!  Caveat: This method assumes that the unsigned int constants SG39_ROTOR_0, ..., SG39_ROTOR_9 from the file sg39.h
 *   have the values 0-9.
 */    
void sg39_configurator::get_config(map<string, string>& config_data, rotor_machine *configured_machine)
{
    schluesselgeraet39 *machine = dynamic_cast<schluesselgeraet39 *>(configured_machine);    
    rmsk::simple_assert(machine == NULL, "programmer error: machine is not SG39");    
    config_data.clear();
    string help;
    vector<unsigned int> help_vec;
    sg39_stepping_gear *stepper = machine->get_sg39_stepper();
    
    // Retrieve a representation of the rotors currently inserted into the schluesselgeraet39 to which
    // machine points.
    help += stepper->get_descriptor(ROTOR_4).id.r_id + '0';
    help += stepper->get_descriptor(ROTOR_3).id.r_id + '0';    
    help += stepper->get_descriptor(ROTOR_2).id.r_id + '0';    
    help += stepper->get_descriptor(ROTOR_1).id.r_id + '0';
    
    config_data[KW_SG39_ROTORS] = help;
    help = "";

    // Retrieve a representation of the ring positions/offsets
    help += stepper->get_descriptor(ROTOR_4).ring->get_offset() + 'a';
    help += stepper->get_descriptor(ROTOR_3).ring->get_offset() + 'a';
    help += stepper->get_descriptor(ROTOR_2).ring->get_offset() + 'a';
    help += stepper->get_descriptor(ROTOR_1).ring->get_offset() + 'a';
    
    config_data[KW_SG39_RING_POS] = help;
    
    // Retrieve current plugboard setting
    boost::shared_ptr<encryption_transform> plugboard = machine->get_input_transform();
    config_data[KW_SG39_ENTRY_PLUGS] = get_entry_plugboard(plugboard.get());

    // Retrieve current reflector setting
    config_data[KW_SG39_REFLECTOR_PLUGS] = get_reflector(stepper->get_descriptor(UKW_SG39).r->get_perm());
    
    // Retrieve current pin settings of drive wheels
    stepper->get_wheel_data(ROTOR_1, help_vec);
    config_data[KW_SG39_PINS_WHEEL_1] = bool_to_string(help_vec);    
    stepper->get_wheel_data(ROTOR_2, help_vec);
    config_data[KW_SG39_PINS_WHEEL_2] = bool_to_string(help_vec);    
    stepper->get_wheel_data(ROTOR_3, help_vec);
    config_data[KW_SG39_PINS_WHEEL_3] = bool_to_string(help_vec);    

    // Retrieve current pin settings of rotors
    stepper->get_descriptor(ROTOR_1).ring->get_ring_data(help_vec);
    config_data[KW_SG39_PINS_ROTOR_1] = bool_to_string(help_vec);  
    stepper->get_descriptor(ROTOR_2).ring->get_ring_data(help_vec);
    config_data[KW_SG39_PINS_ROTOR_2] = bool_to_string(help_vec);  
    stepper->get_descriptor(ROTOR_3).ring->get_ring_data(help_vec); 
    config_data[KW_SG39_PINS_ROTOR_3] = bool_to_string(help_vec);         
}

/*! Caveat: This method assumes that the unsigned int constants SG39_ROTOR_0, ..., SG39_ROTOR_9 from the file sg39.h
 *  have the values 0-9.
 */ 
unsigned int sg39_configurator::parse_config(map<string, string>& config_data)
{
    unsigned int result = CONFIGURATOR_OK;
    bool test_result = true;  
    vector<unsigned int> zero_21(21, 0), zero_23(23, 0), zero_25(25, 0), zero_26(26, 0);
    string ringstellung_temp;
    
    do
    {  
        // Verifiy that a value is given for each keyword  
        if (!check_for_completeness(config_data))
        {
            result = CONFIGURATOR_INCONSISTENT;
            break;
        }
        
        // Verifiy that pin data is syntactically correct and fill pin data variables 
        test_result &= check_pin_spec(config_data[KW_SG39_PINS_WHEEL_1], 'a', 'u', 21);
        test_result &= check_pin_spec(config_data[KW_SG39_PINS_WHEEL_2], 'a', 'w', 23);
        test_result &= check_pin_spec(config_data[KW_SG39_PINS_WHEEL_3], 'a', 'y', 25);
        test_result &= check_pin_spec(config_data[KW_SG39_PINS_ROTOR_1], 'a', 'z', 26);
        test_result &= check_pin_spec(config_data[KW_SG39_PINS_ROTOR_2], 'a', 'z', 26);
        test_result &= check_pin_spec(config_data[KW_SG39_PINS_ROTOR_3], 'a', 'z', 26);                
            
        if (!test_result)
        {
            result = CONFIGURATOR_INCONSISTENT;
            break;
        }

        wheel_1_pins = zero_21;
        string_to_bool(wheel_1_pins, config_data[KW_SG39_PINS_WHEEL_1]);
        wheel_2_pins = zero_23;
        string_to_bool(wheel_2_pins, config_data[KW_SG39_PINS_WHEEL_2]);
        wheel_3_pins = zero_25;
        string_to_bool(wheel_3_pins, config_data[KW_SG39_PINS_WHEEL_3]);
        
        rotor_1_pins = zero_26;
        string_to_bool(rotor_1_pins, config_data[KW_SG39_PINS_ROTOR_1]);
        rotor_2_pins = zero_26;
        string_to_bool(rotor_2_pins, config_data[KW_SG39_PINS_ROTOR_2]);
        rotor_3_pins = zero_26;
        string_to_bool(rotor_3_pins, config_data[KW_SG39_PINS_ROTOR_3]);        


        // Verifiy that rotor setting is syntactically correct          
        if (!check_rotor_spec(config_data[KW_SG39_ROTORS], '0', '9', 4))
        {
            result = CONFIGURATOR_INCONSISTENT;
            break;            
        } 
        
        rotors.clear();
        
        // Store given rotor data in variable rotors 
        for (unsigned int count = 4; count > 0; count--)
        {
            rotors.push_back(rotor_id(config_data[KW_SG39_ROTORS][count - 1] - '0', false));
        }        
        
        // Verify ring positions
        ringstellung_temp = config_data[KW_SG39_RING_POS];
        
        if (!check_rotor_spec(ringstellung_temp, 'a', 'z', 4, false))
        {
            result = CONFIGURATOR_INCONSISTENT;
            break;            
        }                

        // Store ring positions in variable ring_positions        
        ring_positions.clear();
        
        for (unsigned int count = 4; count > 0; count--)
        {
            ring_positions.push_back(ringstellung_temp[count - 1] - 'a');
        }        
        
        // Verify entry and reflector permutations 
        test_result = check_for_perm(config_data[KW_SG39_ENTRY_PLUGS]);
        test_result &= check_for_perm(config_data[KW_SG39_REFLECTOR_PLUGS]);
        
        if (!test_result)
        {
            result = CONFIGURATOR_INCONSISTENT;
            break;            
        } 
        
        reflector.clear();
        
        // Populate reflector variable with previously verified values        
        for (unsigned int count = 0; count < 13; count++)
        {
            reflector.push_back(pair<char, char>(config_data[KW_SG39_REFLECTOR_PLUGS][2 * count], config_data[KW_SG39_REFLECTOR_PLUGS][(2 * count) + 1]));
        }
        
        // Set entry/plugboard permutation contained in variable entry_perm
        entry_perm.clear();
        entry_perm = rmsk::std_alpha()->to_vector(config_data[KW_SG39_ENTRY_PLUGS]);

    } while(0);
    
    return result;    
}
    
unsigned int sg39_configurator::configure_machine(map<string, string>& config_data, rotor_machine *machine_to_configure)
{
    unsigned int result = CONFIGURATOR_OK;
    schluesselgeraet39 *machine = dynamic_cast<schluesselgeraet39 *>(machine_to_configure);
    
    if (machine == NULL)
    {
        result = CONFIGURATOR_ERROR;
    }
    else
    {
        sg39_stepping_gear *stepper = machine->get_sg39_stepper();        
        result = parse_config(config_data);
        
        if (result == CONFIGURATOR_OK)
        {            
            // Insert rotors into machine
            machine->prepare_rotor(rotor_set_name.c_str(), rotors[0], ROTOR_1);
            machine->prepare_rotor(rotor_set_name.c_str(), rotors[1], ROTOR_2);            
            machine->prepare_rotor(rotor_set_name.c_str(), rotors[2], ROTOR_3);                        
            machine->prepare_rotor(rotor_set_name.c_str(), rotors[3], ROTOR_4);                          
            
            // Set pin data on rotors
            stepper->get_descriptor(ROTOR_3).ring->set_ring_data(rotor_3_pins);
            stepper->get_descriptor(ROTOR_1).ring->set_ring_data(rotor_1_pins);
            stepper->get_descriptor(ROTOR_2).ring->set_ring_data(rotor_2_pins);
            
            // Set pin data on wheels
            stepper->set_wheel_data(ROTOR_1, wheel_1_pins);
            stepper->set_wheel_data(ROTOR_2, wheel_2_pins);
            stepper->set_wheel_data(ROTOR_3, wheel_3_pins);                
            
            // Set reflector
            machine->set_reflector(reflector);
            // Set plugboard permutation
            machine->set_input_transform(boost::shared_ptr<encryption_transform>(new permutation(entry_perm)));            

            // Set ring position on rotors            
            stepper->get_descriptor(ROTOR_1).ring->set_offset(ring_positions[0]);
            stepper->get_descriptor(ROTOR_2).ring->set_offset(ring_positions[1]);
            stepper->get_descriptor(ROTOR_3).ring->set_offset(ring_positions[2]);
            stepper->get_descriptor(ROTOR_4).ring->set_offset(ring_positions[3]);

            // Move all non stationary rotors and all wheels to the 'a' position
            for (unsigned int count = 0; count < 3; count++)
            {
                stepper->get_descriptor(count).ring->set_pos(0);
                stepper->get_descriptor(count).mod_int_vals["wheelpos"].set_val(0);
            }
            
            // Move stationary rotor to 'a' position
            stepper->get_descriptor(3).ring->set_pos(0);
                        
        }
    }
    
    return result;
}

rotor_machine *sg39_configurator::make_machine(map<string, string>& config_data)
{
    rotor_machine *result = NULL;
    
    if (parse_config(config_data) == CONFIGURATOR_OK)
    {
        // Throws exception upon failure
        // Beware: machine is constructed using the default rotor set!
        result = new schluesselgeraet39(rotors[0].r_id, rotors[1].r_id, rotors[2].r_id, rotors[3].r_id);
        // Also call configure_machine in order to make sure that the correct rotor_set as specified in rotor_set_name is used.
        // This call does not change the machine's configuration as long as rotor_set_name == DEFAULT_SET.
        // Result casted to void. Call can not fail.
        (void)configure_machine(config_data, result);
    }
    
    return result;
}

/* ----------------------------------------------------------- */

typex_configurator::typex_configurator()
{
    ;
}

void typex_configurator::get_keywords(vector<key_word_info>& infos)
{
    infos.clear();

   /*
    * Determines which rotors are placed in the machine and in what sequence. There are seven rotors (a-g). Each
    * one of them can be placed in Normal or in Reverse orientation in the machine. Therefore for each rotor its 
    * designation and orientation (N or R) have to be specified. Exactly five of the possible seven rotors have to 
    * be placed in the machine.   
    */    
    infos.push_back(key_word_info(KW_TYPEX_ROTORS, KEY_STRING));    

   /*
    * Like Enigma rotors Typex rotors have a letter ring that can be set to 26 different positions with respect to
    * the wiring core. The positions are designated by the letters a-z. Therefore this setting consists of five letters.   
    */
    infos.push_back(key_word_info(KW_TYPEX_RINGS, KEY_STRING));        

   /*
    * As with some Enigma variants the reflector of the Typex can be set in the field. The reflector is specified by 13
    * pairs of letters. An example of a valid refelctor setting is: ar by cu dh eq fs gl ix jp kn mo tw vz. Each letter 
    * can and must occur once in this seting.   
    */
    infos.push_back(key_word_info(KW_TYPEX_REFLECTOR, KEY_STRING));
}

/*! Caveat: This method assumes that the unsigned int constants TYPEX_SP_02390_A, ..., TYPEX_SP_02390_G from the file enigma_rotor_set.h
 *  have consecutive values.
 */ 
string typex_configurator::transform_typex_rotor_info(rotor_id& id)
{
    string help = "N", rotor_name;
    
    if (id.insert_inverse)
    {
        help = "R";
    }
    
    rotor_name += (id.r_id - TYPEX_SP_02390_A) + 'a';
    
    return (rotor_name + help);
}

void typex_configurator::get_config(map<string, string>& config_data, rotor_machine *configured_machine)
{
    typex *machine = dynamic_cast<typex *>(configured_machine);    
    rmsk::simple_assert(machine == NULL, "programmer error: machine is not Typex");    
    string help;
    typex_stepper *stepper = dynamic_cast<typex_stepper *>(machine->get_stepping_gear());
    
    config_data.clear();
    
    // Retrieve rotor information from machine
    help += transform_typex_rotor_info(machine->get_stepping_gear()->get_descriptor(SLOW).id);
    help += transform_typex_rotor_info(machine->get_stepping_gear()->get_descriptor(MIDDLE).id);    
    help += transform_typex_rotor_info(machine->get_stepping_gear()->get_descriptor(FAST).id);    
    help += transform_typex_rotor_info(machine->get_stepping_gear()->get_descriptor(STATOR2).id);        
    help += transform_typex_rotor_info(machine->get_stepping_gear()->get_descriptor(STATOR1).id);
    
    config_data[KW_TYPEX_ROTORS] = help;
    help = "";
    
    // Retrieve current ringstellung from machine
    help += stepper->get_ringstellung(SLOW);    
    help += stepper->get_ringstellung(MIDDLE);    
    help += stepper->get_ringstellung(FAST);    
    help += stepper->get_ringstellung(STATOR2);    
    help += stepper->get_ringstellung(STATOR1);                    

    config_data[KW_TYPEX_RINGS] = help;
    //help = "";
    
    // Retrieve current reflector setting from machine
    config_data[KW_TYPEX_REFLECTOR] = get_reflector(machine->get_stepping_gear()->get_descriptor(UMKEHRWALZE).r->get_perm());
}

unsigned int typex_configurator::configure_machine(map<string, string>& config_data, rotor_machine *machine_to_configure)
{
    unsigned int result = CONFIGURATOR_OK;
    typex *machine = dynamic_cast<typex *>(machine_to_configure);
    
    if (machine == NULL)
    {
        result = CONFIGURATOR_ERROR;
    }
    else
    {
        typex_stepper *stepper = dynamic_cast<typex_stepper *>(machine->get_stepping_gear());        
        result = parse_config(config_data);
        
        if (result == CONFIGURATOR_OK)
        {
            // Insert rotors into machine                   
            machine->prepare_rotor(rotor_set_name.c_str(), rotors[0], SLOW);
            machine->prepare_rotor(rotor_set_name.c_str(), rotors[1], MIDDLE);            
            machine->prepare_rotor(rotor_set_name.c_str(), rotors[2], FAST);                        
            machine->prepare_rotor(rotor_set_name.c_str(), rotors[3], STATOR2);                          
            machine->prepare_rotor(rotor_set_name.c_str(), rotors[4], STATOR1);                                      
            
            // Set ringstellung
            stepper->set_ringstellung(SLOW, ringstellung[0]);
            stepper->set_ringstellung(MIDDLE, ringstellung[1]);
            stepper->set_ringstellung(FAST, ringstellung[2]);
            stepper->set_ringstellung(STATOR2, ringstellung[3]);
            stepper->set_ringstellung(STATOR1, ringstellung[4]);                                                
            
            // Set reflector            
            machine->set_reflector(reflector);
                           
            // Reset rotor positions. stepper->reset() must not be called as this would also reset
            // the ringstellung          
            stepper->set_rotor_pos(SLOW, 'a');
            stepper->set_rotor_pos(MIDDLE, 'a');
            stepper->set_rotor_pos(FAST, 'a');
            stepper->set_rotor_pos(STATOR2, 'a');
            stepper->set_rotor_pos(STATOR1, 'a');                  
        }
    }
    
    return result;
}

rotor_machine *typex_configurator::make_machine(map<string, string>& config_data)
{
    rotor_machine *result = NULL;
    
    if (parse_config(config_data) == CONFIGURATOR_OK)
    {
        // Throws exception upon failure
        // Beware: machine is constructed using the default rotor set!
        result = new typex(TYPEX_SP_02390_UKW, rotors[0].r_id, rotors[1].r_id, rotors[2].r_id, rotors[3].r_id, rotors[4].r_id);
        // Also call configure_machine in order to make sure that the correct rotor_set as specified in rotor_set_name is used.
        // This call does not change the machine's configuration as long as rotor_set_name == DEFAULT_SET.
        // Result casted to void. Call can not fail.
        (void)configure_machine(config_data, result);
    }
    
    return result;
}

unsigned int typex_configurator::parse_config(map<string, string>& config_data)
{
    unsigned int result = CONFIGURATOR_OK;
    string help, value;
    set<char> uniqueness_test;  
    
    do
    {  
        // Verify that there is a value for all keywords   
        if (!check_for_completeness(config_data))
        {
            result = CONFIGURATOR_INCONSISTENT;
            break;
        }
        
        // Verify ringstellung and store in the variable of the same name        
        ringstellung = config_data[KW_TYPEX_RINGS];
        
        if (!check_rotor_spec(ringstellung, 'a', 'z', 5, false))
        {
            result = CONFIGURATOR_INCONSISTENT;
            break;            
        }        
        
        // verify rotor selection and populate rotors variable
        value = config_data[KW_TYPEX_ROTORS];        
        if (value.length() != 10)
        {
            result = CONFIGURATOR_INCONSISTENT;
            break;            
        }
        
        rotors.clear();
        
        // Parse rotor information
        for (unsigned int count = 0; (count < 5) && (result == CONFIGURATOR_OK); count++)
        {
            help = value.substr(count * 2, 2);
            
            // First character has to be from the range 'a'-'g'
            // second character has to be 'R' or 'N'
            if (((help[0] >= 'a') && (help[0] <= 'g')) && ((help[1] == 'N' || (help[1] == 'R'))))
            {
                rotors.push_back(rotor_id(help[0] - 'a' + TYPEX_SP_02390_A, help[1] == 'R'));
                uniqueness_test.insert(help[0]);
            }
            else
            {
                result = CONFIGURATOR_ERROR;
            }
        }            
        
        if (result != CONFIGURATOR_OK)
        {
            break;
        }
        
        // verify that each rotor has been used at most once
        if (uniqueness_test.size() != 5)
        {
            result = CONFIGURATOR_INCONSISTENT;
            break;            
        }
        
        // verify reflector specification and set reflector variable
        if (!check_for_perm(config_data[KW_TYPEX_REFLECTOR]))
        {
            result = CONFIGURATOR_INCONSISTENT;
            break;            
        } 
        
        reflector.clear();
        
        // Interpret keyword data as a sequence of pairs that specify the reflector plugs        
        for (unsigned int count = 0; count < 13; count++)
        {
            reflector.push_back(pair<char, char>(config_data[KW_TYPEX_REFLECTOR][2 * count], config_data[KW_TYPEX_REFLECTOR][(2 * count) + 1]));
        }
        
    } while(0);
    
    return result;    
}

/* ----------------------------------------------------------- */

void kl7_configurator::get_keywords(vector<key_word_info>& infos)
{
    infos.clear();

   /*
    * Determines which rotors are placed in the machine and in what sequence. There are thirteen rotors
    * to choose from (a-m). Exactly eight of them have to be placed in the machine and therefore a string 
    * consisting of eight letters has to be specified in this field. Each rotor can appear at most once in this string.   
    */    
    infos.push_back(key_word_info(KW_KL7_ROTORS, KEY_STRING));    
    
   /*
    * The alphabet rings can be rotated relative the wiring core. The position to which the alphabet ring is to 
    * be moved is a number between 1 and 36. Therefore this field has to contain 8 such numbers, each seperated from
    * the preceeeding one by a space.   
    */
    infos.push_back(key_word_info(KW_KL7_ALPHA_POS, KEY_STRING));        

   /*
    * Determines which notch rings are attached to the rotors. There are eleven notch rings to choose from (1-11). 
    * Exactly eight of them have to be attached to the rotors in the machine. There is a 12th special ring, called 
    * the wide ring, which is always attached to the non-moving fourth rotor. The remaining seven rings can be be
    * chosen from the set of eleven mentioned above. This field therefore has to contain seven numbers between 
    * 1 and 11, each seperated from the preceeeding one by a space. Each notch ring can appear at most once in this field.   
    */
    infos.push_back(key_word_info(KW_KL7_NOTCH_RINGS, KEY_STRING));        

   /*
    * Determines to what positions the notch rings are moved after they have been attached to the correponding rotors. 
    * As the wide ring, which is always attached to the non-moving fourth rotor, can not be moved this field has to 
    * specifiy the positions of seven notch rings. Each position is either a letter between a and z or a letter between 
    * a and z followed by a + sign.   
    */
    infos.push_back(key_word_info(KW_KL7_NOTCH_POS, KEY_STRING));            
}

/*! Caveat: This method assumes that KL7_ROTOR_A = 0, KL7_ROTOR_B = 1, ..., KL7_ROTOR_M = 12. On top of that
 *  it assumes that KL7_RING_1 to KL7_RING_11 are consecutive numbers.
 */
void kl7_configurator::get_config(map<string, string>& config_data, rotor_machine *configured_machine)
{
    kl7 *machine = dynamic_cast<kl7 *>(configured_machine);
    // make sure we are dealing with a kl7 object    
    rmsk::simple_assert(machine == NULL, "programmer error: machine is not KL7"); 
    kl7_stepping_gear *stepper = machine->get_kl7_stepper();    
    string help;
    
    config_data.clear();
    
    // Determine rotor names
    help += stepper->get_descriptor(KL7_ROT_1).id.r_id + 'a';
    help += stepper->get_descriptor(KL7_ROT_2).id.r_id + 'a';
    help += stepper->get_descriptor(KL7_ROT_3).id.r_id + 'a';
    help += stepper->get_descriptor(KL7_ROT_4).id.r_id + 'a';            
    help += stepper->get_descriptor(KL7_ROT_5).id.r_id + 'a';
    help += stepper->get_descriptor(KL7_ROT_6).id.r_id + 'a';
    help += stepper->get_descriptor(KL7_ROT_7).id.r_id + 'a';
    help += stepper->get_descriptor(KL7_ROT_8).id.r_id + 'a';            
    
    config_data[KW_KL7_ROTORS] = help;
    help = "";    
    
    // Determine notch ring ids
    help += boost::lexical_cast<string>(stepper->get_descriptor(KL7_ROT_1).id.ring_id - KL7_RING_1 + 1);
    help += " " + boost::lexical_cast<string>(stepper->get_descriptor(KL7_ROT_2).id.ring_id - KL7_RING_1 + 1);
    help += " " + boost::lexical_cast<string>(stepper->get_descriptor(KL7_ROT_3).id.ring_id - KL7_RING_1 + 1);
    help += " " + boost::lexical_cast<string>(stepper->get_descriptor(KL7_ROT_5).id.ring_id - KL7_RING_1 + 1);
    help += " " + boost::lexical_cast<string>(stepper->get_descriptor(KL7_ROT_6).id.ring_id - KL7_RING_1 + 1);
    help += " " + boost::lexical_cast<string>(stepper->get_descriptor(KL7_ROT_7).id.ring_id - KL7_RING_1 + 1);
    help += " " + boost::lexical_cast<string>(stepper->get_descriptor(KL7_ROT_8).id.ring_id - KL7_RING_1 + 1);           
    
    config_data[KW_KL7_NOTCH_RINGS] = help;
    help = "";    
    
    // Determine letter ring offsets. One is added to each get_letter_offset() result as letter ring offset
    // values start with one in the user interface
    help += boost::lexical_cast<string>(stepper->get_letter_offset(KL7_ROT_1) + 1);
    help += " " + boost::lexical_cast<string>(stepper->get_letter_offset(KL7_ROT_2) + 1);
    help += " " + boost::lexical_cast<string>(stepper->get_letter_offset(KL7_ROT_3) + 1);
    help += " " + boost::lexical_cast<string>(stepper->get_stationary_rotor_ring_pos() + 1);           
    help += " " + boost::lexical_cast<string>(stepper->get_letter_offset(KL7_ROT_5) + 1);
    help += " " + boost::lexical_cast<string>(stepper->get_letter_offset(KL7_ROT_6) + 1);
    help += " " + boost::lexical_cast<string>(stepper->get_letter_offset(KL7_ROT_7) + 1);
    help += " " + boost::lexical_cast<string>(stepper->get_letter_offset(KL7_ROT_8) + 1);           

    config_data[KW_KL7_ALPHA_POS] = help;
    help = "";    

    // Determine notch ring offsets
    help += transform_notch_ring_pos(stepper->get_notch_offset(KL7_ROT_1));
    help += transform_notch_ring_pos(stepper->get_notch_offset(KL7_ROT_2));
    help += transform_notch_ring_pos(stepper->get_notch_offset(KL7_ROT_3));
    help += transform_notch_ring_pos(stepper->get_notch_offset(KL7_ROT_5));
    help += transform_notch_ring_pos(stepper->get_notch_offset(KL7_ROT_6));
    help += transform_notch_ring_pos(stepper->get_notch_offset(KL7_ROT_7));
    help += transform_notch_ring_pos(stepper->get_notch_offset(KL7_ROT_8));           

    config_data[KW_KL7_NOTCH_POS] = help;    
}

string kl7_configurator::transform_notch_ring_pos(unsigned int pos)
{
    string result;
    alphabet<char> kl7_ring_alpha(KL7_RING_CIRCUMFENCE_HELP, KL7_ROTOR_SIZE);    
    // Transform pos to a character using the alphabet "ab1cde2fg3hij4klm5no6pqr7st8uvw9xyz0"
    char help = kl7_ring_alpha.to_val(pos);
    
    if ((help >= '0') && (help <= '9'))
    {
        // pos designates a '+' position
        result += kl7_ring_alpha.to_val(pos - 1); // This works as '+' positions are always preceeded by a letter position
        result += '+';
    }
    else
    {
        result += help;
    }
    
    return result;
}
    
unsigned int kl7_configurator::configure_machine(map<string, string>& config_data, rotor_machine *machine_to_configure)
{
    unsigned int result = CONFIGURATOR_OK;
    kl7 *machine = dynamic_cast<kl7 *>(machine_to_configure);
    
    if (machine == NULL)
    {
        result = CONFIGURATOR_ERROR;
    }
    else
    {
        result = parse_config(config_data);
        
        if (result == CONFIGURATOR_OK)
        {    
            kl7_stepping_gear *stepper = machine->get_kl7_stepper();
            
            // Insert rotors and notch rings
            machine->prepare_rotor(rotor_set_name.c_str(), rotors[0], KL7_ROT_1);
            machine->prepare_rotor(rotor_set_name.c_str(), rotors[1], KL7_ROT_2);            
            machine->prepare_rotor(rotor_set_name.c_str(), rotors[2], KL7_ROT_3);                        
            machine->prepare_rotor(rotor_set_name.c_str(), rotors[3], KL7_ROT_4);                          
            machine->prepare_rotor(rotor_set_name.c_str(), rotors[4], KL7_ROT_5);                    
            machine->prepare_rotor(rotor_set_name.c_str(), rotors[5], KL7_ROT_6);  
            machine->prepare_rotor(rotor_set_name.c_str(), rotors[6], KL7_ROT_7);                    
            machine->prepare_rotor(rotor_set_name.c_str(), rotors[7], KL7_ROT_8);
            
            // Set letter and notch ring offsets
            stepper->set_kl7_rings(KL7_ROT_1, letter_ring_offsets[0], notch_ring_offsets[0]);    
            stepper->set_kl7_rings(KL7_ROT_2, letter_ring_offsets[1], notch_ring_offsets[1]);    
            stepper->set_kl7_rings(KL7_ROT_3, letter_ring_offsets[2], notch_ring_offsets[2]);    
            stepper->set_stationary_rotor_ring_pos(letter_ring_offsets[3]);
            stepper->set_kl7_rings(KL7_ROT_5, letter_ring_offsets[4], notch_ring_offsets[3]);
            stepper->set_kl7_rings(KL7_ROT_6, letter_ring_offsets[5], notch_ring_offsets[4]); 
            stepper->set_kl7_rings(KL7_ROT_7, letter_ring_offsets[6], notch_ring_offsets[5]);  
            stepper->set_kl7_rings(KL7_ROT_8, letter_ring_offsets[7], notch_ring_offsets[6]);
            
            // Move all rotors to a position such that 'A' appears in the rotor window
            stepper->move_to_letter_ring_pos(KL7_ROT_1, 0);                                             
            stepper->move_to_letter_ring_pos(KL7_ROT_2, 0);
            stepper->move_to_letter_ring_pos(KL7_ROT_3, 0);                        
            stepper->move_to_letter_ring_pos(KL7_ROT_5, 0);
            stepper->move_to_letter_ring_pos(KL7_ROT_6, 0);                        
            stepper->move_to_letter_ring_pos(KL7_ROT_7, 0);            
            stepper->move_to_letter_ring_pos(KL7_ROT_8, 0);            
        }
    }
    
    return result;
}

rotor_machine *kl7_configurator::make_machine(map<string, string>& config_data)
{
    rotor_machine *result = NULL;
    
    if (parse_config(config_data) == CONFIGURATOR_OK)
    {
        // Throws exception upon failure
        // Beware: machine is constructed using the default rotor set!
        result = new kl7(rotors);
        // Also call configure_machine in order to make sure that the correct rotor_set as specified in rotor_set_name is used.
        // This call does not change the machine's configuration as long as rotor_set_name == DEFAULT_SET.
        // Result casted to void. Call can not fail.
        (void)configure_machine(config_data, result);
    }
    
    return result;
}

/*! Caveat: This method assumes that KL7_ROTOR_A has id 0, KL7_ROTOR_B id 1 and so on. Additionally it assumed that the notch ring
 *  ids are consecutive numbers between 100 and 111.
 */
unsigned int kl7_configurator::parse_config(map<string, string>& config_data)
{
    unsigned int result = CONFIGURATOR_OK;
    set<char> uniqeness_test;
    alphabet<char> kl7_ring_alpha(KL7_RING_CIRCUMFENCE_HELP, KL7_ROTOR_SIZE);
    unsigned int count_plus;
    string help;
    
    do
    {
        // Verify that there is a value for all keywords     
        if (!check_for_completeness(config_data))
        {
            result = CONFIGURATOR_INCONSISTENT;
            break;
        }
        
        // Verify letter ring offsets    
        if (!parse_numeric_vector(config_data[KW_KL7_ALPHA_POS], letter_ring_offsets))
        {
            result = CONFIGURATOR_INCONSISTENT;
            break;            
        }
        
        if (!check_vector(letter_ring_offsets, 1, 36, 8))
        {
            result = CONFIGURATOR_INCONSISTENT;
            break;
        }
        
        // In the user interface letter ring offsets have values between 1 and 36 are used
        // Internally values are zero based
        for (unsigned int count = 0; count < letter_ring_offsets.size(); count++)
        {
            letter_ring_offsets[count]--;
        }           

        // Verify notch ring ids
        if (!parse_numeric_vector(config_data[KW_KL7_NOTCH_RINGS], notch_rings))
        {
            result = CONFIGURATOR_INCONSISTENT;
            break;            
        }
        
        if (!check_vector(notch_rings, 1, 11, 7, true))
        {
            result = CONFIGURATOR_INCONSISTENT;
            break;
        }
        
        // Verify rotor names
        rotor_names = config_data[KW_KL7_ROTORS];
        
        if (!check_rotor_spec(rotor_names, 'a', 'm', 8))
        {
            result = CONFIGURATOR_INCONSISTENT;        
            break;        
        }
        
        // Verify notch ring offsets
        help = config_data[KW_KL7_NOTCH_POS];
        
        // The code contained in the for loop below assumes that help has length of at least one ...
        if (!(help.length() > 0))
        {
            result = CONFIGURATOR_INCONSISTENT;        
            break;
        }
        
        // ... and that the first element is not a '+' sign
        if (!((help[0] >= 'a') && (help[0] <= 'z')))
        {
            result = CONFIGURATOR_INCONSISTENT;        
            break;
        }
        
        notch_ring_offsets.clear();
        // number of consecutive '+' signs encountered. Must never be bigger than 1
        count_plus = 0;
        for (unsigned int count = 0; (count < help.length()) && (result == CONFIGURATOR_OK); count++)
        {
            // Only 'a' - 'z' and '+' are allowed
            if (((help[count] >= 'a') && (help[count] <= 'z')) || (help[count] == '+'))
            {
                // Is current character a '+'?
                if (help[count] == '+')
                {
                    // Yes!!
                                        
                    if (count_plus != 0)
                    {
                        // Previous character also was a '+' => error
                        result = CONFIGURATOR_INCONSISTENT;    
                    }
                    else
                    {
                        // Previous character was a letter => Real offset is the current value plus one
                        notch_ring_offsets[notch_ring_offsets.size() - 1]++;
                    }
                    
                    // Increment '+' counter
                    count_plus++;
                }
                else
                {
                    // No!!
                
                    // transform character value into a numeric value according to the alphabet
                    // "ab1cde2fg3hij4klm5no6pqr7st8uvw9xyz0"
                    notch_ring_offsets.push_back(kl7_ring_alpha.from_val(help[count]));
                    
                    // Reset '+' counter
                    count_plus = 0;
                }
            }
            else
            {
                result = CONFIGURATOR_INCONSISTENT;
            }
        }

        if (result != CONFIGURATOR_OK)
        {
            break;
        }
        
        // Verify that there are 7 notch ring offsets each having a value between 0 and 35
        if (!check_vector(notch_ring_offsets, 0, 35, 7))
        {
            result = CONFIGURATOR_INCONSISTENT;
            break;
        }
        
        rotors.clear();
        
        // Create the rotor_id "objects" to store in the instance variable rotors. Here it is assumed that
        // KL7_ROTOR_A has id 0, KL7_ROTOR_B id 1 and so on. Additionally it assumed that the notch ring ids
        // are consecutive numbers between 100 and 111.
        rotors.push_back(rotor_id(rotor_names[0] - 'a', notch_rings[0] + 99));
        rotors.push_back(rotor_id(rotor_names[1] - 'a', notch_rings[1] + 99));   
        rotors.push_back(rotor_id(rotor_names[2] - 'a', notch_rings[2] + 99));
        rotors.push_back(rotor_id(rotor_names[3] - 'a', KL7_RING_WIDE));          
        rotors.push_back(rotor_id(rotor_names[4] - 'a', notch_rings[3] + 99));
        rotors.push_back(rotor_id(rotor_names[5] - 'a', notch_rings[4] + 99)); 
        rotors.push_back(rotor_id(rotor_names[6] - 'a', notch_rings[5] + 99));   
        rotors.push_back(rotor_id(rotor_names[7] - 'a', notch_rings[6] + 99));                                            
        
    } while(0);
    
    return result;
}

/* ----------------------------------------------------------- */

nema_configurator::nema_configurator() 
{ 
    war_machine = true; 

    // Notch rings 12, 13, 14, 15, 17 and 18 can be used with war machine
    notch_ring_war.insert(12);
    notch_ring_war.insert(13);
    notch_ring_war.insert(14);
    notch_ring_war.insert(15);
    notch_ring_war.insert(17);
    notch_ring_war.insert(18);

    // Notch rings 16, 19, 20 and 21 can be used with training machine    
    notch_ring_training.insert(16);
    notch_ring_training.insert(19);
    notch_ring_training.insert(20);
    notch_ring_training.insert(21);
}

void nema_configurator::get_keywords(vector<key_word_info>& infos)
{
    infos.clear();
    
   /*
    * Determines which rotors are placed in the machine and in what sequence. Depending on the machine
    * type (war or training) there are four (a-d) or six rotors (a-f) to choose from. Exactly four of them 
    * have to be placed in the machine and therefore a string consisting of four letters has to be specified
    * in this field.   
    */    
    infos.push_back(key_word_info(KW_NEMA_ROTORS, KEY_STRING));    

   /*
    * Determines which notch rings are placed in what sequence in the simulator. The notch rings are designated 
    * by numbers and the rings that are available are different for the war and the training machine. The notch rings
    * used by the training machine were 16, 19, 20, 21 and the notch rings intended for the war machine were 
    * 12, 13, 14, 15, 17, 18. The field has to specify exactly four notch rings, where the four numbers are seperated
    * by a space character.   
    */
    infos.push_back(key_word_info(KW_NEMA_RINGS, KEY_STRING));   
    
   /*
    * Determines whether the simulated machine is of type war (CONF_TRUE) or training (CONF_FALSE)
    */         
    infos.push_back(key_word_info(KW_NEMA_WAR_MACHINE, KEY_BOOL));  
}

/*! Caveat: This method assumes that NEMA_ROTOR_A = 0, NEMA_ROTOR_B = 1, ..., NEMA_ROTOR_F = 5. It also assumes
 *  that NEMA_DRIVE_WHEEL_12, NEMA_DRIVE_WHEEL_13, ..., NEMA_DRIVE_WHEEL_23 are consecutive numbers.
 */    
void nema_configurator::get_config(map<string, string>& config_data, rotor_machine *configured_machine)
{
    nema *machine = dynamic_cast<nema *>(configured_machine);    
    rmsk::simple_assert(machine == NULL, "programmer error: machine is not Nema"); 
    nema_stepper *stepper = dynamic_cast<nema_stepper *>(machine->get_stepping_gear());    
    string help;

    config_data.clear();
    
    // Determine rotor specification
    help += stepper->get_descriptor(CONTACT_8).id.r_id + 'a';
    help += stepper->get_descriptor(CONTACT_6).id.r_id + 'a';
    help += stepper->get_descriptor(CONTACT_4).id.r_id + 'a';
    help += stepper->get_descriptor(CONTACT_2).id.r_id + 'a';            
    
    config_data[KW_NEMA_ROTORS] = help;
    help = "";
    
    // Determine drive wheel specification
    unsigned int translate_val = NEMA_DRIVE_WHEEL_12 - 12;
    help += boost::lexical_cast<string>(stepper->get_descriptor(DRIVE_9).id.ring_id - translate_val);
    help += " " + boost::lexical_cast<string>(stepper->get_descriptor(DRIVE_7).id.ring_id - translate_val);
    help += " " + boost::lexical_cast<string>(stepper->get_descriptor(DRIVE_5).id.ring_id - translate_val);
    help += " " + boost::lexical_cast<string>(stepper->get_descriptor(DRIVE_3).id.ring_id - translate_val);
    config_data[KW_NEMA_RINGS] = help;
    
    // Determine machine type
    if (stepper->get_descriptor(DRIVE_RED_1).id.r_id == NEMA_DRIVE_WHEEL_22)
    {
        config_data[KW_NEMA_WAR_MACHINE] = CONF_TRUE;        
    }
    else
    {
        config_data[KW_NEMA_WAR_MACHINE] = CONF_FALSE;        
    }               
}
    
unsigned int nema_configurator::configure_machine(map<string, string>& config_data, rotor_machine *machine_to_configure)
{
    unsigned int result = CONFIGURATOR_OK;
    // Make sure that we are dealing with a nema object
    nema *machine = dynamic_cast<nema *>(machine_to_configure);
    string old_set_name;
    nema_stepper *stepper;
    
    if (machine == NULL)
    {
        result = CONFIGURATOR_ERROR;
    }
    else
    {
        stepper = dynamic_cast<nema_stepper *>(machine->get_stepping_gear());
        result = parse_config(config_data);
        
        if (result == CONFIGURATOR_OK)
        {
            // Save rotor_set name currently in use in this Nema    
            old_set_name = machine->get_default_set_name();
            // Set rotor_set in use on this Nema to default value specified in this configurator object
            machine->set_default_set_name(rotor_set_name);            
            // Insert red wheel using the rotor_set specified in rotor_set_name           
            machine->prepare_red_wheel(left_red_drive_wheel, right_red_drive_wheel);
            // Restore original rotor_set name
            machine->set_default_set_name(old_set_name); 
            
            // Insert rotor assemblies                        
            machine->prepare_rotor(rotor_set_name.c_str(), rotor_settings[3].contact_rotor_id, CONTACT_2);
            machine->prepare_rotor(rotor_set_name.c_str(), rotor_settings[3].drive_wheel_id, DRIVE_3);

            machine->prepare_rotor(rotor_set_name.c_str(), rotor_settings[2].contact_rotor_id, CONTACT_4);
            machine->prepare_rotor(rotor_set_name.c_str(), rotor_settings[2].drive_wheel_id, DRIVE_5);

            machine->prepare_rotor(rotor_set_name.c_str(), rotor_settings[1].contact_rotor_id, CONTACT_6);
            machine->prepare_rotor(rotor_set_name.c_str(), rotor_settings[1].drive_wheel_id, DRIVE_7);

            machine->prepare_rotor(rotor_set_name.c_str(), rotor_settings[0].contact_rotor_id, CONTACT_8);
            machine->prepare_rotor(rotor_set_name.c_str(), rotor_settings[0].drive_wheel_id, DRIVE_9);
            
            // Insert reflector
            machine->prepare_rotor(rotor_set_name.c_str(), NEMA_UKW, CONTACT_UKW_10);
            
            // Reset all contact rotors and drive wheels to 'A' position
            stepper->reset();
        }
    }
    
    return result;
}
rotor_machine *nema_configurator::make_machine(map<string, string>& config_data)
{
    rotor_machine *result = NULL;
    
    if (parse_config(config_data) == CONFIGURATOR_OK)
    {
        // Throws exception upon failure
        // Beware: machine is constructed using the default rotor set!
        result = new nema(rotor_settings, left_red_drive_wheel, right_red_drive_wheel);
        // Also call configure_machine in order to make sure that the correct rotor_set as specified in rotor_set_name is used.
        // This call does not change the machine's configuration as long as rotor_set_name == DEFAULT_SET.
        // Result casted to void. Call can not fail.
        (void)configure_machine(config_data, result);
    }
    
    return result;
}

/*! Caveat: This method assumes that NEMA_ROTOR_A = 0, NEMA_ROTOR_B = 1, ..., NEMA_ROTOR_F = 5. It also assumes
 *  that NEMA_DRIVE_WHEEL_12, NEMA_DRIVE_WHEEL_13, ..., NEMA_DRIVE_WHEEL_23 are consecutive numbers.
 */
unsigned int nema_configurator::parse_config(map<string, string>& config_data)
{
    unsigned int result = CONFIGURATOR_OK;
    char upper_limit;
    set<unsigned int> *ref_set;

    do
    {
        // Verify that there is a value for all keywords     
        if (!check_for_completeness(config_data))
        {
            result = CONFIGURATOR_INCONSISTENT;
            break;
        }
        
        // Set reference values depending on type (war, training) of machine 
        if ((war_machine = (config_data[KW_NEMA_WAR_MACHINE] == CONF_TRUE)))
        {
            // War machine can use rotors 'a' - 'f'
            upper_limit = NEMA_UPPER_LIMIT_WAR;
            ref_set = &notch_ring_war;
            left_red_drive_wheel = NEMA_DRIVE_WHEEL_22;
            right_red_drive_wheel = NEMA_DRIVE_WHEEL_1;
        }
        else
        {
            // Training machine can use rotors 'a' - 'd'        
            upper_limit = NEMA_UPPER_LIMIT_TRAINING;        
            ref_set = &notch_ring_training; 
            left_red_drive_wheel = NEMA_DRIVE_WHEEL_23;
            right_red_drive_wheel = NEMA_DRIVE_WHEEL_2;                  
        }
        
        // Verify rotor specification
        rotor_names = config_data[KW_NEMA_ROTORS];
        
        if (!check_rotor_spec(rotor_names, 'a', upper_limit, 4))
        {
            result = CONFIGURATOR_INCONSISTENT;
            break;            
        }
        
        // Verify ring/drive wheel specification
        if (!parse_numeric_vector(config_data[KW_NEMA_RINGS], ring_ids))
        {
            result = CONFIGURATOR_INCONSISTENT;
            break;            
        }
        
        if (!check_vector(ring_ids, *ref_set, 4, true))
        {
            result = CONFIGURATOR_INCONSISTENT;
            break;
        }
        
        rotor_settings.clear();
        
        unsigned int translate_val = NEMA_DRIVE_WHEEL_12 - 12;        
        // Create rotor_assembly objects        
        rotor_settings.push_back(rotor_assembly(ring_ids[0] + translate_val, rotor_names[0] - 'a'));
        rotor_settings.push_back(rotor_assembly(ring_ids[1] + translate_val, rotor_names[1] - 'a'));
        rotor_settings.push_back(rotor_assembly(ring_ids[2] + translate_val, rotor_names[2] - 'a'));
        rotor_settings.push_back(rotor_assembly(ring_ids[3] + translate_val, rotor_names[3] - 'a'));                        
    
    } while(0);
    
    return result;
}

