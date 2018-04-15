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

/*! \file kl7.cpp
 *  \brief This file contains the implmentation of a simulator for the KL7.
 */ 

#include<boost/scoped_ptr.hpp>
#include<boost/lexical_cast.hpp>
#include<kl7.h>
#include<rmsk_globals.h>
#include<configurator.h>

// Notches on the rings. Information created by kl7prep.py
#define KL7RING_1  "100010000100011010100011001100110001"
#define KL7RING_2  "001101010001011000100110010100010011"
#define KL7RING_3  "110000110100001000100001010101010010"
#define KL7RING_4  "101000100100011001010000111001110100"
#define KL7RING_5  "101001100010010001100101010110000110"
#define KL7RING_6  "000001110011000101000110110110010001"
#define KL7RING_7  "110010011001100001000101000110110100"
#define KL7RING_8  "001110010001010110011100101000011011"
#define KL7RING_9  "111010110000100110010100100110111000"
#define KL7RING_10 "010000010110011101000100010011001110"
#define KL7RING_11 "110010000110001011000100110000101000"
#define KL7RING_WIDE "000000000000000000000000000000000000"

#define BASE_PLATE_WIRING "qp0nfc3yom9gr8ui7bh2vtw6xs4jlz5dkea1"

/*! \brief Specifies the offset with respect to the current position where the notches are sensed.
 */
const unsigned int NOTCH_SENSE_OFFSET = 10;

/*! \brief In the KL7 ten contacts are fed back through the rotor_stack.
 */
const unsigned int NUM_FEEDBACK_POINTS = 10;

// Permutations of the rotors. Information created by kl7prep.py
static unsigned int kl7_rotor_A[KL7_ROTOR_SIZE] = {28,26,13,7,34,3,27,10,4,12,19,2,24,32,17,14,6,11,33,15,16,0,8,29,23,22,1,31,9,18,5,25,35,21,30,20};
static unsigned int kl7_rotor_B[KL7_ROTOR_SIZE] = {22,18,25,15,1,12,13,34,20,3,16,30,24,2,17,26,11,33,35,9,29,5,6,14,27,0,10,32,28,19,31,23,4,21,7,8};
static unsigned int kl7_rotor_C[KL7_ROTOR_SIZE] = {18,25,27,35,5,24,30,17,26,9,4,0,31,8,10,32,22,16,28,11,12,1,15,14,34,7,23,21,29,2,33,13,6,19,20,3};
static unsigned int kl7_rotor_D[KL7_ROTOR_SIZE] = {14,25,35,12,0,30,24,32,2,31,20,22,16,28,6,21,19,23,11,9,13,29,18,27,3,34,4,7,5,8,15,26,1,10,33,17};
static unsigned int kl7_rotor_E[KL7_ROTOR_SIZE] = {12,3,1,15,16,29,20,4,32,6,28,7,8,35,34,22,33,24,19,21,27,14,0,18,23,26,9,10,5,11,31,25,13,2,17,30};
static unsigned int kl7_rotor_F[KL7_ROTOR_SIZE] = {25,33,26,13,1,0,30,35,10,8,34,17,14,11,3,6,28,7,22,18,2,29,19,16,27,20,21,4,24,32,15,12,23,5,9,31};
static unsigned int kl7_rotor_G[KL7_ROTOR_SIZE] = {19,18,8,31,35,1,5,32,11,27,3,9,2,23,28,15,21,17,29,16,6,33,14,22,30,24,26,0,20,25,7,4,12,34,10,13};
static unsigned int kl7_rotor_H[KL7_ROTOR_SIZE] = {27,18,22,4,16,35,26,13,15,19,20,6,11,21,10,34,12,14,0,31,7,17,32,3,8,28,25,23,24,33,9,5,2,29,1,30};
static unsigned int kl7_rotor_I[KL7_ROTOR_SIZE] = {24,5,34,11,20,21,18,31,19,22,29,17,0,15,30,10,23,12,32,6,35,8,33,1,9,7,25,28,14,16,3,27,13,2,26,4};
static unsigned int kl7_rotor_J[KL7_ROTOR_SIZE] = {7,30,0,27,19,5,31,4,32,20,29,11,3,13,14,33,6,34,15,17,28,21,24,25,35,10,22,18,2,1,12,26,23,9,16,8};
static unsigned int kl7_rotor_K[KL7_ROTOR_SIZE] = {14,12,35,22,5,20,31,17,30,19,0,23,9,34,18,27,6,7,25,11,28,21,24,29,4,8,1,26,15,3,16,2,33,13,10,32};
static unsigned int kl7_rotor_L[KL7_ROTOR_SIZE] = {7,17,14,32,6,25,19,15,33,22,35,26,11,23,18,12,1,2,13,28,0,5,31,9,24,29,8,4,27,16,21,30,3,10,20,34};
static unsigned int kl7_rotor_M[KL7_ROTOR_SIZE] = {35,5,28,27,23,25,20,21,19,34,14,18,22,29,0,7,11,6,12,26,30,31,8,32,9,15,13,17,33,1,2,16,24,10,3,4};

