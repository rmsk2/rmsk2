/***************************************************************************
 * Copyright 2018 Martin Grap
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
#include<memory>
#include<algorithm>
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

protected:
    /*! \brief Method to demonstrate that the resulting tranformation of the Uhr depends
     *         on the sequence of the Uhr cabling
     */    
    bool uhr_cabling_test();

    /*! \brief Contains code which was used to verify the KL7 implementation against Mr. Rijmenants simulator.
     */     
    bool kl7_verification_test();

    /*! \brief Contains code for Enigma configurator experiments
     */    
    bool enigma_configurator_test();

    /*! \brief Demonstration that SG39 can be operated in such a way that it is compatible with an M4
     */        
    bool sg39_as_m4_test();

    /*! \brief Randomizes the default enigma rotor set and saves it.
     */        
    bool rand_rotor_set_test();
    
    /*! \brief Prints permutations.
     */        
    bool active_perms_test(); 

    /*! \brief Append the vector of ustrings as notes of this test.
     */            
    void append_ustr_vector(vector<ustring>& msgs);
};

void alles_andere::append_config_notes(map<string, string> config_data)
{
    string help;
        
    for (const auto &iter: config_data)
    {
        help = iter.first + ": " + iter.second;
        append_note(help);
    }    
}

void alles_andere::append_ustr_vector(vector<ustring>& msgs)
{
    for (const auto &count : msgs)
    {
        append_note(count.c_str());
    }    
}

bool alles_andere::enigma_configurator_test()
{
    bool result = true;

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
    
    return result;
}

bool alles_andere::rand_rotor_set_test()
{
    bool result = true;
    Glib::KeyFile ini1, ini2, ini3;

    append_note("Enigma rotor set randomization test start");  
    
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
    string default_set_name = machine.get_default_set_name();
    machine.get_rotor_set(default_set_name)->save_ini(ini1);
    machine.get_rotor_set(default_set_name)->replace_permutations();
    machine.get_rotor_set(default_set_name)->save_ini(ini2);

    map<string, string> kw;    
    string enigma_model = "Services";
    boost::scoped_ptr<configurator> cnf2(configurator_factory::get_configurator(enigma_model));
    kw[KW_ENIG_ROTOR_SELECTION] = "1153";
    kw[KW_ENIG_RINGSTELLUNG] = "abc";
    kw[KW_ENIG_STECKERBRETT] = "17:adcnetflgijvkzpuqywx";
    kw[KW_USES_UHR] = CONF_TRUE;
    kw[KW_UKW_D_PERM] = "azbpcxdqetfogshvirknlmuw";
    boost::scoped_ptr<rotor_machine> test_machine(cnf2->make_machine(kw));
    
    test_machine->get_rotor_set(default_set_name)->save_ini(ini3);
    
    result = (ini1.to_data() != ini2.to_data()) && (ini2.to_data() == ini3.to_data());
    
    append_note("Enigma rotor set randomization test end");  
    
    return result;
}

bool alles_andere::sg39_as_m4_test()
{
    bool result = true;

    append_note("SG39 as M4 test begin");    
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
    append_note("SG39 as M4 test end");    
    
    return result;
}

bool alles_andere::kl7_verification_test()
{
    bool result = true;

    //ustring plain("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"), cipher;
    ustring plain("bqvwjbfitszteyfjljdhiyluhkwqxrypkbqwokucfjphao"), cipher;
    vector<string> rotor_set_names;
        
    
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
    
    unique_ptr<kl7> enc(new kl7(rotor_spec));
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
    
    for_each(rotor_set_names.begin(), rotor_set_names.end(), [this] (const string& set_name) { append_note(set_name); } );
        
    append_note("KL7 rotor sets end");
    
    return result;
}

bool alles_andere::uhr_cabling_test()
{
    bool result = true;

    append_note("Uhr Test start");
    enigma_uhr uhr;
    
    // First cabling ba.......
    uhr.set_cabling("bacdefghijklmnopqrst");
    uhr.set_dial_pos(27);
    vector<unsigned int> enc_res, dec_res;
    
    auto get_enc_dec_result = [&enc_res, &dec_res, &uhr] (unsigned int input) { enc_res.push_back(uhr.encrypt(input)); dec_res.push_back(uhr.decrypt(enc_res.back())); };
    
    for (unsigned int count = 0; count < 26; count++)
    {
        get_enc_dec_result(count);
    }
    
    append_note(rmsk::std_alpha()->to_string(enc_res));

    // Second cabling ab......
    uhr.set_cabling("abcdefghijklmnopqrst");
    uhr.set_dial_pos(27);
    enc_res.clear(); dec_res.clear();
    
    for (unsigned int count = 0; count < 26; count++)
    {
        get_enc_dec_result(count); 
    }
    
    append_note(rmsk::std_alpha()->to_string(enc_res));
    append_note("Uhr Test end");
    
    return result;
}

