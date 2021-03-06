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
#define SECT_PLUGBOARD "plugboard"
#define KEY_PLUGBOARD "entry"

#define RAND_PARM_SP02390 "sp02390"
#define RAND_PARM_Y269 "y269"
#define RAND_PARM_PLUGS_SP02390 "plugs02390"
#define RAND_PARM_PLUGS_Y269 "plugsy269"
#define RAND_PARM_PLUGS_Y2695 "plugsy2695"


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

/*! \brief Rotor and ring data for Typex rotor set Y_2695
 */
rotor_set typex_set_Y_2695(26);

/*! \brief Holds all Typex rotor sets
 */
map<string, rotor_set*> typex_rotor_sets::typex_sets;

rotor_set *typex_rotor_sets::get_rotor_set(const char *set_name)
{
    string real_set_name = DEFAULT_SET;
    string set_name_to_test(set_name);

    // Fill typex_sets if empty
    if (typex_sets.size() == 0)
    {
        typex_sets[DEFAULT_SET] = &typex_set_SP_02390;
        typex_sets[Y269] = &typex_set_Y_269;
        typex_sets[Y2695] = &typex_set_Y_2695;
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
            
            enigma_rotor_factory::get_rotor_set()->slice_rotor_set(*typex_sets[real_set_name], sp_02390_ids, sp_02390_ids);
            
            set<unsigned int> const_ids_sp02390 = {TYPEX_ETW, TYPEX_SP_02390_UKW};
            typex_sets[real_set_name]->set_const_ids(const_ids_sp02390);
                                    
        }

        if (real_set_name == Y269)
        {
            vector<unsigned int> y_269_ids = {TYPEX_Y_269_A, TYPEX_Y_269_B, TYPEX_Y_269_C, TYPEX_Y_269_D, TYPEX_Y_269_E, 
                                              TYPEX_Y_269_F, TYPEX_Y_269_G, TYPEX_Y_269_H, TYPEX_Y_269_I, TYPEX_Y_269_J,
                                              TYPEX_Y_269_K, TYPEX_Y_269_L, TYPEX_Y_269_M, TYPEX_Y_269_N, TYPEX_ETW, 
                                              TYPEX_Y_269_UKW};          
            
            enigma_rotor_factory::get_rotor_set()->slice_rotor_set(*typex_sets[real_set_name], y_269_ids, y_269_ids);  
            set<unsigned int> const_ids_y269 = {TYPEX_ETW, TYPEX_Y_269_UKW};
            typex_sets[real_set_name]->set_const_ids(const_ids_y269);
        }

        if (real_set_name == Y2695)
        {
            //                                    A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z
            vector<unsigned int> new_ring_data = {0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1};
        
            vector<unsigned int> y_2695_ids = {TYPEX_Y_269_A, TYPEX_Y_269_B, TYPEX_Y_269_C, TYPEX_Y_269_D, TYPEX_Y_269_E, 
                                              TYPEX_Y_269_F, TYPEX_Y_269_G, TYPEX_Y_269_H, TYPEX_Y_269_I, TYPEX_Y_269_J,
                                              TYPEX_Y_269_K, TYPEX_Y_269_L, TYPEX_Y_269_M, TYPEX_Y_269_N, TYPEX_ETW, 
                                              TYPEX_Y_269_UKW};          
            
           vector<unsigned int> y_2695_ring_ids = {TYPEX_Y_269_A, TYPEX_Y_269_B, TYPEX_Y_269_C, TYPEX_Y_269_D, TYPEX_Y_269_E, 
                                              TYPEX_Y_269_F, TYPEX_Y_269_G, TYPEX_Y_269_H, TYPEX_Y_269_I, TYPEX_Y_269_J,
                                              TYPEX_Y_269_K, TYPEX_Y_269_L, TYPEX_Y_269_M, TYPEX_Y_269_N};          
            
            enigma_rotor_factory::get_rotor_set()->slice_rotor_set(*typex_sets[real_set_name], y_2695_ids, y_2695_ids);  
            set<unsigned int> const_ids_y2695 = {TYPEX_ETW, TYPEX_Y_269_UKW};
            typex_sets[real_set_name]->set_const_ids(const_ids_y2695);
            typex_sets[real_set_name]->replace_ring_data(y_2695_ring_ids, new_ring_data);
        }
    }
    
    return typex_sets[real_set_name];
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

string typex::map_rand_parm_to_set_name(string& rand_param)
{
    string result = get_default_set_name();
    
    if ((rand_param == RAND_PARM_Y269) || (rand_param == RAND_PARM_PLUGS_Y269))
    {
        result = Y269;
    }

    if (rand_param == RAND_PARM_PLUGS_Y2695)
    {
        result = Y2695;
    }
    
    if ((rand_param == RAND_PARM_SP02390) || (rand_param == RAND_PARM_PLUGS_SP02390))
    {
        result = DEFAULT_SET;
    }        
    
    return result;
}


