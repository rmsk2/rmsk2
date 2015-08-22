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

#include<iostream>
#include<boost/shared_ptr.hpp>
#include<boost/lexical_cast.hpp>
#include<rmsk_globals.h>
#include<stepping_test.h>

/*! \file stepping_test.cpp
 *  \brief This file contains classes that implement unit tests for the
 *         stepping_gear class.
 */ 

#define PERM_I    "ekmflgdqvzntowyhxuspaibrcj"
#define PERM_II   "ajdksiruxblhwtmcqgznpyfvoe"
#define PERM_III  "bdfhjlcprtxvznyeiwgakmusqo"
#define PERM_IV   "esovpzjayquirhxlnftgkdcmwb"

const unsigned int NUM_FEEDBACK_POINTS = 6;

namespace test_stepping {

/*! \brief Base class for the other test cases that are implemented in this file
 *
 *  It implements a rotor machine with four rotors, three of which step in an odometer like
 *  fashion. One of the permutations can be configured to be used as a reflector.
 */
class stepping_test : public test_case {
public:
    /*! \brief Constructor that accepts the test case name as an STL string object.
     */  
    stepping_test(const string& n) : test_case(n) { setup_data(); }
    
    /*! \brief Constructor that accepts the test case name as a zero terminated string.
     */     
    stepping_test(const char *n) : test_case(n) { setup_data(); }
    
    /*! \brief Encrypts the string specified in the parameter data and stores the output in data_out
     */     
    void encrypt_string(string& data, string& data_out);

    /*! \brief Decrypts the string specified in the parameter data and stores the output in data_out
     */     
    void decrypt_string(string& data, string& data_out);
    
    /*! \brief Destructor
     */         
    virtual ~stepping_test() { ; }
    
protected:

    /*! \brief Permutation to be used in the fast moving rotor
     */     
    boost::shared_ptr<permutation> fast_perm;

    /*! \brief Permutation to be used in the rotor that moves with medium speed
     */     
    boost::shared_ptr<permutation> slow_perm;
    
    /*! \brief Permutation to be used in the slow moving rotor
     */         
    boost::shared_ptr<permutation> slower_perm;

    /*! \brief Permutation to be used in the slowest moving rotor
     */         
    boost::shared_ptr<permutation> perm_iv;
    
    /*! \brief Names of the rotor slots used in this test
     */         
    vector<string> rotor_names;
    
    /*! \brief Points to an odometer_stepping_gear object that is used to step the machine and which holds
     *         the rotor_stack to which the permutations are added
     */         
    boost::shared_ptr<odometer_stepping_gear> stepper;
    
    /*! \brief Instantiates an odometer_stepping_gear object and adds the permutations to it
     */             
    void make_sample_odo();
    
    /*! \brief Instantiates permutation objects and assignes them to the corresponding _perm variables
     */    
    void setup_data();
        
};

/*! \brief A class that tests whether a stepping_gear is able to successfully encrypt and decrypt by first
 *         encrypting a test string and then decrypting it again. 
 *
 *  The test is successfull, if the decryption result matches the original input string. 
 */
class enc_dec_test : public stepping_test {
public:
    /*! \brief Constructor that accepts the test case name as an STL string object. On top of that the parameter t_string contains the
     *         the plantext string that is to be encrypted. The parameter reflecting_flag is uesed to specify whether the ::rotor_stack
     *         should be used in a reflecting configuration (= true) or not (= false).
     */  
     enc_dec_test(const string& name, string& t_string, bool reflecting_flag) : stepping_test(name) { test_string = t_string; is_reflecting = reflecting_flag; }

    /*! \brief Constructor that accepts the test case name as well as the test string as a C-style strings. Other parameters are the same as in 
     *         enc_dec_test(const string&, string&, bool)
     */  
     enc_dec_test(const char *name, const char *t_string, bool reflecting_flag) : stepping_test(name) { test_string = string(t_string); is_reflecting = reflecting_flag; }

    /*! \brief Constructor that accepts the test case name as a C-style string. Other parameters are the same as in enc_dec_test(const string&, string&, bool)
     */  
     enc_dec_test(const char *name, string& t_string, bool reflecting_flag) : stepping_test(name) { test_string = t_string; is_reflecting = reflecting_flag; }
     
    /*! \brief This method has to be called if the ::rotor_stack is intended to be used in a feedback configuration during the test. The parameter
     *         points specifies the the indices of the feedback points in an unsigned int native array and the parameter num_points gives the length
     *         of that array.
     */       
     virtual void set_feedback_points(unsigned int *points, unsigned int num_points);
     
