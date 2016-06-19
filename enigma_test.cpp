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

/*! \file enigma_test.cpp
 *  \brief Implementation of tests to verify the correct implementation of the Enigma
 *         variants.
 */ 
#include<set>
#include<utility>

#include<rmsk_globals.h>
#include<enigma_test.h>
#include<enigma_sim.h>
#include<enigma_uhr.h>
#include<typex.h>
#include<machine_config.h>

/*! \brief Verification test for Services Enigma.
 */
decipherment_test enigma_i_test("Enigma I Test");

/*! \brief Verification test for Services Enigma with Uhr.
 */
decipherment_test enigma_uhr_test("Enigma-Uhr Test");

/*! \brief Verification test for machine config with Uhr
 */
decipherment_test enigma_machine_config_test("Machine config Test Uhr");

/*! \brief Verification test for machine config with UKW D
 */
decipherment_test enigma_machine_config_ukw_d_test("Machine config Test UKWD");

/*! \brief Verification test for Abwehr Enigma.
 */
decipherment_test enigma_abwehr_test("Abwehr Enigma Test");

/*! \brief Verification test for Railway Engima.
 */
decipherment_test enigma_railway_test("Railway Enigma Test");

/*! \brief Verification test for Tirpitz Enigma.
 */
decipherment_test enigma_tirpitz_test("Tirpitz Enigma Test");

/*! \brief Verification test for KD Enigma.
 */
decipherment_test enigma_kd_test("Enigma KD Test");

/*! \brief Verification test No. 1 test for M4 Enigma.
 */
decipherment_test enigma_m4_test_1("Enigma M4 Test 1");

/*! \brief Verification test No. 2 test for M4 Enigma.
 */
decipherment_test enigma_m4_test_2("Enigma M4 Test 2");

