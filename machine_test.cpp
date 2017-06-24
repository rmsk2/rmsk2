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

/*! \file machine_test.cpp
 *  \brief This file contains a class that implements a simple rotor_machine
 *         using an odometer_stepping_gear which is used in a test_case.
 */ 

#include<memory>
#include<string>
#include<boost/shared_ptr.hpp>

#include<alphabet.h>
#include<rmsk_globals.h>
#include<machine_test.h>
#include<rotor_machine.h>

using namespace std;

#define PERM_I    "ekmflgdqvzntowyhxuspaibrcj"
#define PERM_II   "ajdksiruxblhwtmcqgznpyfvoe"
#define PERM_III  "bdfhjlcprtxvznyeiwgakmusqo"
#define PERM_V    "vzbrgityupsdnhlxawmjqofeck"
#define PERM_VI   "jpgvoumfyqbenhzrdkasxlictw"
#define PERM_UKW_B     "yruhqsldpxngokmiebfzcwvjat"

namespace test_machine {

/*! \brief A class that tests whether a test_machine is able to successfully encrypt and decrypt by first
 *         encrypting a test string and then decrypting it again. 
 *
 *  The test is successfull, if the decryption result matches the original input string. 
 */
class machine_test_case : public test_case {
public:

    /*! \brief Constructor that accepts the test case name as an STL string object.
     */  
    machine_test_case(const string& n) : test_case(n) { ; }
    
    /*! \brief Constructor that accepts the test case name as a zero terminated string.
     */    
    machine_test_case(const char *n) : test_case(n) { ; }    
    
    /*! \brief Performs the en/decryption test.
     */           
    virtual bool test();

    /*! \brief Destructor.
     */               
    virtual ~machine_test_case() { ; }
    
protected:

    /*! \brief Method that creates the ::test_machine.
     */       
    boost::shared_ptr<rotor_machine> make_machine();    
    
    /*! \brief Variable that holds the test_machine that is used in this test_case.
     */           
    boost::shared_ptr<rotor_machine> machine;
};


/*! \brief A class that implements a rotor_machine with four rotors that uses an odometer_stepping_gear.
 */
class test_machine : public rotor_machine {
public:
    /*! \brief Constructor.
     */                   
    test_machine();   
     
    /*! \brief Destructor.
     */               
    virtual ~test_machine() { ; }
    
protected:

    /*! \brief Points to the permutation used in the fast rotor.
     */               
    boost::shared_ptr<permutation> p_fast;

    /*! \brief Points to the permutation used in the medium rotor.
     */               
    boost::shared_ptr<permutation> p_medium;
    
    /*! \brief Points to the permutation used in the slow rotor.
     */                   
    boost::shared_ptr<permutation> p_slow;
    
    /*! \brief Points to the permutation used in slowest rotor which can also be used
     *         as the reflecting rotor.
     */                   
    boost::shared_ptr<permutation> p_reflecting;
    
    /*! \brief Points to the permutation used in the input tranformation.
     */                   
    boost::shared_ptr<permutation> p_in;

    /*! \brief Points to the permutation used in the output tranformation.
     */                   
    boost::shared_ptr<permutation> p_out;
    
    /*! \brief Points to the odometer_stepping_gear used by this machine.
     */                       
    shared_ptr<odometer_stepping_gear> odo_step;
};

/*! The permutations are borrowed from the Enigma simulator.
 */ 
test_machine::test_machine()
    : rotor_machine()
{
    vector<string> rotor_names;
    vector<boost::shared_ptr<rotor> > rotors;        
    
    p_in = boost::shared_ptr<permutation>(rmsk::std_alpha()->make_perm_ptr(PERM_V));
    p_out = boost::shared_ptr<permutation>(rmsk::std_alpha()->make_perm_ptr(PERM_VI));
    
    p_fast = boost::shared_ptr<permutation>(rmsk::std_alpha()->make_perm_ptr(PERM_I));
    p_medium = boost::shared_ptr<permutation>(rmsk::std_alpha()->make_perm_ptr(PERM_II));
    p_slow = boost::shared_ptr<permutation>(rmsk::std_alpha()->make_perm_ptr(PERM_III));
    p_reflecting = boost::shared_ptr<permutation>(rmsk::std_alpha()->make_perm_ptr(PERM_UKW_B));
        
    rotor_names.push_back("fast");
    rotor_names.push_back("medium");
    rotor_names.push_back("slow");
    rotor_names.push_back("reflecting");
    
    odo_step.reset(new odometer_stepping_gear(rotor_names, rmsk::std_alpha()->get_size()));
    
    rotors.push_back(boost::shared_ptr<rotor>(new rotor(p_fast, NULL)));
    rotors.push_back(boost::shared_ptr<rotor>(new rotor(p_medium, NULL)));
    rotors.push_back(boost::shared_ptr<rotor>(new rotor(p_slow, NULL)));
    rotors.push_back(boost::shared_ptr<rotor>(new rotor(p_reflecting, NULL)));
    
    odo_step->insert_all_rotors(rotors);
    
    set_stepping_gear(odo_step.get());
    set_input_transform(p_in);
    set_output_transform(p_out);
}

boost::shared_ptr<rotor_machine> machine_test_case::make_machine()
{
    return boost::shared_ptr<rotor_machine>(new test_machine());
}


/*! This test first encrypts a test string and then decrypts it. In order to be successfull the
 *  decrypted string has to match the original input string. After the first test the machine is
 *  configured to use the last rotor as reflecting rotor and the en/decryption is performed a second
 *  time.
 */ 
bool machine_test_case::test()
{
    bool result = test_case::test();
    machine = make_machine();
    alphabet_transformer<char> transformer(rmsk::std_alpha(), machine.get());
    string plain("hallodiesisteintollerwahnsinnstestfuerdietestrotormaschine");
    string cipher, plain_again;
    
    append_note("Plaintext : " + plain);
    transformer.encrypt(plain, cipher);
    append_note("Ciphertext: " + cipher);
    machine->get_stepping_gear()->reset();
    transformer.decrypt(cipher, plain_again);
    
    result = (plain == plain_again);
    if (!result)
    {
        append_note("Decryption failed!");
    }    
    
    machine->get_stepping_gear()->reset();
    
    append_note("Switching to reflecting machine");
    // Turn last rotor into reflector
    machine->get_stepping_gear()->get_stack().set_reflecting_flag(true);
    
    append_note("Plaintext : " + plain);
    transformer.encrypt(plain, cipher);
    append_note("Ciphertext: " + cipher);
    machine->get_stepping_gear()->reset();
    transformer.decrypt(cipher, plain_again);
    
    result = (plain == plain_again);
    if (!result)
    {
        append_note("Decryption failed!");
    }    
    
    return result;
}

}

test_machine::machine_test_case machine_test("Whole machine test");

void test_machine::register_tests(composite_test_case *container)
{
    container->add(&machine_test);
}