/*! \brief Contacts at the end of the rotor bank which are fed back to its beginning.
 */
static unsigned int kl7_feedback_points[NUM_FEEDBACK_POINTS] = {2,6,10,13,16,19,23,26,30,35};

/*! \brief Alphabet used to interpret the base plate wiring which is, so to say, the KL7's Eintrittswalze.
 */
alphabet<char> kl7_alpha("abcdefghijklmnopqrstuvwxyz1234567890", KL7_ROTOR_SIZE);

/*! \brief Alphabet used to visualize the rotor positions. Uses 0-9 where the letter rings of the
 *         real machine are empty.
 */
alphabet<char> kl7_ring_alpha(KL7_RING_CIRCUMFENCE_HELP, KL7_ROTOR_SIZE);

/*! \brief Input characters used when machine is in letters mode. > is the character that switches the
 *         machine in figures mode. Note that there is no character < that switches the machine in letters mode.
 */
static ustring kl7_str_norm_chars =    "abcdefghi>klmnopqrstuvwxy ";

/*! \brief Input characters used when machine is in figures mode. > is the character that switches the
 *         machine in figures mode. Character < switches the machine back in letters mode.
 */
static ustring kl7_str_shifted_chars = "abcd3fgh8>klmn9014s57<2x6 ";

/*! \brief Output characters.
 */
static ustring kl7_str_output_chars =  "abcdefghijklmnopqrstuvwxyz";

/*! \brief Holds the default rotor_set object.
 */
rotor_set kl7_rotor_factory::kl7_set_data(KL7_ROTOR_SIZE);

rotor_set *kl7_rotor_factory::kl7_set = &kl7_rotor_factory::kl7_set_data;