bool alles_andere::active_perms_test()
{
    bool result = true;
    
    append_note("Print active permutations test start");
    
    append_note("------ Enigma ------");
    map<string, string> kw;    
    string enigma_model = "Services";
    boost::scoped_ptr<configurator> cnf2(configurator_factory::get_configurator(enigma_model));
    kw[KW_ENIG_ROTOR_SELECTION] = "1153";
    kw[KW_ENIG_RINGSTELLUNG] = "abc";
    kw[KW_ENIG_STECKERBRETT] = "17:adcnetflgijvkzpuqywx";
    kw[KW_USES_UHR] = CONF_TRUE;
    kw[KW_UKW_D_PERM] = "azbpcxdqetfogshvirknlmuw";
    boost::scoped_ptr<rotor_machine> test_machine(cnf2->make_machine(kw));
    
    auto all_perms = test_machine->visualize_active_permutations();
    append_ustr_vector(all_perms);
    
    append_note("------ Enigma ------");    
    
    append_note("------ Nema ------");
    boost::scoped_ptr<configurator> cnf_nema(configurator_factory::get_configurator(MNAME_NEMA));
    map<string, string> kw_nema;    
    kw_nema[KW_NEMA_ROTORS] = "abcd";
    kw_nema[KW_NEMA_RINGS] = "12 13 14 15";
    kw_nema[KW_NEMA_WAR_MACHINE] = CONF_TRUE;
    boost::scoped_ptr<rotor_machine> test_nema(cnf_nema->make_machine(kw_nema));
    
    auto all_perms_nema = test_nema->visualize_active_permutations();
    append_ustr_vector(all_perms_nema);
    
    append_note("------ Nema ------");   
    
    append_note("------ KL7 ------");
    boost::scoped_ptr<configurator> cnf_kl7(configurator_factory::get_configurator(MNAME_KL7));
    map<string, string> kw_kl7;    
    kw_kl7[KW_KL7_ROTORS] = "lfcgabhd";
    kw_kl7[KW_KL7_ALPHA_POS] = "17 1 1 23 1 36 1 1";
    kw_kl7[KW_KL7_NOTCH_RINGS] = "2 4 3 11 7 1 10";
    kw_kl7[KW_KL7_NOTCH_POS] = "eaaag+aa";
    boost::scoped_ptr<rotor_machine> test_kl7(cnf_kl7->make_machine(kw_kl7));
    
    auto all_perms_kl7 = test_kl7->visualize_active_permutations();
    append_ustr_vector(all_perms_kl7);
    
    append_note("------ KL7 ------");    
    
    append_note("------ SIGABA ------");
    boost::scoped_ptr<configurator> cnf_sigaba(configurator_factory::get_configurator(MNAME_SIGABA));
    map<string, string> kw_sigaba;    
    kw_sigaba[KW_CIPHER_ROTORS] = "0N1N2R3N4N";
    kw_sigaba[KW_CONTROL_ROTORS] = "5N6N7R8N9N";
    kw_sigaba[KW_INDEX_ROTORS] = "0N1N2R3N4N";
    kw_sigaba[KW_CSP_2900_FLAG] = CONF_FALSE;
    boost::scoped_ptr<rotor_machine> test_sigaba(cnf_sigaba->make_machine(kw_sigaba));
    
    auto all_perms_sigaba = test_sigaba->visualize_active_permutations();
    append_ustr_vector(all_perms_sigaba);

    append_note("------ SIGABA ------");                
    
    append_note("Print active permutations test end");
    
    return result;
}

/*! 
 *  Currently this method calls testroutines for the Enigma (Uhr), SG39 and the KL7. Put
 *  your own tests here if you do not want to implement them in a separate class or method.
 *  
 */
bool alles_andere::test()
{
    bool result = test_case::test();
    string note;
    
    append_note("**************************************************");
    append_note("********* Demonstrations and experiments *********");
    append_note("**************************************************");

    result = result && uhr_cabling_test();
    result = result && kl7_verification_test();
    result = result && enigma_configurator_test();
    result = result && sg39_as_m4_test();
    result = result && active_perms_test();    
    result = result && rand_rotor_set_test();
    
    append_note("UKW D notation test start");
    
    string  bp_perm("afcwduelgqhyisjrktmznvpxbo");
    string gaf_perm("avboctdmezfngxhqiskrlupwjy");    
    
    note = "GAF to BP UKW D: ";
    string gaf_as_bp = ukw_d_wiring_helper::GAF_to_BP_wiring(gaf_perm);
    auto plugs = ukw_d_wiring_helper::string_to_plugs(gaf_as_bp);    
    for_each(plugs.begin(), plugs.end(), [&note](pair<char, char> in) { note += (string("") + in.first + in.second + " "); });
    append_note(note);

    note = "BP to GAF UKW D: ";        
    string bp_as_gaf = ukw_d_wiring_helper::BP_to_GAF_wiring(bp_perm);
    plugs = ukw_d_wiring_helper::string_to_plugs(bp_as_gaf);
    for_each(plugs.begin(), plugs.end(), [&note](pair<char, char> in) { note += (string("") + in.first + in.second + " "); });
    append_note(note);

    append_note("UKW D notation test end");    
                
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

