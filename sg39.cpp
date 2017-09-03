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

/*! \file sg39.cpp
 *  \brief This file contains the implementation of a simulator for the Schluesselgeraet 39
 */ 

#include<algorithm>
#include<exception>
#include<boost/scoped_ptr.hpp>
#include<boost/lexical_cast.hpp>
#include<rmsk_globals.h>
#include<alphabet.h>
#include<enigma_sim.h>
#include<sg39.h>
#include<configurator.h>

/*! \brief This constant specifies the maximum number of tries before randomization of a machine
 *         is considered to have failed.
 */
const unsigned int MAX_RAND_TRIES = 20;

#ifdef SG39_ASYMMETRIC

/*! \brief Output characters when doing decryptions and input characters for encryptions.
 */
static ustring str_plain_chars_ger =  "abcdefghijklmnop rstuvwxyz";

/*! \brief Output characters when doing encryptions and input characters when doing decryptions.
 */
static ustring str_cipher_chars_ger = "abcdefghijklmnopqrstuvwxyz";

#endif

/*! \brief This class is used to construct exception objects to signal errors when randomization
 *         of an SG39 state fails.
 */
class sg39_randomize_exception : public exception {
public:
    virtual const char* what() const throw()
    {
        return "Unable to randomize SG39 state";
    }
};

/*! \brief Definition of sg39_set */
rotor_set sg39_rotor_factory::sg39_set(rmsk::std_alpha()->get_size());

/*! \brief Definition of m4_set */
rotor_set sg39_rotor_factory::m4_set(rmsk::std_alpha()->get_size());

/*! \brief Definition of rotor id mapping */
map<unsigned int, unsigned int> sg39_rotor_factory::id_mapping = { {WALZE_I, SG39_ROTOR_1}, {WALZE_II, SG39_ROTOR_2}, {WALZE_III, SG39_ROTOR_3}, {WALZE_IV, SG39_ROTOR_4},
                                                                   {WALZE_V, SG39_ROTOR_5}, {WALZE_VI, SG39_ROTOR_6}, {WALZE_VII, SG39_ROTOR_7}, {WALZE_VIII, SG39_ROTOR_8},
                                                                   {WALZE_BETA, SG39_ROTOR_0}, {WALZE_GAMMA, SG39_ROTOR_9}, {UKW_B_DN, UKW_B_DN}, {UKW_C_DN, UKW_C_DN}
};


/*! The document describing the Schlüsselgerät 39 contained no info about the wiring of the rotors or
 *  how many rotors were provided. Below you find 10 random fix point free permutations.
 */
rotor_set& sg39_rotor_factory::get_rotor_set()
{
    vector<unsigned int> sg39_ring_data(26, 0);    
    
    sg39_ring_data[0] = 1;
    
    if (sg39_set.get_num_rotors() == 0)
    {        
        // Normal rotors
        sg39_set.add_rotor_and_ring(SG39_ROTOR_0, rmsk::std_alpha()->to_vector(string("iymhkeqgbdtuosajvzlwrfpcxn")), sg39_ring_data);
        sg39_set.add_rotor_and_ring(SG39_ROTOR_1, rmsk::std_alpha()->to_vector(string("nxlpymdvrzieohsafjqctkbguw")), sg39_ring_data);
        sg39_set.add_rotor_and_ring(SG39_ROTOR_2, rmsk::std_alpha()->to_vector(string("xomuvpktbyswalfhjndercizgq")), sg39_ring_data);
        sg39_set.add_rotor_and_ring(SG39_ROTOR_3, rmsk::std_alpha()->to_vector(string("ptrbhkwyqaeozumflgxvdijcns")), sg39_ring_data);
        sg39_set.add_rotor_and_ring(SG39_ROTOR_4, rmsk::std_alpha()->to_vector(string("kuzxvrmqycghftbwanldpoeisj")), sg39_ring_data);
        sg39_set.add_rotor_and_ring(SG39_ROTOR_5, rmsk::std_alpha()->to_vector(string("fslczopdmqeruhxkywiagntbjv")), sg39_ring_data);
        sg39_set.add_rotor_and_ring(SG39_ROTOR_6, rmsk::std_alpha()->to_vector(string("nljapsiukmogfvetzwxchqydbr")), sg39_ring_data);
        sg39_set.add_rotor_and_ring(SG39_ROTOR_7, rmsk::std_alpha()->to_vector(string("wklogxuzrheqbvcmfdjaynpist")), sg39_ring_data);
        sg39_set.add_rotor_and_ring(SG39_ROTOR_8, rmsk::std_alpha()->to_vector(string("jeoimprbavxqcsgnkywldtzuhf")), sg39_ring_data);
        sg39_set.add_rotor_and_ring(SG39_ROTOR_9, rmsk::std_alpha()->to_vector(string("xaryumpscfijzwktdgvonqbelh")), sg39_ring_data);
        
        // Reflector
        sg39_set.add_rotor(ID_SG39_UKW,  rmsk::std_alpha()->to_vector(string("ugvhpmbdolyjfqienwxzacrskt")));                     
    }
    
    return sg39_set;
}

