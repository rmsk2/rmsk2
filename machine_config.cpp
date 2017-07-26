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

/*! \file machine_config.cpp
 *  \brief Implements a class that describes the characteristics of the Enigma variants simulated by this software.
 */ 

#include<boost/lexical_cast.hpp>
#include<boost/regex.hpp>
#include<memory>
#include<machine_config.h>
#include<enigma_sim.h>
#include<rand_gen.h>
#include<configurator.h>

/*! \brief A simple structure that can be used to hold a rotor's position (as shown in its corresponding window), its ring setting
 *         and its index with respect to the contents of rotor_family_descriptor.rotor_names 
 */
struct temp_settings {
    char rotor_pos, ring_setting;
    unsigned int active_rotor;
};

machine_config::machine_config()
{
    make_m4_config();
}

enigma_base *machine_config::make_machine(Glib::ustring& subtype)
{
    enigma_base *enigma;

    if ((subtype == "M3") or (subtype == "Services"))
    {
        enigma = new enigma_I(UKW_B, WALZE_I, WALZE_II, WALZE_III, subtype == "M3");
    }
    else
    {
        if (subtype == "Abwehr")
        {
            enigma = new abwehr_enigma(WALZE_ABW_I, WALZE_ABW_II, WALZE_ABW_III);    
        }
        else
        {
            if (subtype == "Railway")
            {
                enigma = new railway_enigma(WALZE_RB_I, WALZE_RB_II, WALZE_RB_III);
            }
            else
            {
                if (subtype == "Tirpitz")
                {
                    enigma = new tirpitz_enigma(WALZE_T_I, WALZE_T_II, WALZE_T_III);
                }
                else
                {
                    if (subtype == "KD")
                    {
                        enigma = new kd_enigma(WALZE_KD_I, WALZE_KD_II, WALZE_KD_III);
                    }
                    else                   
                    {                                        
                        enigma = new enigma_M4(UKW_B_DN, WALZE_BETA, WALZE_I, WALZE_II, WALZE_III);
                    }
                }
            }
        }
    }
    
    return enigma;
}

void machine_config::extract_rotor_names(map<Glib::ustring, unsigned int>& source, vector<string>& extracted_names)
{
    map<Glib::ustring, unsigned int>::iterator iter;
    
    extracted_names.clear();
    
    for (iter = source.begin(); iter != source.end(); ++iter)
    {
        extracted_names.push_back(iter->first);
    }        
}

void machine_config::make_m4_config()
{
    all_descriptors.clear();
    inserted_plugs.clear();
    ukw.rotor_names.clear();
    ukw.available_rotors.clear();    
    slow.rotor_names.clear();
    slow.available_rotors.clear();    
    middle.rotor_names.clear();
    middle.available_rotors.clear();    
    fast.rotor_names.clear();
    fast.available_rotors.clear();    
    greek.rotor_names.clear();
    greek.available_rotors.clear();    

    has_plugboard = true;
    wheels_are_numeric = false;  
    uses_uhr = false;
    uses_schreibmax = false;
    // The plugs of the Uhr did not fit in M4s plugboard sockets
    uhr_capable = false;
    uhr_dial_pos = 0;
    string ukwd_help("fowulaqysrtezvbxgjikdncphm");
    vector<unsigned int> ukwd_data = rmsk::std_alpha()->to_vector(ukwd_help);
    permutation ukwd_perm_help(ukwd_data);
    ukwd_perm = ukwd_perm_help;
    machine_type = "M4";  
          
    ukw.wheel_identifier = UMKEHRWALZE;
    ukw.available_rotors["UKW B dünn"] = UKW_B_DN;
    ukw.available_rotors["UKW C dünn"] = UKW_C_DN;
    extract_rotor_names(ukw.available_rotors, ukw.rotor_names);
    ukw.ring_setting = 'A';
    ukw.rotor_pos = 'A';
    ukw.rotor_index_active = 0;
    ukw.rotor_selection_state = true;
    ukw.ring_selection_state = false;
    ukw.has_rotor_window = false;

    greek.wheel_identifier = GRIECHENWALZE;
    greek.available_rotors["Beta"] = WALZE_BETA;
    greek.available_rotors["Gamma"] = WALZE_GAMMA;
    extract_rotor_names(greek.available_rotors, greek.rotor_names);
    greek.ring_setting = 'A';
    greek.rotor_pos = 'V';
    greek.rotor_index_active = 0;
    greek.rotor_selection_state = true;
    greek.ring_selection_state = true;
    greek.has_rotor_window = true;

    slow.wheel_identifier = SLOW;
    slow.available_rotors["WALZE I"] = WALZE_I;
    slow.available_rotors["WALZE II"] = WALZE_II;
    slow.available_rotors["WALZE III"] = WALZE_III;
    slow.available_rotors["WALZE IV"] = WALZE_IV;
    slow.available_rotors["WALZE V"] = WALZE_V;
    slow.available_rotors["WALZE VI"] = WALZE_VI;
    slow.available_rotors["WALZE VII"] = WALZE_VII;
    slow.available_rotors["WALZE VIII"] = WALZE_VIII;
    extract_rotor_names(slow.available_rotors, slow.rotor_names);    
    slow.ring_setting = 'A';
    slow.rotor_pos = 'J';
    slow.rotor_index_active = 1;
    slow.rotor_selection_state = true;
    slow.ring_selection_state = true;
    slow.has_rotor_window = true;

    middle.wheel_identifier = MIDDLE;    
    middle.available_rotors["WALZE I"] = WALZE_I;
    middle.available_rotors["WALZE II"] = WALZE_II;
    middle.available_rotors["WALZE III"] = WALZE_III;
    middle.available_rotors["WALZE IV"] = WALZE_IV;
    middle.available_rotors["WALZE V"] = WALZE_V;
    middle.available_rotors["WALZE VI"] = WALZE_VI;
    middle.available_rotors["WALZE VII"] = WALZE_VII;
    middle.available_rotors["WALZE VIII"] = WALZE_VIII;   
    extract_rotor_names(middle.available_rotors, middle.rotor_names); 
    middle.ring_setting = 'A';
    middle.rotor_pos = 'N';
    middle.rotor_index_active = 3;
    middle.rotor_selection_state = true;
    middle.ring_selection_state = true;
    middle.has_rotor_window = true;

    fast.wheel_identifier = FAST;    
    fast.available_rotors["WALZE I"] = WALZE_I;
    fast.available_rotors["WALZE II"] = WALZE_II;
    fast.available_rotors["WALZE III"] = WALZE_III;
    fast.available_rotors["WALZE IV"] = WALZE_IV;
    fast.available_rotors["WALZE V"] = WALZE_V;
    fast.available_rotors["WALZE VI"] = WALZE_VI;
    fast.available_rotors["WALZE VII"] = WALZE_VII;
    fast.available_rotors["WALZE VIII"] = WALZE_VIII;
    extract_rotor_names(fast.available_rotors, fast.rotor_names);    
    fast.ring_setting = 'V';
    fast.rotor_pos = 'A';
    fast.rotor_index_active = 0;
    fast.rotor_selection_state = true;
    fast.ring_selection_state = true;
    fast.has_rotor_window = true;

    inserted_plugs.push_back(pair<char, char>('a', 't'));
    inserted_plugs.push_back(pair<char, char>('b', 'l'));
    inserted_plugs.push_back(pair<char, char>('d', 'f'));
    inserted_plugs.push_back(pair<char, char>('g', 'j'));
    inserted_plugs.push_back(pair<char, char>('h', 'm'));                
    inserted_plugs.push_back(pair<char, char>('n', 'w'));
    inserted_plugs.push_back(pair<char, char>('o', 'p'));
    inserted_plugs.push_back(pair<char, char>('q', 'y'));
    inserted_plugs.push_back(pair<char, char>('r', 'z'));
    inserted_plugs.push_back(pair<char, char>('v', 'x'));        
    
    all_descriptors.push_back(fast);
    all_descriptors.push_back(middle);
    all_descriptors.push_back(slow);
    all_descriptors.push_back(greek);
    all_descriptors.push_back(ukw);    
}

