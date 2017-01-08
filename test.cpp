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

/*! \file test.cpp
 *  \brief This file contains the main function for the rmsk program that executes all 
 *         defined unit tests.
 */ 

#include<vector>
#include<iostream>
#include<boost/scoped_ptr.hpp>
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
#include<machine_config.h>

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
    
    /*! \brief Append notes which spcifiy the contents of the machine_config specified in the parameter conf.
     */    
    virtual void append_config_notes(map<string, string> config_data);

    /*! \brief Implements the tests.
     */   
    virtual bool test();

    /*! \brief Destructor.
     */       
    virtual ~alles_andere() { ; }
};

void alles_andere::append_config_notes(map<string, string> config_data)
{
    map<string, string>::iterator conifg_iter;
    string help;
        
    for (conifg_iter = config_data.begin(); conifg_iter != config_data.end(); ++conifg_iter)
    {
        help = conifg_iter->first + ": " + conifg_iter->second;
        append_note(help);
    }    
}

/*! 
 *  Currently this method implements testroutines for the Enigma Uhr and the KL7. Put
 *  your own test here if you do not want to implement them in a separate class.
 *  
 */
bool alles_andere::test()
{
    bool result = test_case::test();
    
    append_note("**************************************************");
    append_note("********* Demonstrations and experiments *********");
    append_note("**************************************************");
    
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
        
        append_note("Enigma configurator get_config test start");    
        
        vector<pair<char, char> > cabling;
        
        cabling.push_back(pair<char, char>('a', 'd'));
        cabling.push_back(pair<char, char>('c', 'n'));    
        cabling.push_back(pair<char, char>('e', 't')); 
        cabling.push_back(pair<char, char>('f', 'l'));       
        cabling.push_back(pair<char, char>('g', 'i'));    
        cabling.push_back(pair<char, char>('j', 'v'));  
        cabling.push_back(pair<char, char>('k', 'z')); 
        cabling.push_back(pair<char, char>('p', 'u'));  
        cabling.push_back(pair<char, char>('q', 'y'));           
        cabling.push_back(pair<char, char>('w', 'x'));                   
        
        enigma_I machine(UKW_B, WALZE_II, WALZE_III, WALZE_V);
        machine.get_enigma_stepper()->set_ringstellung("slow", 'q');
        machine.get_enigma_stepper()->set_ringstellung("middle", 'r');
        machine.get_enigma_stepper()->set_ringstellung("fast", 'b');
        machine.move_all_rotors("cfm");
        
        machine.set_stecker_brett(cabling, false);
        
        map<string, string> config_data;
        string egal = machine.get_machine_type();
        boost::scoped_ptr<configurator> cnf(configurator_factory::get_configurator(egal));
        
        try
        {
            cnf->get_config(config_data, &machine);
            append_config_notes(config_data);
        }
        catch(...)
        {
            append_note("ERROR: Unable to retrieve config"); 
            result = false;
        }
        
        string visualized_rotor_positions = machine.visualize_all_positions();
        append_note("rotor positions: " + visualized_rotor_positions);
                
        append_note("Enigma configurator get_config test end"); 
        append_note("Enigma configurator make_machine test start"); 

        machine_config test_conf2;
        
        map<string, string> kw;
        string enigma_model = "Services";
        boost::scoped_ptr<configurator> cnf2(configurator_factory::get_configurator(enigma_model));
        kw[KW_ENIG_ROTOR_SELECTION] = "1153";
        kw[KW_ENIG_RINGSTELLUNG] = "abc";
        kw[KW_ENIG_STECKERBRETT] = "17:adcnetflgijvkzpuqywx";
        kw[KW_USES_UHR] = CONF_TRUE;
        kw[KW_UKW_D_PERM] = "azbpcxdqetfogshvirknlmuw";
        boost::scoped_ptr<rotor_machine> test_machine(cnf2->make_machine(kw));
        
        result = result && (test_machine.get() != NULL);
        
        if (result)
        {        
            try
            {
                cnf2->get_config(config_data, test_machine.get());
                append_config_notes(config_data);
            }
            catch(...)
            {
                append_note("ERROR: Unable to retrieve config"); 
                result = false;
            }        

            visualized_rotor_positions = test_machine->visualize_all_positions();
            append_note("rotor positions: " + visualized_rotor_positions);       
        }
        else
        {
            append_note("Unable to create machine object");
        }
                         
        append_note("Enigma configurator make_machine test end");    
        append_note("SG39 as M4 test begin");    
        
        {
            enigma_M4 *enigma_t2 = new enigma_M4(UKW_B_DN, WALZE_BETA, WALZE_II, WALZE_IV, WALZE_I);
            vector<pair<char, char> > stecker_settings_t2;
            
            // "at", "bl", "df", "gj", "hm", "nw", "op", "qy", "rz", "vx"        
            stecker_settings_t2.push_back(pair<char, char>('a', 't'));
            stecker_settings_t2.push_back(pair<char, char>('b', 'l'));
            stecker_settings_t2.push_back(pair<char, char>('d', 'f'));
            stecker_settings_t2.push_back(pair<char, char>('g', 'j'));
            stecker_settings_t2.push_back(pair<char, char>('h', 'm'));                
            stecker_settings_t2.push_back(pair<char, char>('n', 'w'));
            stecker_settings_t2.push_back(pair<char, char>('o', 'p'));
            stecker_settings_t2.push_back(pair<char, char>('q', 'y'));
            stecker_settings_t2.push_back(pair<char, char>('r', 'z'));
            stecker_settings_t2.push_back(pair<char, char>('v', 'x'));
            
            enigma_t2->set_stecker_brett(stecker_settings_t2, false);

            enigma_t2->get_enigma_stepper()->set_ringstellung("griechenwalze", 'a');
            enigma_t2->get_enigma_stepper()->set_ringstellung("slow", 'a');
            enigma_t2->get_enigma_stepper()->set_ringstellung("middle", 'a');
            enigma_t2->get_enigma_stepper()->set_ringstellung("fast", 'v');
            enigma_t2->move_all_rotors("vjna");
                
            ustring spruch1 = ustring("nczwvusxpnyminhzxmqxsfwxwlkjahshnmcoccakuqpmkcsmhkseinjusblkiosxckubhmllxcsjusrrdvkohulxwccbgvliyxeoahxrhkkfvdrewez");
            ustring spruch2 = ustring("lxobafgyujqukgrtvukameurbveksuhhvoyhabcjwmaklfklmyfvnrizrvvrtkofdanjmolbgffleoprgtflvrhowopbekvwmuqfmpwparmfhagkxiibg");  
            ustring spruch = spruch1 + spruch2;
            ustring plain;
            
            schluesselgeraet39 *sg39 = new schluesselgeraet39(SG39_ROTOR_5, SG39_ROTOR_1, SG39_ROTOR_4, SG39_ROTOR_3);                      
            
            sg39->configure_from_m4(enigma_t2);
            sg39->save("sg39_as_m4.ini");
            sg39->get_keyboard()->symbols_typed_decrypt(spruch, plain);            
            
            append_note(plain.c_str());
        }
        
        append_note("SG39 as M4 test end");    
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