rotor_set& sg39_rotor_factory::get_m4_rotor_set()
{
    vector<unsigned int> m4_ids = {WALZE_I, WALZE_II, WALZE_III, WALZE_IV, WALZE_V, WALZE_VI, WALZE_VII, WALZE_VIII, UKW_B_DN, UKW_C_DN, WALZE_BETA, WALZE_GAMMA};

    if (m4_set.get_num_rotors() == 0)
    {        
        rotor_set& enigma_set = enigma_rotor_factory::get_rotor_set();
        
        enigma_set.slice_rotor_set(m4_set, m4_ids, m4_ids);
        m4_set.change_ids(id_mapping, id_mapping);
        m4_set.add_rotor(ID_SG39_UKW, rmsk::std_alpha()->to_vector(string("ugvhpmbdolyjfqienwxzacrskt"))); 
    }
    
    return m4_set;
}

bool sg39_stepping_gear::wheel_is_at_notch(const char *identifier)
{
    return ((get_descriptor(identifier).binary_vals["wheeldata"])[get_descriptor(identifier).mod_int_vals["wheelpos"]] != 0);
}

void sg39_stepping_gear::advance_wheel(const char *identifier)
{
    get_descriptor(identifier).mod_int_vals["wheelpos"].increment();
}

void sg39_stepping_gear::step_rotors()
{
    stepping_gear::step_rotors();
    bool one_steps = false, two_steps = false, three_steps = false;
            
    one_steps = wheel_is_at_notch(ROTOR_1) or rotor_is_at_notch(ROTOR_3);

    two_steps = wheel_is_at_notch(ROTOR_2) or rotor_is_at_notch(ROTOR_1) or rotor_is_at_notch(ROTOR_2);

    three_steps = wheel_is_at_notch(ROTOR_3) or rotor_is_at_notch(ROTOR_2);

    if (one_steps)
    {
        advance_rotor(ROTOR_1);
    }

    if (two_steps)
    {
        advance_rotor(ROTOR_2);
    }

    if (three_steps)
    {
        advance_rotor(ROTOR_3);
    }
    
    advance_wheel(ROTOR_1);    
    advance_wheel(ROTOR_2);    
    advance_wheel(ROTOR_3);        
}

void sg39_stepping_gear::reset()
{
    unsigned int count = 0;
    
    stepping_gear::reset();
    
    for (count = 0; count < num_rotors; count++)
    {
        set_rotor_displacement(count, 0);        
        get_descriptor(count).ring->set_offset(0);
        get_descriptor(count).mod_int_vals["wheelpos"].set_val(0);
    }    
}

void sg39_stepping_gear::set_wheel_pos(const char *rotor_name, unsigned int new_pos)
{
    rmsk::simple_assert(rotors.count(rotor_name) == 0, "programmer error: rotor identifier unknown");
    
    get_descriptor(rotor_name).mod_int_vals["wheelpos"] = new_pos;
}

unsigned int sg39_stepping_gear::get_wheel_pos(const char *rotor_name)
{
    rmsk::simple_assert(rotors.count(rotor_name) == 0, "programmer error: rotor identifier unknown");
    
    return get_descriptor(rotor_name).mod_int_vals["wheelpos"];
}

