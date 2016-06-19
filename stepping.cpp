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

/*! \file stepping.cpp
 *  \brief Contains the implementation of the classes that make up the stepping gear of a rotor machine.
 */ 


#include<iostream>
#include<stepping.h>

void rotor_stack::set_rotors_help(vector<rotor *>& new_rotors) 
{ 
    unsigned int count = 0;
    
    stack_size = new_rotors.size();
    
    for (count = 0; (count < stack_size) and (count < MAX_ROTORS); count++)
    {
        stack[count] = new_rotors[count];
    }   
}

void rotor_stack::set_feedback_points(const unsigned int *new_feedback_points, unsigned int num_feedback_points)
{
    set<unsigned int> new_feedbacks;
    
    for (unsigned int count = 0; count < num_feedback_points; count++)
    {
        new_feedbacks.insert(new_feedback_points[count]);
    }
    
    set_feedback_points(new_feedbacks);
}

void rotor_stack::set_feedback_points(set<unsigned int>& new_feedback_points)
{
    rmsk::simple_assert(stack_size < 1, "programmer error: number of rotors too small for a feedback rotor stack");    
    
    feedback_points = new_feedback_points;
    
    if (!feedback_points.empty())
    {
        is_reflecting_stack = false; // a feedback_rotor_stack cannot be reflecting
        
        if (feedback_permutation.get_size() == 0)
        {
            feedback_permutation = permutation::get_identity(get_size());
        }
    }
}

void rotor_stack::copy(const rotor_stack& s)
{
    vector<rotor *> help;
    
    is_reflecting_stack = s.is_reflecting_stack;
    stack_size = s.stack_size;
    
    for (unsigned int count = 0; count < stack_size; count++)
    {
        stack[count] = s.stack[count];
    }
    
    feedback_points = s.feedback_points;
    feedback_permutation = s.feedback_permutation;    
}

rotor_stack *rotor_stack::clone()
{
    return new rotor_stack(*this);
}

/*! 
 *  This method simulates the passing of an eletric current that is applied to a contact of the ::rotor at position 0
 *  through the stack. The signal exits the stack at certain position. This is the result of the encryption operation.
 *  The position of the contact to which the simulated current is applied is determined by the value of in_char. 
 *
 */
unsigned int rotor_stack::encrypt(unsigned int in_char)
{
    unsigned int count = 0, result = in_char;
    
    if (is_reflecting_stack)
    {
        // Signal travels from the entry to the reflecting rotor
        for (count = 0; count < (stack_size - 1); count++)
        {
            result = stack[count]->rot_enc(result);
        }        

        // Signal is reflected
        result = stack[stack_size - 1]->rot_enc(result);

        // Signal travels back to the entry of the stack
        for (count = (stack_size - 1); count >= 1; count--)
        {
            result = stack[count - 1]->rot_dec(result);
        }        
    }
    else
    {
        set<unsigned int>::iterator iter;        
        
        do 
        {
            // Signal travels through the stack
            for (count = 0; count < stack_size; count++)
            {
                result = stack[count]->rot_enc(result);
            }
            
            iter = feedback_points.find(result);
            
            // Did we hit a feedback point?
            if (iter != feedback_points.end())
            {
                // Yes, then pass the signal on to another feedback position
                // In the KL7 the feedback_permutation is the identity permutation
                result = feedback_permutation.encrypt(result);
            }
        
        // Signal is returned to the entry of the stack if we have hit a feedback point
        } while(iter != feedback_points.end());     
    }
    
    return result;
}

/*! 
 *  This simulates the passing of a signal through the stack in case of a decryption. When the stack
 *  is reflecting this is the same operation as during rotor_stack::encrypt. If the stack is not
 *  reflecting the signal path is reversed, i.e. the input signal is applied to the last rotor in the
 *  stack and not to the first.
 *
 */
unsigned int rotor_stack::decrypt(unsigned int in_char)
{
    unsigned int count = 0, result = in_char;

    // The same as in encrypt()
    if (is_reflecting_stack)
    {
        for (count = 0; count < (stack_size - 1); count++)
        {
            result = stack[count]->rot_enc(result);
        }        

        result = stack[stack_size - 1]->rot_dec(result);

        for (count = (stack_size - 1); count >= 1; count--)
        {
            result = stack[count - 1]->rot_dec(result);
        }                
    }
    else
    {
        set<unsigned int>::iterator iter;        
        
        // Reverse of the operations in encrypt()
        do 
        {            
            for (count = stack_size; count >= 1; count--)
            {
                result = stack[count - 1]->rot_dec(result);
            }
             
            iter = feedback_points.find(result);
            
            if (iter != feedback_points.end())
            {
                result = feedback_permutation.decrypt(result);
            }
        
        } while(iter != feedback_points.end());                   
    }
    
    return result;
}

