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

/*! \file decipherment_test.cpp
 *  \brief Implementation of the decipherment_test class.
 */ 

#include<decipherment_test.h>

bool decipherment_test::test()
{
    bool result = test_case::test();
    ustring decryption_result;
    string ini_name = name + ".ini";

    // first decryption
    result = !machine_enc->save(ini_name);
    
    if (!result)
    {
        append_note("Saving state failed");
        return result;
    }

    append_note("Rotor start positions: " + machine_enc->visualize_all_positions());
    decryption_result = machine_enc->get_keyboard()->symbols_typed_decrypt(spruch);
    append_note(decryption_result);
    append_note("Rotor end positions: " + machine_enc->visualize_all_positions());    

    result = (decryption_result == plain);
    
    if (!result)
    {
        append_note("Decryption one failed");
        return result;
    }    

    // second decryption
    result = !machine_dec->load(ini_name);
    
    if (!result)
    {
        append_note("Loading state failed");
        return result;
    }

    decryption_result = machine_dec->get_keyboard()->symbols_typed_decrypt(spruch);
    result = (decryption_result == plain);
    
    if (!result)
    {
        append_note("Decryption two failed");
        return result;
    }    
    
    return result;
}

void decipherment_test::set_test_parms(ustring& cipher_text, ustring& expected_plain_text, rotor_machine *enc, rotor_machine *dec)
{
    spruch = cipher_text;
    plain = expected_plain_text;
    machine_enc = enc;
    machine_dec = dec;    
}