void machine_config::make_config(Glib::ustring& machine_name)
{
    make_m4_config();

    do
    {
        if (machine_name == "M4 Schreibmax")
        {
            uses_schreibmax = true;  
            break;
        }
    
        // Differences between M4 and M3 configuration
        if (machine_name == "M3")
        {
            // M3 used no greek wheel
            all_descriptors[3].rotor_selection_state = false;
            all_descriptors[3].ring_selection_state = false;
            
            all_descriptors[4].rotor_names.clear();
            all_descriptors[4].available_rotors.clear();
            
            // M3 used "normal" UKWs in contrast to the thin UKWs of M4
            all_descriptors[4].available_rotors["UKW B"] = UKW_B;
            all_descriptors[4].available_rotors["UKW C"] = UKW_C;
            all_descriptors[4].available_rotors["UKW D"] = UKW_D;
            extract_rotor_names(all_descriptors[4].available_rotors, all_descriptors[4].rotor_names);            
            
            all_descriptors[0].ring_setting = 'M';
            all_descriptors[0].rotor_pos = 'V';
            all_descriptors[0].rotor_index_active = 7;

            all_descriptors[1].ring_setting = 'H';
            all_descriptors[1].rotor_pos = 'Z';
            all_descriptors[1].rotor_index_active = 5;
            
            all_descriptors[2].ring_setting = 'A';
            all_descriptors[2].rotor_pos = 'U';
            all_descriptors[2].rotor_index_active = 2;
                        
            inserted_plugs.clear();
            
            inserted_plugs.push_back(pair<char, char>('a', 'n'));
            inserted_plugs.push_back(pair<char, char>('e', 'z'));
            inserted_plugs.push_back(pair<char, char>('h', 'k'));
            inserted_plugs.push_back(pair<char, char>('i', 'j'));
            inserted_plugs.push_back(pair<char, char>('l', 'r'));                
            inserted_plugs.push_back(pair<char, char>('m', 'q'));
            inserted_plugs.push_back(pair<char, char>('o', 't'));
            inserted_plugs.push_back(pair<char, char>('p', 'v'));
            inserted_plugs.push_back(pair<char, char>('s', 'w'));
            inserted_plugs.push_back(pair<char, char>('u', 'x')); 
            
            //uhr_capable = true;       
                        
            machine_type = machine_name;
            break;
        }

        // Differences between M4 and Services configuration
        if (machine_name == "Services")
        {
            // Services Enigma used no greek wheel
            all_descriptors[3].rotor_selection_state = false;
            all_descriptors[3].ring_selection_state = false;
            
            all_descriptors[4].rotor_names.clear();
            all_descriptors[4].available_rotors.clear();
            
            // Services Enigma used "normal" UKWs in contrast to the thin UKWs of M4
            all_descriptors[4].available_rotors["UKW B"] = UKW_B;
            all_descriptors[4].available_rotors["UKW C"] = UKW_C;
            all_descriptors[4].available_rotors["UKW D"] = UKW_D;
            extract_rotor_names(all_descriptors[4].available_rotors, all_descriptors[4].rotor_names);                    

            all_descriptors[0].rotor_names.clear();
            all_descriptors[0].available_rotors.clear();
            all_descriptors[1].rotor_names.clear();
            all_descriptors[1].available_rotors.clear();
            all_descriptors[2].rotor_names.clear();
            all_descriptors[2].available_rotors.clear();
            
            // Services Enigma only used five wheels
            all_descriptors[0].available_rotors["WALZE I"] = WALZE_I;
            all_descriptors[0].available_rotors["WALZE II"] = WALZE_II;
            all_descriptors[0].available_rotors["WALZE III"] = WALZE_III;
            all_descriptors[0].available_rotors["WALZE IV"] = WALZE_IV;
            all_descriptors[0].available_rotors["WALZE V"] = WALZE_V;
            extract_rotor_names(all_descriptors[0].available_rotors, all_descriptors[0].rotor_names);

            // Services Enigma only used five wheels
            all_descriptors[1].available_rotors["WALZE I"] = WALZE_I;
            all_descriptors[1].available_rotors["WALZE II"] = WALZE_II;
            all_descriptors[1].available_rotors["WALZE III"] = WALZE_III;
            all_descriptors[1].available_rotors["WALZE IV"] = WALZE_IV;
            all_descriptors[1].available_rotors["WALZE V"] = WALZE_V;
            extract_rotor_names(all_descriptors[1].available_rotors, all_descriptors[1].rotor_names);

            // Services Enigma only used five wheels
            all_descriptors[2].available_rotors["WALZE I"] = WALZE_I;
            all_descriptors[2].available_rotors["WALZE II"] = WALZE_II;
            all_descriptors[2].available_rotors["WALZE III"] = WALZE_III;
            all_descriptors[2].available_rotors["WALZE IV"] = WALZE_IV;
            all_descriptors[2].available_rotors["WALZE V"] = WALZE_V;
            extract_rotor_names(all_descriptors[2].available_rotors, all_descriptors[2].rotor_names);

            all_descriptors[0].ring_setting = 'H';
            all_descriptors[0].rotor_pos = 'Z';
            all_descriptors[0].rotor_index_active = 2;

            all_descriptors[1].ring_setting = 'Z';
            all_descriptors[1].rotor_pos = 'T';
            all_descriptors[1].rotor_index_active = 3;
            
            all_descriptors[2].ring_setting = 'P';
            all_descriptors[2].rotor_pos = 'R';
            all_descriptors[2].rotor_index_active = 0;
            
            
            inserted_plugs.clear();
            
            inserted_plugs.push_back(pair<char, char>('a', 'd'));
            inserted_plugs.push_back(pair<char, char>('c', 'n'));
            inserted_plugs.push_back(pair<char, char>('e', 't'));
            inserted_plugs.push_back(pair<char, char>('f', 'l'));
            inserted_plugs.push_back(pair<char, char>('g', 'i'));                
            inserted_plugs.push_back(pair<char, char>('j', 'v'));
            inserted_plugs.push_back(pair<char, char>('k', 'z'));
            inserted_plugs.push_back(pair<char, char>('p', 'u'));
            inserted_plugs.push_back(pair<char, char>('q', 'y'));
            inserted_plugs.push_back(pair<char, char>('w', 'x'));        
                                
            machine_type = machine_name;
            // Services Enigma used numbers on the rings
            wheels_are_numeric = true;
            // Services Enigma was able to use the Uhr
            uhr_capable = true;
            break;
        }

        if (machine_name == "Abwehr")
        {
            // Abwehr Enigma used no greek wheel
            all_descriptors[3].rotor_selection_state = false;
            all_descriptors[3].ring_selection_state = false;        
            machine_type = machine_name;
            // Abwehr Enigma had no plugboard
            has_plugboard = false;
            inserted_plugs.clear();
            
            all_descriptors[4].rotor_names.clear();
            all_descriptors[4].available_rotors.clear();
            
            // Abwehr Enigma had only one UKW
            all_descriptors[4].rotor_names.push_back("UKW");
            all_descriptors[4].available_rotors["UKW"] = UKW_ABW;
            all_descriptors[4].ring_selection_state = true;
            // but the UKW was settable by hand
            all_descriptors[4].has_rotor_window = true;
            
            all_descriptors[0].rotor_names.clear();
            all_descriptors[0].available_rotors.clear();
            all_descriptors[1].rotor_names.clear();
            all_descriptors[1].available_rotors.clear();
            all_descriptors[2].rotor_names.clear();
            all_descriptors[2].available_rotors.clear();
            
            // Abwehr Enigma had only three wheels to choose from
            all_descriptors[0].available_rotors["WALZE I"] = WALZE_ABW_I;
            all_descriptors[0].available_rotors["WALZE II"] = WALZE_ABW_II;
            all_descriptors[0].available_rotors["WALZE III"] = WALZE_ABW_III;
            extract_rotor_names(all_descriptors[0].available_rotors, all_descriptors[0].rotor_names);

            // Abwehr Enigma had only three wheels to choose from
            all_descriptors[1].available_rotors["WALZE I"] = WALZE_ABW_I;
            all_descriptors[1].available_rotors["WALZE II"] = WALZE_ABW_II;
            all_descriptors[1].available_rotors["WALZE III"] = WALZE_ABW_III;
            extract_rotor_names(all_descriptors[1].available_rotors, all_descriptors[1].rotor_names);

            // Abwehr Enigma had only three wheels to choose from
            all_descriptors[2].available_rotors["WALZE I"] = WALZE_ABW_I;
            all_descriptors[2].available_rotors["WALZE II"] = WALZE_ABW_II;
            all_descriptors[2].available_rotors["WALZE III"] = WALZE_ABW_III;
            extract_rotor_names(all_descriptors[2].available_rotors, all_descriptors[2].rotor_names);

            all_descriptors[0].ring_setting = 'A';
            all_descriptors[0].rotor_pos = 'A';
            all_descriptors[0].rotor_index_active = 0;

            all_descriptors[1].ring_setting = 'A';
            all_descriptors[1].rotor_pos = 'A';
            all_descriptors[1].rotor_index_active = 1;
            
            all_descriptors[2].ring_setting = 'A';
            all_descriptors[2].rotor_pos = 'A';
            all_descriptors[2].rotor_index_active = 2;            
            
            break;
        }

        if (machine_name == "Railway")
        {
            // Railway Enigma used no greek wheel
            all_descriptors[3].rotor_selection_state = false;
            all_descriptors[3].ring_selection_state = false;        
            machine_type = machine_name;
            // Railway Enigma had no plugboard
            has_plugboard = false;
            inserted_plugs.clear();
            
            all_descriptors[4].rotor_names.clear();
            all_descriptors[4].available_rotors.clear();
            
            // Railway Enigma had only one UKW
            all_descriptors[4].rotor_names.push_back("UKW");
            all_descriptors[4].available_rotors["UKW"] = UKW_RB;
            all_descriptors[4].ring_selection_state = true;
            // but the UKW was settable by hand
            all_descriptors[4].has_rotor_window = true;
            
            all_descriptors[0].rotor_names.clear();
            all_descriptors[0].available_rotors.clear();
            all_descriptors[1].rotor_names.clear();
            all_descriptors[1].available_rotors.clear();
            all_descriptors[2].rotor_names.clear();
            all_descriptors[2].available_rotors.clear();
            
            // Railway Enigma had only three wheels to choose from
            all_descriptors[0].available_rotors["WALZE I"] = WALZE_RB_I;
            all_descriptors[0].available_rotors["WALZE II"] = WALZE_RB_II;
            all_descriptors[0].available_rotors["WALZE III"] = WALZE_RB_III;
            extract_rotor_names(all_descriptors[0].available_rotors, all_descriptors[0].rotor_names);

            // Railway Enigma had only three wheels to choose from
            all_descriptors[1].available_rotors["WALZE I"] = WALZE_RB_I;
            all_descriptors[1].available_rotors["WALZE II"] = WALZE_RB_II;
            all_descriptors[1].available_rotors["WALZE III"] = WALZE_RB_III;
            extract_rotor_names(all_descriptors[1].available_rotors, all_descriptors[1].rotor_names);

            // Railway Enigma had only three wheels to choose from
            all_descriptors[2].available_rotors["WALZE I"] = WALZE_RB_I;
            all_descriptors[2].available_rotors["WALZE II"] = WALZE_RB_II;
            all_descriptors[2].available_rotors["WALZE III"] = WALZE_RB_III;
            extract_rotor_names(all_descriptors[2].available_rotors, all_descriptors[2].rotor_names);

            all_descriptors[0].ring_setting = 'A';
            all_descriptors[0].rotor_pos = 'A';
            all_descriptors[0].rotor_index_active = 2;

            all_descriptors[1].ring_setting = 'A';
            all_descriptors[1].rotor_pos = 'A';
            all_descriptors[1].rotor_index_active = 1;
            
            all_descriptors[2].ring_setting = 'A';
            all_descriptors[2].rotor_pos = 'A';
            all_descriptors[2].rotor_index_active = 0;            
            
            break;
        }

        if (machine_name == "Tirpitz")
        {
            // Tirpitz Enigma used no greek wheel
            all_descriptors[3].rotor_selection_state = false;
            all_descriptors[3].ring_selection_state = false;        
            machine_type = machine_name;
            // Tirpitz Enigma had no plugboard
            has_plugboard = false;
            inserted_plugs.clear();
            
            all_descriptors[4].rotor_names.clear();
            all_descriptors[4].available_rotors.clear();
            
            // Tirpitz Enigma had only one UKW
            all_descriptors[4].rotor_names.push_back("UKW");
            all_descriptors[4].available_rotors["UKW"] = UKW_T;
            all_descriptors[4].ring_selection_state = true;
            // but the UKW was settable by hand
            all_descriptors[4].has_rotor_window = true;
            
            all_descriptors[0].rotor_names.clear();
            all_descriptors[0].available_rotors.clear();
            all_descriptors[1].rotor_names.clear();
            all_descriptors[1].available_rotors.clear();
            all_descriptors[2].rotor_names.clear();
            all_descriptors[2].available_rotors.clear();
            
            // Tirpitz Enigma had eight wheels to choose from
            all_descriptors[0].available_rotors["WALZE I"] = WALZE_T_I;
            all_descriptors[0].available_rotors["WALZE II"] = WALZE_T_II;
            all_descriptors[0].available_rotors["WALZE III"] = WALZE_T_III;
            all_descriptors[0].available_rotors["WALZE IV"] = WALZE_T_IV;
            all_descriptors[0].available_rotors["WALZE V"] = WALZE_T_V;
            all_descriptors[0].available_rotors["WALZE VI"] = WALZE_T_VI;
            all_descriptors[0].available_rotors["WALZE VII"] = WALZE_T_VII;
            all_descriptors[0].available_rotors["WALZE VIII"] = WALZE_T_VIII;
            extract_rotor_names(all_descriptors[0].available_rotors, all_descriptors[0].rotor_names);            
            
            // Tirpitz Enigma had eight wheels to choose from
            all_descriptors[1].available_rotors["WALZE I"] = WALZE_T_I;
            all_descriptors[1].available_rotors["WALZE II"] = WALZE_T_II;
            all_descriptors[1].available_rotors["WALZE III"] = WALZE_T_III;
            all_descriptors[1].available_rotors["WALZE IV"] = WALZE_T_IV;
            all_descriptors[1].available_rotors["WALZE V"] = WALZE_T_V;
            all_descriptors[1].available_rotors["WALZE VI"] = WALZE_T_VI;
            all_descriptors[1].available_rotors["WALZE VII"] = WALZE_T_VII;
            all_descriptors[1].available_rotors["WALZE VIII"] = WALZE_T_VIII;
            extract_rotor_names(all_descriptors[1].available_rotors, all_descriptors[1].rotor_names);

            // Tirpitz Enigma had eight wheels to choose from
            all_descriptors[2].available_rotors["WALZE I"] = WALZE_T_I;
            all_descriptors[2].available_rotors["WALZE II"] = WALZE_T_II;
            all_descriptors[2].available_rotors["WALZE III"] = WALZE_T_III;
            all_descriptors[2].available_rotors["WALZE IV"] = WALZE_T_IV;
            all_descriptors[2].available_rotors["WALZE V"] = WALZE_T_V;
            all_descriptors[2].available_rotors["WALZE VI"] = WALZE_T_VI;
            all_descriptors[2].available_rotors["WALZE VII"] = WALZE_T_VII;
            all_descriptors[2].available_rotors["WALZE VIII"] = WALZE_T_VIII;   
            extract_rotor_names(all_descriptors[2].available_rotors, all_descriptors[2].rotor_names);         

            all_descriptors[0].ring_setting = 'B';
            all_descriptors[0].rotor_pos = 'M';
            all_descriptors[0].rotor_index_active = 4;

            all_descriptors[1].ring_setting = 'R';
            all_descriptors[1].rotor_pos = 'F';
            all_descriptors[1].rotor_index_active = 7;
            
            all_descriptors[2].ring_setting = 'Q';
            all_descriptors[2].rotor_pos = 'C';
            all_descriptors[2].rotor_index_active = 6;            
            
            all_descriptors[4].ring_setting = 'K';
            all_descriptors[4].rotor_pos = 'A';
            all_descriptors[4].rotor_index_active = 0;            
                        
            break;
        }
        
        if (machine_name == "KD")
        {
            // KD Enigma used no greek wheel
            all_descriptors[3].rotor_selection_state = false;
            all_descriptors[3].ring_selection_state = false;        
            machine_type = machine_name;
            // KD Enigma had no plugboard
            has_plugboard = false;
            inserted_plugs.clear();
            
            all_descriptors[4].rotor_names.clear();
            all_descriptors[4].available_rotors.clear();
            
            // KD Enigma only used UKW D
            all_descriptors[4].rotor_names.push_back("UKW D");
            all_descriptors[4].available_rotors["UKW D"] = UKW_D;
            all_descriptors[4].ring_selection_state = false;
            all_descriptors[4].has_rotor_window = false;
            
            all_descriptors[0].rotor_names.clear();
            all_descriptors[0].available_rotors.clear();
            all_descriptors[1].rotor_names.clear();
            all_descriptors[1].available_rotors.clear();
            all_descriptors[2].rotor_names.clear();
            all_descriptors[2].available_rotors.clear();
                        
            // KD Enigma had six wheels to choose from
            all_descriptors[0].available_rotors["WALZE I"] = WALZE_KD_I;
            all_descriptors[0].available_rotors["WALZE II"] = WALZE_KD_II;
            all_descriptors[0].available_rotors["WALZE III"] = WALZE_KD_III;
            all_descriptors[0].available_rotors["WALZE IV"] = WALZE_KD_IV;
            all_descriptors[0].available_rotors["WALZE V"] = WALZE_KD_V;
            all_descriptors[0].available_rotors["WALZE VI"] = WALZE_KD_VI;
            extract_rotor_names(all_descriptors[0].available_rotors, all_descriptors[0].rotor_names);

            // KD Enigma had six wheels to choose from
            all_descriptors[1].available_rotors["WALZE I"] = WALZE_KD_I;
            all_descriptors[1].available_rotors["WALZE II"] = WALZE_KD_II;
            all_descriptors[1].available_rotors["WALZE III"] = WALZE_KD_III;
            all_descriptors[1].available_rotors["WALZE IV"] = WALZE_KD_IV;
            all_descriptors[1].available_rotors["WALZE V"] = WALZE_KD_V;
            all_descriptors[1].available_rotors["WALZE VI"] = WALZE_KD_VI;
            extract_rotor_names(all_descriptors[1].available_rotors, all_descriptors[1].rotor_names);
            
            // KD Enigma had six wheels to choose from
            all_descriptors[2].available_rotors["WALZE I"] = WALZE_KD_I;
            all_descriptors[2].available_rotors["WALZE II"] = WALZE_KD_II;
            all_descriptors[2].available_rotors["WALZE III"] = WALZE_KD_III;
            all_descriptors[2].available_rotors["WALZE IV"] = WALZE_KD_IV;
            all_descriptors[2].available_rotors["WALZE V"] = WALZE_KD_V;
            all_descriptors[2].available_rotors["WALZE VI"] = WALZE_KD_VI;
            extract_rotor_names(all_descriptors[2].available_rotors, all_descriptors[2].rotor_names);

            all_descriptors[0].ring_setting = 'B';
            all_descriptors[0].rotor_pos = 'M';
            all_descriptors[0].rotor_index_active = 4;

            all_descriptors[1].ring_setting = 'R';
            all_descriptors[1].rotor_pos = 'F';
            all_descriptors[1].rotor_index_active = 5;
            
            all_descriptors[2].ring_setting = 'Q';
            all_descriptors[2].rotor_pos = 'C';
            all_descriptors[2].rotor_index_active = 1;            
                                    
            break;
        }
        

    } while(0);
}

