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

#include<rotor.h>
#include<iostream>

/*! \file rotor.cpp
 *
 *  \brief Implementation of the rotor class
 *
 */

rotor::rotor(boost::shared_ptr<permutation> p, unsigned int *d)
   : perm(p)
{
    displacement = d;
    perm_size = p->get_size();
}

rotor::rotor(unsigned int *d)
{
    displacement = d;
}

rotor::rotor()
{
    displacement = &dummy;
    dummy = 0;
}

rotor::~rotor()
{
    //cout << this << endl;
    ;
}

