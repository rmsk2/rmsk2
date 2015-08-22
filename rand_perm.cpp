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

/*!\file rand_perm.cpp 
 * \brief Contains a program that allows to generate random permutations, involutions and fixpoint 
 *        free permutations.
 */

#include<iostream>
#include<alphabet.h>
#include<rmsk_globals.h>

using namespace std;

unsigned int const NUM_PERMS = 10;

/*! \brief A class containing static helper methods for implementing the rand_perm program.
 */
class rand_perm_helper {
public:
    /*! \brief Prints the permutations given in the STL vector perms to stdout.
     */
    static void pretty_print(vector<permutation>& perms);

    /*! \brief Prints the permutations given in the STL vector perms to stdout in a format that can be pasted
     *         in the C++ file implementing the SG39.
     */    
    static void pretty_print_sg39(vector<permutation>& perms);

    /*! \brief Checks whether the permutation referenced through perm has a "fix point", i.e. it checks     
     *         whether the specified permutation maps at leat one input to itself. Returns false if at least
     *         one input value is found that is mapped to itself.
     */
    static bool is_fix_point_free(permutation& perm);

    /*! \brief Counts how often the permutation referenced through perm shifts a character only one position ahead.
     *
     *   Example of shifting a character one position ahead would be perm.ecnrypt(4) = 5. 
     */
    static unsigned short num_of_single_shifts(permutation& perm);

    /*! \brief Returns true if there is no position in which at least two of the permutations in parameter perms return
     *         the same value.
     */
    static bool check_different_results(vector<permutation>& perms);

};

void rand_perm_helper::pretty_print(vector<permutation>& perms)
{
    vector<permutation>::iterator iter;
    
    cout << "abcdefghijklmnopqrstuvwxyz" << endl;
    cout << "--------------------------" << endl;
    
    for (iter = perms.begin(); iter != perms.end(); ++iter)
    {
        rmsk::std_alpha()->print_perm(*iter);
    }    
}

void rand_perm_helper::pretty_print_sg39(vector<permutation>& perms)
{
    vector<permutation>::iterator iter;
    unsigned int  count = 0;
    
    for (iter = perms.begin(); iter != perms.end(); ++iter)
    {
        cout << "sg39_set.add_rotor(SG39_ROTOR_" << count << ", rmsk::std_alpha()->to_vector(string(\"";
        cout << rmsk::std_alpha()->perm_as_string(*iter) << "\")));" << endl;
        count++;
    }    
}


bool rand_perm_helper::is_fix_point_free(permutation& perm)
{
    bool result = true;
    
    for (unsigned int count = 0; (count < perm.get_size()) && result; count++)
    {
        result = result && (perm.encrypt(count) != count);
    }
    
    return result;
}

unsigned short rand_perm_helper::num_of_single_shifts(permutation& perm)
{
    unsigned short result = 0;
    unsigned int perm_size = perm.get_size();
    
    for (unsigned int count = 0; count < perm_size; count++)
    {
        result += (perm.encrypt(count) == ((count + 1) % perm_size) ? 1 : 0);
    }
    
    return result;
}

bool rand_perm_helper::check_different_results(vector<permutation>& perms)
{
    bool result = true;
    set<unsigned int> test_set;
    
    if (perms.size() != 0)
    {
        for (unsigned int count = 0; (count < perms[0].get_size()) and result; count++)            
        {
            test_set.clear();
            
            for (unsigned int i = 0; (i < perms.size()) and result; i++)
            {
                test_set.insert(perms[i].encrypt(count));
            }
            
            result = (test_set.size() == perms.size());
        }
    }
    
    return result;
}

/*! \brief main function implementing the rand_perm program.
 */
int main()
{
    vector<permutation> perms;
    
    // Determine random permutations
    cout << "Random permutations:" << endl;
    cout << endl;
    for(unsigned int count = 0; count < NUM_PERMS; count++)
    {
        perms.push_back(rmsk::std_alpha()->get_random_permutation());
    }    
    rand_perm_helper::pretty_print(perms);

    perms.clear();    
    cout << endl;
    
    // Determine random involutions
    cout << "Random involutions:" << endl;
    cout << endl;
    for(unsigned int count = 0; count < NUM_PERMS; count++)
    {
        perms.push_back(rmsk::std_alpha()->get_random_involution());
    }
    rand_perm_helper::pretty_print(perms);

    perms.clear();    
    cout << endl;
    
    // Determine random fixpoint free permutations
    cout << "SG39 permutations:" << endl;
    cout << endl;
    while(perms.size() != NUM_PERMS)
    {
        permutation perm = rmsk::std_alpha()->get_random_permutation();
        
        if ((rand_perm_helper::is_fix_point_free(perm)) and (rand_perm_helper::num_of_single_shifts(perm) == 0))
        {
            perms.push_back(perm);
        }
    }

    rand_perm_helper::pretty_print(perms);    
    cout << endl << "As code:" << endl;
    cout << "--------------------------" << endl;
    rand_perm_helper::pretty_print_sg39(perms);

    return 0;
}
