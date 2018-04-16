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

/*! \file nema.cpp
 *  \brief This file contains the implmentation of a simulator for the Nema machine.
 */ 

#include<stdexcept>
#include<boost/scoped_ptr.hpp>
#include<boost/lexical_cast.hpp>
#include<rmsk_globals.h>
#include<nema.h>
#include<configurator.h>

#define PERM_ID  "ijklmnopqrstuvwxyzabcdefgh"
#define ETW         "etw"

/*! \brief Nema alphabet which puts the letter 'i' on position/contact zero.
 */
alphabet<char> nema_alpha(PERM_ID, 26);

/*! \brief Holds the rotor_set used by nema_rotor_factory.
 */
rotor_set nema_rotor_factory::nema_set_data(nema_alpha.get_size());

rotor_set *nema_rotor_factory::nema_set = &nema_rotor_factory::nema_set_data;

/*! \brief Offset from position 0, where the notches of the drive wheels and the left side of the
 *         red wheel are sensed.
 */
const unsigned int NOTCH_SENSE_OFFSET_LEFT = 16;

/*! \brief Offset from position 0, where the notches on the right side of the red wheel are sensed
 */
const unsigned int NOTCH_SENSE_OFFSET_RIGHT = 17;

/*! \brief Even though the letter 'i' is mapped to contact zero, the letter 'a' is still indicates
 *         the "neutral" position.
 */
const unsigned int ROTOR_START_POS = nema_alpha.from_val('a');

/*! \brief The rotor position is not determined by looking at the topmost row of characters on the rotor bank.
 *         The benchmark for reading the rotor positions points to the row of characters, which is two characters
 *         below the topmost row.
 */
const unsigned int RING_OFFSET = 2;

/* ----------------------------------------------------------- */
// Implement rotor factory

vector<unsigned int> nema_rotor_factory::create_ring_data(const char *notch_positions)
{
    string temp(notch_positions);
    vector<unsigned int> result(nema_alpha.get_size(), 0);    

    for (unsigned int count = 0; count < temp.length(); count++)
    {
        result[nema_alpha.from_val(temp[count])] = 1;
    }
    
    return result;
}

void nema_rotor_factory::create_ring_data_double(vector<unsigned int>& ring_data, vector<unsigned int>& data_left, vector<unsigned int>& data_right)
{       
    for (unsigned int count = 0; count < data_left.size(); count++)
    {
        // combine ring data
        ring_data[count] = (data_left[count] << 1) | (data_right[count]);
    }    
}

alphabet<char> *nema_rotor_factory::get_nema_alpha()
{
    return &nema_alpha;
}

rotor_set *nema_rotor_factory::get_rotor_set()
{    
    if (nema_set->get_num_rotors() == 0)
    {
        // rotors a.k.a. contact wheels        
        nema_set->add_rotor(NEMA_ROTOR_A, nema_alpha.to_vector(string("mvwaujdrlzxhfqegpbnskitcoy")));
        nema_set->add_rotor(NEMA_ROTOR_B, nema_alpha.to_vector(string("lozqbwpsxirfagudvckjyntmeh")));
        nema_set->add_rotor(NEMA_ROTOR_C, nema_alpha.to_vector(string("zyahvrwojsgbqmkcxlpnfutedi")));
        nema_set->add_rotor(NEMA_ROTOR_D, nema_alpha.to_vector(string("mdtazkxpiqhsvlgonrwjbeyfuc")));
        nema_set->add_rotor(NEMA_ROTOR_E, nema_alpha.to_vector(string("nacfujeswlzigdpokbhrqtyvxm")));
        nema_set->add_rotor(NEMA_ROTOR_F, nema_alpha.to_vector(string("wlnshpoafyixkbdrjczvgeqmut")));
        nema_set->add_rotor(NEMA_UKW,     nema_alpha.to_vector(string("vtazudycxsrjmibqolkwpnghef")));
        nema_set->add_rotor(NEMA_ETW,     nema_alpha.to_vector(string("qmnbvcxylkjhgfdsapoiuztrew")));

        // drive wheels
        nema_set->add_ring(NEMA_DRIVE_WHEEL_1, create_ring_data("bjsyz"));
        nema_set->add_ring(NEMA_DRIVE_WHEEL_2, create_ring_data("bdehq"));
        nema_set->add_ring(NEMA_DRIVE_WHEEL_12, create_ring_data("bcdefghijklpqrsuvwxyz"));
        nema_set->add_ring(NEMA_DRIVE_WHEEL_13, create_ring_data("abdefgjkmnpqrtuvwxy"));
        nema_set->add_ring(NEMA_DRIVE_WHEEL_14, create_ring_data("cefghjklmnopqsvxz"));
        nema_set->add_ring(NEMA_DRIVE_WHEEL_15, create_ring_data("adegmopqrstvxyz"));
        nema_set->add_ring(NEMA_DRIVE_WHEEL_16, create_ring_data("abcdefhijklmnoqrstuvwxy"));
        nema_set->add_ring(NEMA_DRIVE_WHEEL_17, create_ring_data("bhijkqsuvxy"));
        nema_set->add_ring(NEMA_DRIVE_WHEEL_18, create_ring_data("abcdefghijklmoqrstuvwyz"));
        nema_set->add_ring(NEMA_DRIVE_WHEEL_19, create_ring_data("abcefghlmnopqrstuvxyz"));
        nema_set->add_ring(NEMA_DRIVE_WHEEL_20, create_ring_data("abcdefhijlnprtuwxyz"));
        nema_set->add_ring(NEMA_DRIVE_WHEEL_21, create_ring_data("acdeghijlmnoqrsux"));
        nema_set->add_ring(NEMA_DRIVE_WHEEL_22, create_ring_data("abeghkmnpqrsvwx"));
        nema_set->add_ring(NEMA_DRIVE_WHEEL_23, create_ring_data("acdefghijklnopqrstuvwxy"));
        
        set<unsigned int> nema_const = {NEMA_ETW, NEMA_DRIVE_WHEEL_1, NEMA_DRIVE_WHEEL_2, NEMA_DRIVE_WHEEL_12, NEMA_DRIVE_WHEEL_13, NEMA_DRIVE_WHEEL_14, NEMA_DRIVE_WHEEL_15,
                                        NEMA_DRIVE_WHEEL_16, NEMA_DRIVE_WHEEL_17, NEMA_DRIVE_WHEEL_18, NEMA_DRIVE_WHEEL_19, NEMA_DRIVE_WHEEL_20, NEMA_DRIVE_WHEEL_21,
                                        NEMA_DRIVE_WHEEL_22, NEMA_DRIVE_WHEEL_23};
                                        
        nema_set->set_const_ids(nema_const);
    }
    
    return nema_set;
}

