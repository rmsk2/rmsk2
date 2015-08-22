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

/*! \file rand_gen.cpp
 *  \brief This file contains the implementation of several classes dealing with random numbers.
 */ 

#include<boost/smart_ptr.hpp>
#include<rand_gen.h>
//#include<permutation.h>
#include<rmsk_globals.h>

void random_generator::read_symbols(vector<unsigned int>& rand_symbols, unsigned int symbols_to_read, unsigned int alphabet_size)
{
    unsigned int help;

    rand_symbols.clear();

    for (unsigned int count = 0; count < symbols_to_read; count++)
    {
        read_symbol(&help, alphabet_size);
        rand_symbols.push_back(help);
    }
}

/*! This method is implemented as follows. At first it determines the biggest multiple of alphabet_size that is smaller than or
 *  equal to 2^24 and stores that value in the variable max_val_allowed. After that a random number is drawn via the read method and
 *  reduced to a number < 2^24. If that number is smaller than max_val_allowed then it is reduced modulo alphabet_size. If it is
 *  bigger or equal another number is drawn. This algorithm ensures that all numbers returned are uniformly distributed in the range
 *  0 ... alphabet_size-1. 
 * 
 */
void random_generator::read_symbol(unsigned int *rand_symbol, unsigned int alphabet_size)
{
    unsigned max_val_allowed, current_val;
    
    alphabet_size = alphabet_size % (16777216 + 1);
    
    if (alphabet_size == 0)
    {
        alphabet_size = 1;
    }
    
    max_val_allowed = (16777216 / alphabet_size) * alphabet_size;
    
    do
    {
        read(&current_val);
        current_val >>= 8;
        
    } while (current_val >= max_val_allowed);
    
    *rand_symbol = current_val % alphabet_size;        
}

urandom_generator::urandom_generator()
    : in("/dev/urandom", ios::binary | ios::in)
{
    ;
}

void urandom_generator::read(unsigned int *rand_num)
{
    in.read((char *)rand_num, sizeof(unsigned int));
}

void urandom_generator::read(vector<unsigned char>& rand_bytes, unsigned int bytes_to_read)
{
    // auto_ptr for arrays generated with new []
    boost::scoped_array<char> help(new char[bytes_to_read]);
    
    rand_bytes.clear();
    
    // Fill an array with random data
    in.read(help.get(), bytes_to_read);
    
    // Copy array to vector    
    for (unsigned int count = 0; count < bytes_to_read; count++)
    {
        rand_bytes.push_back((unsigned char)help[count]);
    } 
}

bool urandom_generator::is_ok()
{
    return in;
}

urandom_generator::~urandom_generator()
{
    in.close();
}

void random_bit_source::reset()
{
    bit_count = 0;
    rand_bits.clear();
}

unsigned int random_bit_source::get_next_val()
{
    unsigned int result = 0;
    vector<unsigned char> raw_rand;

    if (bit_count == 0)
    {
        // Get raw random data
        rand_gen->read(raw_rand, get_rotor_size());
        rmsk::simple_assert(!rand_gen->is_ok(), "random number generator failure");
        
        // Fill buffer by extracting the least significant bit of each byte of raw random data
        for (unsigned int count = 0; count < get_rotor_size(); count++)
        {
            rand_bits.push_back(((unsigned int)raw_rand[count]) & 1);
        }
    }
    
    result = rand_bits[bit_count];
    bit_count++;
    
    if (bit_count >= get_rotor_size())
    {
        reset();
    }
    
    return result;
}

