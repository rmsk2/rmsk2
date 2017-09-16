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

/*!\file rand_perm.cpp 
 * \brief Contains a program that allows to generate random permutations, involutions and fixpoint 
 *        free permutations.
 */

#include<iostream>
#include<alphabet.h>
#include<rmsk_globals.h>

using namespace std;

unsigned int const NUM_PERMS = 10;


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