/* ----------------------------------------------------------- */
// Implement stepping_gear

void nema_stepper::reset()
{
    unsigned int count = 0;
    
    stepping_gear::reset();
    
    set_rotor_displacement(ETW, 0);
    get_descriptor(ETW).ring->set_offset(0);
    
    for (count = 1; count < num_rotors; count++)
    {
        // Set rotor/drive wheel position to 'a'
        get_descriptor(count).ring->set_offset(RING_OFFSET);
        // Compensate for benchmark position that indicates where the rotor position is to be read
        get_descriptor(count).ring->set_pos(ROTOR_START_POS);
    }
}

void nema_stepper::red_notches(bool& left_notch, bool& right_notch)
{
    unsigned int raw_data_left = get_descriptor(DRIVE_RED_1).ring->get_current_data(NOTCH_SENSE_OFFSET_LEFT);
    unsigned int raw_data_right = get_descriptor(DRIVE_RED_1).ring->get_current_data(NOTCH_SENSE_OFFSET_RIGHT);
    
    // Separate previously combined ring data
    left_notch = raw_data_left >> 1;
    right_notch = raw_data_right & 1;
}

void nema_stepper::step_rotors()
{
    stepping_gear::step_rotors();
    bool red_notch_left, red_notch_right;
    map<string, bool> step_info;
    map<string, bool>::iterator step_info_iter;
    
    red_notches(red_notch_left, red_notch_right);
    
    // Determine which of the rotors/drive wheels are to step
    step_info[CONTACT_2] = red_notch_left;    
    step_info[DRIVE_3] = red_notch_right;
    step_info[CONTACT_4] = red_notch_right && rotor_is_at_notch(DRIVE_3, NOTCH_SENSE_OFFSET_LEFT);
    step_info[CONTACT_6] = rotor_is_at_notch(DRIVE_5, NOTCH_SENSE_OFFSET_LEFT);
    step_info[DRIVE_7] = red_notch_right;
    step_info[CONTACT_8] = red_notch_right && rotor_is_at_notch(DRIVE_7, NOTCH_SENSE_OFFSET_LEFT);
    step_info[CONTACT_UKW_10] = rotor_is_at_notch(DRIVE_9, NOTCH_SENSE_OFFSET_LEFT);
    
    // DRIVE_RED_1, DRIVE_5 and DRIVE_9 always step    
    step_rotor_back(DRIVE_RED_1);
    step_rotor_back(DRIVE_5);
    step_rotor_back(DRIVE_9);
    
    // Step the remaining rotors according to contents of step_info
    for (step_info_iter = step_info.begin(); step_info_iter != step_info.end(); ++step_info_iter)
    {
        if (step_info_iter->second)
        {
            string help = step_info_iter->first;
            step_rotor_back(help);
        }
    }
}