void sg39_stepping_gear::set_wheel_data(const char *rotor_name, vector<unsigned int>& new_data)
{
    rmsk::simple_assert(rotors.count(rotor_name) == 0, "programmer error: rotor identifier unknown");
    rmsk::simple_assert(get_descriptor(rotor_name).mod_int_vals["wheelpos"].get_mod() > new_data.size(), "programmer error: wheel data too short");    
    
    get_descriptor(rotor_name).binary_vals["wheeldata"] = new_data;
}

void sg39_stepping_gear::get_wheel_data(const char *rotor_name, vector<unsigned int>& data)
{
    rmsk::simple_assert(rotors.count(rotor_name) == 0, "programmer error: rotor identifier unknown");
    
    data = get_descriptor(rotor_name).binary_vals["wheeldata"];    
}

bool sg39_stepping_gear::load_additional_components(string& identifier, Glib::KeyFile& ini_file)
{
    bool result = false;
    string section_name = "rotor_" + identifier;
    vector<int> temp_wheel_data; 
    vector<unsigned int> wheel_data, current_wheel_data;
    int temp_wheelpos;

    // load wheel data (only for the first three rotors)
    if ((identifier == ROTOR_1) or (identifier == ROTOR_2) or (identifier == ROTOR_3))
    {    
        do
        {
            if ((result = !ini_file.has_key(section_name, "wheelpos")))
            {
                break;
            }
            
            temp_wheelpos = ini_file.get_integer(section_name, "wheelpos");       
            
            if ((result = !ini_file.has_key(section_name, "wheeldata")))
            {
                break;
            }
            
            temp_wheel_data = ini_file.get_integer_list(section_name, "wheeldata");
            // Get current wheel data in order to check whether data from ini file has correct size
            get_wheel_data(identifier.c_str(), current_wheel_data);
            
            if ((result = (temp_wheel_data.size() != current_wheel_data.size())))
            {
                break;
            }
            
            // From this point on no error can occur. Make the necessary changes to this object.
            set_wheel_pos(identifier.c_str(), (unsigned int)temp_wheelpos);
            
            // Cast wheel data to unsigned int
            for (unsigned int count = 0; count < temp_wheel_data.size(); count++)
            {
                wheel_data.push_back((unsigned int)temp_wheel_data[count]);
            }
            
            set_wheel_data(identifier.c_str(), wheel_data);
        
        } while(0);    
    }
    
    return result;
}

void sg39_stepping_gear::save_additional_components(string& identifier, Glib::KeyFile& ini_file)
{
    string section_name = "rotor_" + identifier;
    vector<int> temp_wheel_data; 
    vector<unsigned int> wheel_data;
    
    // Save wheeldata and wheelpos currently in use (only for the first three rotors)
    if ((identifier == ROTOR_1) or (identifier == ROTOR_2) or (identifier == ROTOR_3))
    {
        ini_file.set_integer(section_name, "wheelpos", (int)get_wheel_pos(identifier.c_str()));
     
        get_wheel_data(identifier.c_str(), wheel_data); 
        // Cast wheel data to int  
        for (unsigned int count = 0; count < wheel_data.size(); count++)
        {
            temp_wheel_data.push_back((int)wheel_data[count]);
        }
        
        ini_file.set_integer_list(section_name, "wheeldata", temp_wheel_data);
    }
}

