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

#include<rotor_ring.h>
#include<rmsk_globals.h>

/*! \file rotor_ring.cpp
 *  \brief Implementation of the rotor_ring class.
 */

rotor_ring::rotor_ring(rotor *r, vector<unsigned int>& data) 
{ 
    rot = r; 
    ring_data = NULL; 
    offset = 0; 
    
    if (rot != NULL) 
    { 
        rot_size = rot->get_size();         
    }
    
    set_ring_data_priv(data); 
}

rotor_ring::rotor_ring(vector<unsigned int>& data) 
{ 
    rot = NULL; 
    ring_data = NULL; 
    offset = 0; 
    
    rot_size = data.size(); 
    set_ring_data_priv(data); 
}    

void rotor_ring::set_pos(unsigned int new_pos)
{   
    new_pos -= offset;
    
    if (new_pos & 0x80000000)
    {
        new_pos += rot_size;
    }
    
    *(rot->get_displacement_var()) = new_pos;
}

unsigned int rotor_ring::get_current_data(unsigned int desired_offset)
{
    unsigned int read_pos = get_pos();
    
    desired_offset = desired_offset % rot_size;
        
    read_pos += desired_offset;
    
    if (read_pos >= rot_size)
    {
        read_pos -= rot_size;
    }
    
    return ring_data[read_pos];
}

void rotor_ring::copy(const rotor_ring& r)
{
    vector<unsigned int> help;
    
    offset = r.offset;
    rot = r.rot;
    
    for (unsigned int count = 0; count < r.rot_size; count++)
    {
        help.push_back(r.ring_data[count]);
    }
    
    set_ring_data_priv(help);    
}

void rotor_ring::set_ring_data_priv(vector<unsigned int>& r_dat) 
{
    rmsk::simple_assert((rot != NULL) && (r_dat.size() != rot->get_size()), "Wrong size of ring data");
     
    if (ring_data != NULL)
    {
        delete[] ring_data;
    }
    
    ring_data = new unsigned int[rot_size];
    
    for (unsigned int count = 0; (count < r_dat.size()) and (count < rot_size); count++)
    {
        ring_data[count] = r_dat[count];
    }
}

void rotor_ring::get_ring_data_priv(vector<unsigned int>& r_dat) 
{ 
    r_dat.clear();
    
    for (unsigned int count = 0; count < rot_size; count++)
    {
        r_dat.push_back(ring_data[count]);
    }
}

