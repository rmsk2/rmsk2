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

/*! \file rotor_set.cpp
 *  \brief Implementation of the rotor_set class.
 */ 

#include<boost/lexical_cast.hpp>

#include<glibmm/iochannel.h>
#include<rotor_set.h>
#include<rmsk_globals.h>
#include<algorithm>

#define GENERAL "general"


void rotor_set::get_ids(vector<unsigned int>& ids)
{
    map<unsigned int, vector<unsigned int> >::iterator iter;
    
    ids.clear();
    
    for (iter = perms.begin(); iter != perms.end(); ++iter)
    {
        ids.push_back(iter->first);
    }
    
    sort(ids.begin(), ids.end());
}

void rotor_set::get_ring_ids(vector<unsigned int>& ids)
{
    map<unsigned int, vector<unsigned int> >::iterator iter;
    
    ids.clear();
    
    for (iter = ring_data_vals.begin(); iter != ring_data_vals.end(); ++iter)
    {
        ids.push_back(iter->first);
    }
    
    sort(ids.begin(), ids.end());
}

/*! If the ring_id contained in the r_id parameter is unknown in the context of this rotor_set the 
 *  ring data of the returned ring is all zero. 
 */    
pair<boost::shared_ptr<rotor>, boost::shared_ptr<rotor_ring> > rotor_set::make_rotor(rotor_id  r_id)
{
    rmsk::simple_assert(perms.count(r_id.r_id) == 0, "Unknown rotor ID");
    
    boost::shared_ptr<rotor> rot;
    boost::shared_ptr<permutation> rotor_perm;
    boost::shared_ptr<rotor_ring> ring;
    // Default ring data is all zeros
    vector<unsigned int> rotor_data(get_rotor_size(), 0);
    vector<unsigned int> inverse_help;
    
    // Create rotor permutation specified by rotor id
    rotor_perm = boost::shared_ptr<permutation>(new permutation(perms[r_id.r_id]));
    
    if (r_id.insert_inverse)
    {
        // Transform rotor permutation so that it matches a rotor that has been inserted in reverse
        for (unsigned int count = 0; count < get_rotor_size(); count++)
        {
            simple_mod_int contact_pos_in(count, get_rotor_size());
            simple_mod_int contact_pos_out(rotor_perm->decrypt(-contact_pos_in), get_rotor_size());
            
            inverse_help.push_back(-contact_pos_out);
        }
        
        rotor_perm = boost::shared_ptr<permutation>(new permutation(inverse_help));            
    }
    
    // Create rotor
    rot = boost::shared_ptr<rotor>(new rotor());
    rot->set_perm(rotor_perm);
    
    // ring id known?
    if (ring_data_vals.count(r_id.ring_id) != 0)
    {
        // Yes! Retrieve ring data and overwrite the all zeros default
        rotor_data = ring_data_vals[r_id.ring_id];
    }

    // Create ring and associate it with the newly created rotor
    ring = boost::shared_ptr<rotor_ring>(new rotor_ring(rot.get(), rotor_data));
    pair<boost::shared_ptr<rotor>, boost::shared_ptr<rotor_ring> > result(rot, ring);    

    return result;    
}

boost::shared_ptr<rotor_ring> rotor_set::make_ring(unsigned int ring_id)
{    
    boost::shared_ptr<rotor_ring> result;
    vector<unsigned int> rotor_data(get_rotor_size(), 0);
        
    if (ring_data_vals.count(ring_id) != 0)
    {
        rotor_data = ring_data_vals[ring_id];
    }

    result = boost::shared_ptr<rotor_ring>(new rotor_ring(rotor_data));

    return result;
}

void rotor_set::save_ini(Glib::KeyFile& ini_file)
{
    map<unsigned int, vector<unsigned int> >::iterator iter;
    vector<int> data_temp;
    vector<unsigned int> all_ids;
    string section_name;
    
    get_ids(all_ids);
    
    for (unsigned int count = 0; count < all_ids.size(); count++)
    {
        data_temp.push_back((int)(all_ids[count]));
    }    
    
    ini_file.set_integer_list(GENERAL, "ids", data_temp);
    
    for (iter = perms.begin(); iter != perms.end(); ++iter)
    {
        section_name = "rotorid_" + boost::lexical_cast<string>(iter->first);
    
        data_temp.clear();
        
        for (unsigned int count = 0; count < get_rotor_size(); count++)
        {
            data_temp.push_back((int)((iter->second)[count]));
        }
        
        ini_file.set_integer_list(section_name, "permutation", data_temp);
        
        if (ring_data_vals.count(iter->first) != 0)
        {
            data_temp.clear();
            
            for (unsigned int count = 0; count < get_rotor_size(); count++)
            {
                data_temp.push_back((int)(ring_data_vals[iter->first][count]));
            }
            
            ini_file.set_integer_list(section_name, "ringdata", data_temp);            
        }
    }    
}