void nema_stepper::set_rotor_to_pos(string& identifier, char pos)
{
    rmsk::simple_assert(rotors.count(identifier) == 0, "Unknown NEMA rotor/drive wheel");
    
    get_descriptor(identifier).ring->set_pos(nema_alpha.from_val(pos));        
}

void nema_stepper::set_rotor_to_pos(unsigned int rotor_num, char pos)
{
    rmsk::simple_assert(rotor_num >= num_rotors, "Wrong rotor number");
    
    set_rotor_to_pos(rotor_positions[rotor_num], pos);
}

char nema_stepper::get_rotor_pos(string& identifier)
{
    rmsk::simple_assert(rotors.count(identifier) == 0, "Unknown NEMA rotor/drive wheel");
    
    return nema_alpha.to_val(get_descriptor(identifier).ring->get_pos());
}

string nema_stepper::get_all_positions()
{
    string result;    
    
    // Read rotor positions from left to right as seen by the user of the machine
    for (unsigned int count = 10; count > 0; count--)
    {
        result += get_rotor_pos(rotor_positions[count]);
    }
    
    return result;
}

void nema_stepper::set_all_positions(string& new_positions)
{
    rmsk::simple_assert(new_positions.length() != 10, "Illegal number of rotor positions");
    
    for (unsigned int count = 0; count < 10 ; count++)
    {
        set_rotor_to_pos(10 - count, new_positions[count]);
    }
}

/* ----------------------------------------------------------- */
// Implement machine itself

ustring nema::visualize_rotor_pos(string& rotor_identifier)
{
    ustring result;
    
    result += get_nema_stepper()->get_rotor_pos(rotor_identifier);
    
    return result;
}

vector<ustring> nema::visualize_active_permutations()
{
    vector<unsigned int> positions_to_visualize = {0, 2, 4, 6, 8, 10};
    
    return rotor_perm_visualizer_help(positions_to_visualize, nema_alpha);
}


bool nema::move_all_rotors(ustring& new_positions)
{
    bool result = false;
    string pos_help;
    
    result = (new_positions.length() != 10);
    
    for (unsigned int count = 0; (count < new_positions.length()) && (!result); count++)
    {
        result = !rmsk::std_uni_alpha()->contains_symbol(new_positions[count]);
        
        if (!result)
        {
            pos_help += rmsk::std_alpha()->to_val(rmsk::std_uni_alpha()->from_val(new_positions[count]));
        }
    }
    
    if (!result)
    {
        get_nema_stepper()->set_all_positions(pos_help);
    }
    
    return result;
}

pair<boost::shared_ptr<rotor>, boost::shared_ptr<rotor_ring> > nema::make_red_wheel(unsigned int rotor_id_l, unsigned int rotor_id_r)
{
    // Construct selected rings
    pair<boost::shared_ptr<rotor>, boost::shared_ptr<rotor_ring> > driver_left = make_rotor(rotor_id_l);
    pair<boost::shared_ptr<rotor>, boost::shared_ptr<rotor_ring> > driver_right = make_rotor(rotor_id_r);
    
    vector<unsigned int> rotor_data_left(nema_alpha.get_size(), 0);
    vector<unsigned int> rotor_data_right(nema_alpha.get_size(), 0);
    vector<unsigned int> rotor_data(nema_alpha.get_size(), 0);    
    
    // retrieve ring data of the two chosen rings 
    driver_left.second->get_ring_data(rotor_data_left);
    driver_right.second->get_ring_data(rotor_data_right);
    // Combine ring data of the two rings
    nema_rotor_factory::create_ring_data_double(rotor_data, rotor_data_left, rotor_data_right);
    
    // Assign combined ring data to newly created ring
    driver_left.second->set_ring_data(rotor_data);

    return driver_left;
}

void nema::prepare_red_wheel(unsigned int left_red_drive_wheel, unsigned int right_red_drive_wheel)
{
    pair<boost::shared_ptr<rotor>, boost::shared_ptr<rotor_ring> > help;
    nema_stepper *s = dynamic_cast<nema_stepper *>(get_stepping_gear());
    
    help = make_red_wheel(left_red_drive_wheel, right_red_drive_wheel);
    
    // Insert the newly constructed red wheel into the stepping gear    
    s->insert_rotor_and_ring(DRIVE_RED_1, help);
    s->get_descriptor(DRIVE_RED_1).id.r_id = left_red_drive_wheel;
    s->get_descriptor(DRIVE_RED_1).id.ring_id = left_red_drive_wheel;    
    s->get_descriptor(DRIVE_RED_1).id.insert_inverse = false;    
}