void schluesselgeraet39::fill_wheel_spec(randomize_help wheel_spec, unsigned int num_ones)
{
    bool spec_found = false;
    urandom_generator rand;       
    unsigned int try_count = 0, candidate_count = 0, one_count = 0; 
    string sorted_spec;
    char final_char = rmsk::std_alpha()->to_val(wheel_spec.size - 1);
    bool last_first_pins_not_consecutive = false;
    
    do
    {
        // clean candidate wheel spec
        permutation wheel_spec_perm = permutation::get_random_permutation(rand,  wheel_spec.size);
        wheel_spec.spec->clear();
        candidate_count = 0;
        one_count = 0;        
        
        // create candidate wheel spec
        while ((one_count < num_ones) && (candidate_count < wheel_spec.size))
        {
            char candidate = rmsk::std_alpha()->to_val(wheel_spec_perm.encrypt(candidate_count));
            
            // Test whether two consecutive pins are set
            if (diff_test(*wheel_spec.spec, candidate))
            {
                // No consecutive pins.
                wheel_spec.spec->push_back(candidate);
                one_count++;
            }
            
            candidate_count++;
        }
        
        // sort candidate wheel spec
        sorted_spec = (*wheel_spec.spec);
        sort(sorted_spec.begin(), sorted_spec.end());
        
        try_count++;
        // make sure that the two pins at the first and last character which are next to each other on the rotor ring are not both set
        last_first_pins_not_consecutive = ((one_count >= 1) && ((sorted_spec[0] != 'a') || (sorted_spec[one_count - 1] != final_char)));
        
        spec_found = (one_count == num_ones) && last_first_pins_not_consecutive;                     
        
    } while((not spec_found) && (try_count <= MAX_RAND_TRIES));
        
    if (try_count > MAX_RAND_TRIES)
    {
        throw sg39_randomize_exception();
    }
}

bool schluesselgeraet39::diff_test(string& wheel_spec_candidate, char new_pin_pos)
{
    bool result = true;
    string to_test = wheel_spec_candidate + new_pin_pos;
    
    if (to_test.length() > 1)
    {
        sort(to_test.begin(), to_test.end());
        
        for (unsigned int count = 1; (count < to_test.length()) && result; count++)
        {
            result &= ((to_test[count] - to_test[count - 1]) != 1);
        }
    }
    
    return result;
}

bool schluesselgeraet39::set_test(string& wheel_spec1, string& wheel_spec2, unsigned int max_overlap)
{
    bool result = false;
    set<char> set1;
    string::iterator iter;
    unsigned int intersect_count = 0;
    
    for (iter = wheel_spec1.begin(); iter != wheel_spec1.end(); ++iter)
    {
        set1.insert(*iter);
    }

    for (iter = wheel_spec2.begin(); iter != wheel_spec2.end(); ++iter)
    {
        if (set1.count(*iter) != 0)
        {
            intersect_count++;
        }
    }

    result = intersect_count <= max_overlap;
    
    return result;
}