void rotor_stack::set_reflecting_flag(bool new_val)
{
    rmsk::simple_assert(stack_size < 2, "programmer error: number of rotors too small for a reflecting rotor stack");

    is_reflecting_stack = new_val; 
}

/* ----------------------------------------------------------- */

bool rotor_descriptor::load_from_config(string& identifier, Glib::KeyFile& ini_file)
{
    bool result = false;
    string section_name = "rotor_" + identifier;    
    int temp_ringoffset, temp_rotordisplacement;
    vector<int> temp_permutation, temp_ringdata;
    vector<unsigned int> perm_data, ring_data;
    
    do
    {
        // load permutation data
        if ((result = !ini_file.has_key(section_name, "permutation")))
        {
            break;
        }
        
        temp_permutation = ini_file.get_integer_list(section_name, "permutation");
        
        if ((result = (temp_permutation.size() != r->get_size())))
        {
            break;
        }       
        
        // load displacement information
        if ((result = !ini_file.has_key(section_name, "rotordisplacement")))
        {
            break;
        }
        
        temp_rotordisplacement =  ini_file.get_integer(section_name, "rotordisplacement");                       

        // load rotor id
        if ((result = !ini_file.has_key(section_name, "rid")))
        {
            break;
        }
        
        id.r_id =  (unsigned int)ini_file.get_integer(section_name, "rid");                       

        // load ring id
        if ((result = !ini_file.has_key(section_name, "ringid")))
        {
            break;
        }
        
        id.ring_id =  (unsigned int)ini_file.get_integer(section_name, "ringid");                       

        // Determine if the rotor was inserted in reverse
        if ((result = !ini_file.has_key(section_name, "insertinverse")))
        {
            break;
        }
        
        id.insert_inverse =  (unsigned int)ini_file.get_boolean(section_name, "insertinverse");                       

        if (ring.get() != NULL)
        {
            // load ring data
            if ((result = !ini_file.has_key(section_name, "ringdata")))
            {
                break;
            }

            temp_ringdata = ini_file.get_integer_list(section_name, "ringdata");
            
            if ((result = (temp_ringdata.size() != r->get_size())))
            {
                break;
            }       

            // load ring offset
            if ((result = !ini_file.has_key(section_name, "ringoffset")))
            {
                break;
            }
            
            temp_ringoffset =  ini_file.get_integer(section_name, "ringoffset");
            
            // Cast ring data to unsigned int
            for (unsigned int count = 0; count < r->get_size(); count++)
            {
                ring_data.push_back((unsigned int)temp_ringdata[count]);
            }

            ring->set_offset((unsigned int)temp_ringoffset);
            ring->set_ring_data(ring_data);                                        
        }
        
        // Cast permutation data to unsigned int
        for (unsigned int count = 0; count < r->get_size(); count++)
        {
            perm_data.push_back((unsigned int)temp_permutation[count]);
        }
        
        boost::shared_ptr<permutation> new_perm(new permutation(perm_data));        
        r->set_perm(new_perm);
        
        current_displacement = (unsigned int)temp_rotordisplacement;
            
    } while(0);
                   
    return result;
}

void rotor_descriptor::save_to_config(string& identifier, Glib::KeyFile& ini_file)
{
    vector<int> perm_data, ring_data;
    vector<unsigned int> ring_data_temp;
    
    string section_name = "rotor_" + identifier;

    for (unsigned int count = 0; count < r->get_size(); count++)
    {
        perm_data.push_back(r->get_perm()->encrypt(count));
    }
    
    ini_file.set_integer_list(section_name, "permutation", perm_data);
    ini_file.set_integer(section_name, "rid", (int)id.r_id);
    ini_file.set_integer(section_name, "ringid", (int)id.ring_id);    
    ini_file.set_boolean(section_name, "insertinverse", id.insert_inverse);    
    
    if (ring.get() != NULL)
    {
        ring->get_ring_data(ring_data_temp);
        
        for (unsigned int count = 0; count < r->get_size(); count++)
        {
            ring_data.push_back(ring_data_temp[count]);
        }
        
        ini_file.set_integer_list(section_name, "ringdata", ring_data);
        
        ini_file.set_integer(section_name, "ringoffset", ring->get_offset());
    }
    
    ini_file.set_integer(section_name, "rotordisplacement", current_displacement);
}

/* ----------------------------------------------------------- */

