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

/*! \file sigaba.cpp
 *  \brief This file contains the implementation of a simulator for the SIGABA.
 */ 

#include<stdexcept>
#include<boost/scoped_ptr.hpp>

#include<rmsk_globals.h>
#include<alphabet.h>
#include<sigaba.h>
#include<configurator.h>

/*! \brief Position of character O when rotor is *not* inserted in reverse.
 */
const unsigned int ZERO_POS = 14;

/*! \brief Position of character O when rotor is inserted in reverse.
 */
const unsigned int ZERO_POS_INVERSE = 12;

/*! \brief Used to model the contacts of the driver rotors in CSP 2900 which are not
 *         connected to the index rotors. 
 */
const unsigned int N = 1000;

/*! \brief The element alphabet used to visualize the rotor position of the index rotors.
 */
alphabet<char> index_alphabet("0123456789", 10);

/*! \brief Holds the default rotor_set for driver and cipher rotors.
 */
rotor_set sigaba_rotor_factory::normal_set(rmsk::std_alpha()->get_size());

/*! \brief Holds the default rotor_set for index rotors.
 */
rotor_set sigaba_rotor_factory::index_set(10);

/*! \brief Specifies how the 26 output contacts of the CSP 889 driver rotors are wired to the 10 input contacts 
 *         of the index machine.
 */
unsigned int csp_889_mapping[] =  {9, 1, 2, 3, 3, 4, 4, 4, 5, 5, 5, 6, 6, 6, 6, 7, 7, 7, 7, 7, 8, 8, 8, 8, 8, 8};

/*! \brief Specifies how the 26 output contacts of the CSP 2900 driver rotors are wired to the 10 input contacts 
 *         of the index machine. The three contacts that are mapped to the vlaue N are not connected to any index
 *         rotor.
 */
unsigned int csp_2900_mapping[] = {9, 1, 2, 3, 3, 4, 4, 4, 5, 5, 5, 6, 6, 6, 6, N, N, N, 7, 7, 0, 0, 8, 8, 8, 8};

/*! \brief Output characters when doing decryptions and input characters for encryptions.
 */
ustring str_plain_chars =  "abcdefghijklmnopqrstuvwxy ";

/*! \brief Output characters when doing encryptions and input characters when doing decryptions.
 */
ustring str_cipher_chars = "abcdefghijklmnopqrstuvwxyz";

/* ----------------------------------------------------------- */

rotor_set& sigaba_rotor_factory::get_cipher_rotor_set()
{    
    if (normal_set.get_num_rotors() == 0)
    {
        vector<unsigned int> ring_data(rmsk::std_alpha()->get_size(), 0);
        
        // Permutations for cipher and driver rotors
        normal_set.add_rotor_and_ring(SIGABA_ROTOR_0, rmsk::std_alpha()->to_vector(string("ychlqsugbdixnzkerpvjtawfom")), ring_data);
        normal_set.add_rotor_and_ring(SIGABA_ROTOR_1, rmsk::std_alpha()->to_vector(string("inpxbwetguysaochvldmqkzjfr")), ring_data);
        normal_set.add_rotor_and_ring(SIGABA_ROTOR_2, rmsk::std_alpha()->to_vector(string("wndriozptaxhfjyqbmsvekucgl")), ring_data);
        normal_set.add_rotor_and_ring(SIGABA_ROTOR_3, rmsk::std_alpha()->to_vector(string("tzghobkrvuxlqdmpnfwcjyeias")), ring_data);
        normal_set.add_rotor_and_ring(SIGABA_ROTOR_4, rmsk::std_alpha()->to_vector(string("ywtahrqjvlcexungbipzmsdfok")), ring_data);
        normal_set.add_rotor_and_ring(SIGABA_ROTOR_5, rmsk::std_alpha()->to_vector(string("qslrbtekogaicfwyvmhjnxzudp")), ring_data);
        normal_set.add_rotor_and_ring(SIGABA_ROTOR_6, rmsk::std_alpha()->to_vector(string("chjdqignbsakvtuoxfwleprmzy")), ring_data);
        normal_set.add_rotor_and_ring(SIGABA_ROTOR_7, rmsk::std_alpha()->to_vector(string("cdfajxtimnbeqhsugrylwzkvpo")), ring_data);
        normal_set.add_rotor_and_ring(SIGABA_ROTOR_8, rmsk::std_alpha()->to_vector(string("xhfeszdnrbcgkqijltvmuoyapw")), ring_data);
        normal_set.add_rotor_and_ring(SIGABA_ROTOR_9, rmsk::std_alpha()->to_vector(string("ezjqxmogytcsfriupvnadlhwbk")), ring_data);        
    }

    return normal_set;
}

