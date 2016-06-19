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

/*! \file nema_test.cpp
 *  \brief Implements verification tests for the Nema.
 */ 

#include<boost/scoped_ptr.hpp>
#include<nema_test.h>
#include<nema.h>
#include<configurator.h>

/*! Verifies the Nema implementation by performing a test decryption.
 */
decipherment_test nema_test_case("NEMA Test");

namespace test_nema {


void register_tests(composite_test_case *container)
{
    // Reference values have been created using the Nema simulator available at http://cryptocellar.web.cern.ch/cryptocellar/simula/nema/index.html
    vector<rotor_assembly> settings, settings_l;
    
    // Create first machine using the confifurator interface
    map<string, string> nema_conf;
    nema_conf[KW_NEMA_ROTORS] = "abcd";
    nema_conf[KW_NEMA_RINGS] = "12 13 14 15";
    nema_conf[KW_NEMA_WAR_MACHINE] = CONF_TRUE;
    boost::scoped_ptr<configurator> c(configurator_factory::get_configurator(MNAME_NEMA));  
    nema *n = dynamic_cast<nema *>(c->make_machine(nema_conf));
    
    // Create second machine using the explicit C++ API    
    settings_l.push_back(rotor_assembly(NEMA_DRIVE_WHEEL_13, NEMA_ROTOR_D));
    settings_l.push_back(rotor_assembly(NEMA_DRIVE_WHEEL_12, NEMA_ROTOR_C));
    settings_l.push_back(rotor_assembly(NEMA_DRIVE_WHEEL_15, NEMA_ROTOR_B));
    settings_l.push_back(rotor_assembly(NEMA_DRIVE_WHEEL_14, NEMA_ROTOR_A));    

    nema *n_load = new nema(settings_l, NEMA_DRIVE_WHEEL_23, NEMA_DRIVE_WHEEL_2);       
    
    // Set up reference data
    ustring expected_plain = "aaaaaaaaaaaaaaaaaaaaaaaaaaa";
    ustring spruch =         "hrsbvootzucrwlmgrmgvwywovnf";   
    
    // Set first machine to defined rotor position
    n->move_all_rotors("mqqpzqsoev");
    
    nema_test_case.set_test_parms(spruch, expected_plain, n, n_load);            

    container->add(&nema_test_case);
}

}

