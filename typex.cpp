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

/*! \file typex.cpp
 *  \brief Contains an implementation of a simulator for the Typex machine.
 */

#include<stdexcept>
#include<set>
#include<rmsk_globals.h>

#include<typex.h>

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

typex::typex(unsigned int ukw_id, rotor_id slow_id, rotor_id middle_id, rotor_id fast_id, rotor_id stat2_id, rotor_id stat1_id) 
    : enigma_base() 
{ 
    vector<string> rotor_names;
    vector<gunichar> norm_alph, shift_alph, out_alph;
    
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
    
    get_stepping_gear()->reset();
}


