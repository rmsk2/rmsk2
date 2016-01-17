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

/*! \file machine_config.cpp
 *  \brief Implements a class that describes the characteristics of the Enigma variants simulated by this software.
 */ 

#include<machine_config.h>
#include<enigma_sim.h>
#include<rand_gen.h>

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
    string ukwd_help("nzxfsdvtrwyuqapomiehlgjckb");
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
            
            uhr_capable = true;       
                        
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
    enigma_base *enigma = NULL;
    
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
            enigma = make_machine(get_machine_type());
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
                steckered_enigma *steckered_machine = dynamic_cast<steckered_enigma *>(enigma);
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
    
    delete enigma;
        
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


