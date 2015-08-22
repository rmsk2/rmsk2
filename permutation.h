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

/*! \file permutation.h
 *  \brief Provides an interface for a class that implements permutations over non-negative integers.
 */ 
#ifndef __permuation_h__
#define __permuation_h__

#include<vector>
#include<set>
#include<utility>
#include<transforms.h>
#include<rand_gen.h>

using namespace std;

/*! \brief A class that abstracts the notion of a permutation of the numbers 0 ... n-1.
 *
 *  A permutation is a bijective mapping of the integers 0 ... n-1 onto themselves. n is called the size of the permutation.
 *  Permutations are used all over rmsk. A physical rotor for instance is a permutation which is implemented by wiring
 *  contacts that are placed on different sides of a thin cylinder. A permutation is also the simplest form of
 *  encryption_transform that rmsk provides. This class implements a permutation. 
 *
 *  This class makes use of the fact that each vector or array of type unsigned int and length n implicitly specifies a mapping of the integers
 *  between 0 and n-1 to the set of all unsigned integers. The result of the mapping of some value k, 0 <= k <= n-1 is simply the value of the array/vector
 *  at position k. Therefore the array/vector specifies a mapping of the values 0 ... n-1 onto themselves if it only contains values between
 *  0 and n-1. On top of that it specifies a permutation if it contains each value between 0 and n-1 exactly once.
 */ 
class permutation : public encryption_transform {
public:
    /*! \brief Copy constructor. 
     */
    permutation(const permutation& p);
    
    /*! \brief Constructs a permutation from a vector of unsigned integers. 
     */
    permutation(vector<unsigned int>& vec);
    
    /*! \brief Constructs a permutation from a C-style array of length size given in the parameter vec.  
     */
    permutation(unsigned int *vec, unsigned int size);
    
    /*! \brief Default constructor: Constructs a permutation of size 0. Permutations of size 0 can not be used to encrypt() and decrypt(). 
     */
    permutation() { perm = NULL; inv_perm = NULL; perm_size = 0; }
    
    /*! \brief Applies the permutation to the value given in the parameter in_char. 
     */
    virtual unsigned int encrypt(unsigned int in_char) { return perm[in_char]; }
    
    /*! \brief Applies the inverse permutation to the value given in the parameter in_char. 
     */
    virtual unsigned int decrypt(unsigned int in_char) { return inv_perm[in_char]; }
    
    /*! \brief Returns the size of the permutation. 
     */
    virtual unsigned int get_size() { return perm_size; }
    
    /*! \brief If this permutation specifies a valid involution the set cycle_pairs is filled with the commutations that make up the involution. 
     *         Otherwise the set is cleared.
     */
    virtual void test_for_involution(set<pair<unsigned int, unsigned int> >& cycle_pairs);
    
    /*! \brief Inline version of encrypt(). 
     */
    inline unsigned int permute(unsigned int c) { return perm[c]; }
    
    /*! \brief Fills the parameter v with a representation of this permutation. 
     */
    virtual void to_vec(vector<unsigned int>& v);
    
    /*! \brief Returns a permutation object that implements the inverse of this permutation. 
     */
    virtual permutation get_inverse();
    
    /*! \brief Switches this permutation to its inverse. 
     */
    virtual void switch_to_inverse() { unsigned int *temp = perm; perm = inv_perm; inv_perm = temp; } 
    
    /*! \brief Modifies this permutation by swapping the values on the positions given by the parameter swaps. 
     */
    virtual void modify(vector<unsigned int>& swaps);
    
    /*! \brief Prints a representation of this permutation to stdout. 
     */
    virtual void print();
    
    /*! \brief Inline version of decrypt(). 
     */
    inline unsigned int inv(unsigned int c) { return inv_perm[c]; }
    
    /*! \brief Returns a permutation object representing a permutation of a given size that has been randomly chosen using the specified random_generator. 
     */
    static permutation get_random_permutation(random_generator& in, unsigned int size);
    
    /*! \brief Returns the permutation of the specified size that maps each number between 0 and size-1 to itself. 
     */
    static permutation get_identity(unsigned int size);    
    
    /*! \brief Assignment operator. 
     */
    virtual permutation& operator=(const permutation& p);
    
    /*! \brief Destructor. 
     */
    virtual ~permutation() { delete[] perm; delete[] inv_perm; }

protected:
    
    /*! \brief Helper method that fills perm and inv_perm and set perm_size. 
     */
    virtual void set_permutation(vector<unsigned int>& vec);
    
    /*! \brief Uses the data members of the permutation given in parameter p to set the members of this permutation. 
     */
    void copy(const permutation& p);

    /*! \brief Size of this permutation. 
     */    
    unsigned int perm_size;
    
    /*! \brief Mapping that specifies the permutation. 
     */
    unsigned int *perm;
    
    /*! \brief Mapping that specifies the inverse permutation. 
     */    
    unsigned int *inv_perm;
};


#endif /* __permuation_h__ */