unsigned int machine_config::get_active_rotor_id(rotor_family_descriptor& desc)
{    
    return desc.available_rotors[desc.rotor_names[desc.rotor_index_active]];
}

bool machine_config::configure_machine(enigma_base *machine, string& rotor_set_name)
{
    // Machine type has to match the type of this configuration
    bool result = (machine->get_machine_type () != machine_type);    
        
    steckered_enigma *e = dynamic_cast<steckered_enigma *>(machine);
    
    if (!result)
    {
        machine->get_stepping_gear()->reset();
    
        // Iterate over all rotor slots in conf
        for (unsigned int count = 0; count < get_all_descriptors().size(); count++)
        {
            // Do something only if the rotor slot number count is in use in this machine as determined by conf
            if (get_desc_at(count).rotor_selection_state)
            {
                // Replace rotor in slot number count by new one as prescribed by the configuration contained in conf
                machine->prepare_rotor(rotor_set_name, get_active_rotor_id(get_desc_at(count)), get_desc_at(count).wheel_identifier);
                
                // If UKW D is in use as the reflector replace the reflector with a new one constructed from the current value in conf
                if (get_active_rotor_id(get_desc_at(count)) == UKW_D)
                {
                    boost::shared_ptr<permutation> new_reflector(new permutation(get_ukw_d_perm()));                
                    machine->get_stepping_gear()->get_descriptor(UMKEHRWALZE).r->set_perm(new_reflector);                            
                }
                
                // Change ringstellung to the one determined by conf if the ring in slot number count is settable           
                if (get_desc_at(count).ring_selection_state)
                {
                    machine->get_enigma_stepper()->set_ringstellung(get_desc_at(count).wheel_identifier, tolower(get_desc_at(count).ring_setting));
                }
                
                // Change rotor position to the one specified in conf
                machine->get_enigma_stepper()->set_rotor_pos(get_desc_at(count).wheel_identifier, tolower(get_desc_at(count).rotor_pos));
            }        
        } 
        
        if (get_has_plugboard())
        {
            e->set_stecker_brett(get_inserted_plugs(), get_uses_uhr());
                    
            if (get_uses_uhr())
            {
                e->get_uhr()->set_dial_pos(get_uhr_dial_pos());
            }
        }                   
    }    
    
    return result;    
}

