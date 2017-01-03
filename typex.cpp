/***************************************************************************
 * Copyright 2016 Martin Grap
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

/*! \file typex.cpp
 *  \brief Contains an implementation of a simulator for the Typex machine.
 */

#include<stdexcept>
#include<set>
#include<boost/scoped_ptr.hpp>
#include<rmsk_globals.h>

#include<typex.h>
#include<configurator.h>

#define ETW "eintrittswalze"

/*! \brief Set of input chars used when in letters mode.
 */
ustring str_norm_chars =    "abcdefghijklmnopqrstu<w y>";

/*! \brief Set of input chars used when in figures mode.
 */
ustring str_shifted_chars = "-'vz3%x£8*().,9014/57<2 6>";

/*! \brief Set of output chars.
 */
ustring str_output_chars =  "abcdefghijklmnopqrstuvwxyz";

/*! \brief Rotor and ring data for Typex rotor set SP_02390
 */
rotor_set typex_set_SP_02390(26);

/*! \brief Rotor and ring data for Typex rotor set Y_269
 */
rotor_set typex_set_Y_269(26);

/*! \brief Holds all Typex rotor sets
 */
map<string, rotor_set*> typex_rotor_sets::typex_sets;

rotor_set& typex_rotor_sets::get_rotor_set(const char *set_name)
{
    string real_set_name = DEFAULT_SET;
    string set_name_to_test(set_name);

    // Fill typex_sets if empty
    if (typex_sets.size() == 0)
    {
        typex_sets[DEFAULT_SET] = &typex_set_SP_02390;
        typex_sets[Y269] = &typex_set_Y_269;
    }
    
    // Check if specified rotor set name is valid
    if (typex_sets.count(set_name_to_test) != 0)
    {
        real_set_name = set_name_to_test;        
    }
    
    // Fill named rotor set if it is empty
    if (typex_sets[real_set_name]->get_num_rotors() == 0)
    {
        if (real_set_name == DEFAULT_SET)
        {
            vector<unsigned int> sp_02390_ids = {TYPEX_SP_02390_A, TYPEX_SP_02390_B, TYPEX_SP_02390_C, TYPEX_SP_02390_D, TYPEX_SP_02390_E, 
                                                 TYPEX_SP_02390_F, TYPEX_SP_02390_G, TYPEX_ETW, TYPEX_SP_02390_UKW};
            
            enigma_rotor_factory::get_rotor_set().slice_rotor_set(*typex_sets[real_set_name], sp_02390_ids, sp_02390_ids);                        
        }

        if (real_set_name == Y269)
        {
            vector<unsigned int> y_269_ids = {TYPEX_Y_269_A, TYPEX_Y_269_B, TYPEX_Y_269_C, TYPEX_Y_269_D, TYPEX_Y_269_E, 
                                              TYPEX_Y_269_F, TYPEX_Y_269_G, TYPEX_Y_269_H, TYPEX_Y_269_I, TYPEX_Y_269_J,
                                              TYPEX_Y_269_K, TYPEX_Y_269_L, TYPEX_Y_269_M, TYPEX_Y_269_N, TYPEX_ETW, 
                                              TYPEX_Y_269_UKW};          
            
            enigma_rotor_factory::get_rotor_set().slice_rotor_set(*typex_sets[real_set_name], y_269_ids, y_269_ids);  
        }

    }
    
    return *typex_sets[real_set_name];
}

void typex_stepper::reset()
{
    enigma_stepper::reset();
    
    set_ringstellung(STATOR1, 'a');
    set_ringstellung(STATOR2, 'a');
    set_rotor_pos(STATOR1, 'a');
    set_rotor_pos(STATOR2, 'a');
}

void typex::set_reflector(vector<pair<char, char> >& data)
{
    boost::shared_ptr<permutation> new_reflector(rmsk::std_alpha()->make_involution_ptr(data));
    
    get_stepping_gear()->get_descriptor(UMKEHRWALZE).r->set_perm(new_reflector);
}