void stepping_gear::copy(const stepping_gear& s)
{
    num_rotors = s.num_rotors;  
    char_count = s.char_count;
    alphabet_size = s.alphabet_size;
    rotors = s.rotors;
    stack = (s.stack)->clone();
    rotor_positions = s.rotor_positions;
    

    for (unsigned count = 0; count < num_rotors; count++)
    {
        descriptors[count] = &get_descriptor(count);
    }    
}

stepping_gear::stepping_gear(vector<string>& rotor_identifiers, unsigned int alpha_size, rotor_stack *r_stack)    
{
    rmsk::simple_assert(rotor_identifiers.size() > MAX_ROTORS, "Too many rotors");
    
    unsigned int count = 0;
    vector<rotor *> stack_rotors;
    string count_id;
    
    alphabet_size = alpha_size;

    // rotor identifiers contains the identifiers which are to be used in this stepping gear
    rotor_positions = rotor_identifiers;
    num_rotors = rotor_positions.size();
    char_count = 0;
    
    // Create the rotor_descriptors, where the ring and the rotor point to NULL.
    // Fill the std::map variable rotors that maps the rotor identifiers to the 
    // newly created rotor_descriptors.
    for (count = 0; count < num_rotors; count++)
    {
        rotor_descriptor rotor_desc;
        
        count_id = rotor_positions[count];
        
        rotor_desc.current_displacement = 0;
        
        rotors[count_id] = rotor_desc;
        
        //r.get() is always NULL at this moment
        stack_rotors.push_back(rotor_desc.r.get());
    }
    
    // Create a new rotor stack in case none was specified
    if (r_stack == NULL)
    {
        r_stack = new rotor_stack;
    }
    
    stack = r_stack;
    // Populate rotor stack
    stack->set_rotors(stack_rotors);
    
    // Fill desccriptor array with pointers to the newly created rotor_descriptors
    for (count = 0; count < num_rotors; count++)
    {
        descriptors[count] = &get_descriptor(count);
    }    
}

/*! This is the core method that is responsible for restoring the state of a stepping_gear. 
 */
bool stepping_gear::load_ini(Glib::KeyFile& ini_file)
{
    bool result = false;
    
    for (unsigned int count = 0; (count < num_rotors) and (!result); count++)
    {
        // Restore rotor_descriptor
        result = rotors[rotor_positions[count]].load_from_config(rotor_positions[count], ini_file); 
        
        if (!result)
        {
            // Restore additional values for this rotor identifier
            result = load_additional_components(rotor_positions[count], ini_file);
        }
    }
        
    return result;
}

/*! Kind of wrapper, that only loads the settings file and then calls load_ini(Glib::KeyFile& ini_file). 
 */
bool stepping_gear::load(string& file_name)
{
    return rmsk::settings_file_load(file_name, sigc::mem_fun(*this, &stepping_gear::load_ini));
}

/*! This is the core method that is responsible for saving the state of a stepping_gear. 
 */
void stepping_gear::save_ini(Glib::KeyFile& ini_file)
{
    for (unsigned int count = 0; count < num_rotors; count++)
    {
        // Save rotor_descriptor
        rotors[rotor_positions[count]].save_to_config(rotor_positions[count], ini_file); 
        // Save additional values for this rotor identifier
        save_additional_components(rotor_positions[count], ini_file);
    }    
}

/*! Kind of wrapper, that calls save_ini(Glib::KeyFile& ini_file) and stores the resulting KeyFile object
 *  in a file. 
 */
bool stepping_gear::save(string& file_name)
{
    return rmsk::settings_file_save(file_name, sigc::mem_fun(*this, &stepping_gear::save_ini));
}

void stepping_gear::get_rotor_identifiers(vector<string>& ids)
{
    unsigned int count = 0;    
    
    ids.clear();
    
    for (count = 0; count < num_rotors; count++)
    {
        ids.push_back(rotor_positions[count]);
    }
}

void stepping_gear::insert_all_rotors(vector<boost::shared_ptr<rotor> >& new_rotors)
{
    unsigned int count = 0;
    
    rmsk::simple_assert(new_rotors.size() != num_rotors, "programmer error: number of rotors given is wrong");
    
    for (count = 0; count < num_rotors; count++)
    {
        insert_rotor(rotor_positions[count], new_rotors[count]);
    }
}

void stepping_gear::set_all_displacements(vector<unsigned int>& new_positions)
{
    unsigned int count = 0;
    
    rmsk::simple_assert(new_positions.size() != num_rotors, "programmer error: number of placements does not match number of rotors");
        
    for (count = 0; count < num_rotors; count++)
    {
        set_rotor_displacement(rotor_positions[count], new_positions[count]);
    }
}