rotor_set& sigaba_rotor_factory::get_index_rotor_set()
{    
    if (index_set.get_num_rotors() == 0)
    {
        // Permutations for index rotors
        index_set.add_rotor(SIGABA_INDEX_0, index_alphabet.to_vector(string("7591482630")));
        index_set.add_rotor(SIGABA_INDEX_1, index_alphabet.to_vector(string("3810592764")));
        index_set.add_rotor(SIGABA_INDEX_2, index_alphabet.to_vector(string("4086153297")));
        index_set.add_rotor(SIGABA_INDEX_3, index_alphabet.to_vector(string("3980526174")));
        index_set.add_rotor(SIGABA_INDEX_4, index_alphabet.to_vector(string("6497135280")));        
    }

    return index_set;
}

/* ----------------------------------------------------------- */

/*! Visualizing the rotor windows is a bit more difficult for the SIGABA than for other machines. The reason
 *  for this is that when SIGABA rotors are inserted in reverse the inscription of the letters on their
 *  circumfence effectively "changes". This is best explained by an example. The normal stepping order of
 *  a cipher or driver rotor that is inserted in the normal way is AZYXWV... . When the same rotor is inserted
 *  in reverse the stepping sequence becomes ABCDEFGH... where are all the letters are upside down.
 */
ustring sigaba_base_machine::visualize_sigaba_rotor_pos(string& rotor_identifier, alphabet<char>& alpha)
{
    ustring result;
    
    if (get_stepping_gear()->get_descriptor(rotor_identifier).id.insert_inverse)
    {
        simple_mod_int help(get_stepping_gear()->get_descriptor(rotor_identifier).ring->get_pos(), alpha.get_size());
        
        // When inserted in reverse the character shown in the rotor window can be determined by mapping the
        // additve inverse modulo alpha.get_size() of the current position to a letter of the alphabet referenced
        // by parameter alpha.
        result += alpha.to_val(-help);
    }
    else
    {
        result += alpha.to_val(get_stepping_gear()->get_descriptor(rotor_identifier).ring->get_pos());
    }
    
    return result;       
}

/* ----------------------------------------------------------- */

sigaba_index_machine::sigaba_index_machine(rotor_id null_id, rotor_id one_id, rotor_id two_id, rotor_id three_id, rotor_id four_id)
{
    add_rotor_set(DEFAULT_SET, sigaba_rotor_factory::get_index_rotor_set());
    vector<string> rotor_names;
    boost::shared_ptr<rotor> help;
    
    stepper = NULL;
    
    machine_name = MNAME_SIGABA;        

    // Set up names of rotor slots    
    rotor_names.push_back(I_ZERO);
    rotor_names.push_back(I_ONE);
    rotor_names.push_back(I_TWO);
    rotor_names.push_back(I_THREE);
    rotor_names.push_back(I_FOUR);
    
    set_stepping_gear(new stepping_gear(rotor_names, index_alphabet.get_size()));

    // Insert rotors into machine
    prepare_rotor(null_id, I_ZERO);
    prepare_rotor(one_id, I_ONE);
    prepare_rotor(two_id, I_TWO);
    prepare_rotor(three_id, I_THREE);
    prepare_rotor(four_id, I_FOUR);
    
    reset();
}

ustring sigaba_index_machine::visualize_rotor_pos(string& rotor_identifier)
{
    return visualize_sigaba_rotor_pos(rotor_identifier, index_alphabet);
}

void sigaba_index_machine::reset()
{
    // Set all rotors to position 0
    get_stepping_gear()->set_rotor_displacement(I_ZERO, 0);
    get_stepping_gear()->set_rotor_displacement(I_ONE, 0);    
    get_stepping_gear()->set_rotor_displacement(I_TWO, 0);
    get_stepping_gear()->set_rotor_displacement(I_THREE, 0);    
    get_stepping_gear()->set_rotor_displacement(I_FOUR, 0);    
    get_stepping_gear()->reset();
}