    /*! \brief Performs the en/decryption test
     */       
     virtual bool test();
     
    /*! \brief Destructor
     */       
     virtual ~enc_dec_test() { ; }
     
protected:

    /*! \brief Holds the test string
     */       
    string test_string;
    
    /*! \brief Stores the value of the parameter reflecting_flag of the constructor
     */           
    bool is_reflecting;
    
    /*! \brief Stores the feedback points set by the set_feedback_points() method
     */           
    set<unsigned int> feedback_points;
};

/*! \brief A class that tests whether the various ways of setting and getting information about the current ::rotor displacements work as intended
 */
class displacement_test : public stepping_test {
public:
    /*! \brief Constructor that accepts the test case name as an STL string object.
     */
     displacement_test(string& name) : stepping_test(name) { test_string = "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz"; }
     
    /*! \brief Constructor that accepts the test case name as a zero terminated string.
     */          
     displacement_test(const char *name) : stepping_test(name) { test_string = "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz"; }
     
    /*! \brief Implements the actual test.
     */          
     virtual bool test();
     
    /*! \brief Destructor
     */          
     virtual ~displacement_test() { ; }
     
protected:

    /*! \brief Stores the string that is used for the tests
     */     
    string test_string;
};

/*! \brief Feedback points used for "Feedback enc/dec" test
 */     
unsigned int test_feedback[NUM_FEEDBACK_POINTS] = {25, 24, 23, 22, 21, 20};

/*! \brief Test string that is to be en/decrypted
 */     
string short_test("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");

/*! \brief Test case that tests en/decryptions in a non feedback and non reflecting configuration
 */     
enc_dec_test non_reflecting("Non reflecting enc/dec", short_test, false);

/*! \brief Test case that tests en/decryptions in a reflecting configuration
 */     
enc_dec_test reflecting("Reflecting enc/dec", short_test, true);

/*! \brief Test case that tests en/decryptions in a feedback configuration
 */     
enc_dec_test with_feedback("Feedback enc/dec", short_test, false);

/*! \brief Displacement test case
 */     
displacement_test disp_test("Displacement");

/*! \brief composite_test_case used to store the en/decryption test cases
 */     
composite_test_case enc_dec_tests("Enc/Dec Tests");

}

/* ----------------------------------------------------------------- */

void test_stepping::stepping_test::setup_data()
{
    fast_perm = boost::shared_ptr<permutation>(rmsk::std_alpha()->make_perm_ptr(PERM_I));
    slow_perm = boost::shared_ptr<permutation>(rmsk::std_alpha()->make_perm_ptr(PERM_IV));
    slower_perm = boost::shared_ptr<permutation>(rmsk::std_alpha()->make_perm_ptr(PERM_III));
    perm_iv = boost::shared_ptr<permutation>(rmsk::std_alpha()->make_perm_ptr(PERM_II));
}

void test_stepping::stepping_test::encrypt_string(string& data, string& data_out)
{
    alphabet_transformer<char> trans(rmsk::std_alpha(), &stepper->get_stack());
    
    data_out.clear();

    for (unsigned int count = 0; count < data.length(); count++)
    {
        data_out.push_back(trans.encrypt(data[count]));
        stepper->step_rotors();
    }    
}

void test_stepping::stepping_test::decrypt_string(string& data, string& data_out)
{
    alphabet_transformer<char> trans(rmsk::std_alpha(), &stepper->get_stack());
    
    data_out.clear();

    for (unsigned int count = 0; count < data.length(); count++)
    {
        data_out.push_back(trans.decrypt(data[count]));
        stepper->step_rotors();
    }    
}

void test_stepping::stepping_test::make_sample_odo()
{   
    vector<boost::shared_ptr<rotor> > all_rotors;
    vector<string> rotor_identifiers;

    rotor_names.clear();
    rotor_names.push_back("fast");
    rotor_names.push_back("slow");
    rotor_names.push_back("slower");
    rotor_names.push_back("reflecting");

    stepper = boost::shared_ptr<odometer_stepping_gear>(new odometer_stepping_gear(rotor_names, 26)); 

    all_rotors.push_back(boost::shared_ptr<rotor>(new rotor(fast_perm, NULL)));
    all_rotors.push_back(boost::shared_ptr<rotor>(new rotor(slow_perm, NULL)));
    all_rotors.push_back(boost::shared_ptr<rotor>(new rotor(slower_perm, NULL)));    
    all_rotors.push_back(boost::shared_ptr<rotor>(new rotor(perm_iv, NULL)));
            
    stepper->insert_all_rotors(all_rotors);
}