void stepping_gear::set_all_ring_offsets(vector<unsigned int>& new_offsets)
{
    unsigned int count = 0;
    
    rmsk::simple_assert(new_offsets.size() != num_rotors, "programmer error: number of offsets does not match number of rotors");
        
    for (count = 0; count < num_rotors; count++)
    {
        if (rotors[rotor_positions[count]].ring.get() != NULL)
        {
            rotors[rotor_positions[count]].ring->set_offset(new_offsets[count]);
        }
    }
    
}

void stepping_gear::get_all_displacements(vector<unsigned int>& current_positions)
{
    unsigned int count = 0;
    
    current_positions.clear();
    
    for (count = 0; count < num_rotors; count++)
    {
        current_positions.push_back(get_rotor_displacement(rotor_positions[count]));
    }    
}

/*! In addition to add the ring to the rotor_descriptor this method also attaches the ::rotor_ring given in parameter
 *  ring to the ::rotor contained in the descriptor. If the rotor in the descriptor is still NULL when this method is
 *  called, then an exception of type runtime_exception is thrown. The same is true if an unknown identfier is used.
 */
void stepping_gear::attach_ring(string& identifier, boost::shared_ptr<rotor_ring> ring)
{
    rmsk::simple_assert(rotors.count(identifier) == 0, "programmer error: rotor identifier unknown");
    rmsk::simple_assert(rotors[identifier].r.get() == NULL, "programmer error: corresponding rotor empty");
    
    rotors[identifier].ring = ring;
    rotors[identifier].ring->set_rotor(rotors[identifier].r.get());
}

/*! If the rotor given in rotor_to_insert replaces another rotor that has been placed in the stepping_gear before
 *  and if a ring was attached to this "older" rotor, then the ring is reattached to the new rotor given in rotor_to_insert.
 */
void stepping_gear::insert_rotor(string& identifier, boost::shared_ptr<rotor> rotor_to_insert)
{
    vector<rotor *> stack_rotors;
    unsigned int count = 0;
        
    rmsk::simple_assert(rotors.count(identifier) == 0, "programmer error: rotor identifier unknown");
    rmsk::simple_assert(rotor_to_insert->get_size() != alphabet_size, "programmer error: alphabet size of rotor does not match");
    
    // Put new rotor in descriptor
    rotors[identifier].r = rotor_to_insert;
    rotors[identifier].r->set_displacement_var(&rotors[identifier].current_displacement);
    
    // Reattach ring if necessary
    if (rotors[identifier].ring.get() != NULL)
    {
        rotors[identifier].ring->set_rotor(rotors[identifier].r.get());
    }
    
    // Recreate the contents of the rotor stack due to the changes.    
    for (count = 0; count < num_rotors; count++)
    {
        stack_rotors.push_back(rotors[rotor_positions[count]].r.get());
    }
    
    stack->set_rotors(stack_rotors);
}

void stepping_gear::permute_rings(permutation& ring_permutation)
{
    rmsk::simple_assert(ring_permutation.get_size() != num_rotors, "programmer error: ring permutation has wrong size");
    
    vector<boost::shared_ptr<rotor_ring> > all_rings;
    
    // Collect all the rings currently attached
    for (unsigned int count = 0; count < num_rotors; count++)
    {
        all_rings.push_back(get_descriptor(count).ring);
    }
    
    // Reattach the rings in permuted order
    for (unsigned int count = 0; count < num_rotors; count++)
    {
        attach_ring(count, all_rings[ring_permutation.permute(count)]);
    }
}

void stepping_gear::insert_rotor_and_ring(string& identifier, pair<boost::shared_ptr<rotor>, boost::shared_ptr<rotor_ring> >& assembly)
{
    insert_rotor(identifier, assembly.first);
    attach_ring(identifier, assembly.second);        
}

unsigned int stepping_gear::get_rotor_displacement(string& identifier)
{    
    rmsk::simple_assert(rotors.count(identifier) == 0, "programmer error: rotor identifier unknown");
    
    return rotors[identifier].current_displacement;
}

unsigned int stepping_gear::get_ring_pos(string& identifier)
{
    rmsk::simple_assert(rotors.count(identifier) == 0, "programmer error: rotor identifier unknown");
    
    unsigned int result;
    
    if (rotors[identifier].ring.get() != NULL)
    {
        result = rotors[identifier].ring->get_pos();
    }
    else
    {
        result = rotors[identifier].current_displacement;
    }
    
    return result;
}