/* ----------------------------------------------------------- */

sigaba_driver::sigaba_driver(rotor_id stat_l_id, rotor_id slow_id, rotor_id fast_id, rotor_id middle_id, rotor_id stat_r_id)
    : sigaba_base_machine()
{
    add_rotor_set(DEFAULT_SET, sigaba_rotor_factory::get_cipher_rotor_set());
    vector<string> rotor_names;
    
    stepper = NULL;    
    machine_name = MNAME_SIGABA;
    
    // Set up names of rotor slots
    rotor_names.push_back(STATOR_L);
    rotor_names.push_back(S_SLOW);
    rotor_names.push_back(S_FAST);
    rotor_names.push_back(S_MIDDLE);
    rotor_names.push_back(STATOR_R);
    
    set_stepping_gear(new sigaba_driver_stepper(rotor_names));

    // Insert rotors into machine
    prepare_rotor(stat_r_id, STATOR_R);
    prepare_rotor(middle_id, S_MIDDLE);    
    prepare_rotor(fast_id, S_FAST);    
    prepare_rotor(slow_id, S_SLOW);
    prepare_rotor(stat_l_id, STATOR_L);    
        
    get_stepping_gear()->reset();    
}

ustring sigaba_driver::visualize_rotor_pos(string& rotor_identifier)
{
    return visualize_sigaba_rotor_pos(rotor_identifier, *rmsk::std_alpha());
}

/* ----------------------------------------------------------- */

void sigaba_driver_stepper::reset()
{
    unsigned int count = 0;
    
    stepping_gear::reset();
    
    for (count = 0; count < num_rotors; count++)
    {
        // The number of the contact that is identified by the letter O depends on whether
        // the rotor is inserted normal or in reverse
        if (!get_descriptor(count).id.insert_inverse)
        {
            set_rotor_displacement(count, ZERO_POS);        
        }
        else
        {
            set_rotor_displacement(count, ZERO_POS_INVERSE);        
        }
        
        rotors[rotor_positions[count]].ring->set_offset(0);
    }
}

void sigaba_driver_stepper::step_rotors()
{
    bool middle_steps, slow_steps;
    
    // Do not move rotors when currently uncoupled
    if (!uncouple_stepper)
    {
        stepping_gear::step_rotors();
        
        // The number of the contact that is identified by the letter O depends on whether
        // the rotor is inserted normal or in reverse       
         
        // Middle rotor steps if fast rotor is on position O
        if (get_descriptor(S_FAST).id.insert_inverse)
        {
            middle_steps = (get_ring_pos(S_FAST) == ZERO_POS_INVERSE);
        }
        else
        {
            middle_steps = (get_ring_pos(S_FAST) == ZERO_POS);        
        }        

        // Slow rotor steps if the middle rotor steps and the middle rotor is on position O
        if (get_descriptor(S_MIDDLE).id.insert_inverse)
        {
            slow_steps = middle_steps && (get_ring_pos(S_MIDDLE) == ZERO_POS_INVERSE);
        }
        else
        {
            slow_steps = middle_steps && (get_ring_pos(S_MIDDLE) == ZERO_POS);        
        }
        
        // The fast rotor always steps
        step_rotor_back(S_FAST);
        
        // Move the remaining rotors
        if (middle_steps)
        {
            step_rotor_back(S_MIDDLE);
        }
        
        if (slow_steps)
        {
            step_rotor_back(S_SLOW);
        }
    }
}

/* ----------------------------------------------------------- */

sigaba_stepper::sigaba_stepper(vector<string>& rotor_identifiers, bool csp_2900_flag)
    : stepping_gear(rotor_identifiers, rmsk::std_alpha()->get_size())
{
    driver = NULL;
    index = NULL;

    prepare_machine_type(csp_2900_flag);    
}