bool schluesselgeraet39::randomize(string& param)
{
    bool result = false;
    urandom_generator rand;    
    map<string, string> machine_conf;
    random_bit_source wheel_pin_source(15);    
    boost::scoped_ptr<configurator> c(configurator_factory::get_configurator(machine_name));
    string rotors, pins_wheel_1, pins_wheel_2, pins_wheel_3; 
    vector<unsigned int> rotor_pos, wheel_pos;
    const char *help = "abcdefghijklmnopqrstuvwxy";  
    alphabet<char> wheel1_alpha(help, 21), wheel2_alpha(help, 23), wheel3_alpha(help, 25); 
    vector<unsigned int> num_pins_slow, num_pins_middle;
    string pins_rotor_1, pins_rotor_2, pins_rotor_3; 
    unsigned int key_gen_selector;
    map<string, unsigned int> rand_parm_map;
    
    rand_parm_map["one"] = 0;
    rand_parm_map["two"] = 1;
    rand_parm_map["three"] = 2;
    rand_parm_map["special"] = 3;
    rand_parm_map["enigmam4"] = 6;    
    
    try
    {
        permutation plugboard_perm = permutation::get_random_permutation(rand, 26);
        permutation reflector_perm = permutation::get_random_permutation(rand, 26);          
        permutation rotor_pin_perm = permutation::get_random_permutation(rand, 26);
        permutation rotor_selection_perm = permutation::get_random_permutation(rand, 10);
        permutation stepping_selection_perm = permutation::get_random_permutation(rand, 4);        

        // Determine rotor sequence
        for(unsigned int count = 0; count < 4; count++)
        {
            rotors += '0' + (char)(rotor_selection_perm.encrypt(count));
        }
        
        key_gen_selector = stepping_selection_perm.encrypt(0);
        
        if (rand_parm_map.count(param) != 0)
        {
            key_gen_selector = rand_parm_map[param];
        }
        
        if (key_gen_selector != 6)
        {                
            // Determine stepping motion
            switch(key_gen_selector)
            {
                case 0: /* one */
                {
                    pins_wheel_1 = "abcdefghijklmnopqrstu";               

                    randomize_help wheel_2_rand(&pins_wheel_2, 23);
                    fill_wheel_spec(wheel_2_rand, 3);                                        

                    randomize_help wheel_3_rand(&pins_wheel_3, 25);
                    fill_wheel_spec(wheel_3_rand, 7);                                                            
                }
                break;                
                case 1: /* two */
                {
                    pins_wheel_1 = "abcdefghijklmnopqrstu";               

                    randomize_help wheel_2_rand(&pins_wheel_2, 23);
                    fill_wheel_spec(wheel_2_rand, 5);                                        

                    randomize_help wheel_3_rand(&pins_wheel_3, 25);
                    fill_wheel_spec(wheel_3_rand, 7);                                                            
                }
                break;
                case 3: /* special */
                {
                    pins_wheel_2 = "abcdefghijklmnopqrstuvw";               

                    randomize_help wheel_1_rand(&pins_wheel_1, 21);
                    fill_wheel_spec(wheel_1_rand, 5);                                        

                    randomize_help wheel_3_rand(&pins_wheel_3, 25);
                    fill_wheel_spec(wheel_3_rand, 11);                                                            
                }
                break;
                default: /* three */
                {
                    pins_wheel_1 = "abcdefghijklmnopqrstu";               

                    randomize_help wheel_2_rand(&pins_wheel_2, 23);
                    fill_wheel_spec(wheel_2_rand, 7);                                        

                    randomize_help wheel_3_rand(&pins_wheel_3, 25);
                    fill_wheel_spec(wheel_3_rand, 9);                                        

                }
                break;
            }
            
            machine_conf[KW_SG39_ROTOR_SET] = get_default_set_name();                
            machine_conf[KW_SG39_ROTORS] = rotors;
            machine_conf[KW_SG39_RING_POS] = rmsk::std_alpha()->get_random_string(4);
            machine_conf[KW_SG39_ENTRY_PLUGS] = rmsk::std_alpha()->perm_as_string(plugboard_perm);
            machine_conf[KW_SG39_REFLECTOR_PLUGS] = rmsk::std_alpha()->perm_as_string(reflector_perm);
            machine_conf[KW_SG39_PINS_WHEEL_1] = pins_wheel_1;
            machine_conf[KW_SG39_PINS_WHEEL_2] = pins_wheel_2;            
            machine_conf[KW_SG39_PINS_WHEEL_3] = pins_wheel_3;

            c->configure_machine(machine_conf, this);    
            
            // Set random rotor positions
            rotor_pos = rmsk::std_alpha()->to_vector(rmsk::std_alpha()->get_random_string(4));
            rotor_pos.push_back(0);
            get_sg39_stepper()->set_all_displacements(rotor_pos);
            
            // Set wheel positions
            wheel_pos = wheel1_alpha.to_vector(wheel1_alpha.get_random_string(2));
            get_sg39_stepper()->set_wheel_pos(ROTOR_1, wheel_pos[0]);
            wheel_pos = wheel2_alpha.to_vector(wheel2_alpha.get_random_string(2));
            get_sg39_stepper()->set_wheel_pos(ROTOR_2, wheel_pos[0]);
            wheel_pos = wheel3_alpha.to_vector(wheel3_alpha.get_random_string(2));
            get_sg39_stepper()->set_wheel_pos(ROTOR_3, wheel_pos[0]);                        
        }
        else
        {
            string dummy;
            enigma_M4 enigma_M4(UKW_B_DN, WALZE_BETA, WALZE_II, WALZE_IV, WALZE_I);
            enigma_M4.randomize(dummy);
            
            configure_from_m4(&enigma_M4);        
        }        
    }
    catch(...)
    {
        result = true;
    }    
    
    return result;
}

