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

/*! \file permutation.cpp
 *  \brief Implementation of the ::permutation class.
 */ 

#include<set>
#include<list>
#include<fstream>
#include<stdexcept>
#include<iostream>
#include<permutation.h>

/*! \brief A struct which is used by permutation::get_random_permutation() to choose a random permutation
 *
 */ 
struct rand_helper {
    /*! \brief Random value as returned by the random_generator  
     */
    unsigned int data;
    
    /*! \brief Index of the random value in the unsorted array.  
     */
    unsigned int index;
    
    /*! \brief Operator that allows sorting of rand_helper elements with respect to the random values contained in data. 
     */
    bool operator<(rand_helper& other) { return this->data < other.data; }
};

void permutation::copy(const permutation& p)
{
    vector<unsigned int> help;
    
    perm_size = p.perm_size;
        
    for (unsigned int count = 0; count < get_size(); count++)
    {
        help.push_back(p.perm[count]);
    }
    
    set_permutation(help);
}

permutation::permutation(const permutation& p)
{
    perm = NULL;
    inv_perm = NULL;   
    perm_size = 0;

    copy(p);
}

permutation& permutation::operator=(const permutation& p)
{
    copy(p);
    return *this;
}

permutation::permutation(const vector<unsigned int>& vec)
{
    perm = NULL;
    inv_perm = NULL;
    set_permutation(vec);
}

permutation::permutation(unsigned int *vec, unsigned int size)
{
    vector<unsigned int> help(size);

    perm = NULL;
    inv_perm = NULL;
    
    for (unsigned int count = 0; count < size; count++)
    {
        help[count] = vec[count];
    }
    
    set_permutation(help);
}

void permutation::to_vec(vector<unsigned int>& v)
{
    v.clear();

    for (unsigned int count = 0; count < get_size(); count++)
    {
        v.push_back(perm[count]);
    }
}

void permutation::set_permutation(const vector<unsigned int>& vec)
{   
    if (perm != NULL)
    {
        delete[] perm;
    }
    
    perm = new unsigned int[vec.size()];
    
    if (inv_perm != NULL)
    {
        delete[] inv_perm;
    }
    
    inv_perm = new unsigned int[vec.size()];
    
    perm_size = vec.size();
    
    for (unsigned int count = 0; count < get_size(); count++)
    {
        perm[count] = vec[count] % get_size();
    }    
    
    for (unsigned int count = 0; count < get_size(); count++)
    {
        inv_perm[perm[count]] = count;
    }    
}

/*! \param swaps [in] A vector that specifies which elements of this permutation are to be swapped.
 * 
 * This method allows to modify a permutation by swapping elements of the mapping that defines it. The  
 * vector swaps is interpreted as a sequence of pairs where the first and second elements of the pair
 * determine which elements of the defining mapping are swapped.
 */
void permutation::modify(vector<unsigned int>& swaps)
{
    unsigned int help, inv_help_1, inv_help_2, swap_pos_1, swap_pos_2;

    for(unsigned int count = 0; count < (swaps.size() / 2); count++)
    {
        swap_pos_1 = swaps[2 * count] % perm_size;
        swap_pos_2 = swaps[(2 * count) + 1] % perm_size;
        inv_help_1 = perm[swap_pos_1];
        inv_help_2 = perm[swap_pos_2];  

        perm[swap_pos_1] = inv_help_2;
        perm[swap_pos_2] = inv_help_1;
        
        help = inv_perm[inv_help_1];
        inv_perm[inv_help_1] = inv_perm[inv_help_2];
        inv_perm[inv_help_2] = help;
    }
}

void permutation::print()
{
    for (unsigned int count = 0; count < perm_size; count++)
    {
        cout << perm[count] << " ";
    }
    
    cout << endl;
}

/*! \param cycle_pairs [out] Filled when this permutation specifies an involution. Cleared otherwise.
 * 
 * An involution in the sense relevant here is a permutation which is the inverse of itself 
 * and does not encrypt any letter to itself. If the permutation is not an involution, the  
 * set cycle_pairs is empty. Otherwise it is filled with the pairs of cycles of the permutation. 
 * 
 */
void permutation::test_for_involution(set<pair<unsigned int, unsigned int> >& cycle_pairs)
{
    bool char_encrypted_to_itself = false, difference_found = false;    
    vector<unsigned int> self, other;
    permutation my_inverse = get_inverse();
    set<unsigned int> already_used;
    unsigned int c = 0;
    set<unsigned int>::iterator iter;
    
    cycle_pairs.clear();
    to_vec(self);
    my_inverse.to_vec(other);
    
    // Searches for chars that encrypt to themselves
    for (unsigned int count = 0; (count < get_size()) and (!char_encrypted_to_itself); count++)
    {
        char_encrypted_to_itself = (count == permute(count));
    }
    
    // Checks whether this permutation is the inverse of itself
    for (unsigned int count = 0; (count < get_size()) and (!difference_found); count++)
    {
        difference_found = (self[count] != other[count]);
    }
    
    if ((!char_encrypted_to_itself) && (!difference_found))
    {
        // Generates cycle pairs
        c = 0;
        while(already_used.size() != get_size())
        {
            if (already_used.find(c) == already_used.end())
            {
                cycle_pairs.insert(pair<unsigned int, unsigned int>(c, permute(c)));
                already_used.insert(c);
                already_used.insert(permute(c));
            }
            
            c++;
        }
    }
}

permutation permutation::get_inverse()
{
    return permutation(inv_perm, perm_size);
}

permutation permutation::get_identity(unsigned int size)
{
    vector<unsigned int> result;
    
    for (unsigned int count = 0; count < size; count++)
    {
        result.push_back(count);
    }
    
    return permutation(result);        
}

/*! \param in [in] random_generator used to choose random permutation.
 * 
 *  \param size [in] size of the random permutation
 *
 * This method can throw exceptions, if the random_generator signals an error. There are more efficient algorithms
 * for generating ranom permutations, but this algorithm works reasonably fast for the intended purposes.
 */
permutation permutation::get_random_permutation(random_generator& in, unsigned int size)
{
    vector<unsigned int> result;
    list<rand_helper> helper;
    list<rand_helper>::iterator iter;
    set<unsigned int> already_used;
    unsigned int temp;
    unsigned int numbers_drawn = 0;
    
    if (!in.is_ok())
    {
        throw runtime_error("random number generator failure");
    }
    else
    {
        while(numbers_drawn < size)
        {
            in.read_symbol(&temp, size);
            
            if (!in.is_ok())
            {
                throw runtime_error("random number generator failure");
            }
            else
            {
                if (already_used.count(temp) == 0)
                {
                    rand_helper h; h.data = temp; h.index = numbers_drawn;
                    already_used.insert(temp);
                    helper.push_back(h);
                    numbers_drawn++;                    
                }
            }
        }
        
        helper.sort();
        
        for (iter = helper.begin(); iter != helper.end(); ++iter)
        {
            result.push_back((*iter).index);
        }
    }
    
    return permutation(result);    
}

