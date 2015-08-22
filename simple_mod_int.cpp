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

#include<simple_mod_int.h>
/*! \file simple_mod_int.cpp
 *
 *  \brief Implementation of the simple_mod_int class
 *
 */ 

simple_mod_int::simple_mod_int(unsigned int val, unsigned int mod)
{
    value = val % mod;
    modulus = mod;
}

void simple_mod_int::decrement()
{
    if (value == 0)
    {
        value = modulus - 1;
    }
    else
    {
        value--;
    }
}

simple_mod_int simple_mod_int::operator-()
{
    simple_mod_int result(modulus);
    result.value = reduce(modulus - value);
    
    return result;    
}