namespace test_enigma {

/*! Implementation of verification tests for all Enigma variants.
 */
void register_tests(composite_test_case *container)
{
    // Services Enigma verification test
    // Taken from http://de.wikipedia.org/wiki/Enigma_%28Maschine%29
    // Not an authentic message
        
    enigma_I *enigma_t1 = new enigma_I(UKW_B, WALZE_I, WALZE_IV, WALZE_III);
    vector<pair<char, char> > stecker_settings;
    
    // "ad", "cn", "et", "fl", "gi", "jv", "kz", "pu", "qy", "wx"
    stecker_settings.push_back(pair<char, char>('a', 'd'));
    stecker_settings.push_back(pair<char, char>('c', 'n'));
    stecker_settings.push_back(pair<char, char>('e', 't'));
    stecker_settings.push_back(pair<char, char>('f', 'l'));
    stecker_settings.push_back(pair<char, char>('g', 'i'));                
    stecker_settings.push_back(pair<char, char>('j', 'v'));
    stecker_settings.push_back(pair<char, char>('k', 'z'));
    stecker_settings.push_back(pair<char, char>('p', 'u'));
    stecker_settings.push_back(pair<char, char>('q', 'y'));
    stecker_settings.push_back(pair<char, char>('w', 'x'));
    
    enigma_t1->set_stecker_brett(stecker_settings, false);

    enigma_t1->get_enigma_stepper()->set_ringstellung("slow", 'p');
    enigma_t1->get_enigma_stepper()->set_ringstellung("middle", 'z');
    enigma_t1->get_enigma_stepper()->set_ringstellung("fast", 'h');
    enigma_t1->get_enigma_stepper()->set_rotor_pos("slow", 'r');
    enigma_t1->get_enigma_stepper()->set_rotor_pos("middle", 't');
    enigma_t1->get_enigma_stepper()->set_rotor_pos("fast", 'z');    
    
    ustring spruch1("ljpqhsvdwclyxzqfxhiuvwdjobjnzxrcweotvnjciontfqnsxwisxkhjdagdjvakukvmjajhszqqjhzoiavzowmsckasrdnxkksrfhcxcmpjgxyijcckisyysh");
    ustring spruch2("etxvvovdqlzytnjxnuwkzrxujfxmbdibrvmjkrhtcujqpteeiynynjbeaqjclmuodfwmarqcfobwn");
    ustring spruch = spruch1 + spruch2;
    ustring expected_plain1("dasoberkommandoderwehrmaqtgibtbekanntxaachenxaachenxistgerettetxdurqgebuendelteneinsatzderhilfskraeftekonntediebedrohungabge");
    ustring expected_plain2("wendetunddierettungderstadtgegenxeinsxaqtxnullxnullxuhrsiqergestelltwerdenx");
    ustring expected_plain = expected_plain1 + expected_plain2;
    
    enigma_I *enigma_t1_load = new enigma_I(UKW_C, WALZE_II, WALZE_I, WALZE_IV);
    
    enigma_i_test.set_test_parms(spruch, expected_plain, enigma_t1, enigma_t1_load);
    
    container->add(&enigma_i_test);

/* ----------------------------------------------------------------- */
    
    // M4 Enigma verification test No. 1
    // This is a real message that was sent by the commander of U-264 Hartwig Looks on 19th of November 1942
    // See http://www.bytereef.org/m4-project-first-break.html or http://de.wikipedia.org/wiki/Enigma-M4
    
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
        
    spruch1 = ustring("nczwvusxpnyminhzxmqxsfwxwlkjahshnmcoccakuqpmkcsmhkseinjusblkiosxckubhmllxcsjusrrdvkohulxwccbgvliyxeoahxrhkkfvdrewez");
    spruch2 = ustring("lxobafgyujqukgrtvukameurbveksuhhvoyhabcjwmaklfklmyfvnrizrvvrtkofdanjmolbgffleoprgtflvrhowopbekvwmuqfmpwparmfhagkxiibg");
    expected_plain1 = ustring("vonvonjlooksjhffttteinseinsdreizwoyyqnnsneuninhaltxxbeiangriffunterwassergedruecktywabosxletztergegnerstandnulachtdreinuluhr");
    expected_plain2 = ustring("marquantonjotaneunachtseyhsdreiyzwozwonulgradyachtsmystossenachxeknsviermbfaelltynnnnnnooovierysichteinsnull");
    spruch = spruch1 + spruch2;
    expected_plain = expected_plain1 + expected_plain2;  
    
    enigma_M4 *enigma_t2_load = new enigma_M4(UKW_C_DN, WALZE_GAMMA, WALZE_III, WALZE_I, WALZE_II);
      
    enigma_m4_test_1.set_test_parms(spruch, expected_plain, enigma_t2, enigma_t2_load);
    
    container->add(&enigma_m4_test_1);
    
/* ----------------------------------------------------------------- */    
    
    // M4 Enigma verification test No. 2
    // This is another authentic M4 message sent by Hermann Schroeder commander of U-623
    // See http://www.bytereef.org/m4-project-second-break.html
    
    enigma_M4 *enigma_t3 = new enigma_M4(UKW_B_DN, WALZE_BETA, WALZE_II, WALZE_IV, WALZE_I);
    vector<pair<char, char> > stecker_settings_t3;
    
    // "at", "cl", "dh", "ep", "fg", "io", "jn", "kq", "mu", "rx"        
    stecker_settings_t3.push_back(pair<char, char>('a', 't'));
    stecker_settings_t3.push_back(pair<char, char>('c', 'l'));
    stecker_settings_t3.push_back(pair<char, char>('d', 'h'));
    stecker_settings_t3.push_back(pair<char, char>('e', 'p'));
    stecker_settings_t3.push_back(pair<char, char>('f', 'g'));                
    stecker_settings_t3.push_back(pair<char, char>('i', 'o'));
    stecker_settings_t3.push_back(pair<char, char>('j', 'n'));
    stecker_settings_t3.push_back(pair<char, char>('k', 'q'));
    stecker_settings_t3.push_back(pair<char, char>('m', 'u'));
    stecker_settings_t3.push_back(pair<char, char>('r', 'x'));
            
    enigma_t3->set_stecker_brett(stecker_settings_t3, false);
        
    enigma_t3->get_enigma_stepper()->set_ringstellung("griechenwalze", 'a');
    enigma_t3->get_enigma_stepper()->set_ringstellung("slow", 'a');
    enigma_t3->get_enigma_stepper()->set_ringstellung("middle", 'n');
    enigma_t3->get_enigma_stepper()->set_ringstellung("fast", 'v');
    enigma_t3->get_enigma_stepper()->set_rotor_pos("griechenwalze", 'm');
    enigma_t3->get_enigma_stepper()->set_rotor_pos("slow", 'c');
    enigma_t3->get_enigma_stepper()->set_rotor_pos("middle", 's');
    enigma_t3->get_enigma_stepper()->set_rotor_pos("fast", 'f');
    spruch1 = ustring("tmkfnwzxffiiyxutihwmdhxifzeqvkdvmqswbqndyozftiwmjhxhyrpaczugrremvpanwxgtkthnrlvhkzpgmnmvsecvckhoinplhhpvpxkmbhokccpdpevxvvhozzqbiyieous");
    spruch2 = ustring("eznhjkwhydagtxdjdjkjpkcsdsuztqcxjdvlpamgqkkshphvksvpcbuwzfizpfuup");        
    expected_plain1 = ustring("vvvjschreederjaufgeleitkursfuenffuenfgradnichtsgefundenymarscaierebefohlenesquadratxstanrortmarquantonjotadreineunneunfuenf");
    expected_plain2 = ustring("xsssooovieryseedremyeinsnulyyeinsnulbedecktyzwoachtmbsteigtynbbelsichteinssmt");
    spruch = spruch1 + spruch2;
    expected_plain = expected_plain1 + expected_plain2;
    
    enigma_M4 *enigma_t3_load = new enigma_M4(UKW_C_DN, WALZE_GAMMA, WALZE_III, WALZE_I, WALZE_II);
        
    enigma_m4_test_2.set_test_parms(spruch, expected_plain, enigma_t3, enigma_t3_load);

    container->add(&enigma_m4_test_2);
    
/* ----------------------------------------------------------------- */    
    
    // Abwehr Enigma verification test
    // Test message created with the Abwehr Enigma simulator of the CSG http://cryptocellar.web.cern.ch/cryptocellar/simula/abwehr/index.html
    
    abwehr_enigma *enigma_abw = new abwehr_enigma(WALZE_ABW_III, WALZE_ABW_II, WALZE_ABW_I);
    expected_plain = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";
    spruch = "gjuiycmdguvttffqpzmxkvctzusobzldzumhqmjxwtzwmqnnuwidyeqpgvfzetolb";
    
    abwehr_enigma *enigma_abw_load = new abwehr_enigma(WALZE_ABW_I, WALZE_ABW_III, WALZE_ABW_II);
    
    enigma_abwehr_test.set_test_parms(spruch, expected_plain, enigma_abw, enigma_abw_load);        
    container->add(&enigma_abwehr_test);
    
/* ----------------------------------------------------------------- */    
    
    // Railway Enigma verification test
    // Test message created with the Railway Enigma simulator of CSG http://cryptocellar.web.cern.ch/cryptocellar/simula/enigma/index.html

    railway_enigma *enigma_rb = new railway_enigma(WALZE_RB_I, WALZE_RB_II, WALZE_RB_III);
    expected_plain = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";
    spruch = "zbijbjetellsdidqbyocxeohngdsxnwlifuuvdqlzsyrbtbwlwlxpgujbhurbikgtkdztgtexjxhulfkiuqnjbeqgccryitomeyirckuji";
    
    railway_enigma *enigma_rb_load = new railway_enigma(WALZE_RB_III, WALZE_RB_II, WALZE_RB_II);
    
    enigma_railway_test.set_test_parms(spruch, expected_plain, enigma_rb, enigma_rb_load);        
    container->add(&enigma_railway_test);
    
/* ----------------------------------------------------------------- */    
    
    // Tirpitz Enigma verification test
    // Test message contained in simulator of CSG http://cryptocellar.web.cern.ch/cryptocellar/simula/tirpitz/index.html
    // Apparently created with a real Tirpitz Enigma
    
    tirpitz_enigma *enigma_t = new tirpitz_enigma(WALZE_T_VII, WALZE_T_VIII, WALZE_T_V);
    enigma_t->get_enigma_stepper()->set_ringstellung("umkehrwalze", 'k');
    enigma_t->get_enigma_stepper()->set_ringstellung("slow", 'q');
    enigma_t->get_enigma_stepper()->set_ringstellung("middle", 'r');
    enigma_t->get_enigma_stepper()->set_ringstellung("fast", 'b');
    enigma_t->move_all_rotors("acfm");
      
    expected_plain = "anxrommelxspruchnummerxeins";
    spruch = "rhmbwnbzgmmnkperufvnyjfkyqg";
    
    tirpitz_enigma *enigma_t_load = new tirpitz_enigma(WALZE_T_VI, WALZE_T_VI, WALZE_T_I);
    
    enigma_tirpitz_test.set_test_parms(spruch, expected_plain, enigma_t, enigma_t_load);        
    container->add(&enigma_tirpitz_test); 
    
/* ----------------------------------------------------------------- */

    // KD Enigma verification test
    // As there is neither an authentic KD message I know of nor another simulator that is known to be authentic this
    // test message only verifies that the KD simulator is able to decrypt its own messages.
    
    kd_enigma *enigma_kd = new kd_enigma(WALZE_KD_II, WALZE_KD_VI, WALZE_KD_V);
    enigma_kd->get_enigma_stepper()->set_ringstellung("slow", 'q');
    enigma_kd->get_enigma_stepper()->set_ringstellung("middle", 'r');
    enigma_kd->get_enigma_stepper()->set_ringstellung("fast", 'b');
    enigma_kd->get_enigma_stepper()->set_rotor_pos("slow", 'c');
    enigma_kd->get_enigma_stepper()->set_rotor_pos("middle", 'f');
    enigma_kd->get_enigma_stepper()->set_rotor_pos("fast", 'm');      
      
    expected_plain = "obwohldierotorverdrahtungenderkdenigmanichtbekanntsindsimulierenwirdiesemaschine";
    spruch = "xlmwoizeczzbfvmahnhrzerhnpwkjjorrxtebozcxncvdemaexvcfuxokbyntyjdongpgwwchftplrzr";
    
    kd_enigma *enigma_kd_load = new kd_enigma(WALZE_KD_I, WALZE_KD_II, WALZE_KD_III);
    
    enigma_kd_test.set_test_parms(spruch, expected_plain, enigma_kd, enigma_kd_load);        
    container->add(&enigma_kd_test); 
    
/* ----------------------------------------------------------------- */    

    // Services Enigma with Uhr verification test
    // Test message prepared with this Enigma-Uhr simulator CSG http://www.hut-six.co.uk/uhr/
    
    enigma_I *enigma_mit_uhr = new enigma_I(UKW_B, WALZE_I, WALZE_IV, WALZE_III);    
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
    
    enigma_mit_uhr->set_stecker_brett(cabling, true);
    enigma_mit_uhr->get_uhr()->set_dial_pos(27);
        
    enigma_mit_uhr->get_enigma_stepper()->set_ringstellung("slow", 'p');
    enigma_mit_uhr->get_enigma_stepper()->set_ringstellung("middle", 'z');
    enigma_mit_uhr->get_enigma_stepper()->set_ringstellung("fast", 'h');
    enigma_mit_uhr->move_all_rotors("rtz");
        
    spruch1 = "ukpfhallqcdnbffcghudlqukrbpyiyrdlwyalykcvossffxsyjbhbghdxawukjadkelptyklgfxqahxmmfpioqnjsgaufoxzggomjfryhqpccdivyicgvyx";
    spruch2 = "dshvosujnuuaahobhkfxzkkspozkjyjbahrsuebrthacdkqggxqsxqzbqywafstpmwrujffbrkbjfvyy";
    spruch = spruch1 + spruch2;
    expected_plain1 = "dasoberkommandoderwehrmaqtgibtbekanntxaachenxaachenxistgerettetxdurqgebuendelteneinsatzderhilfskraeftekonntediebedrohungabge";
    expected_plain2 = "wendetunddierettungderstadtgegenxeinsxaqtxnullxnullxuhrsiqergestelltwerdenx";
    expected_plain = expected_plain1 + expected_plain2;
    
    enigma_I *enigma_t1_load_uhr = new enigma_I(UKW_C, WALZE_II, WALZE_I, WALZE_IV);
    
    enigma_uhr_test.set_test_parms(spruch, expected_plain, enigma_mit_uhr, enigma_t1_load_uhr);
    
    container->add(&enigma_uhr_test);       

/* ----------------------------------------------------------------- */    

    // Services Enigma with Uhr verification test
    // Test message prepared with this Enigma-Uhr simulator CSG http://www.hut-six.co.uk/uhr/
    
    enigma_I *enigma_mit_uhr_m_config = new enigma_I(UKW_B, WALZE_I, WALZE_IV, WALZE_III);    
        
    enigma_mit_uhr_m_config->set_stecker_brett(cabling, true);
    enigma_mit_uhr_m_config->get_uhr()->set_dial_pos(27);
        
    enigma_mit_uhr_m_config->get_enigma_stepper()->set_ringstellung("slow", 'p');
    enigma_mit_uhr_m_config->get_enigma_stepper()->set_ringstellung("middle", 'z');
    enigma_mit_uhr_m_config->get_enigma_stepper()->set_ringstellung("fast", 'h');
    enigma_mit_uhr_m_config->get_enigma_stepper()->set_rotor_pos("slow", 'r');
    enigma_mit_uhr_m_config->get_enigma_stepper()->set_rotor_pos("middle", 't');
    enigma_mit_uhr_m_config->get_enigma_stepper()->set_rotor_pos("fast", 'z');
    
    machine_config test_config;
    Glib::ustring type = "Services";
    test_config.make_config(type);
    bool call_result = false;
    call_result = call_result || test_config.randomize();
    call_result = call_result || test_config.get_config(enigma_mit_uhr_m_config);
    string dummy = "";
    call_result = call_result || enigma_mit_uhr_m_config->randomize(dummy);
    call_result = call_result || test_config.configure_machine(enigma_mit_uhr_m_config);
        
    spruch1 = "ukpfhallqcdnbffcghudlqukrbpyiyrdlwyalykcvossffxsyjbhbghdxawukjadkelptyklgfxqahxmmfpioqnjsgaufoxzggomjfryhqpccdivyicgvyx";
    spruch2 = "dshvosujnuuaahobhkfxzkkspozkjyjbahrsuebrthacdkqggxqsxqzbqywafstpmwrujffbrkbjfvyy";
    spruch = spruch1 + spruch2;
    expected_plain1 = "dasoberkommandoderwehrmaqtgibtbekanntxaachenxaachenxistgerettetxdurqgebuendelteneinsatzderhilfskraeftekonntediebedrohungabge";
    expected_plain2 = "wendetunddierettungderstadtgegenxeinsxaqtxnullxnullxuhrsiqergestelltwerdenx";
    expected_plain = expected_plain1 + expected_plain2;
    
    if (call_result)
    {
        expected_plain = "Das war wohl nix";
    }
    
    enigma_I *load_uhr_m_config = new enigma_I(UKW_C, WALZE_II, WALZE_I, WALZE_IV);
    
    enigma_machine_config_test.set_test_parms(spruch, expected_plain, enigma_mit_uhr_m_config, load_uhr_m_config);
    
    container->add(&enigma_machine_config_test);       
/* ----------------------------------------------------------------- */

    // UKW D verification test for machine config getting/setting
    // As there is neither an authentic KD message I know of nor another simulator that is known to be authentic this
    // test message only verifies that the KD simulator is able to decrypt its own messages.
    
    kd_enigma *enigma_kd_m_config = new kd_enigma(WALZE_KD_II, WALZE_KD_VI, WALZE_KD_V);
    enigma_kd_m_config->get_enigma_stepper()->set_ringstellung("slow", 'q');
    enigma_kd_m_config->get_enigma_stepper()->set_ringstellung("middle", 'r');
    enigma_kd_m_config->get_enigma_stepper()->set_ringstellung("fast", 'b');
    enigma_kd_m_config->get_enigma_stepper()->set_rotor_pos("slow", 'c');
    enigma_kd_m_config->get_enigma_stepper()->set_rotor_pos("middle", 'f');
    enigma_kd_m_config->get_enigma_stepper()->set_rotor_pos("fast", 'm');      

    expected_plain = "obwohldierotorverdrahtungenderkdenigmanichtbekanntsindsimulierenwirdiesemaschine";
    machine_config test_config2;
    Glib::ustring type2 = "KD";
    test_config2.make_config(type2);
    call_result = false;
    call_result = call_result || enigma_kd_m_config->randomize(dummy);
    call_result = call_result || test_config2.randomize();    
    call_result = call_result || test_config2.get_config(enigma_kd_m_config);    
    spruch = enigma_kd_m_config->get_keyboard()->symbols_typed_encrypt(expected_plain);
    call_result = call_result || enigma_kd_m_config->randomize(dummy);
    call_result = call_result || test_config2.configure_machine(enigma_kd_m_config);      

    if (call_result)
    {
        expected_plain = "Das war wohl nix";
    }
    
    kd_enigma *enigma_kd_load_m_config = new kd_enigma(WALZE_KD_I, WALZE_KD_II, WALZE_KD_III);
    
    enigma_machine_config_ukw_d_test.set_test_parms(spruch, expected_plain, enigma_kd_m_config, enigma_kd_load_m_config);        
    container->add(&enigma_machine_config_ukw_d_test);

}

}