void test_stepping::enc_dec_test::set_feedback_points(unsigned int *points, unsigned int num_points)
{   
    for (unsigned int count = 0; count < num_points; count++)
    {
        feedback_points.insert(points[count]);
    }
}

/* ----------------------------------------------------------------- */

bool test_stepping::enc_dec_test::test()
{
    bool result = test_case::test();
    string test_result, test_result2;
    
    make_sample_odo();

    stepper->get_stack().set_reflecting_flag(is_reflecting);
    
    if (!feedback_points.empty())
    {
        (stepper->get_stack()).set_feedback_points(feedback_points);
    }

    encrypt_string(test_string, test_result);
    append_note("cipher: " + test_result);
    stepper->reset();    
    decrypt_string(test_result, test_result2);
    append_note("plain : " + test_result2);
    
    result = (test_string == test_result2);
    if (!result)
    {
        append_note("plaintext does not match decrypted ciphertext");
        return result;
    }
    
    result = (stepper->get_char_count() == test_string.length());
        
    return result;
}

/* ----------------------------------------------------------------- */

bool test_stepping::displacement_test::test()
{
    bool result = test_case::test();
    string test_result;
    string slow("slow");
    unsigned int count = 0;
    vector<string> all_ids;
    vector<unsigned int> new_displacements, retrieved_displacements;
    
    make_sample_odo();
    
    encrypt_string(test_string, test_result);
    
    result = (stepper->get_rotor_displacement("fast") == 0) && (stepper->get_rotor_displacement("slow") == 2);
    if (!result)
    {
        append_note("Unexpected rotor displacements");
        return result;
    }
    
    stepper->set_rotor_displacement((unsigned int)0, 3);
    stepper->set_rotor_displacement(slow, 4);
    
    result = (stepper->get_rotor_displacement("fast") == 3) && (stepper->get_rotor_displacement(1) == 4);    
    if (!result)
    {
        append_note("Problem with getting/setting rotor displacements");
        return result;
    }

    stepper->get_rotor_identifiers(all_ids);
    result = ((all_ids.size() == stepper->get_num_rotors()) && (all_ids == rotor_names));
    if (!result)
    {
        append_note("Problem with retrieved rotor ids");
        return result;
    }
    
    for (count = 0; count < stepper->get_num_rotors(); count++)
    {
        new_displacements.push_back(count);        
    }
    
    stepper->set_all_displacements(new_displacements);
    stepper->get_all_displacements(retrieved_displacements);
    
    result = (new_displacements == retrieved_displacements);
    if (!result)
    {
        append_note("Problem with retrieved displacements");
        return result;
    }    
    
    result = (stepper->get_rotor_displacement(slow) == 1);
    if (!result)
    {
        append_note("get_rotor_displacement problem");
        return result;
    }
    
    stepper->set_rotor_displacement(slow, 5);    
    string temp_str = boost::lexical_cast<string>(stepper->get_rotor_displacement(1));
    temp_str = "Displacement before first move: " + temp_str;
    append_note(temp_str);

    
    stepper->move_rotor((unsigned int)1, 7);
    temp_str = boost::lexical_cast<string>(stepper->get_rotor_displacement(1));
    temp_str = "Displacement after first move: " + temp_str;
    append_note(temp_str);
    
    result = (stepper->get_rotor_displacement(slow) == 12);
    if (!result)
    {
        append_note("move_rotor problem I");
        return result;
    }
    
    stepper->move_rotor(slow, 15);
    temp_str = boost::lexical_cast<string>(stepper->get_rotor_displacement(1));
    temp_str = "Displacement after second move: " + temp_str;
    append_note(temp_str);
    
    result = (stepper->get_rotor_displacement(slow) == 1);
    if (!result)
    {
        append_note("move_rotor problem II");
        return result;
    }
    
    return result;      
}

/* ----------------------------------------------------------------- */

void test_stepping::register_tests(composite_test_case *container)
{
    test_stepping::enc_dec_tests.add(&test_stepping::non_reflecting);
    test_stepping::enc_dec_tests.add(&test_stepping::reflecting); 
    test_stepping::with_feedback.set_feedback_points(test_feedback, NUM_FEEDBACK_POINTS);       
    test_stepping::enc_dec_tests.add(&test_stepping::with_feedback);        
    container->add(&test_stepping::enc_dec_tests);
    container->add(&test_stepping::disp_test);        
}