bool nema::randomize(string& param)
{
    bool result = false;
    urandom_generator rand;    
    map<string, string> machine_conf;
    boost::scoped_ptr<configurator> c(configurator_factory::get_configurator(machine_name));    
    bool is_war_machine = get_nema_stepper()->get_descriptor(DRIVE_RED_1).id.r_id == NEMA_DRIVE_WHEEL_22;
    unsigned int num_rotors = 6;
    vector<unsigned int> rings;
    string rotors, selected_rings, positions;
    
    if (param == "training")
    {
        is_war_machine = false;
    }

    if (param == "war")
    {
        is_war_machine = true;
    }
    
    // Set possible rings for war machine
    rings.push_back(12); rings.push_back(13); rings.push_back(14); rings.push_back(15);
    rings.push_back(17); rings.push_back(18);
    
    if (!is_war_machine)
    {
        // Set possible rings for training machine
        rings.clear();
        rings.push_back(16); rings.push_back(19); rings.push_back(20); rings.push_back(21);
        num_rotors = 4;
    }
    
    try
    {
        permutation rotor_selection_perm = permutation::get_random_permutation(rand, num_rotors);
        permutation ring_selection_perm = permutation::get_random_permutation(rand, rings.size());            
        
        // Construct random rotor setting    
        for(unsigned int count = 0; count < 4; count++)
        {
            rotors += 'a' + (char)(rotor_selection_perm.encrypt(count));
        }

        // Construct random ring setting
        for(unsigned int count = 0; count < 4; count++)
        {
            selected_rings += (boost::lexical_cast<string>(rings[ring_selection_perm.encrypt(count)]) + " ");
        }        

        machine_conf[KW_NEMA_ROTORS] = rotors;
        machine_conf[KW_NEMA_RINGS] = selected_rings;
        machine_conf[KW_NEMA_WAR_MACHINE] = (is_war_machine ? CONF_TRUE : CONF_FALSE);

        c->configure_machine(machine_conf, this);   

        positions = nema_alpha.get_random_string(10);
        get_nema_stepper()->set_all_positions(positions);      
    }
    catch(...)
    {
        result = true;
    }    
    
    return result;
}

nema::nema(vector<rotor_assembly>& rotor_settings, unsigned int left_red_drive_wheel, unsigned int right_red_drive_wheel) 
    : rotor_machine() 
{
    add_rotor_set(DEFAULT_SET, nema_rotor_factory::get_rotor_set());
    pair<boost::shared_ptr<rotor>, boost::shared_ptr<rotor_ring> > help;
        
    vector<string> rotor_names;
    
    is_pre_step = true;
    machine_name = MNAME_NEMA;
    
    // Set up names of rotor slots 
    rotor_names.push_back(ETW);
    rotor_names.push_back(DRIVE_RED_1);
    rotor_names.push_back(CONTACT_2);
    rotor_names.push_back(DRIVE_3);
    rotor_names.push_back(CONTACT_4);
    rotor_names.push_back(DRIVE_5);
    rotor_names.push_back(CONTACT_6);
    rotor_names.push_back(DRIVE_7);
    rotor_names.push_back(CONTACT_8);
    rotor_names.push_back(DRIVE_9);
    rotor_names.push_back(CONTACT_UKW_10);
    
    nema_stepper *s = new nema_stepper(rotor_names);    
    
    set_stepping_gear(s);
    
    // Place rotors and drive wheels in machine
    prepare_rotor(NEMA_ETW, ETW, true);        
    
    prepare_red_wheel(left_red_drive_wheel, right_red_drive_wheel);       
    
    prepare_rotor(rotor_settings[3].contact_rotor_id, CONTACT_2);
    prepare_rotor(rotor_settings[3].drive_wheel_id, DRIVE_3);

    prepare_rotor(rotor_settings[2].contact_rotor_id, CONTACT_4);
    prepare_rotor(rotor_settings[2].drive_wheel_id, DRIVE_5);

    prepare_rotor(rotor_settings[1].contact_rotor_id, CONTACT_6);
    prepare_rotor(rotor_settings[1].drive_wheel_id, DRIVE_7);

    prepare_rotor(rotor_settings[0].contact_rotor_id, CONTACT_8);
    prepare_rotor(rotor_settings[0].drive_wheel_id, DRIVE_9);
    
    prepare_rotor(NEMA_UKW, CONTACT_UKW_10);
    
    // Set up printing_device
    boost::shared_ptr<printing_device> prt(new symmetric_printing_device(ustring("ijklmnopqrstuvwxyzabcdefgh")));
    set_printer(prt);

    // Set up keyboard    
    boost::shared_ptr<rotor_keyboard> kbd(new symmetric_keyboard(ustring("ijklmnopqrstuvwxyzabcdefgh")));
    set_keyboard(kbd); 
    
    randomizer_params.push_back(randomizer_descriptor("war", "Force war machine"));
    randomizer_params.push_back(randomizer_descriptor("training", "Force training machine"));        
    
    unvisualized_rotor_names.insert(ETW);                          
        
    get_stepping_gear()->reset(); 
}

