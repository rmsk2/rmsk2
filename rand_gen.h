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

#ifndef __rand_gen_h__
#define __rand_gen_h__

/*! \file rand_gen.h
 *  \brief Header file several classes dealing with random numbers.
 */ 


#include<fstream>
#include<vector>

using namespace std;


/*! \brief An abstract class providing an interface for an object that knows how to generate random data.
 */ 
class random_generator {
public:

    /*! \brief Returns a random unsigned int and stores it in the unsigned int to which the parameter rand_num points.
     */ 
    virtual void read(unsigned int *rand_num) = 0;

    /*! \brief Returns a random unsigned int in the range between 0 and alphabet_size - 1 and stores it in the unsigned int
     *         to which the parameter rand_symbol points. alphabet_size must be from the range 1..2^24.
     */ 
    virtual void read_symbol(unsigned int *rand_symbol, unsigned int alphabet_size);

    /*! \brief Fills the vector rand_bytes with bytes_to_read random bytes.
     */ 
    virtual void read(vector<unsigned char>& rand_bytes, unsigned int bytes_to_read) = 0;
    
    /*! \brief Fills the vector rand_symbols with symbols_to_read unsigned ints, where each of the vector elements is between
     *         0 and alphabet_size-1
     */     
    virtual void read_symbols(vector<unsigned int>& rand_symbols, unsigned int symbols_to_read, unsigned int alphabet_size);
    
    /*! \brief Returns true if the state of the random generator indicates that it is *not* in error.
     */     
    virtual bool is_ok() = 0;

    /*! \brief Destructor.
     */         
    virtual ~random_generator() { ; }
};

/*! \brief A class that implements the random_generator interface by reading from /dev/urandom
 */
class urandom_generator : public random_generator {
public:

    /*! \brief Default constructor.
     */
    urandom_generator();

    /*! \brief Reads sizeof(unsigned int) random bytes from /dev/urandom and stores them in the unsigned int to which
     *         the parameter rand_num points.
     */
    virtual void read(unsigned int *rand_num);

    /*! \brief Fills the vector rand_symbols with symbols_to_read unsigned ints, where each of the vector elements is between
     *         0 and alphabet_size-1 and are read from /dev/urandom.
     */ 
    virtual void read(vector<unsigned char>& rand_bytes, unsigned int bytes_to_read);

    /*! \brief Returns true if the underlying std::istream object is OK, i.e. not in an error state.
     */
    virtual bool is_ok();

    /*! \brief Destructor.
     */    
    virtual ~urandom_generator();

protected:
    ifstream in;
};

/*! \brief An abstract class providing an interface for an object that knows how to generate random ring data for use with
 *         rotor_ring objects.
 */ 
class rotor_ring_random_source {
public:
    /*! \brief Constructs a rotor_ring_random_source instance given the size of the ring data vectors to generate. The size
     *         of the ring data vectors is equal to the size of the rotors to which the rings are intended to be attached.
     *
     *  Uses a urandom_generator object as underlying random generator.
     */
    rotor_ring_random_source(unsigned int rotor_size) { size = rotor_size; rand_gen = &urand; }

    /*! \brief Constructs a rotor_ring_random_source instance given the size of the ring data vectors to generate. The size
     *         of the ring data vectors is equal to the size of the rotors to which the rings are intended to be attached. In
     *         addition this constructor also requires a random_generator object which is subsequently used as the underlying
     *         random_generator for this rotor_ring_random_source object.
     */
    rotor_ring_random_source(unsigned int rotor_size, random_generator *r_gen) { size = rotor_size; rand_gen = r_gen; }    

    /*! \brief Returns an unsigned int that can be used as a piece of ring data.
     */
    virtual unsigned int get_next_val() = 0;

    /*! \brief Returns the rotor size.
     */
    virtual unsigned int get_rotor_size() { return size; }
    
    /*! \brief Resets the state of this rotor_ring_random_source object.
     *
     *  This method is intended to clear any ring data from this rotor_ring_random_source which may have been created
     *  in advance. Calling this method ensures that "fresh" random data is returned by the next call made to get_next_val().
     */    
    virtual void reset() { ; }

    /*! \brief Returns a pointer to the random_generator object that is used to generate random data in this
     *         rotor_ring_random_source instance.
     */        
    virtual random_generator *get_random_generator() { return rand_gen; }

    /*! \brief Destructor.
     */
    virtual ~rotor_ring_random_source() { ; }

protected:

    /*! \brief Holds the size of the ring data vectors to create.
     */
    unsigned int size;

    /*! \brief Holds the urandom_generator that is used as a default random_generator.
     */    
    urandom_generator urand;

    /*! \brief Holds the random_generator object that is currently used by this rotor_ring_random_source instance.
     */    
    random_generator *rand_gen;
};


/*! \brief A class that allows to retrieve random ring data for rotor_ring objects which consists of zeros and ones.
 */
class random_bit_source : public rotor_ring_random_source {
public:
    /*! \brief Constructs a random_bit_source instance using the ring size/rotor size and a random_generator object that is
     *         subsequently used to generate the raw random data.
     *
     *  This class generates a whole ring data vector in one go and returns the generated values bit by bit. The counter in
     *  bit_count is used to determine when all the precomputed data is used up and a new ring data vector has to be 
     *  generated.
     */    
    random_bit_source(unsigned int size, random_generator *r_gen) : rotor_ring_random_source(size, r_gen) { bit_count = 0; }

    /*! \brief Constructs a random_bit_source instance using the ring size/rotor size only. The internal urandom_generator is
     *         used for random number generation.
     */    
    random_bit_source(unsigned int size) : rotor_ring_random_source(size) { bit_count = 0; }
    
    /*! \brief Returns a 0 or a 1 that can be used as ring data.
     */    
    virtual unsigned int get_next_val();

    /*! \brief Resets the bit_count variable to zero and clears the rand_bits vector.
     */    
    virtual void reset();
    
    /*! \brief Destructor.
     */    
    virtual ~random_bit_source() { ; }

protected:
    /*! \brief Holds a counter that indicates how many precomputed values have already been delivered
     *         the the caller.
     */
    unsigned int bit_count;

    /*! \brief Holds the precomputed values that are returned piece by piece through calls to get_next_val().
     */
    vector<unsigned int> rand_bits;
};


#endif /* __rand_gen_h__ */