void sigaba_stepper::prepare_machine_type(bool csp_2900_flag)
{
    backstepping_rotors.clear();

    if (!csp_2900_flag)
    {
        // CSP 889
    
        // Four contacts of the driver machine are energized to produce the stepping
        // information for the cipher rotors
        energized_contacts = "fghi";
        contact_mapping = csp_889_mapping;
    }
    else
    {
        // CSP 2900
    
        // Six contacts of the driver machine are energized to produce the stepping
        // information for the cipher rotors
        energized_contacts = "defghi";
        contact_mapping = csp_2900_mapping;
        // The cipher rotors are numbered from left to right. I.e. in CSP 2900
        // the rotors that are next to the leftmost and rightmost rotors step 
        // in the opposite direction
        backstepping_rotors.insert(1);
        backstepping_rotors.insert(3);    
    }
    
    is_csp_2900 = csp_2900_flag;
}

unsigned int sigaba_stepper::produce_control_output(unsigned int in_char)
{
    unsigned int result = 0, temp;
    
    // Current passes through the driver machine from right to left instead from left to right.
    // Therefore we have to use the decrypt method of the driver machine
    temp = driver->get_stepping_gear()->get_stack().decrypt(in_char);
    // Map driver output to index inputs
    result = contact_mapping[temp];
    
    // Test if any output contact of driver machine was energized
    if (result != N)
    {
        // Pass current through index machine
        result = index->encrypt(result);
    }
    
    return result;
}

void sigaba_stepper::step_rotors()
{
    // Contains the rotor numbers of the rotors which will step in this cycle
    set<unsigned int> rotors_who_step;
    set<unsigned int>::iterator set_iter;
    unsigned int temp;
    // Mapping that describes which output contacts of the index rotors are used to determine
    // the stepping motion. I.e. if the index machine output contacts 0 or 9 are energized 
    // cipher rotor 0 steps, if the contacts 1 or 2 are energized rotor 4 steps and so on.
    unsigned int ind_mapping[] = {0, 4, 4, 3, 3, 2, 2, 1, 1, 0};

    // Simulate that the input contacts of the driver rotors which are specified by the instance variable
    // energized_contacts are energized
    for (unsigned int count = 0; count < energized_contacts.length(); count++)
    {
        // Let current pass through the driver and index rotors
        temp = produce_control_output(rmsk::std_alpha()->from_val(energized_contacts[count]));
        
        // The value N means that no output contact of the index rotors is energized
        if (temp != N)
        {
            // An output contact of the index rotors is energized. Determine which cipher rotor
            // has to be stepped
            rotors_who_step.insert(ind_mapping[temp]);
        }
    }
    
    // Now the rotors of the driver machine are stepped
    driver->step_rotors();
    
    // Move the cipher rotors
    for (set_iter = rotors_who_step.begin(); set_iter != rotors_who_step.end(); ++set_iter)
    {
        // In CSP 2900 some cipher rotors move in the opposite direction
        if (backstepping_rotors.count(*set_iter) == 0)
        {
            step_rotor_back(rotor_positions[*set_iter]);
        }
        else
        {
            advance_rotor(rotor_positions[*set_iter]);
        }
    }
}

void sigaba_stepper::setup_step(string& rotor_name)
{
    sigaba_driver_stepper *dr_step = dynamic_cast<sigaba_driver_stepper *>(driver->get_stepping_gear());    
    bool old_couple_state = dr_step->get_uncouple_state();
    
    // Uncouple driver stepper
    dr_step->set_uncouple_state(true);
    
    // Advance given rotor of driver machine
    dr_step->step_rotor_back(rotor_name);
    // Step cipher rotors
    step_rotors();
    
    // Recouple driver stepper
    dr_step->set_uncouple_state(old_couple_state);
}

void sigaba_stepper::setup_step(string& rotor_name, unsigned int num_steps)
{
    for (unsigned int count = 0; count < num_steps; count++)
    {
        setup_step(rotor_name);
    }
}

void sigaba_stepper::reset()
{
    unsigned int count = 0;
    
    stepping_gear::reset();

    // Reset positions of driver rotors    
    if (driver != NULL)
    {
        driver->get_stepping_gear()->reset();
    }
    
    // Reset positions of index rotors    
    if (index != NULL)
    {
        index->reset();
    }
    
    // The number of the contact that is identified by the letter O depends on whether
    // the rotor is inserted normal or in reverse          
    
    // Reset positions of cipher rotors            
    for (count = 0; count < num_rotors; count++)
    {
        if (!get_descriptor(count).id.insert_inverse)
        {
            set_rotor_displacement(count, ZERO_POS);        
        }
        else
        {
            set_rotor_displacement(count, ZERO_POS_INVERSE);        
        }
              
        rotors[rotor_positions[count]].ring->set_offset(0);
    }

}