bool machine_config::get_config(enigma_base *machine)
{
    bool result = false;
    
    Glib::ustring machine_type = machine->get_machine_type();
    make_config(machine_type);    
    steckered_enigma *e = dynamic_cast<steckered_enigma *>(machine);
    enigma_stepper_base *stepper = machine->get_enigma_stepper();
    unsigned int active_index_found;
    
    // Do we have a Steckerbrett?
    if (get_has_plugboard())
    {
        // Is Uhr in use?
        get_uses_uhr() = e->uses_uhr();        
    } 
    
    for (unsigned int count = 0; (count < get_all_descriptors().size()) and (!result); count++)
    {
        rotor_family_descriptor &family_desc = get_desc_at(count);
        // Do something only if the rotor slot number count is in use in this machine configuration
        if (family_desc.rotor_selection_state)
        {
            // determine slot identifer
            string identifier = family_desc.wheel_identifier;
            // determine rotor_descriptor inserted into machine
            rotor_descriptor &stepper_desc = stepper->get_descriptor(identifier);            
            // Search rotor id currently in use in the machine in the ids allowed for the current slot and machine type
            result = find_rotor_index(family_desc, stepper_desc.id.r_id, active_index_found);
            
            if (result)
            {
                continue;
            }
            
            family_desc.rotor_index_active = active_index_found;
                        
            // If UKW D is in use retrieve permutation and store it in instance variable ukwd_perm.
            if (stepper_desc.id.r_id == UKW_D)
            {
                vector<unsigned int> temp;
                stepper->get_descriptor(UMKEHRWALZE).r->get_perm()->to_vec(temp);
                
                ukwd_perm = permutation(temp);
            }
            
            // Retrieve ringstellung
            if (family_desc.ring_selection_state)
            {
                family_desc.ring_setting = toupper(stepper->get_ringstellung(identifier));
            }
            
            // Retrieve rotor position
            family_desc.rotor_pos = toupper(stepper->get_rotor_pos(identifier));
        }        
    }
    
    if ((!result) and get_has_plugboard())
    {
        e->get_stecker_brett(inserted_plugs);
        
        if (get_uses_uhr())
        {
            uhr_dial_pos = e->get_uhr()->get_dial_pos();
        }        
    } 
    
    return result;   
}

