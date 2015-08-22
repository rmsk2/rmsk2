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

/*! \file enigma_uhr.cpp
 *  \brief Implementation of the Enigma Uhr
 */ 

#include<enigma_uhr.h>

enigma_uhr::enigma_uhr()
    : encryption_transform()
{
    // Source of the permutation given below: German wikipedia article on Enigma-Uhr 
    // Permutation of scrambler disc that specifies how the inner contacts are connected
    // to the outer contacts
    unsigned int scrambler_data[] = {26, 11, 24, 21,  2, 31,  0, 25, 30, 39, 28, 13, 22, 35, 20, 37,  6, 23,  4, 33,
                                     34, 19, 32,  9, 18,  7, 16, 17, 10,  3,  8,  1, 38, 27, 36, 29, 14, 15, 12,  5};
    
    unsigned int perm_inner_contacts[] = {1, 4, 7, 9, 6, 3, 0, 2, 5, 8};
    
    boost::shared_ptr<permutation> scrambler_perm(new permutation(scrambler_data, 40));    
    // Change permutation to specify how the outer contacts are connected to the inner ones
    scrambler_perm->switch_to_inverse();
    permutation help_perm(perm_inner_contacts, 10);
    scrambler_disk.set_perm(scrambler_perm);
    scrambler_disk.set_displacement_var(&dial_pos);
    
    inner_contacts = help_perm;
    dial_pos = 0;    
}

unsigned int enigma_uhr::encrypt(unsigned int in_char)
{
    unsigned int result = in_char;
    map<unsigned int, unsigned int>::iterator iter;

    // Is in_char steckered to a red plug?
    if ((iter = outer_plugs.find(in_char)) != outer_plugs.end())
    {
        // Yes! Current enters on thick wire of outer contact iter->second * 4 and passes through
        // the scrambler disc.
        result = scrambler_disk.encrypt(iter->second * 4);
        // reduce the number of the resulting inner contact to the number of the black/white inner plug
        result = (result - 2) / 4;
        result = inner_contacts.decrypt(result);
        // Determine to which character the inner thin output wire is steckered
        result = inner_plugs_inv[result];        
    }
    
    // Is in_char steckered to a black/white plug?
    if ((iter = inner_plugs.find(in_char)) != inner_plugs.end())
    {
        // Yes! Current enters on thick wire of inner contact inner_contacts.encrypt(iter->second) * 4 
        // and passes through the scrambler disc.        
        result = scrambler_disk.decrypt(inner_contacts.encrypt(iter->second) * 4);
        // reduce the number of the resulting outer contact to the number of the red outer plug
        result = (result - 2) / 4;
        // Determine to which character the outer thin output wire is steckered        
        result = outer_plugs_inv[result];            
    }
    
    // If in_char is neither steckered to a red plug nor to a black/white plug in_char is returned
    // unchanged    
    
    return result;
}

unsigned int enigma_uhr::decrypt(unsigned int in_char)
{
    unsigned int result = in_char;
    map<unsigned int, unsigned int>::iterator iter;

    // Is in_char steckered to a red plug?    
    if ((iter = outer_plugs.find(in_char)) != outer_plugs.end())
    {
        // Yes! Current enters on thin wire of outer contact (iter->second * 4) + 2 and passes through
        // the scrambler disc.
        result = scrambler_disk.encrypt((iter->second * 4) + 2);
        // reduce the number of the resulting inner contact to the number of the black/white inner plug
        result = result / 4;
        result = inner_contacts.decrypt(result);
        // Determine to which character the inner thick output wire is steckered        
        result = inner_plugs_inv[result];        
    }

    // Is in_char steckered to a black/white plug?    
    if ((iter = inner_plugs.find(in_char)) != inner_plugs.end())
    {
        // Yes! Current enters on thin wire of inner contact (inner_contacts.encrypt(iter->second) * 4) + 2
        // and passes through the scrambler disc.        
        result = scrambler_disk.decrypt((inner_contacts.encrypt(iter->second) * 4) + 2);
        // reduce the number of the resulting outer contact to the number of the red outer plug        
        result = result / 4;
        // Determine to which character the outer thick output wire is steckered                
        result = outer_plugs_inv[result];            
    }    
    
    // If in_char is neither steckered to a red plug nor to a black/white plug in_char is returned
    // unchanged
    
    return result;
}

void enigma_uhr::set_cabling(vector<pair<char, char> >& plugs)
{
    rmsk::simple_assert(plugs.size() != 10, "The Uhr requires exactly 10 plugs");
    string cable_spec;
    
    for (unsigned int count = 0; count < plugs.size(); count++)
    {
        cable_spec += plugs[count].first;
        cable_spec += plugs[count].second;        
    }
    
    set_cabling(cable_spec);
}

void enigma_uhr::set_cabling(string& cable_spec)
{
    rmsk::simple_assert(cable_spec.length() != 20, "The Uhr requires exactly 10 plugs");
    unsigned int in_char;
    
    cable_settings = cable_spec;
    inner_plugs.clear();
    outer_plugs.clear();
    inner_plugs_inv.clear();
    outer_plugs_inv.clear();

    
    for (unsigned int count = 0; count < 10; count++)
    {
        in_char = rmsk::std_alpha()->from_val(cable_spec[2 * count]);
        outer_plugs[in_char] = count;
        outer_plugs_inv[count] = in_char;

        in_char =  rmsk::std_alpha()->from_val(cable_spec[(2 * count) + 1]);
        inner_plugs[in_char] = count;
        inner_plugs_inv[count] = in_char; 
    }
}

void enigma_uhr::get_cabling(string& cable_spec)
{
    cable_spec = cable_settings;
}