/* ----------------------------------------------------------- */

void sigaba::save_additional_components(Glib::KeyFile& ini_file)
{
    sigaba_index_machine *index = get_sigaba_stepper()->get_index_bank();
    sigaba_driver *driver = get_sigaba_stepper()->get_driver_machine();    

    ini_file.set_boolean("stepper", "is_csp_2900", get_sigaba_stepper()->is_2900());
    index->save_ini(ini_file);
    driver->save_ini(ini_file);
}

bool sigaba::load_additional_components(Glib::KeyFile& ini_file)
{
    bool result = false, temp_csp_2900_flag;
    sigaba_index_machine *index = get_sigaba_stepper()->get_index_bank();
    sigaba_driver *driver = get_sigaba_stepper()->get_driver_machine();    
    
    do
    {
        // Retrieve operating mode
        if ((result = !ini_file.has_key("stepper", "is_csp_2900")))
        {
            break;
        }
        
        temp_csp_2900_flag = ini_file.get_boolean("stepper", "is_csp_2900");
        get_sigaba_stepper()->prepare_machine_type(temp_csp_2900_flag);
        
        // Restore state of index machine
        if ((result = index->load_ini(ini_file)))
        {
            break;            
        }
        
        // Restore state of driver machine        
        if ((result = driver->load_ini(ini_file)))
        {
            break;            
        }                
    
    } while(0);
        
    return result;
}

void sigaba::set_default_set_name(string default_set_name)
{
    rotor_machine::set_default_set_name(default_set_name);
    get_sigaba_stepper()->get_driver_machine()->set_default_set_name(default_set_name);
    get_sigaba_stepper()->get_index_bank()->set_default_set_name(default_set_name);
}

ustring sigaba::visualize_rotor_pos(string& rotor_identifier)
{
    return visualize_sigaba_rotor_pos(rotor_identifier, *rmsk::std_alpha());
}

ustring sigaba::visualize_all_positions()
{
    ustring result;
    string temp;
    
    temp = get_sigaba_stepper()->get_index_bank()->visualize_all_positions();
    reverse(temp.begin(), temp.end());
    result = temp;

    temp = get_sigaba_stepper()->get_driver_machine()->visualize_all_positions();
    reverse(temp.begin(), temp.end());
    result += temp;
    
    temp = rotor_machine::visualize_all_positions();    
    reverse(temp.begin(), temp.end());
    result += temp;
    
    return result;
}

string sigaba::get_description()
{
    string result;

    if (get_sigaba_stepper()->is_2900())
    {
        result = "CSP2900";
    }
    else
    {
        result = "CSP889";    
    }
    
    return result;
}