void stepping_gear::set_ring_pos(string& identifier, unsigned int new_pos)
{
    rmsk::simple_assert(rotors.count(identifier) == 0, "programmer error: rotor identifier unknown");
    
    if (rotors[identifier].ring.get() != NULL)
    {
        rotors[identifier].ring->set_pos(new_pos);
    }
    else
    {
        set_rotor_displacement(identifier, new_pos);
    } 
}

void stepping_gear::set_ring_pos(unsigned int rotor_num, unsigned int new_pos)
{   
    string identifier = rotor_positions[rotor_num % num_rotors];
    
    set_ring_pos(identifier, new_pos);
}

rotor_descriptor& stepping_gear::get_descriptor(string& identifier)
{
    rmsk::simple_assert(rotors.count(identifier) == 0, "programmer error: rotor identifier unknown");
    
    return rotors[identifier];
}

rotor_descriptor& stepping_gear::get_descriptor(unsigned int rotor_num)
{
    return get_descriptor(rotor_positions[rotor_num % num_rotors]);    
}


void stepping_gear::set_rotor_displacement(string& identifier, unsigned int new_pos)
{
    rmsk::simple_assert(rotors.count(identifier) == 0, "programmer error: rotor identifier unknown");

    rotors[identifier].current_displacement = new_pos % alphabet_size;
}

void stepping_gear::set_rotor_displacement(unsigned rotor_num, unsigned int new_pos)
{
    set_rotor_displacement(rotor_positions[rotor_num % num_rotors], new_pos);
}

void stepping_gear::move_rotor(string& identifier, unsigned int distance)
{
    unsigned int new_pos = get_rotor_displacement(identifier) + distance;
    
    set_rotor_displacement(identifier, new_pos);
}

void stepping_gear::move_rotor(unsigned rotor_num, unsigned int distance)
{
    move_rotor(rotor_positions[rotor_num % num_rotors], distance);
}

void stepping_gear::advance_rotor(string& identifier)
{
    rmsk::simple_assert(rotors.count(identifier) == 0, "programmer error: rotor identifier unknown");

    rotors[identifier].current_displacement += 1;
    rotors[identifier].current_displacement = rotors[identifier].current_displacement % alphabet_size;
}

void stepping_gear::step_rotor_back(string& identifier)
{
    rmsk::simple_assert(rotors.count(identifier) == 0, "programmer error: rotor identifier unknown");

    rotors[identifier].current_displacement += (alphabet_size - 1);
    rotors[identifier].current_displacement = rotors[identifier].current_displacement % alphabet_size;
}


unsigned int stepping_gear::get_rotor_displacement(unsigned int rotor_num)
{
    return get_rotor_displacement(rotor_positions[rotor_num % num_rotors]);    
}

unsigned int stepping_gear::get_ring_pos(unsigned int rotor_num)
{
    return get_ring_pos(rotor_positions[rotor_num % num_rotors]);    
}


stepping_gear::~stepping_gear()
{
    delete stack;
}

bool stepping_gear::rotor_is_at_notch(string& identifier, unsigned int offset)
{   
    return (rotors[identifier].ring.get() != NULL) && (rotors[identifier].ring->get_current_data(offset) != 0);
}

bool stepping_gear::rotor_is_at_notch(const char *identifier, unsigned int offset) 
{ 
    string temp(identifier);
    
    return rotor_is_at_notch(temp, offset);
}

/* ----------------------------------------------------------- */

void odometer_stepping_gear::step_rotors()
{
    unsigned count;
    simple_mod_int current_pos(alphabet_size);
    bool stop = false;

    stepping_gear::step_rotors();

    count = 0;

    // rotor 0 is always stepped
    do
    {
        advance_rotor(rotor_positions[count]);
        // If the rotor that was stepped in the previous line is not at position 0 after the stepping
        // then end the loop. If it is at position 0 then continue the loop and step the next rotor
        // in the following iteration.
        stop = (0 != rotors[rotor_positions[count]].current_displacement);   
        count++;
        
    } while((count < num_rotors) && (!stop));    
}

void odometer_stepping_gear::reset()
{
    unsigned int count = 0;
    
    stepping_gear::reset();
    
    for (count = 0; count < num_rotors; count++)
    {
        // Set position to 0
        set_rotor_displacement(count, 0);
        
        if (rotors[rotor_positions[count]].ring.get() != NULL)
        {
            // Set ring offset to 0 if there is a ring
            rotors[rotor_positions[count]].ring->set_offset(0);
        }
    }
}

void odometer_stepping_gear::print_displacements()
{
    for (unsigned int count = 0; count < num_rotors; count++)
    {
        cout << rotor_positions[count] << ": " << rotors[rotor_positions[count]].current_displacement << endl;
    }
    
    cout << endl;
    
}

