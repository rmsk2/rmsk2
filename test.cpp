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

/*! \file test.cpp
 *  \brief This file contains the main function for the rmsk program that executes all 
 *         defined unit tests.
 */ 

#include<vector>
#include<iostream>
#include<rmsk_globals.h>
#include<stepping.h>
#include<alphabet.h>
#include<simple_test.h>
#include<stepping_test.h>
#include<rotor_test.h>
#include<machine_test.h>
#include<enigma_sim.h>
#include<enigma_test.h>
#include<typex_test.h>
#include<sigaba_test.h>
#include<nema_test.h>
#include<sg39.h>
#include<sg39_test.h>
#include<enigma_uhr.h>
#include<kl7.h>
#include<kl7_test.h>

using namespace std;

/*! \brief A class implementing the test_case interface in order to provide a home for simple test cases
 *         that are not worthwhile to be implemented in a separate class. 
 *
 *  If you want to quickly test something out this is the place where to put it.
 */
class alles_andere : public test_case {
public:
    /*! \brief Default constructor.
     */   
    alles_andere() : test_case("Alles andere") { ; }    

    /*! \brief Implements the tests.
     */   
    virtual bool test();

    /*! \brief Destructor.
     */       
    virtual ~alles_andere() { ; }
};

/*! 
 *  Currently this method implements testroutines for the Enigma Uhr and the KL7. Put
 *  your own test here if you do not want to implement them in a separate class.
 *  
 */
bool alles_andere::test()
{
    bool result = test_case::test();
    
    append_note("********* Demonstrations and experiments *********");
    
    // Code to demonstrate that the resulting tranformation of the Uhr depends
    // on the sequence of the Uhr cabling
    append_note("Uhr Test start");
    enigma_uhr uhr;
    
    // First cabling ba.......
    uhr.set_cabling("bacdefghijklmnopqrst");
    uhr.set_dial_pos(27);
    vector<unsigned int> enc_res, dec_res;
    
    for (unsigned int count = 0; count < 25; count++)
    {
        enc_res.push_back(uhr.encrypt(count));
        dec_res.push_back(uhr.decrypt(enc_res[count]));
    }
    
    append_note(rmsk::std_alpha()->to_string(enc_res));

    // Second cabling ab......
    uhr.set_cabling("abcdefghijklmnopqrst");
    uhr.set_dial_pos(27);
    enc_res.clear(); dec_res.clear();
    
    for (unsigned int count = 0; count < 25; count++)
    {
        enc_res.push_back(uhr.encrypt(count));
        dec_res.push_back(uhr.decrypt(enc_res[count]));
    }
    
    append_note(rmsk::std_alpha()->to_string(enc_res));
    append_note("Uhr Test end");
    
    // Code which was used to verify the KL7 implementation against Mr. Rijmenants simulator
    {
        //ustring plain("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"), cipher;
        ustring plain("bqvwjbfitszteyfjljdhiyluhkwqxrypkbqwokucfjphao"), cipher;
        vector<string> rotor_set_names;
        vector<string>::iterator iter;
        
        
        rotor_id id_1(KL7_ROTOR_A, KL7_RING_1);
        rotor_id id_2(KL7_ROTOR_B, KL7_RING_2);
        rotor_id id_3(KL7_ROTOR_C, KL7_RING_3);
        rotor_id id_4(KL7_ROTOR_L, KL7_RING_WIDE);
        rotor_id id_5(KL7_ROTOR_D, KL7_RING_4);
        rotor_id id_6(KL7_ROTOR_E, KL7_RING_5);
        rotor_id id_7(KL7_ROTOR_F, KL7_RING_6);
        rotor_id id_8(KL7_ROTOR_G, KL7_RING_7);        
        
        vector<rotor_id> rotor_spec;
        rotor_spec.push_back(id_1);
        rotor_spec.push_back(id_2);
        rotor_spec.push_back(id_3);        
        rotor_spec.push_back(id_4);
        rotor_spec.push_back(id_5);
        rotor_spec.push_back(id_6);    
        rotor_spec.push_back(id_7);    
        rotor_spec.push_back(id_8);        
        
        kl7 *enc = new kl7(rotor_spec);
        unsigned int letter_ring_offset = 26;
        unsigned int notch_ring_offset = 13;
        unsigned int l_ring_offset = 16;
        unsigned int rotor_pos = 0;
        enc->get_kl7_stepper()->set_kl7_rings(KL7_ROT_1, letter_ring_offset, notch_ring_offset);
        enc->get_kl7_stepper()->move_to_letter_ring_pos(KL7_ROT_1, rotor_pos);
        enc->get_kl7_stepper()->set_stationary_rotor_ring_pos(l_ring_offset);
                
        enc->step_rotors();
        
        cipher = enc->get_keyboard()->symbols_typed_decrypt(plain); 
        append_note("KL-7 Testdecryption");           
        append_note(cipher);
        append_note("KL-7 Testdecryption end"); 
                
        // Code that prints the rotor sets which are currently supported by the KL7 implementation
        rotor_set_names = enc->get_rotor_set_names();
        append_note("KL7 rotor sets:");
        
        for (iter = rotor_set_names.begin(); iter != rotor_set_names.end(); ++iter)
        {
            append_note(*iter);
        }
        
        append_note("KL7 rotor sets end");
        
        delete enc;       
    }
            
    return result;
}


/*! 
 *  \brief This is the main function that calls all defined unit tests. If you implement your own, you
 *         should put it here.
 */
int main()
{
    int ret_val = 0;

    composite_test_case all_tests("rmsk tests");
    alles_andere rest;
    
    test_stepping::register_tests(&all_tests);
    test_rotor::register_tests(&all_tests);
    test_machine::register_tests(&all_tests);
    test_enigma::register_tests(&all_tests);
    test_typex::register_tests(&all_tests);
    test_sigaba::register_tests(&all_tests);
    test_nema::register_tests(&all_tests);
    test_sg39::register_tests(&all_tests);
    test_kl7::register_tests(&all_tests);
    all_tests.add(&rest);
    
    if (!all_tests.test())
    {
        ret_val = 42;
    }
    
    all_tests.print_notes(); 
    rmsk::clean_up();       
    
    return ret_val;
}