bool typex::randomize(string& param)
{
    bool result = false;
    random_bit_source reverse_rotors(5);
    urandom_generator rand;    
    string known_rotors("abcdefg");
    string name_rotor_set = get_default_set_name();
    string ring_positions, rotor_positions, selected_rotors;
    map<string, string> machine_conf;
    
    if (param == "y269")
    {
        name_rotor_set = Y269;
    }
    
    if (param == "sp02390")
    {
        name_rotor_set = DEFAULT_SET;
    }    
    
    if (name_rotor_set == DEFAULT_SET)
    {
        known_rotors = "abcdefg";
    }

    if (name_rotor_set == Y269)
    {
        known_rotors = "abcdefghijklmn";
    }
    
    try
    {
        permutation rotor_selection_perm = permutation::get_random_permutation(rand, known_rotors.length());
        permutation reflector_perm = permutation::get_random_permutation(rand, 26);        
        ring_positions = rmsk::std_alpha()->get_random_string(5);
        rotor_positions = rmsk::std_alpha()->get_random_string(5);        
        
        for(unsigned int count = 0; count < 5; count++)
        {
            selected_rotors += known_rotors[rotor_selection_perm.encrypt(count)];
            selected_rotors += ((reverse_rotors.get_next_val() == 0) ? 'N' : 'R');
        }
        
        machine_conf[KW_TYPEX_ROTOR_SET] = name_rotor_set;
        machine_conf[KW_TYPEX_ROTORS] = selected_rotors;
        machine_conf[KW_TYPEX_RINGS] = ring_positions;
        machine_conf[KW_TYPEX_REFLECTOR] = rmsk::std_alpha()->perm_as_string(reflector_perm);
        boost::scoped_ptr<configurator> c(configurator_factory::get_configurator(machine_name));
        c->configure_machine(machine_conf, this);
        
        get_enigma_stepper()->set_rotor_pos(STATOR1, rotor_positions[0]);
        get_enigma_stepper()->set_rotor_pos(STATOR2, rotor_positions[1]);
        get_enigma_stepper()->set_rotor_pos(FAST, rotor_positions[2]);
        get_enigma_stepper()->set_rotor_pos(MIDDLE, rotor_positions[3]);
        get_enigma_stepper()->set_rotor_pos(SLOW, rotor_positions[4]);        
    }
    catch(...)
    {
        result = true;
    }    
    
    return result;
}

typex::typex(unsigned int ukw_id, rotor_id slow_id, rotor_id middle_id, rotor_id fast_id, rotor_id stat2_id, rotor_id stat1_id) 
    : enigma_family_base() 
{ 
    vector<string> rotor_names;
    vector<gunichar> norm_alph, shift_alph, out_alph;
    
    add_rotor_set(DEFAULT_SET, typex_rotor_sets::get_rotor_set(DEFAULT_SET));
    add_rotor_set(Y269, typex_rotor_sets::get_rotor_set(Y269));
    
    stepper = NULL;
    machine_name = MNAME_TYPEX;
    
    // Set rotor slot names
    rotor_names.push_back(ETW);
    rotor_names.push_back(STATOR1);
    rotor_names.push_back(STATOR2);
    rotor_names.push_back(FAST);
    rotor_names.push_back(MIDDLE);
    rotor_names.push_back(SLOW);
    rotor_names.push_back(UMKEHRWALZE);
    
    set_stepping_gear(new typex_stepper(rotor_names));
    get_stepping_gear()->get_stack().set_reflecting_flag(true);
    
    // Set up the printing_device
    shifting_printing_device *typex_printer = new shifting_printing_device(21, 25);
    printing_device::ustr_to_vec(str_norm_chars, norm_alph);
    printing_device::ustr_to_vec(str_shifted_chars, shift_alph);    
    printing_device::ustr_to_vec(str_output_chars, out_alph);        
    boost::shared_ptr<alphabet<gunichar> > letter_alpha(new alphabet<gunichar>(norm_alph));
    boost::shared_ptr<alphabet<gunichar> > figure_alpha(new alphabet<gunichar>(shift_alph));
    boost::shared_ptr<alphabet<gunichar> > output_alpha(new alphabet<gunichar>(out_alph));    
    
    typex_printer->set_letters_alphabet(letter_alpha);    
    typex_printer->set_figures_alphabet(figure_alpha); 
    typex_printer->set_cipher_alphabet(output_alpha);
    
    set_printer(boost::shared_ptr<printing_device>(typex_printer));
    
    // Set up the rotor_keyboard
    boost::shared_ptr<shifting_keyboard> kbd(new shifting_keyboard(21, 25));    

    kbd->set_letters_alphabet(letter_alpha);    
    kbd->set_figures_alphabet(figure_alpha); 
    kbd->set_cipher_alphabet(output_alpha);

    set_keyboard(kbd);
    
    // Set up rotors
    prepare_rotor(TYPEX_ETW, ETW, true);
    prepare_rotor(stat1_id, STATOR1);
    prepare_rotor(stat2_id, STATOR2);    
    prepare_rotor(fast_id, FAST);
    prepare_rotor(middle_id, MIDDLE);
    prepare_rotor(slow_id, SLOW);    
    prepare_rotor(ukw_id, UMKEHRWALZE);
    
    randomizer_params.push_back(randomizer_descriptor("sp02390", "Force rotor set SP02390"));
    randomizer_params.push_back(randomizer_descriptor("y269", "Force rotor set Y269"));            
    
    unvisualized_rotor_names.insert(ETW);        
    unvisualized_rotor_names.insert(UMKEHRWALZE);  
    
    get_stepping_gear()->reset();
}