void schluesselgeraet39::configure_from_m4(enigma_M4 *m4_enigma)
{
    sg39_stepping_gear *stepper = get_sg39_stepper();
    enigma_stepper_base *se = m4_enigma->get_enigma_stepper();
    vector<unsigned int> dat_1(21, 1), dat_2(23, 0), dat_3(25, 0), empty_ring(26, 0);
    rotor_id id1(sg39_rotor_factory::id_mapping[se->get_descriptor(0u).id.r_id]);
    rotor_id id2(sg39_rotor_factory::id_mapping[se->get_descriptor(1).id.r_id]);
    rotor_id id3(sg39_rotor_factory::id_mapping[se->get_descriptor(2).id.r_id]);
    rotor_id id4(sg39_rotor_factory::id_mapping[se->get_descriptor(3).id.r_id]);
    vector<unsigned int> entry_perm_data;

    set_default_set_name(M4_SET);
    
    // Insert rotors and rings into machine
    prepare_rotor(id1, ROTOR_1);
    prepare_rotor(id2, ROTOR_2);            
    prepare_rotor(id3, ROTOR_3);                        
    prepare_rotor(id4, ROTOR_4);
    
    // Clear ring of third rotor so that it does not step rotor 1
    stepper->get_descriptor(ROTOR_3).ring->set_ring_data(empty_ring);                          
    
    // Set pin data on wheels
    stepper->set_wheel_data(ROTOR_1, dat_1);
    stepper->set_wheel_data(ROTOR_2, dat_2);
    stepper->set_wheel_data(ROTOR_3, dat_3);                
    
    // Set reflector
    permutation *reflector_perm = new permutation();
    *reflector_perm = *se->get_descriptor(4).r->get_perm();
    set_reflector(boost::shared_ptr<permutation>(reflector_perm));
    
    // Set plugboard permutation    
    for (unsigned int count = 0; count < 26; count++)
    {
        entry_perm_data.push_back(m4_enigma->get_input_transform()->encrypt(count));
    }
    
    permutation *plugboard_perm = new permutation(entry_perm_data);    
    set_input_transform(boost::shared_ptr<encryption_transform>(plugboard_perm));            

    // Set ring position on rotors            
    stepper->get_descriptor(ROTOR_1).ring->set_offset(se->get_descriptor(0u).ring->get_offset());
    stepper->get_descriptor(ROTOR_2).ring->set_offset(se->get_descriptor(1).ring->get_offset());
    stepper->get_descriptor(ROTOR_3).ring->set_offset(se->get_descriptor(2).ring->get_offset());
    stepper->get_descriptor(ROTOR_4).ring->set_offset(se->get_descriptor(3).ring->get_offset());

    // Move all non stationary rotors to correct positions and all wheels to the 'a' position
    for (unsigned int count = 0; count < 3; count++)
    {
        stepper->get_descriptor(count).ring->set_pos(se->get_ring_pos(count));
        stepper->get_descriptor(count).mod_int_vals["wheelpos"].set_val(0);
    }
    
    // Move stationary rotor to correct position
    stepper->get_descriptor(3).ring->set_pos(se->get_ring_pos(3));    
}

/*! The wheels are implemented by storing the wheeldata as well as the wheelpos in key/value pairs of the corresponding 
 *  ::rotor_descriptor object.
 */