bool rotor_set::load_ini(Glib::KeyFile& ini_file)
{
    bool result = false;
    vector<int> data_temp, id_list;
    vector<unsigned int> perm_data, ring_data;
    string section_name;
    map<unsigned int, vector<unsigned int> > perms_temp;
    map<unsigned int, vector<unsigned int> > ring_data_vals_temp;  
    
    
    if (!(result = !ini_file.has_key(GENERAL, "ids")))
    {    
        id_list = ini_file.get_integer_list(GENERAL, "ids");
        
        for (unsigned int count = 0; (count < (unsigned int)id_list.size()) && (!result); count++)
        {
            section_name = "rotorid_" + boost::lexical_cast<string>((unsigned int)id_list[count]);
            
            if (!(result = !ini_file.has_key(section_name, "permutation")))
            {
                data_temp = ini_file.get_integer_list(section_name, "permutation");
                
                if (!(result = (data_temp.size() != get_rotor_size())))
                {
                    perm_data.clear();
                    
                    for (unsigned elem_count = 0; elem_count < get_rotor_size(); elem_count++)
                    {
                        perm_data.push_back((unsigned int)data_temp[elem_count]);
                    }
                    
                    perms_temp[id_list[count]] = perm_data;
                    
                    if (ini_file.has_key(section_name, "ringdata"))
                    {
                        data_temp = ini_file.get_integer_list(section_name, "ringdata");
                        
                        if (!(result = (data_temp.size() != get_rotor_size())))
                        {
                            ring_data.clear();
                            
                            for (unsigned elem_count = 0; elem_count < get_rotor_size(); elem_count++)
                            {
                                ring_data.push_back((unsigned int)data_temp[elem_count]);
                            }
                            
                            ring_data_vals_temp[id_list[count]] = ring_data;                        
                        }                    
                    }
                }                               
            }        
        }
    }
    
    if ((!result) and (id_list.size() > 0))
    {
        perms = perms_temp;
        ring_data_vals = ring_data_vals_temp;
    }
    
    return result;
}

bool rotor_set::load(string& file_name)
{
    return rmsk::settings_file_load(file_name, sigc::mem_fun(*this, &rotor_set::load_ini));
}

bool rotor_set::save(string& file_name)
{
    return rmsk::settings_file_save(file_name, sigc::mem_fun(*this, &rotor_set::save_ini));
}

void rotor_set::get_rand_ring_data(vector<unsigned int>& new_vals, rotor_ring_random_source *rand_source)
{
    new_vals.clear();
    
    for (unsigned int count = 0; count < get_rotor_size(); count++)
    {
        new_vals.push_back(rand_source->get_next_val());
    }
    
    rand_source->reset();
}

void rotor_set::get_rand_perm_data(vector<unsigned int>& new_perm_vals, random_generator *rand_gen)
{
    new_perm_vals.clear();
    permutation new_perm = permutation::get_random_permutation(*rand_gen, get_rotor_size());
    
    for (unsigned int count = 0; count < get_rotor_size(); count++)
    {
        new_perm_vals.push_back(new_perm.permute(count));
    }
}

void rotor_set::randomize(unsigned int num_rotors, rotor_ring_random_source *rand_source)
{
    vector<unsigned int> desired_ids;
    
    for (unsigned int count = 0; count < num_rotors; count++)
    {
        desired_ids.push_back(count);
    }
    
    randomize(desired_ids, rand_source);
}

void rotor_set::randomize(vector<unsigned int> desired_rotor_ids, rotor_ring_random_source *rand_source)
{
    perms.clear();
    ring_data_vals.clear();
    rmsk::simple_assert(((rand_source != NULL) ? rand_source->get_rotor_size() != get_rotor_size() : false), "rotor size of randsource does not match");
    // The default in the random_bit_source class is to use /dev/urandom for randomness
    // and create vectors of zeros and ones as ring data
    random_bit_source dummy_source(get_rotor_size());
    vector<unsigned int> temp;
    vector<unsigned int>::iterator iter;

    // Use /dev/urandom if no "real" rotor_ring_random_source was specified
    if (rand_source == NULL)
    {
        rand_source = &dummy_source;
    }

    for (iter = desired_rotor_ids.begin(); iter != desired_rotor_ids.end(); ++iter)
    {
        // Choose random permutation
        get_rand_perm_data(temp, rand_source->get_random_generator());
        perms[*iter] = temp;
        // Choose random ring data
        get_rand_ring_data(temp, rand_source);
        ring_data_vals[*iter] = temp;
    }
}

/*! Beware: This method not only adds a rotor to this rotor_set but also a rotor ring that uses all zero ring data 
 *  and desired_id as its ring id.
 */ 
void rotor_set::add_rotor(unsigned int desired_id, vector<unsigned int> permutation)
{
    vector<unsigned int> ring_data(get_rotor_size(), 0);
    
    add_rotor_and_ring(desired_id, permutation, ring_data);
}

void rotor_set::add_rotor_and_ring(unsigned int desired_id, vector<unsigned int> permutation, vector<unsigned int> ring_data)
{
    rmsk::simple_assert( (permutation.size() != get_rotor_size()) || (ring_data.size() != get_rotor_size()) , "rotor or ring data size wrong");
    
    perms[desired_id] = permutation;
    ring_data_vals[desired_id] = ring_data; 
}

/*! Beware: This method not only adds a ring to this rotor_set but also a rotor that uses the identity permutation 
 *  as rotor permutation and desired_id as its rotor id.
 */ 
void rotor_set::add_ring(unsigned int desired_id, vector<unsigned int> ring_data)
{
    vector<unsigned int> identity_help;
    
    permutation::get_identity(get_rotor_size()).to_vec(identity_help);
    add_rotor_and_ring(desired_id, identity_help, ring_data);
}

