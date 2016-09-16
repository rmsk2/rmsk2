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

/*! \file sigaba_test.cpp
 *  \brief This file contains the implementation of the verification tests for the SIGABA.
 */ 

#include<boost/scoped_ptr.hpp>
#include<rmsk_globals.h>
#include<sigaba_test.h>
#include<sigaba.h>

/*! Verifies the CSP 889 mode implementation by performing a test decryption.
 */
decipherment_test csp_889_test_case("CSP 889 Test");

/*! Verifies the CSP 2900 mode implementation by performing a test decryption.
 */
decipherment_test csp_2900_test_case("CSP 2900 Test");

namespace test_sigaba {

void register_tests(composite_test_case *container)
{
    ustring spruch;
    ustring plain;
    
    // rotor ids for simple test machine
    rotor_id r5(SIGABA_ROTOR_5), r6(SIGABA_ROTOR_6), r7(SIGABA_ROTOR_7, true), r8(SIGABA_ROTOR_8), r9(SIGABA_ROTOR_9);
    rotor_id r0(SIGABA_ROTOR_0), r1(SIGABA_ROTOR_1), r2(SIGABA_ROTOR_2, true), r3(SIGABA_ROTOR_3), r4(SIGABA_ROTOR_4);
    rotor_id i0(SIGABA_INDEX_0), i1(SIGABA_INDEX_1), i2(SIGABA_INDEX_2, true), i3(SIGABA_INDEX_3), i4(SIGABA_INDEX_4);
 
    // rotor ids for test machine that is loaded from file
    rotor_id r5_l(SIGABA_ROTOR_0), r6_l(SIGABA_ROTOR_1), r7_l(SIGABA_ROTOR_2, true), r8_l(SIGABA_ROTOR_3), r9_l(SIGABA_ROTOR_4);
    rotor_id r0_l(SIGABA_ROTOR_5), r1_l(SIGABA_ROTOR_6), r2_l(SIGABA_ROTOR_7, true), r3_l(SIGABA_ROTOR_8), r4_l(SIGABA_ROTOR_9);
    rotor_id i0_l(SIGABA_INDEX_4), i1_l(SIGABA_INDEX_3), i2_l(SIGABA_INDEX_1, true), i3_l(SIGABA_INDEX_2), i4_l(SIGABA_INDEX_0);
    vector<rotor_id> all_ids, all_ids_l;

    sigaba *s, *s_load;
    sigaba_stepper *st;
    
    // Pack together rotor ids for simple test machine
    all_ids.push_back(r0);
    all_ids.push_back(r1);
    all_ids.push_back(r2);
    all_ids.push_back(r3);
    all_ids.push_back(r4);
    all_ids.push_back(r5);
    all_ids.push_back(r6);
    all_ids.push_back(r7);
    all_ids.push_back(r8);
    all_ids.push_back(r9);
    all_ids.push_back(i0);
    all_ids.push_back(i1);
    all_ids.push_back(i2);
    all_ids.push_back(i3);
    all_ids.push_back(i4);

    // Pack together rotor ids for test machine that is loaded from file
    all_ids_l.push_back(r0_l);
    all_ids_l.push_back(r1_l);
    all_ids_l.push_back(r2_l);
    all_ids_l.push_back(r3_l);
    all_ids_l.push_back(r4_l);
    all_ids_l.push_back(r5_l);
    all_ids_l.push_back(r6_l);
    all_ids_l.push_back(r7_l);
    all_ids_l.push_back(r8_l);
    all_ids_l.push_back(r9_l);
    all_ids_l.push_back(i0_l);
    all_ids_l.push_back(i1_l);
    all_ids_l.push_back(i2_l);
    all_ids_l.push_back(i3_l);
    all_ids_l.push_back(i4_l);    
    
    // CSP 889 test case
    
    // Verification data created with CSG simulator available at http://cryptocellar.org/simula/sigaba/index.html
    plain =  "dies ist ein toller test fuer die sigaba punkt die amis haben damals glatt mit leerxeichen verschluesselt";
    spruch = "hhhtitjpthjbzlqrgncbzikagmfxzuxgjpwpejpuhcswgkblhjkgxhheshmxnxszaxpihzccyfnziodorlpgidxbwrcjkpmomiugxszci";
    
    // construct simple test machine through configurator
    map<string, string> sigaba_conf;
    sigaba_conf[KW_CIPHER_ROTORS] = "0N1N2R3N4N";
    sigaba_conf[KW_CONTROL_ROTORS] = "5N6N7R8N9N";
    sigaba_conf[KW_INDEX_ROTORS] = "0N1N2R3N4N";
    sigaba_conf[KW_CSP_2900_FLAG] = "false";
    boost::scoped_ptr<configurator> c(configurator_factory::get_configurator("SIGABA"));            
    s = dynamic_cast<sigaba *>(c->make_machine(sigaba_conf));             
    
    s_load = new sigaba(all_ids_l, true);
    
    st = dynamic_cast<sigaba_stepper *>(s->get_stepping_gear());
        
    s->move_all_rotors("09000kooonljqmj");
    
    csp_889_test_case.set_test_parms(spruch, plain, s, s_load);

    container->add(&csp_889_test_case);
    
    // CSP 2900 test case

    // Verification data created with CSG simulator available at http://cryptocellar.org/simula/sigaba/index.html    
    plain =  "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";
    spruch = "bsfzeppcipicwhynfpnjxpnqmcleywutmhrhojypwwsflifobk";
    
    s = new sigaba(all_ids, false);
    s->get_sigaba_stepper()->prepare_machine_type(true);
    s_load = new sigaba (all_ids_l, false);
    
    st = dynamic_cast<sigaba_stepper *>(s->get_stepping_gear());
    
    // 14 is m when a rotor has been inserted in reverse
    st->set_rotor_displacement(R_TWO, 14);
    // 14 is m when a rotor has been inserted in reverse
    st->get_driver_machine()->get_stepping_gear()->set_rotor_displacement(S_FAST, 14);    

    // set rotor positions on test machine    
    st->setup_step(S_SLOW);
    st->setup_step(S_SLOW);
    st->setup_step(S_SLOW);
    st->setup_step(S_FAST);
    st->setup_step(S_FAST);
    st->setup_step(S_FAST);
    st->setup_step(S_MIDDLE);
    st->setup_step(S_MIDDLE);
    st->setup_step(S_MIDDLE);
        
    csp_2900_test_case.set_test_parms(spruch, plain, s, s_load);

    container->add(&csp_2900_test_case);
}

}