rotor_set *kl7_rotor_factory::get_rotor_set()
{    
    if (kl7_set->get_num_rotors() == 0)
    {
        vector<unsigned int> ring_1, ring_2, ring_3, ring_4, ring_5, ring_6, ring_7, ring_8, ring_9, ring_10, ring_11, ring_wide;
        
        // parse ring data
        rmsk::str_to_bool_vec((const char *)KL7RING_1, KL7_ROTOR_SIZE, ring_1);
        rmsk::str_to_bool_vec((const char *)KL7RING_2, KL7_ROTOR_SIZE, ring_2);
        rmsk::str_to_bool_vec((const char *)KL7RING_3, KL7_ROTOR_SIZE, ring_3);        
        rmsk::str_to_bool_vec((const char *)KL7RING_4, KL7_ROTOR_SIZE, ring_4);
        rmsk::str_to_bool_vec((const char *)KL7RING_5, KL7_ROTOR_SIZE, ring_5);
        rmsk::str_to_bool_vec((const char *)KL7RING_6, KL7_ROTOR_SIZE, ring_6);        
        rmsk::str_to_bool_vec((const char *)KL7RING_7, KL7_ROTOR_SIZE, ring_7);
        rmsk::str_to_bool_vec((const char *)KL7RING_8, KL7_ROTOR_SIZE, ring_8);
        rmsk::str_to_bool_vec((const char *)KL7RING_9, KL7_ROTOR_SIZE, ring_9);        
        rmsk::str_to_bool_vec((const char *)KL7RING_10, KL7_ROTOR_SIZE, ring_10);
        rmsk::str_to_bool_vec((const char *)KL7RING_11, KL7_ROTOR_SIZE, ring_11);
        rmsk::str_to_bool_vec((const char *)KL7RING_WIDE, KL7_ROTOR_SIZE, ring_wide);

        // add rings
        kl7_set->add_ring(KL7_RING_1, ring_1);
        kl7_set->add_ring(KL7_RING_2, ring_2);        
        kl7_set->add_ring(KL7_RING_3, ring_3);
        kl7_set->add_ring(KL7_RING_4, ring_4);        
        kl7_set->add_ring(KL7_RING_5, ring_5);
        kl7_set->add_ring(KL7_RING_6, ring_6);        
        kl7_set->add_ring(KL7_RING_7, ring_7);
        kl7_set->add_ring(KL7_RING_8, ring_8);        
        kl7_set->add_ring(KL7_RING_9, ring_9);
        kl7_set->add_ring(KL7_RING_10, ring_10);        
        kl7_set->add_ring(KL7_RING_11, ring_11);                
        kl7_set->add_ring(KL7_RING_WIDE, ring_wide);
        
        set<unsigned int> kl7_const_ids = {KL7_RING_1, KL7_RING_2, KL7_RING_3, KL7_RING_4, KL7_RING_5, KL7_RING_6, KL7_RING_7, 
                                           KL7_RING_8, KL7_RING_9, KL7_RING_10, KL7_RING_11, KL7_RING_WIDE}; 
        
        // add rotors
        kl7_set->add_rotor(KL7_ROTOR_A, rmsk::uint_arr_to_vec(kl7_rotor_A, KL7_ROTOR_SIZE));
        kl7_set->add_rotor(KL7_ROTOR_B, rmsk::uint_arr_to_vec(kl7_rotor_B, KL7_ROTOR_SIZE));
        kl7_set->add_rotor(KL7_ROTOR_C, rmsk::uint_arr_to_vec(kl7_rotor_C, KL7_ROTOR_SIZE));
        kl7_set->add_rotor(KL7_ROTOR_D, rmsk::uint_arr_to_vec(kl7_rotor_D, KL7_ROTOR_SIZE));                        
        kl7_set->add_rotor(KL7_ROTOR_E, rmsk::uint_arr_to_vec(kl7_rotor_E, KL7_ROTOR_SIZE));
        kl7_set->add_rotor(KL7_ROTOR_F, rmsk::uint_arr_to_vec(kl7_rotor_F, KL7_ROTOR_SIZE));
        kl7_set->add_rotor(KL7_ROTOR_G, rmsk::uint_arr_to_vec(kl7_rotor_G, KL7_ROTOR_SIZE));
        kl7_set->add_rotor(KL7_ROTOR_H, rmsk::uint_arr_to_vec(kl7_rotor_H, KL7_ROTOR_SIZE));                        
        kl7_set->add_rotor(KL7_ROTOR_I, rmsk::uint_arr_to_vec(kl7_rotor_I, KL7_ROTOR_SIZE));
        kl7_set->add_rotor(KL7_ROTOR_J, rmsk::uint_arr_to_vec(kl7_rotor_J, KL7_ROTOR_SIZE));
        kl7_set->add_rotor(KL7_ROTOR_K, rmsk::uint_arr_to_vec(kl7_rotor_K, KL7_ROTOR_SIZE));
        kl7_set->add_rotor(KL7_ROTOR_L, rmsk::uint_arr_to_vec(kl7_rotor_L, KL7_ROTOR_SIZE));                        
        kl7_set->add_rotor(KL7_ROTOR_M, rmsk::uint_arr_to_vec(kl7_rotor_M, KL7_ROTOR_SIZE));              
        
        kl7_set->set_const_ids(kl7_const_ids);                  
    }
    
    return kl7_set;
}


kl7_stepping_gear::kl7_stepping_gear(vector<string>& rotor_identifiers) 
    : stepping_gear(rotor_identifiers, KL7_ROTOR_SIZE) 
{             
    stack->set_reflecting_flag(false);
}