schluesselgeraet39::schluesselgeraet39(unsigned int rotor_1_id, unsigned int rotor_2_id, unsigned int rotor_3_id, unsigned int rotor_4_id) 
    : rotor_machine() 
{ 
    vector<string> rotor_names;
    // Set default all zero wheeldata    
    vector<unsigned int> dat_1(21, 0), dat_2(23, 0), dat_3(25, 0);
    simple_mod_int mod_1(21), mod_2(23), mod_3(25);
    
    add_rotor_set(DEFAULT_SET, sg39_rotor_factory::get_rotor_set());  
    add_rotor_set(M4_SET, sg39_rotor_factory::get_m4_rotor_set());
    
    is_pre_step = true;
    machine_name = MNAME_SG39;
    
    // Set names of rotor slots
    rotor_names.push_back(ROTOR_1);
    rotor_names.push_back(ROTOR_2);
    rotor_names.push_back(ROTOR_3);
    rotor_names.push_back(ROTOR_4);
    rotor_names.push_back(UKW_SG39);
    
    sg39_stepping_gear *s = new sg39_stepping_gear(rotor_names);    
    
    set_stepping_gear(s);

    // Place rotors in machine and initialize wheeldata and wheelpos
    prepare_rotor(rotor_1_id, ROTOR_1);
    s->get_descriptor(ROTOR_1).binary_vals["wheeldata"] = dat_1;
    s->get_descriptor(ROTOR_1).mod_int_vals["wheelpos"] = mod_1;        

    prepare_rotor(rotor_2_id, ROTOR_2);
    s->get_descriptor(ROTOR_2).binary_vals["wheeldata"] = dat_2;
    s->get_descriptor(ROTOR_2).mod_int_vals["wheelpos"] = mod_2;        

    prepare_rotor(rotor_3_id, ROTOR_3);    
    s->get_descriptor(ROTOR_3).binary_vals["wheeldata"] = dat_3;
    s->get_descriptor(ROTOR_3).mod_int_vals["wheelpos"] = mod_3;        

    // Place rotor 4 and reflector in machine. There is no wheel controlling the 4th rotor or the reflector
    prepare_rotor(rotor_4_id, ROTOR_4);             
    prepare_rotor(ID_SG39_UKW, UKW_SG39);

#ifdef SG39_ASYMMETRIC
    vector<gunichar> plain_alph, cipher_alph;

    // Set up cipher and plaintext alphabets
    printing_device::ustr_to_vec(str_plain_chars_ger, plain_alph);
    printing_device::ustr_to_vec(str_cipher_chars_ger, cipher_alph);        
    boost::shared_ptr<alphabet<gunichar> > plain_alpha(new alphabet<gunichar>(plain_alph));
    boost::shared_ptr<alphabet<gunichar> > cipher_alpha(new alphabet<gunichar>(cipher_alph));
    
    // Set up printing_device
    asymmetric_printing_device *sg39_printer = new asymmetric_printing_device();
    
    sg39_printer->set_plain_alphabet(plain_alpha); 
    sg39_printer->set_cipher_alphabet(cipher_alpha);
    
    set_printer(boost::shared_ptr<printing_device>(sg39_printer));    
    
    // Set up keyboard
    boost::shared_ptr<asymmetric_keyboard> sg39_keyboard(new asymmetric_keyboard());
    sg39_keyboard->set_plain_alphabet(plain_alpha);
    sg39_keyboard->set_cipher_alphabet(cipher_alpha);
    
    set_keyboard(sg39_keyboard);
    
#else    

    // Use standard printing device
    boost::shared_ptr<printing_device> prt(new symmetric_printing_device(ustring("abcdefghijklmnopqrstuvwxyz")));
    set_printer(prt);                   

    // Use standard keyboard
    boost::shared_ptr<rotor_keyboard> kbd(new symmetric_keyboard(ustring("abcdefghijklmnopqrstuvwxyz")));
    set_keyboard(kbd);        

#endif
    
    randomizer_params.push_back(randomizer_descriptor("one", "Type one stepping"));
    randomizer_params.push_back(randomizer_descriptor("two", "Type two stepping"));
    randomizer_params.push_back(randomizer_descriptor("three", "Type three stepping"));       
    randomizer_params.push_back(randomizer_descriptor("special", "Special stepping"));            
    randomizer_params.push_back(randomizer_descriptor("enigmam4", "M4 Enigma compatible"));        
    
    unvisualized_rotor_names.insert(UKW_SG39);               

    get_stepping_gear()->reset();    
}

ustring schluesselgeraet39::visualize_all_positions()
{
    ustring result = rotor_machine::visualize_all_positions();
    
    result += rmsk::std_alpha()->to_val(get_sg39_stepper()->get_wheel_pos(ROTOR_3));
    result += rmsk::std_alpha()->to_val(get_sg39_stepper()->get_wheel_pos(ROTOR_2));
    result += rmsk::std_alpha()->to_val(get_sg39_stepper()->get_wheel_pos(ROTOR_1));        
    
    return result;
}

