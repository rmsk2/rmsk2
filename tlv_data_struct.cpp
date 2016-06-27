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

#include<tlv_data_struct.h>

/*! \file tlv_data_struct.cpp
 *  \brief Implementation of helper classes that use the lower level tlv_entry class to provide more complex data structures.
 */
 
void tlv_map::to_tlv_entry(map<string, string>& data_struc)
{
    map<string, string>::iterator iter;
    vector<tlv_entry> key_value_pairs;

    for (iter = data_struc.begin(); iter != data_struc.end(); ++iter)
    {
        tlv_entry seq, key, value;
        vector<tlv_entry> key_value_pair;
        
        key.to_string(iter->first);
        value.to_string(iter->second);
        
        key_value_pair.push_back(key);
        key_value_pair.push_back(value);
        seq.to_sequence(key_value_pair);
        key_value_pairs.push_back(seq);                
    }
    
    elements.to_sequence(key_value_pairs);
}

void tlv_map::tlv_convert(map<string, string>& data_struc)
{
    data_struc.clear();
    vector<tlv_entry>::iterator iter;
    
    // No checks performed. It is assumed here that the encoding is valid. Validity is ensured by the set_elements() and 
    // to_tlv_entry() methods.
    (void)elements.parse_all();
    
    for (iter = elements.children.begin(); iter != elements.children.end(); ++iter)
    {
        (void)iter->parse_all();
        string key, value;
        
        (void)(iter->children[0]).tlv_convert(key);
        (void)(iter->children[1]).tlv_convert(value);
        
        data_struc[key] = value;        
    }
}

bool tlv_map::set_elements(tlv_entry& new_elements)
{
    bool result = !new_elements.parse_all();
    vector<tlv_entry>::iterator iter;
    string key, value;
    
    result = result || (new_elements.tag != TAG_SEQUENCE);
    
    if (!result)
    {
        for (iter = new_elements.children.begin(); (iter != new_elements.children.end()) && (!result); ++iter)
        {
            result = (iter->tag != TAG_SEQUENCE) || (!iter->parse_all());
            
            if (!result)
            {
                // Lazy evaluation ensures that we do not access invalid indices
                result = ((iter->children).size() != 2) || (!(iter->children[0]).tlv_convert(key)) || (!(iter->children[1]).tlv_convert(value));
            }
        }
    }
    
    if (!result)
    {
        elements.to_sequence(new_elements.children);
    }   
    
    return result;
}