void kl7_stepping_gear::step_rotors()
{
    stepping_gear::step_rotors();
    map<string, bool> step_info;
    map<string, bool>::iterator step_info_iter;    
        
    // Determine which of the rotors are to step
    step_info[KL7_ROT_1] = (!rotor_is_at_notch(KL7_ROT_8, NOTCH_SENSE_OFFSET)) and ((!rotor_is_at_notch(KL7_ROT_2, NOTCH_SENSE_OFFSET)) or (!rotor_is_at_notch(KL7_ROT_3, NOTCH_SENSE_OFFSET)));
    step_info[KL7_ROT_2] = (!rotor_is_at_notch(KL7_ROT_6, NOTCH_SENSE_OFFSET)) or (!rotor_is_at_notch(KL7_ROT_7, NOTCH_SENSE_OFFSET));
    step_info[KL7_ROT_3] = rotor_is_at_notch(KL7_ROT_2, NOTCH_SENSE_OFFSET) or rotor_is_at_notch(KL7_ROT_7, NOTCH_SENSE_OFFSET);
    step_info[KL7_ROT_5] = (!rotor_is_at_notch(KL7_ROT_2, NOTCH_SENSE_OFFSET)) or (!rotor_is_at_notch(KL7_ROT_3, NOTCH_SENSE_OFFSET));
    step_info[KL7_ROT_6] = (!rotor_is_at_notch(KL7_ROT_1, NOTCH_SENSE_OFFSET)) or rotor_is_at_notch(KL7_ROT_3, NOTCH_SENSE_OFFSET);
    step_info[KL7_ROT_7] = rotor_is_at_notch(KL7_ROT_1, NOTCH_SENSE_OFFSET) or rotor_is_at_notch(KL7_ROT_6, NOTCH_SENSE_OFFSET);
    step_info[KL7_ROT_8] = (!rotor_is_at_notch(KL7_ROT_5, NOTCH_SENSE_OFFSET)) and ((!rotor_is_at_notch(KL7_ROT_2, NOTCH_SENSE_OFFSET)) or (!rotor_is_at_notch(KL7_ROT_3, NOTCH_SENSE_OFFSET)));
        
    // Step the rotors according to contents of step_info
    for (step_info_iter = step_info.begin(); step_info_iter != step_info.end(); ++step_info_iter)
    {
        if (step_info_iter->second)
        {
            string help = step_info_iter->first;
            advance_rotor(help);
        }
    }
    
}

void kl7_stepping_gear::reset()
{
    unsigned int count = 0;
    
    stepping_gear::reset();
    
    for (count = 0; count < num_rotors; count++)
    {
        // Set displacement and "real" offset to zero
        set_rotor_displacement(count, 0);        
        get_descriptor(count).ring->set_offset(0);        
        // Set letter and notch ring offest to zero
        get_descriptor(count).mod_int_vals[LETTER_RING_POS] = simple_mod_int(KL7_ROTOR_SIZE);
    } 
}

void kl7_stepping_gear::set_kl7_rings(const char *identifier, unsigned int letter_ring_offset, unsigned int notch_ring_offset)
{    
    get_descriptor(identifier).mod_int_vals[LETTER_RING_POS] = letter_ring_offset;
    simple_mod_int h(notch_ring_offset, KL7_ROTOR_SIZE);
    get_descriptor(identifier).ring->set_offset(h);    
}

void kl7_stepping_gear::set_kl7_rings_and_pos(const char *identifier, unsigned int letter_ring_offset, unsigned int notch_ring_offset, unsigned int new_pos)
{
    set_kl7_rings(identifier, letter_ring_offset, notch_ring_offset);
    
    move_to_letter_ring_pos(identifier, new_pos);
}

unsigned int kl7_stepping_gear::get_notch_offset(const char *identifier)
{
    return get_descriptor(identifier).ring->get_offset();
}

unsigned int kl7_stepping_gear::get_letter_offset(const char *identifier)
{
    return get_descriptor(identifier).mod_int_vals[LETTER_RING_POS];
}

