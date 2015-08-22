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

/*! \file rotor_test.cpp
 *  \brief Implementation of test cases for the ::rotor class.
 */ 

#include<iostream>
#include<alphabet.h>
#include<rmsk_globals.h>
#include<rotor.h>
#include<rotor_test.h>


#define PERM_IV     "esovpzjayquirhxlnftgkdcmwb"

namespace test_rotor {

/*! \brief A class that implements a test case that verifies the behaviour of ::rotor objects
 */
class stator_tester : public test_case {
public:
    /*! \brief Constructor that accepts the test case name as an STL string object.
     */   
    stator_tester(const string& n) : test_case(n) { ; }

    /*! \brief Constructor that accepts the test case name as a zero terminated string.
     */   
    stator_tester(const char *n) : test_case(n)  { ; }
    
    /*! \brief Implementation of test cases.
     */       
    virtual bool test();

};

/*! \brief This method sets the displacement of the rotor object to test and then performs a test
 *         encryption and decryption.
 */
bool stator_tester::test()
{
    bool result = test_case::test();
    boost::shared_ptr<permutation> test_perm = boost::shared_ptr<permutation>(rmsk::std_alpha()->make_perm_ptr(PERM_IV));
    unsigned int d_notch;
    rotor test_notch(test_perm, &d_notch);
    unsigned int in_char, out_char;
    
    d_notch = 4;
    
    result = (test_notch.get_displacement() == 4);
    if (!result)
    {
        append_note("set/get displacement does not work");
        return result;
    }
    
    out_char = test_notch.encrypt(0);
    
    result = (out_char == 11);
    if (!result)
    {
        append_note("unexpected encryption result");
        return result;
    }
    
    in_char = test_notch.decrypt(out_char);
    result = (in_char == 0);
    if (!result)
    {
        append_note("unexpected decryption result");
        return result;
    }
        
    append_note("Notched rotor OK");
    
        
    return result;
}

}

test_rotor::stator_tester test("Rotor test");

void test_rotor::register_tests(composite_test_case *container)
{
    container->add(&test);
}