void machine_config::get_keywords(vector<key_word_info>& infos)
{
    infos.clear();
    
    // All Enigma variants have to be configured with rotor and ring settings
    infos.push_back(key_word_info(KW_ENIG_ROTOR_SELECTION, KEY_STRING, "Enigma rotors"));
    infos.push_back(key_word_info(KW_ENIG_RINGSTELLUNG, KEY_STRING, "Enigma ring settings"));    
    
    // Only the military variants Services, M3 and M4 have a plugboard
    if (has_plugboard)
    {
        infos.push_back(key_word_info(KW_ENIG_STECKERBRETT, KEY_STRING, "Enigma Steckerbrett"));
        
        // Only Services can use the Uhr
        if (uhr_capable)
        {            
            infos.push_back(key_word_info(KW_USES_UHR, KEY_BOOL, "Uses Uhr"));
        }
    }
    
    // Only Services, M3 and KD can use UKW D
    if (is_ukw_d_capable())
    {
        infos.push_back(key_word_info(KW_UKW_D_PERM, KEY_STRING, "UKW D permutation"));
    }
}

void machine_config::to_keywords(map<string, string>& config_data)
{
    string rotor_selection, ring_settings, ukw_d_perm_str, plugs;
    
    config_data.clear();

    // Generate rotor and ring setting keyword information
    for (unsigned int count = 0; count < get_all_descriptors().size(); count++)
    {
        rotor_family_descriptor &family_desc = get_desc_at(count);
        
        // Do something only if the rotor slot number count is in use in this machine configuration
        if (family_desc.rotor_selection_state)
        {
            // Only include rotor selection info if there really is a choice
            if (family_desc.rotor_names.size() > 1)
            {
                rotor_selection += (char)(family_desc.rotor_index_active + '1');                
            }
            
            // Include ringstellung even if there is only one choice for a rotor
            if (family_desc.ring_selection_state)
            {
                ring_settings += tolower(family_desc.ring_setting);
            }            
        }        
    }

    config_data[KW_ENIG_ROTOR_SELECTION] = string(rotor_selection.rbegin(), rotor_selection.rend());
    config_data[KW_ENIG_RINGSTELLUNG] = string(ring_settings.rbegin(), ring_settings.rend());

    // Generate plugboard keyword information
    if (has_plugboard)
    {
        vector<pair<char, char> >::iterator iter;
        
        for (iter = inserted_plugs.begin(); iter != inserted_plugs.end(); ++iter)
        {
            plugs += iter->first;
            plugs += iter->second;
        }
        
        if (uhr_capable)
        {
            if (uses_uhr)
            {
                config_data[KW_USES_UHR] = CONF_TRUE;
                plugs = boost::lexical_cast<string>(uhr_dial_pos) + ':' + plugs;
            }
            else
            {
                config_data[KW_USES_UHR] = CONF_FALSE;
            }            
        }
        
        config_data[KW_ENIG_STECKERBRETT] = plugs;
    }

    // Generate UKW D keyword information
    if (is_ukw_d_capable())
    {
        // Only Services, M3 and KD can use UKW D
        vector<pair<char, char> > res = ukw_d_wiring_helper::perm_to_plugs(ukwd_perm);
        vector<pair<char, char> >::iterator iter;
        
        for (iter = res.begin(); iter != res.end(); ++iter)
        {
            if ((iter->first != 'j') && (iter->second != 'y'))
            {
                ukw_d_perm_str += iter->first;
                ukw_d_perm_str += iter->second;
            }
        }
        
        config_data[KW_UKW_D_PERM] = ukw_d_perm_str;
    }
}