bool sigaba::randomize(string& param)
{
    bool result = false;
    random_bit_source reverse_rotors(15);
    urandom_generator rand;    
    map<string, string> machine_conf;
    boost::scoped_ptr<configurator> c(configurator_factory::get_configurator(machine_name));    
    string cipher_rotors, control_rotors, index_rotors;
    vector<unsigned int> cipher_displacements, control_displacements, index_displacements;
    
    try
    {
        permutation rotor_selection_perm = permutation::get_random_permutation(rand, 10);
        permutation index_selection_perm = permutation::get_random_permutation(rand, 5);            
            
        for(unsigned int count = 0; count < 5; count++)
        {
            cipher_rotors += '0' + (char)(rotor_selection_perm.encrypt(count));
            cipher_rotors += ((reverse_rotors.get_next_val() == 0) ? 'N' : 'R');
        }

        for(unsigned int count = 5; count < 10; count++)
        {
            control_rotors += '0' + (char)(rotor_selection_perm.encrypt(count));
            control_rotors += ((reverse_rotors.get_next_val() == 0) ? 'N' : 'R');
        }

        for(unsigned int count = 0; count < 5; count++)
        {
            index_rotors += '0' + (char)(index_selection_perm.encrypt(count));
            index_rotors += ((reverse_rotors.get_next_val() == 0) ? 'N' : 'R');
        }
        
        machine_conf[KW_CIPHER_ROTORS] = cipher_rotors;
        machine_conf[KW_CONTROL_ROTORS] = control_rotors;
        machine_conf[KW_INDEX_ROTORS] = index_rotors;
        machine_conf[KW_CSP_2900_FLAG] = (get_sigaba_stepper()->is_2900() ? CONF_TRUE : CONF_FALSE);

        c->configure_machine(machine_conf, this); 
        
        cipher_displacements = rmsk::std_alpha()->to_vector(rmsk::std_alpha()->get_random_string(5));
        control_displacements = rmsk::std_alpha()->to_vector(rmsk::std_alpha()->get_random_string(5));        
        index_displacements = index_alphabet.to_vector(index_alphabet.get_random_string(5));
        
        get_sigaba_stepper()->set_all_displacements(cipher_displacements);
        get_sigaba_stepper()->get_driver_machine()->get_stepping_gear()->set_all_displacements(control_displacements);
        get_sigaba_stepper()->get_index_bank()->get_stepping_gear()->set_all_displacements(index_displacements);
    }
    catch(...)
    {
        result = true;
    }    
    
    return result;
}


sigaba::sigaba(vector<rotor_id>& r_ids, bool csp_2900_flag)
    : sigaba_base_machine()
{
    add_rotor_set(DEFAULT_SET, sigaba_rotor_factory::get_cipher_rotor_set());
    // create index machine
    sigaba_index_machine *ind = new sigaba_index_machine(r_ids[10], r_ids[11], r_ids[12], r_ids[13], r_ids[14]);
    // create driver machine
    sigaba_driver *dri = new sigaba_driver(r_ids[5], r_ids[6], r_ids[7], r_ids[8], r_ids[9]);
    pair<boost::shared_ptr<rotor>, boost::shared_ptr<rotor_ring> > help;
    vector<string> rotor_names;
    vector<gunichar> plain_alph, cipher_alph;
    
    // Set up names of rotor slots for cipher rotors
    rotor_names.push_back(R_ZERO);
    rotor_names.push_back(R_ONE);
    rotor_names.push_back(R_TWO);
    rotor_names.push_back(R_THREE);
    rotor_names.push_back(R_FOUR);
    
    machine_name = MNAME_SIGABA;

    // Set up stepping mechanism
    sigaba_stepper *s = new sigaba_stepper(rotor_names, csp_2900_flag);
    s->set_index_bank(ind);
    s->set_driver_machine(dri); 
    
    set_stepping_gear(s);

    // Set up cipher and plaintext alphabets
    printing_device::ustr_to_vec(str_plain_chars, plain_alph);
    printing_device::ustr_to_vec(str_cipher_chars, cipher_alph);        
    boost::shared_ptr<alphabet<gunichar> > plain_alpha(new alphabet<gunichar>(plain_alph));
    boost::shared_ptr<alphabet<gunichar> > cipher_alpha(new alphabet<gunichar>(cipher_alph));
    
    // Set up printing_device
    asymmetric_printing_device *sigaba_printer = new asymmetric_printing_device();
    
    sigaba_printer->set_plain_alphabet(plain_alpha); 
    sigaba_printer->set_cipher_alphabet(cipher_alpha);
    
    set_printer(boost::shared_ptr<printing_device>(sigaba_printer));    
    
    // Set up keyboard
    boost::shared_ptr<asymmetric_keyboard> sigaba_keyboard(new asymmetric_keyboard());
    sigaba_keyboard->set_plain_alphabet(plain_alpha);
    sigaba_keyboard->set_cipher_alphabet(cipher_alpha);
    
    set_keyboard(sigaba_keyboard);

    // Insert cipher rotors
    prepare_rotor(r_ids[0], R_ZERO);
    prepare_rotor(r_ids[1], R_ONE);
    prepare_rotor(r_ids[2], R_TWO);
    prepare_rotor(r_ids[3], R_THREE);
    prepare_rotor(r_ids[4], R_FOUR);            
    
    get_stepping_gear()->reset();        
}

