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

/*! \file kl7_test.cpp
 *  \brief Implements verification tests for the KL7.
 */ 

#include<boost/scoped_ptr.hpp>
#include<kl7_test.h>
#include<kl7.h>
#include<rmsk_globals.h>
#include<decipherment_test.h>
#include<configurator.h>

/*! Verifies the KL7 implementation by performing a test decryption.
 */
decipherment_test kl7_test_case("KL-7 Test");

namespace test_kl7 {

void register_tests(composite_test_case *container)
{
    // Reference values have been created using the KL7 simulator available at http://users.telenet.be/d.rijmenants/en/kl-7sim.htm
    ustring spruch;
    ustring plain;

    // Create first machine using the explicit C++ API
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
    
    kl7 *enc, *dec = new kl7(rotor_spec);

    // Create second machine using the confifurator interface
    map<string, string> kl7_conf;
    kl7_conf[KW_KL7_ROTORS] = "abcldefg";
    kl7_conf[KW_KL7_NOTCH_RINGS] = "1 2 3 4 5 6 7";
    kl7_conf[KW_KL7_ALPHA_POS] = "27 1 1 17 1 1 1 1";
    kl7_conf[KW_KL7_NOTCH_POS] = "kaaaaaa";
    boost::scoped_ptr<configurator> c(configurator_factory::get_configurator(MNAME_KL7));  
    enc = dynamic_cast<kl7 *>(c->make_machine(kl7_conf));
    
    enc->get_kl7_stepper()->move_to_letter_ring_pos(KL7_ROT_1, 7);
    enc->step_rotors();

    // Set up verification data
    plain =  "hallo dies ist wieder ein test vvv 1234567890 aaa";
    spruch = "lpzocrfybrjmwhzrtsiygtxhuodgyyiuogpamxkfcjpplqkhss";

    kl7_test_case.set_test_parms(spruch, plain, enc, dec);

    container->add(&kl7_test_case);    
}

}