bool typex::randomize(string& param)
{
    bool result = false;
    random_bit_source reverse_rotors(5);
    urandom_generator rand;    
    string known_rotors("abcdefg");
    string name_rotor_set = map_rand_parm_to_set_name(param);
    string ring_positions, rotor_positions, selected_rotors;
    string plugs = "";
    map<string, string> machine_conf;
        
    if (name_rotor_set == DEFAULT_SET)
    {
        known_rotors = "abcdefg";
    }

    if ((name_rotor_set == Y269) || (name_rotor_set == Y2695))
    {
        known_rotors = "abcdefghijklmn";
    }
    
    try
    {
        permutation rotor_selection_perm = permutation::get_random_permutation(rand, known_rotors.length());
        permutation reflector_perm = permutation::get_random_permutation(rand, 26);        
        ring_positions = rmsk::std_alpha()->get_random_string(5);
        rotor_positions = rmsk::std_alpha()->get_random_string(5);        
        
        for (unsigned int count = 0; count < 5; count++)
        {
            selected_rotors += known_rotors[rotor_selection_perm.encrypt(count)];
            selected_rotors += ((reverse_rotors.get_next_val() == 0) ? 'N' : 'R');
        }

        if ((param == RAND_PARM_PLUGS_Y269) || (param == RAND_PARM_PLUGS_SP02390) || (param == RAND_PARM_PLUGS_Y2695))
        {
            permutation perm = rmsk::std_alpha()->get_random_permutation(); 
            plugs = rmsk::std_alpha()->perm_as_string(perm);           
        }
        
        machine_conf[KW_TYPEX_ROTOR_SET] = name_rotor_set;
        machine_conf[KW_TYPEX_ROTORS] = selected_rotors;
        machine_conf[KW_TYPEX_RINGS] = ring_positions;
        machine_conf[KW_TYPEX_REFLECTOR] = rmsk::std_alpha()->perm_as_string(reflector_perm);
        machine_conf[KW_TYPEX_PLUGBOARD] = plugs;
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

void typex::save_additional_components(Glib::KeyFile& ini_file)
{
    enigma_family_base::save_additional_components(ini_file);
    vector<int> plugboard_perm;
    
    if (get_input_transform().get() != NULL)
    {
        for (unsigned int count = 0; count < 26; count++)
        {
            plugboard_perm.push_back((int)get_input_transform()->encrypt(count));
        }
        
        ini_file.set_integer_list(SECT_PLUGBOARD, KEY_PLUGBOARD, plugboard_perm);
    }
}

bool typex::load_additional_components(Glib::KeyFile& ini_file)
{
    bool result = enigma_family_base::load_additional_components(ini_file);
    vector<int> plugboard_perm;
    vector<unsigned int> entry_perm;
    
    if (!result)
    {        
        if ((ini_file.has_group(SECT_PLUGBOARD)) && (ini_file.has_key(SECT_PLUGBOARD, KEY_PLUGBOARD)))
        {                        
            plugboard_perm = ini_file.get_integer_list(SECT_PLUGBOARD, KEY_PLUGBOARD);
            
            result = plugboard_perm.size() != 26;
            
            if (!result)
            {
                for (unsigned int count = 0; count < 26; count++)
                {
                    entry_perm.push_back((unsigned int)plugboard_perm[count]);
                }
                
                set_input_transform(boost::shared_ptr<encryption_transform>(new permutation(entry_perm)));
            }
        }
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
    add_rotor_set(Y2695, typex_rotor_sets::get_rotor_set(Y2695));
    
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
    
    randomizer_params.push_back(randomizer_descriptor(RAND_PARM_SP02390, "Force rotor set SP02390 and no plugboard"));
    randomizer_params.push_back(randomizer_descriptor(RAND_PARM_Y269, "Force rotor set Y269 and no plugboard"));
    randomizer_params.push_back(randomizer_descriptor(RAND_PARM_PLUGS_SP02390, "Include plugboard and rotor set SP02390"));
    randomizer_params.push_back(randomizer_descriptor(RAND_PARM_PLUGS_Y269, "Include plugboard and rotor set Y269"));
    randomizer_params.push_back(randomizer_descriptor(RAND_PARM_PLUGS_Y2695, "Include plugboard and rotor set Y269 with five notches"));            
    
    unvisualized_rotor_names.insert(ETW);        
    unvisualized_rotor_names.insert(UMKEHRWALZE);  
    
    get_stepping_gear()->reset();
}