bool schluesselgeraet39::move_all_rotors(ustring& new_positions)
{
    bool result = (new_positions.length() != 7);
    vector<string> ids;
    unsigned int numeric_wheel_pos;
    sg39_stepping_gear *stepper = get_sg39_stepper();
    string identifier;
    vector<pair<string, unsigned int> > numeric_rotor_positions, numeric_wheel_positions;
    
    stepper->get_rotor_identifiers(ids);
    
    for (unsigned int count = 0; (count < 4) && (!result); count++)
    {
        identifier = ids[count];  
        gunichar current_rotor_pos = new_positions[3 - count];
        
        // Verify that current_rotor_pos is a valid position    
        result = !rmsk::std_uni_alpha()->contains_symbol(current_rotor_pos);
        
        if (!result)
        {
            // Set displacement
            numeric_rotor_positions.push_back(pair<string, unsigned int>(identifier, rmsk::std_uni_alpha()->from_val(current_rotor_pos)));
            
            // Only ROTOR_1, ROTOR_2 and ROTOR_3 have a wheel associated with them
            if (identifier != ROTOR_4)
            {
                // Verify that current_wheel_pos is a letter
                gunichar current_wheel_pos = new_positions[6 - count];                
                result = !rmsk::std_uni_alpha()->contains_symbol(current_wheel_pos);
                
                if (!result)
                {
                    // Verify that current_wheel_pos is possible with the current wheel
                    numeric_wheel_pos = rmsk::std_uni_alpha()->from_val(current_wheel_pos);
                    result = (numeric_wheel_pos >= stepper->get_descriptor(identifier).mod_int_vals["wheelpos"].get_mod());
                    
                    if (!result)
                    {
                        // Set wheel position
                        numeric_wheel_positions.push_back(pair<string, unsigned int>(identifier, numeric_wheel_pos));
                    }
                }
            }            
        }
    }
    
    // Everything was checked. Now do modifications or skip them if result == true.
    for (unsigned int count = 0; (count < 4) && (!result); count++)
    {
        stepper->set_ring_pos(numeric_rotor_positions[count].first.c_str(), numeric_rotor_positions[count].second);    
    }        

    for (unsigned int count = 0; (count < 3) && (!result); count++)
    {
        stepper->set_wheel_pos(numeric_wheel_positions[count].first.c_str(), numeric_wheel_positions[count].second);
    }        

    
    return result;
}

void schluesselgeraet39::save_additional_components(Glib::KeyFile& ini_file)
{
    vector<int> perm_data;

    // Is there an input transform ?
    if (input_transform.get() != NULL)
    {
        // Yes
        // Determine the current input transform
        for (unsigned int count = 0; count < get_size(); count++)
        {
            perm_data.push_back(input_transform->encrypt(count));
        }        
    }
    else
    {
        // No
        // Use identity as input transform
        for (int count = 0; count < (int)get_size(); count++)
        {
            perm_data.push_back(count);
        }            
    }
    
    ini_file.set_integer_list("plugboard", "entry", perm_data);    
}

bool schluesselgeraet39::load_additional_components(Glib::KeyFile& ini_file)
{
    bool result = false;
    vector<int> temp_permdata;
    vector<unsigned int> perm_data;
    
    if (!(result = !ini_file.has_key("plugboard", "entry")))
    {
        temp_permdata = ini_file.get_integer_list("plugboard", "entry");
        
        if (!(result = (temp_permdata.size() != get_size())))
        {
            // Cast temp_permdata to unisgned int
            for (unsigned int count = 0; count < get_size(); count++)
            {
                perm_data.push_back((unsigned int)temp_permdata[count]);
            }
            
            // Create new input transform as a permutation initialized by perm_data
            boost::shared_ptr<encryption_transform> input_trans(new permutation(perm_data));
            set_input_transform(input_trans);
        }
    }
    
    return result;
}


void schluesselgeraet39::set_reflector(vector<pair<char, char> >& data)
{
    boost::shared_ptr<permutation> new_reflector = boost::shared_ptr<permutation>(rmsk::std_alpha()->make_involution_ptr(data));
    
    get_stepping_gear()->get_descriptor(UKW_SG39).r->set_perm(new_reflector);
}

void schluesselgeraet39::set_reflector(boost::shared_ptr<permutation> involution)
{
    get_stepping_gear()->get_descriptor(UKW_SG39).r->set_perm(involution);
}