void kl7_stepping_gear::move_to_letter_ring_pos(const char *identifier, unsigned int new_pos)
{
    // KL7_ROT_4 can not be moved
    rmsk::simple_assert(string(identifier) == string(KL7_ROT_4), "programmer error: method cannot be used with 4th rotor");
    
    simple_mod_int n_pos(new_pos, KL7_ROTOR_SIZE);
    
    set_rotor_displacement(identifier, n_pos + get_descriptor(identifier).mod_int_vals[LETTER_RING_POS]);    
}
    
void kl7_stepping_gear::set_stationary_rotor_ring_pos(unsigned int letter_ring_offset)
{
    set_rotor_displacement(KL7_ROT_4, letter_ring_offset);
}

unsigned int kl7_stepping_gear::get_stationary_rotor_ring_pos()
{
    return get_rotor_displacement(KL7_ROT_4);
}

void kl7_stepping_gear::save_additional_components(string& identifier, Glib::KeyFile& ini_file)
{
    string section_name = "rotor_" + identifier;
    
    // Save letter ring offset in ini_file
    ini_file.set_integer(section_name, "letterring", (int)get_letter_offset(identifier.c_str()));
}
    
bool kl7_stepping_gear::load_additional_components(string& identifier, Glib::KeyFile& ini_file)
{
    bool result = false;
    string section_name = "rotor_" + identifier;
    unsigned int letter_ring_pos = 0, notch_ring_pos = 0;
    
    do
    {
        // Read letter ring position from ini_file
        if ((result = !ini_file.has_key(section_name, "letterring")))
        {
            break;
        }
        
        letter_ring_pos = (unsigned int)ini_file.get_integer(section_name, "letterring");       

        // Read notch ring position from ini_file
        if ((result = !ini_file.has_key(section_name, "ringoffset")))
        {
            break;
        }
        
        notch_ring_pos = (unsigned int)ini_file.get_integer(section_name, "ringoffset");
        
        // Set the offsets as read from the ini_file
        set_kl7_rings(identifier.c_str(), letter_ring_pos, notch_ring_pos);
        
    } while(0);

    return result;
}

ustring kl7::visualize_rotor_pos(string& rotor_identifier)
{
    ustring result;
    
    if (rotor_identifier != KL7_ROT_4)
    {
        // Movable rotors
        
        // Determine current rotor displacement
        simple_mod_int rotor_pos(get_stepping_gear()->get_rotor_displacement(rotor_identifier), KL7_ROTOR_SIZE);
        // Determine current letter ring offset
        simple_mod_int letter_offset(get_kl7_stepper()->get_letter_offset(rotor_identifier.c_str()), KL7_ROTOR_SIZE);
        // Holds the current rotor position as shown in the rotor window
        simple_mod_int vis_r_pos(KL7_ROTOR_SIZE);
        
        // Calculate current rotor position using rotor displacement and letter ring offset
        vis_r_pos = rotor_pos - letter_offset;
        
        // Use kl7_ring_alpha to map current position to an output character
        result += kl7_ring_alpha.to_val(vis_r_pos);
    }
    else
    {
        // Stationary rotor KL7_ROT_4
        
        // Simply use kl7_ring_alpha and current rotor ring position to determine output character
        result += kl7_ring_alpha.to_val(get_kl7_stepper()->get_stationary_rotor_ring_pos());
    }
    
    return result;
}

vector<ustring> kl7::visualize_active_permutations()
{
    vector<unsigned int> positions_to_visualize = {0, 1, 2, 3, 4, 5, 6, 7};
    
    return rotor_perm_visualizer_help(positions_to_visualize, kl7_alpha);
}