bool machine_config::from_keywords(map<string, string>& config_data, string& enigma_model)
{
    bool result = false;
    unsigned int count_rotor_specifiers = 0, count_ring_specifiers = 0;
    string rotor_selection, ring_settings, plugs, ukwd_plugs_str;
    unsigned int index_to_test, uhr_dialpos = 100, rotor_selection_pos_count = 0, ring_settings_pos_count = 0;;
    bool uhr_specified = false;
    vector<pair<char, char> > ukwd_plugs;
    Glib::ustring temp_mname = enigma_model;
    
    do
    {
        if ((result = ((config_data.count(KW_ENIG_ROTOR_SELECTION) == 0) || (config_data.count(KW_ENIG_RINGSTELLUNG) == 0))))
        {
            break;    
        }
        
        make_config(temp_mname);
        
        // determine the necessary length of rotor and ring settings
        for (unsigned int count = 0; count < get_all_descriptors().size(); count++)
        {
            rotor_family_descriptor &family_desc = get_desc_at(count);
            
            if (family_desc.rotor_selection_state)
            {
                // Only include rotor selection info if there really is a choice
                if (family_desc.rotor_names.size() > 1)
                {
                    count_rotor_specifiers++;
                }
                
                // Include ringstellung even if there is only one choice for a rotor
                if (family_desc.ring_selection_state)
                {
                    count_ring_specifiers++;
                }            
            }        
        }        
        
        rotor_selection = config_data[KW_ENIG_ROTOR_SELECTION];
        ring_settings = config_data[KW_ENIG_RINGSTELLUNG];
        
        // Check for correct length and composition
        result = !configurator::check_rotor_spec(rotor_selection, '1', '9', count_rotor_specifiers, false); // rotor specifiers are not unique for M3, Services and M4
        result = result || !configurator::check_rotor_spec(ring_settings, 'a', 'z', count_ring_specifiers, false);
        
        if (result)
        {
            break;
        }        
        
        // ************ ToDo: Check for rotors which have been inserted twice
        // reverse rotor and ring settings
        rotor_selection = string(rotor_selection.rbegin(), rotor_selection.rend());
        ring_settings = string(ring_settings.rbegin(), ring_settings.rend());

        // Modify rotor selection and ring settings of this machine_configuration instance
        for (unsigned int count = 0; (count < get_all_descriptors().size()) && (!result); count++)
        {
            rotor_family_descriptor &family_desc = get_desc_at(count);
            
            if (family_desc.rotor_selection_state)
            {
                // Only include rotor selection info if there really is a choice
                if (family_desc.rotor_names.size() > 1)
                {
                    index_to_test = rotor_selection[rotor_selection_pos_count] - '1';
                    
                    if ((result = (index_to_test >= family_desc.rotor_names.size())))
                    {
                        continue;
                    }
                    
                    family_desc.rotor_index_active = index_to_test;
                    rotor_selection_pos_count++;
                }
                
                // Include ringstellung even if there is only one choice for a rotor
                if (family_desc.ring_selection_state)
                {
                    family_desc.ring_setting = toupper(ring_settings[ring_settings_pos_count]);
                    ring_settings_pos_count++;
                }
                
                // reset rotor position
                family_desc.rotor_pos = 'A';            
            }            
        }

        if (result)
        {
            break;
        }        
        
        // Check and modify plugboard information
        if (has_plugboard)
        {
            boost::regex plugboard_exp("^([0-9]{1, 2}):([a-z]{2, 26})$");
            boost::cmatch match_plugboard;
        
            if ((result = (config_data.count(KW_ENIG_STECKERBRETT) == 0)))
            {
                break;
            }                                    
            
            plugs = config_data[KW_ENIG_STECKERBRETT];
            
            // Do we have a dial pos specification?
            if (boost::regex_match(plugs.c_str(), match_plugboard, plugboard_exp))
            {
                // Yes
                uhr_specified = true;
                uhr_dialpos = boost::lexical_cast<unsigned int>(match_plugboard[1].str()); // Can not fail
                plugs = match_plugboard[2].str();
            }
            
            // There has to be an even number of plugs
            if ((result = ((plugs.length() % 2) == 1)))
            {
                break;
            }                                    
            
            // Check for uniqueness
            if ((result = !configurator::check_pin_spec(plugs, 'a', 'z', 26)))
            {
                break;
            }            
            
            if (uhr_capable)
            {
                // We need a KW_USES_UHR entry
                if ((result = config_data.count(KW_USES_UHR) == 0))
                {
                    break;
                }                
                
                uses_uhr = (config_data[KW_USES_UHR] == CONF_TRUE);                              
                
                if ((result = (uses_uhr != uhr_specified)))
                {
                    break;
                }               
                
                if ((result = (uses_uhr && ((uhr_dialpos >= 40) || (plugs.length() != 20)))))
                {
                    break;
                }                
                
                uhr_dial_pos = uhr_dialpos;
            }
            else
            {
                // We are not uhr capable so there must be no dial pos information
                if ((result = uhr_specified))
                {
                    break;
                }
                
                uses_uhr = false;
            }
            
            inserted_plugs.clear();
            
            for (unsigned int count = 0; count < (plugs.length() / 2); count++)
            {
                inserted_plugs.push_back(pair<char, char>(plugs[2 * count], plugs[2 * count+ 1]));
            }            
        }
        
        if (is_ukw_d_capable())
        {
            if ((result = config_data.count(KW_UKW_D_PERM) == 0))
            {
                break;
            }
            
            ukwd_plugs_str = config_data[KW_UKW_D_PERM];
            
            // Check for correct length and uniqueness
            if ((result = ((ukwd_plugs_str.length() != 24) || (!configurator::check_pin_spec(ukwd_plugs_str, 'a', 'z', 24)))))
            {
                break;
            }
            
            ukwd_plugs.clear();
            
            // Check for forbidden values j and y and fill necessary data structures
            for (unsigned int count = 0; (count < 12) && (!result); count++)
            {
                if ((result = (ukwd_plugs_str[count] == 'j') || (ukwd_plugs_str[count] == 'y')))
                {
                    continue;
                }
                
                ukwd_plugs.push_back(pair<char, char>(ukwd_plugs_str[2 * count], ukwd_plugs_str[2 * count + 1]));
            }
            
            if (result)
            {
                break;
            }
            
            // Make permutation
            ukwd_plugs.push_back(pair<char, char>('j', 'y')); // Now add fixed connections
            ukwd_perm = ukw_d_wiring_helper::plugs_to_perm(ukwd_plugs);                        
        }
        
    } while(0);
    
    if (result)
    {
        make_config(temp_mname);
    }
    
    return result;
}

