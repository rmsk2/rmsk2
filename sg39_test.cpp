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

/*! \file sg39_test.cpp
 *  \brief Contains tests to verify the correct implementation of the SG39.
 */ 
#include<rmsk_globals.h>
#include<sg39_test.h>
#include<sg39.h>

/*! \brief Test that decrypts a predetermined ciphertext and checks whether this results in the expected
 *         plaintext. 
 */
decipherment_test sg39_test_case("SG39 Test");

namespace test_sg39 {

/*! As I am not aware of any reference data that could be used to verify the SG39 implementation this 
 *  test data has been chosen more or less arbitrarily.
 */
void register_tests(composite_test_case *container)
{

    schluesselgeraet39 *sg39 = new schluesselgeraet39(SG39_ROTOR_5, SG39_ROTOR_1, SG39_ROTOR_4, SG39_ROTOR_3);
    schluesselgeraet39 *sg39_dec = new schluesselgeraet39(SG39_ROTOR_4, SG39_ROTOR_2, SG39_ROTOR_1, SG39_ROTOR_5);
    ustring plain = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa", spruch;
    ustring cipher = "obkdldrmiqlwiyggqohqlwchogmdskbeiqwrsrhensjkfuegeshbdnjhhkwixefpnmlvophbttzlfvtllmhgdsbrisfeetckcttjqbhzurxowizvtpdtuyqyfdkqjeryoopvqpijowmbvkizjipo";
    vector<unsigned int> step_1, step_2, step_3, ring_3, ring_1, ring_2;
    
    // Create ring data for the wheels
    rmsk::str_to_bool_vec("001000100010001000100", 21, step_1);
    rmsk::str_to_bool_vec("11111111111111111111111", 23, step_2);
    rmsk::str_to_bool_vec("0010010010010010010010000", 25, step_3);
    // Create ring data for the rotors
    rmsk::str_to_bool_vec("10000000000000000000000000", 26, ring_3);
    rmsk::str_to_bool_vec("10000000000000000000000000", 26, ring_1);
    rmsk::str_to_bool_vec("10000000000000000000000000", 26, ring_2);

    // Prepare input transformation, i.e. the plugboard            
    boost::shared_ptr<encryption_transform> entry_stecker(rmsk::std_alpha()->make_perm_ptr("ldtrmihoncpwjkbyevsaxgfzuq"));    
    sg39->set_input_transform(entry_stecker);
    
    // Prepare the reflector
    vector<pair<char, char> > commutation_pairs;
    commutation_pairs.push_back(pair<char, char>('a', 'w'));
    commutation_pairs.push_back(pair<char, char>('b', 'i'));
    commutation_pairs.push_back(pair<char, char>('c', 'v'));
    commutation_pairs.push_back(pair<char, char>('d', 'k'));
    commutation_pairs.push_back(pair<char, char>('e', 't'));
    commutation_pairs.push_back(pair<char, char>('f', 'm'));
    commutation_pairs.push_back(pair<char, char>('g', 'n'));
    commutation_pairs.push_back(pair<char, char>('h', 'z'));
    commutation_pairs.push_back(pair<char, char>('j', 'u'));
    commutation_pairs.push_back(pair<char, char>('l', 'o'));
    commutation_pairs.push_back(pair<char, char>('p', 'q'));
    commutation_pairs.push_back(pair<char, char>('r', 'y'));
    commutation_pairs.push_back(pair<char, char>('s', 'x'));
    boost::shared_ptr<permutation> reflector_stecker(rmsk::std_alpha()->make_involution_ptr(commutation_pairs));
    sg39->set_reflector(reflector_stecker);
    
    // Putting it all together ....
    // Set pins on rotors
    sg39->get_sg39_stepper()->get_descriptor(ROTOR_3).ring->set_ring_data(ring_3);
    sg39->get_sg39_stepper()->get_descriptor(ROTOR_1).ring->set_ring_data(ring_1);
    sg39->get_sg39_stepper()->get_descriptor(ROTOR_2).ring->set_ring_data(ring_2);
    // Set wheel positions
    //sg39->get_sg39_stepper()->set_wheel_pos(ROTOR_2, 15);
    //sg39->get_sg39_stepper()->set_wheel_pos(ROTOR_1, 7);
    // Set rotor positions
    //sg39->get_sg39_stepper()->set_rotor_displacement(ROTOR_1, 3);
    //sg39->get_sg39_stepper()->set_rotor_displacement(ROTOR_2, 16);
    //sg39->get_sg39_stepper()->set_rotor_displacement(ROTOR_3, 17);
    //sg39->get_sg39_stepper()->set_rotor_displacement(ROTOR_4, 5);
    
    sg39->move_all_rotors("frqdaph");
    
    // Set pins on wheels
    sg39->get_sg39_stepper()->set_wheel_data(ROTOR_1, step_1);
    sg39->get_sg39_stepper()->set_wheel_data(ROTOR_2, step_2);
    sg39->get_sg39_stepper()->set_wheel_data(ROTOR_3, step_3);    

    sg39_test_case.set_test_parms(cipher, plain, sg39, sg39_dec);

    container->add(&sg39_test_case);
}

}