bool kl7::move_all_rotors(ustring& new_positions)
{
    bool result = false;    
    string pos_help;
    vector<unsigned int> new_pos;
    vector<gunichar> help;
    vector<string> rotor_names;
    
    // Create Unicode helper alphabet for KL7 letter ring data
    Glib::ustring kl7_chars(KL7_RING_CIRCUMFENCE_HELP);            

    for (unsigned int count = 0; count < kl7_chars.length(); count++)
    {
        help.push_back(kl7_chars[count]);
    }
    
    alphabet<gunichar> kl7_uni_alpha(help);
    get_kl7_stepper()->get_rotor_identifiers(rotor_names);
    
    // Check new_positions has correct length
    result = (new_positions.length() > 8) || (new_positions.length() < 7);
    
    // Convert alphabetic specification into numeric form
    for (unsigned int count = 0; (count < new_positions.length()) && (!result); count++)
    {
        result = !kl7_uni_alpha.contains_symbol(new_positions[count]);
        
        if (!result)
        {
            new_pos.push_back(kl7_uni_alpha.from_val(new_positions[count]));
        }
    }
    
    // Now set all positions using the numeric form
    if (!result)
    {
        int pos_count = new_positions.length() - 1;
        
        for (int rotor_slot_count = get_stepping_gear()->get_num_rotors() - 1; rotor_slot_count >= 0; rotor_slot_count--)
        {
            string identifier = rotor_names[rotor_slot_count];
            
            if (identifier == KL7_ROT_4)
            {
                if (new_positions.length() == 8)
                {
                    get_kl7_stepper()->set_stationary_rotor_ring_pos(new_pos[pos_count]);
                    pos_count--;
                }
            }
            else
            {
                get_kl7_stepper()->move_to_letter_ring_pos(identifier.c_str(), new_pos[pos_count]);
                pos_count--;
            }
        }        
    }    
    
    return result;
}

ustring kl7::visualize_all_positions()
{
    ustring result;
    string temp;
    
    temp = rotor_machine::visualize_all_positions();
    reverse(temp.begin(), temp.end());
    result = temp;
        
    return result;
}

bool kl7::randomize(string& param)
{
    bool result = false;
    urandom_generator rand;    
    map<string, string> machine_conf;
    boost::scoped_ptr<configurator> c(configurator_factory::get_configurator(machine_name));
    string rotors, selected_rings, alpha_pos, notch_pos; 
    vector<unsigned int> alpha_ring_pos, notch_ring_pos, rotor_pos;   
    
    try
    {
        permutation rotor_selection_perm = permutation::get_random_permutation(rand, 13);
        permutation ring_selection_perm = permutation::get_random_permutation(rand, 11);                

        for(unsigned int count = 0; count < 8; count++)
        {
            rotors += 'a' + (char)(rotor_selection_perm.encrypt(count));
        }
        
        for(unsigned int count = 0; count < 7; count++)
        {
            selected_rings += (boost::lexical_cast<string>(ring_selection_perm.encrypt(count) + 1) + " ");
        }        

        alpha_ring_pos = kl7_ring_alpha.to_vector(kl7_ring_alpha.get_random_string(8));

        for(unsigned int count = 0; count < 8; count++)
        {
            alpha_pos += (boost::lexical_cast<string>(alpha_ring_pos[count] + 1) + " ");
        }        

        notch_ring_pos = kl7_ring_alpha.to_vector(kl7_ring_alpha.get_random_string(7));

        for(unsigned int count = 0; count < 7; count++)
        {
            notch_pos += kl7_configurator::transform_notch_ring_pos(notch_ring_pos[count]);
        }        

        machine_conf[KW_KL7_ROTORS] = rotors;
        machine_conf[KW_KL7_ALPHA_POS] = alpha_pos;
        machine_conf[KW_KL7_NOTCH_RINGS] = selected_rings;
        machine_conf[KW_KL7_NOTCH_POS] = notch_pos;

        c->configure_machine(machine_conf, this); 
        
        // Key lists contained only letters for the rotor positions. In order to exhaust the full
        // number of rotor positions the rotors are stepped once before the first en/decryption. This
        // moves some rotors pseudorandomly to a non letter/empty position.
        rotor_pos = kl7_ring_alpha.to_vector(rmsk::std_alpha()->get_random_string(7));
        
        get_kl7_stepper()->move_to_letter_ring_pos(KL7_ROT_1, rotor_pos[0]);                                             
        get_kl7_stepper()->move_to_letter_ring_pos(KL7_ROT_2, rotor_pos[1]);
        get_kl7_stepper()->move_to_letter_ring_pos(KL7_ROT_3, rotor_pos[2]);                        
        get_kl7_stepper()->move_to_letter_ring_pos(KL7_ROT_5, rotor_pos[3]);
        get_kl7_stepper()->move_to_letter_ring_pos(KL7_ROT_6, rotor_pos[4]);                        
        get_kl7_stepper()->move_to_letter_ring_pos(KL7_ROT_7, rotor_pos[5]);            
        get_kl7_stepper()->move_to_letter_ring_pos(KL7_ROT_8, rotor_pos[6]);            
    }
    catch(...)
    {
        result = true;
    }    
    
    return result;
}