void machine_config::print(ostream& out)
{
    string rotor_selection, ringstellung, rotor_positions, plugs;

    out << machine_type << endl;
    out << "Has plugboard: " << has_plugboard << endl;
    
    if (has_plugboard)
    {
        out << "Uhr in use: " << uses_uhr << endl;    
    }
    
    // Iterate over all rotor slots in conf
    for (unsigned int count = 0; count < get_all_descriptors().size(); count++)
    {
        if (get_desc_at(count).rotor_selection_state)
        {            
            rotor_selection += get_desc_at(count).rotor_names[get_desc_at(count).rotor_index_active] + " ";
            
            if (get_desc_at(count).ring_selection_state)
            {
                ringstellung += get_desc_at(count).ring_setting;
            }
            
            if (get_desc_at(count).has_rotor_window)
            {            
                rotor_positions += get_desc_at(count).rotor_pos;
            }
        }        
    } 

    out << "Walzenlage: " << rotor_selection << endl;
    out << "Ringstellung: " << ringstellung << endl;
    out << "Rotor positions: " << rotor_positions << endl;
    
    if (get_has_plugboard())
    {
        vector<pair<char, char> >::iterator iter;
        
        for (iter = inserted_plugs.begin(); iter != inserted_plugs.end(); ++iter)
        {
            plugs += iter->first;
            plugs += iter->second;
        }
        
        out << "Stecker: " << plugs << endl;
                
        if (get_uses_uhr())
        {
            out << "Dial pos: " << uhr_dial_pos << endl;
        }
    }                   

    out << "UKW D permutation: ";

    for (unsigned int count = 0; count < 26; count++)
    {
        out << ukwd_perm.encrypt(count) << " "; 
    }

    out << endl;
}

vector<unsigned int> machine_config::make_random_wheel_order(unsigned int num_ukws, unsigned int num_rotors, unsigned int num_greeks)
{
    vector<unsigned int> result;
    urandom_generator gen;
    permutation perm;

    /* normal rotors */
    perm =  permutation::get_random_permutation(gen, num_rotors);
    result.push_back(perm.permute(0));
    result.push_back(perm.permute(1));
    result.push_back(perm.permute(2));
    
    /* greek rotors */
    perm =  permutation::get_random_permutation(gen, 10);
    result.push_back(perm.permute(0) & 1);
    
    /* reflectors */
    if (num_ukws > 1)
    {
        perm =  permutation::get_random_permutation(gen, 3 * num_ukws);
        result.push_back(perm.permute(0) % num_ukws);
    }
    else
    {
        result.push_back(0);
    }
    
    return result;
}

string machine_config::get_random_quintuple()
{
    string result;
    urandom_generator gen;
    permutation perm;
    
    for (unsigned int count = 0; count < 5; count++)
    {
        perm = permutation::get_random_permutation(gen, 26);
        result += (char)(perm.permute(0) + 'A');
    }
    
    return result;
}

void machine_config::make_random_ukw_d_wiring()
{
    urandom_generator gen;
    permutation temp;
    vector<unsigned int> help, new_ukwd_data;
    vector<pair<char, char> > plugs;
    
    temp = permutation::get_random_permutation(gen, 26);
    temp.to_vec(help);
    
    // Remove contacts UKWD_FIXED_CONTACT_Y and UKWD_FIXED_CONTACT_J from the random permutation
    for (unsigned int count = 0; count < 26; count++)
    {
        if ((help[count] != UKWD_FIXED_CONTACT_Y) and (help[count] != UKWD_FIXED_CONTACT_J))
        {
            new_ukwd_data.push_back(help[count]);
        }
    }
    
    // "Connect" contacts UKWD_FIXED_CONTACT_Y and UKWD_FIXED_CONTACT_J
    new_ukwd_data.push_back(UKWD_FIXED_CONTACT_Y);
    new_ukwd_data.push_back(UKWD_FIXED_CONTACT_J);
    
    // Interpret the data in new_ukw_data as the sequence of pairs of an involution 
    for (unsigned int count = 0; count < 13; count++)
    {
        plugs.push_back(pair<char, char>(rmsk::std_alpha()->to_val(new_ukwd_data[2 * count]), rmsk::std_alpha()->to_val(new_ukwd_data[(2 * count) + 1])));
    }
    
    ukwd_perm = rmsk::std_alpha()->make_involution(plugs);
}

void machine_config::make_random_involution(vector<pair<char, char> >& plugs, unsigned int num_plugs)
{
    urandom_generator gen;
    permutation perm;    

    perm = permutation::get_random_permutation(gen, 26);
    plugs.clear();
    
    // Interpret the values that are created by applying perm to 0..(2*num_plugs) - 1 as the sequence of pairs of an involution 
    for (unsigned int count = 0; count < num_plugs; count++)
    {
        plugs.push_back(pair<char, char>((char)(perm.permute(2 * count) + 'a') ,(char)(perm.permute((2 * count) + 1) + 'a')));
    }    
}

bool machine_config::randomize()
{
    bool result = false;
    vector<pair<char, char> > new_plugs;
    vector<temp_settings> temp;
    vector<unsigned int> new_wheel_order;
    string new_ring_settings, new_rotor_positions;
    
    // random_generator can throw an exception
    try
    {
        new_ring_settings = get_random_quintuple();
        new_rotor_positions = get_random_quintuple();
        new_wheel_order = make_random_wheel_order(all_descriptors[4].rotor_names.size(), all_descriptors[0].rotor_names.size(), 2);
        make_random_ukw_d_wiring();
        
        // First retrieve, then modify and then save the configuration in the variable temp
        for (unsigned int count = 0; count < all_descriptors.size(); count++)
        {
            temp_settings t;

            // retrieve
            t.active_rotor = all_descriptors[count].rotor_index_active;
            t.rotor_pos = all_descriptors[count].rotor_pos;
            t.ring_setting = all_descriptors[count].ring_setting;
            
            // modify
            if (all_descriptors[count].has_rotor_window)
            {
                t.rotor_pos = new_rotor_positions[count];
                t.ring_setting = new_ring_settings[count];
            }
            
            if (all_descriptors[count].rotor_selection_state)
            {
                t.active_rotor = new_wheel_order[count];
            }            
            
            // save
            temp.push_back(t);
        }
                    
        if (has_plugboard)
        {
            make_random_involution(new_plugs);
            
            if (uses_uhr)
            {
                urandom_generator gen;
                permutation perm;
                perm = permutation::get_random_permutation(gen, 40);
                // No exception was thrown up to this point, it is now safe to modify the configuration
                uhr_dial_pos = perm.permute(0);                
            }
            
            inserted_plugs = new_plugs;
        }
        
        // No exception was thrown. Write back the modified configuration.
        for (unsigned int count = 0; count < all_descriptors.size(); count++)
        {
            all_descriptors[count].rotor_pos = temp[count].rotor_pos;
            all_descriptors[count].ring_setting = temp[count].ring_setting;
            all_descriptors[count].rotor_index_active = temp[count].active_rotor;
        }        
    }
    catch(...)
    {
        result = true;
    }
    
    return result;
}

