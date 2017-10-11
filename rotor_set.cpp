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
        
        ini_file.set_boolean(section_name, "isconst", (const_ids.count(iter->first) != 0));
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
    set<unsigned int> const_ids_temp;   
    
    
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
                    
                    if (!result)
                    {
                        // It is an error if the key "isconst" does not exist
                        result = !ini_file.has_key(section_name, "isconst");
                        
                        // If not an error ....
                        if (!result)
                        {
                            if (ini_file.get_boolean(section_name, "isconst"))
                            {
                                const_ids_temp.insert(id_list[count]);
                            }
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
        const_ids = const_ids_temp;
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

void rotor_set::replace_permutations(random_generator *rand_gen)
{
    vector<unsigned int> temp;
    urandom_generator urand_gen;
    map<unsigned int, vector<unsigned int> > new_perms;
    bool perm_found = false;
    permutation new_perm;
    set<pair<unsigned int, unsigned int> > test_cycles;
    permutation identity = permutation::get_identity(get_rotor_size());
    vector<unsigned int> identity_data;
    
    identity.to_vec(identity_data);
    
    if (rand_gen == NULL)
    {
        rand_gen = &urand_gen;
    }

    for (const auto& iter : perms)
    {        
        do
        {
            if (const_ids.count(iter.first) != 0)
            {
                perm_found = true;
                // Dont's change const id permutations
                new_perm = permutation(iter.second);
            }
            else
            {
                new_perm = permutation::get_random_permutation(*rand_gen, get_rotor_size());            
                
                permutation test_perm(iter.second);
                test_perm.test_for_involution(test_cycles);
                
                if (test_cycles.size() != 0)
                {
                    // The original permutaton is in fact an involution. Replace it by another involution.
                    perm_found = true;
                    
                    // Transform permutation into involution
                    new_perm.to_vec(temp);
                    vector<unsigned int> inv_temp(get_rotor_size(), 0);
                    
                    for (unsigned int count = 0; count < (get_rotor_size() / 2); count++)
                    {
                        inv_temp[temp[2 * count]] = temp[(2 * count) + 1];
                        inv_temp[temp[(2 * count) + 1]] = temp[2 * count];
                    }
                    
                    new_perm = permutation(inv_temp);
                }
                else
                {
                    // Normal permutation. Replace it by a (more or less) random permutation.
                    perm_found = rand_perm_helper::is_fix_point_free(new_perm) && (rand_perm_helper::num_of_single_shifts(new_perm) == 0);
                }
            }
                        
        } while (!perm_found);
        
        new_perm.to_vec(temp);
        new_perms[iter.first] = temp;        
    }
    
    perms = new_perms;
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

void rotor_set::change_ids(map<unsigned int, unsigned int>& rotor_id_mapping, map<unsigned int, unsigned int>& ring_id_mapping)
{
    map<unsigned int, unsigned int>::iterator rotor_iter, ring_iter;
    map<unsigned int, vector<unsigned int> > new_perms;
    map<unsigned int, vector<unsigned int> > new_ring_data_vals;
    
    
    for (rotor_iter = rotor_id_mapping.begin(); rotor_iter != rotor_id_mapping.end(); ++rotor_iter)
    {
        if (perms.count(rotor_iter->first) != 0)
        {
            new_perms[rotor_iter->second] = perms[rotor_iter->first];
        }
    }
    
    for (ring_iter = ring_id_mapping.begin(); ring_iter != ring_id_mapping.end(); ++ring_iter)
    {
        if (ring_data_vals.count(ring_iter->first) != 0)
        {
            new_ring_data_vals[ring_iter->second] = ring_data_vals[ring_iter->first];
        }
    }
    
    perms = new_perms;
    ring_data_vals = new_ring_data_vals;
}

void rotor_set::slice_rotor_set(rotor_set& new_set, vector<unsigned int>& rotor_ids_to_copy, vector<unsigned int>& ring_ids_to_copy)
{
    rmsk::simple_assert((new_set.get_rotor_size() != get_rotor_size()), "rotor size of target rotor set is different from the rotor size of the source");
    vector<unsigned int>::iterator rotor_iter, ring_iter;       
    
    new_set.clear();
    
    for (rotor_iter = rotor_ids_to_copy.begin(); rotor_iter != rotor_ids_to_copy.end(); ++rotor_iter)
    {
        if (perms.count(*rotor_iter) != 0)
        {
            new_set.perms[*rotor_iter] = perms[*rotor_iter];
        }
    }
    
    for (ring_iter = ring_ids_to_copy.begin(); ring_iter != ring_ids_to_copy.end(); ++ring_iter)
    {
        if (ring_data_vals.count(*ring_iter) != 0)
        {
            new_set.ring_data_vals[*ring_iter] = ring_data_vals[*ring_iter];
        }
    }
}