kl7::kl7(vector<rotor_id>& r_ids)
    : rotor_machine()
{
    add_rotor_set(DEFAULT_SET, kl7_rotor_factory::get_rotor_set());
    vector<string> rotor_names;
    set<unsigned int> feedback_points;
    permutation *base_plate, *inv_base_plate;
    vector<gunichar> norm_alph, shift_alph, out_alph;        
    
    // Set up rotor slots
    rotor_names.push_back(KL7_ROT_1);
    rotor_names.push_back(KL7_ROT_2);
    rotor_names.push_back(KL7_ROT_3);
    rotor_names.push_back(KL7_ROT_4);
    rotor_names.push_back(KL7_ROT_5);
    rotor_names.push_back(KL7_ROT_6);
    rotor_names.push_back(KL7_ROT_7);
    rotor_names.push_back(KL7_ROT_8);
    
    machine_name = MNAME_KL7;

    kl7_stepping_gear *s = new kl7_stepping_gear(rotor_names);
        
    set_stepping_gear(s);
    
    // Set up "Eintrittswalze"
    base_plate = kl7_alpha.make_perm_ptr(BASE_PLATE_WIRING);
    inv_base_plate = kl7_alpha.make_perm_ptr(BASE_PLATE_WIRING);
    inv_base_plate->switch_to_inverse();
    
    set_input_transform(boost::shared_ptr<permutation>(inv_base_plate));
    set_output_transform(boost::shared_ptr<permutation>(base_plate));

    // Set up input and ouput alphabets
    printing_device::ustr_to_vec(kl7_str_norm_chars, norm_alph);
    printing_device::ustr_to_vec(kl7_str_shifted_chars, shift_alph);    
    printing_device::ustr_to_vec(kl7_str_output_chars, out_alph);        
    boost::shared_ptr<alphabet<gunichar> > letter_alpha(new alphabet<gunichar>(norm_alph));
    boost::shared_ptr<alphabet<gunichar> > figure_alpha(new alphabet<gunichar>(shift_alph));
    boost::shared_ptr<alphabet<gunichar> > output_alpha(new alphabet<gunichar>(out_alph));    
    
    // Set up printing_device
    shifting_printing_device *kl7_printer = new shifting_printing_device(UNDEFINED_SHIFT_CHAR, 9, 21, 9);
    
    kl7_printer->set_letters_alphabet(letter_alpha);    
    kl7_printer->set_figures_alphabet(figure_alpha); 
    kl7_printer->set_cipher_alphabet(output_alpha);
    kl7_printer->set_ltr_key_output("", " ");
    
    set_printer(boost::shared_ptr<printing_device>(kl7_printer));
    
    // Set up keyboard
    boost::shared_ptr<shifting_keyboard> kbd(new shifting_keyboard(UNDEFINED_SHIFT_CHAR, 9, 21, 9));    

    kbd->set_letters_alphabet(letter_alpha);    
    kbd->set_figures_alphabet(figure_alpha); 
    kbd->set_cipher_alphabet(output_alpha);

    set_keyboard(kbd);                

    // Place rotors in machine
    prepare_rotor(r_ids[0], KL7_ROT_1);
    prepare_rotor(r_ids[1], KL7_ROT_2);
    prepare_rotor(r_ids[2], KL7_ROT_3);
    prepare_rotor(r_ids[3], KL7_ROT_4);
    prepare_rotor(r_ids[4], KL7_ROT_5);
    prepare_rotor(r_ids[5], KL7_ROT_6);
    prepare_rotor(r_ids[6], KL7_ROT_7);
    prepare_rotor(r_ids[7], KL7_ROT_8); 
    
    // Set up feedback points
    (s->get_stack()).set_feedback_points(kl7_feedback_points, NUM_FEEDBACK_POINTS);       
    
    get_stepping_gear()->reset();
}