// false means no error occurred
bool machine_config::find_rotor_index(rotor_family_descriptor& to_search, unsigned int search_val, unsigned int& index_found)
{
    bool found = false;
    unsigned int count;
    
    for (count = 0; (count < to_search.rotor_names.size()) and (!found); count++)
    {
        if (to_search.available_rotors[to_search.rotor_names[count]] == search_val)
        {
            found = true;
            index_found = count;
        }
    }
    
    return !found;
}

// false means no error occurred
bool machine_config::load_settings(const Glib::ustring& file_name)
{
    bool result = false;
    Glib::KeyFile ini_file;
    vector<int> rotor_entries, raw_plugs;
    vector<temp_settings> temp;
    vector<pair<char, char> > temp_set; 
    set<char> test_set;
    vector<int> ukwd_wiring_help;
    vector<unsigned int> ukwd_wiring;    
    
    try
    {
        result = !ini_file.load_from_file(file_name);        
    
        do
        {
            if (result)
            {
                break;
            }
            
            // Create an enigma object and use it to read the general settings from the ini file
            unique_ptr<enigma_base> enigma(make_machine(get_machine_type()));
            result = enigma->load_ini(ini_file);
            
            if (result)
            {
                break;
            }            
                        
            if ((result = machine_type != ini_file.get_string("machine", "machinetype")))
            {
                break;
            }
                        
            if ((result = (has_plugboard and !ini_file.has_group("plugboard"))))
            {
                break;
            }            
            
            // Load data from enigma object into this configuration object
            for (unsigned int count = 0; (count < all_descriptors.size()) and !result; count++)
            {
                temp_settings help;
                
                // Is the rotor in use in this variant?
                if (all_descriptors[count].rotor_selection_state)
                {
                    // Yes
                    // Query enigma object for new values
                    rotor_descriptor&  desc = enigma->get_stepping_gear()->get_descriptor(all_descriptors[count].wheel_identifier);                
                    
                    help.rotor_pos =  ((char)(desc.ring->get_pos() % 26)) + 'A';
                    help.ring_setting = ((char)(desc.ring->get_offset() % 26)) + 'A';
                    // Determine index of desc.id.r_id in all_descriptors[count].rotor_names
                    result = find_rotor_index(all_descriptors[count], desc.id.r_id, help.active_rotor);
                    
                    if (!result)
                    {
                        temp.push_back(help);
                    }
                }
                else
                {
                    // No
                    // Copy existing values
                    help.rotor_pos = all_descriptors[count].rotor_pos;
                    help.ring_setting = all_descriptors[count].ring_setting;
                    help.active_rotor = all_descriptors[count].rotor_index_active;
                    temp.push_back(help);
                }
            }
            
            if (result)
            {
                break;
            }
            
            // Read UKW D wiring from ini file
            ukwd_wiring_help = ini_file.get_integer_list("machine", "ukwdwiring");
            
            if ((result = (ukwd_wiring_help.size() != rmsk::std_alpha()->get_size())))
            {
                break;
            }
            
            for (unsigned int count = 0; count < rmsk::std_alpha()->get_size(); count++)
            {
                ukwd_wiring.push_back(((unsigned int)(ukwd_wiring_help[count])) % rmsk::std_alpha()->get_size());
            }
            
            permutation ukwd_perm_help(ukwd_wiring);
            ukwd_perm = ukwd_perm_help;
            
            // Determine plugboard settings from enigma object and save them in this machine_config object
            if (has_plugboard)
            {
                // Cast has to work, because enigma->load_ini() was successfull
                steckered_enigma *steckered_machine = dynamic_cast<steckered_enigma *>(enigma.get());
                steckered_machine->get_stecker_brett(inserted_plugs);                               
                
                uses_uhr = steckered_machine->uses_uhr();
                if (uses_uhr)
                {
                    uhr_dial_pos = steckered_machine->get_uhr()->get_dial_pos();
                }
            }
            
            // It's now safe to write back rotor data into this machine_config object as all operations that may fail have 
            // been successfully completed at this point. 
            for (unsigned int count = 0; count < all_descriptors.size(); count++)
            {
                all_descriptors[count].rotor_pos = temp[count].rotor_pos;
                all_descriptors[count].ring_setting = temp[count].ring_setting;
                all_descriptors[count].rotor_index_active = temp[count].active_rotor;
            }            
        
        } while(0);    
    }
    catch(...)
    {
        result = true; 
    }
        
    return result;
}

bool machine_config::save_settings(const Glib::ustring& file_name, enigma_base *enigma)
{
    bool result = false;
    Glib::KeyFile ini_file;
    Glib::ustring ini_data;
    vector<int> rotor_entries, plugs;
    vector<pair<char, char> >::iterator iter;
    vector<int> ukwd_wiring;
    vector<unsigned int> ukwd_wiring_help;    
    
    enigma->save_ini(ini_file);
    
    // Replace ukwdwiring entry written by save_ini method
    ukwd_perm.to_vec(ukwd_wiring_help);
    
    for (unsigned int count = 0; count < ukwd_perm.get_size(); count++)
    {
        ukwd_wiring.push_back((int)(ukwd_wiring_help[count]));                
    }
    
    ini_file.set_integer_list("machine", "ukwdwiring", ukwd_wiring);
    
    ini_data = ini_file.to_data();
    
    // Save ini data in file specified by the file_name parameter.
    try
    {
        Glib::RefPtr<Glib::IOChannel> out_file = Glib::IOChannel::create_from_file(file_name.raw(), "w");    
        result = out_file->write(ini_data) != Glib::IO_STATUS_NORMAL;
        out_file->close();
    }
    catch(...)
    {
        result = true;
    }

    // false means no error occurred    
    return result;
}

/* ----------------------------------------------------------- */

void enigma_configurator::get_config(map<string, string>& config_data, rotor_machine *configured_machine)
{
    enigma_base *machine = dynamic_cast<enigma_base *>(configured_machine);    
    rmsk::simple_assert(machine == NULL, "programmer error: machine is not an Enigma model");
    
    rmsk::simple_assert(config.get_config(machine), "programmer error: can not retrieve machine configuration");
    config.to_keywords(config_data);
}

void enigma_configurator::get_keywords(vector<key_word_info>& infos)
{
    config.get_keywords(infos);
}

unsigned int enigma_configurator::configure_machine(map<string, string>& config_data, rotor_machine *machine_to_configure)
{
    unsigned int result = CONFIGURATOR_OK;
    enigma_base *machine = dynamic_cast<enigma_base *>(machine_to_configure);
    string enigma_model = string(machine_type.c_str());
    
    if (machine == NULL)
    {
        result = CONFIGURATOR_ERROR;
    }
    else
    {        
        if (config.from_keywords(config_data, enigma_model))
        {
            result = CONFIGURATOR_ERROR;
        }
        else
        {
            if (config.configure_machine(machine, rotor_set_name))
            {
                result = CONFIGURATOR_ERROR;
            }
        }
    }    
    
    return result;
}

rotor_machine *enigma_configurator::make_machine(map<string, string>& config_data)
{
    rotor_machine *result = config.make_machine(machine_type);
    
    if (result != NULL)
    {
        if (configure_machine(config_data, result) != CONFIGURATOR_OK)
        {
            delete result;
            result = NULL;
        }
    }
    
    return result;
}

