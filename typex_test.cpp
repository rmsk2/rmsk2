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

/*! \file typex_test.cpp
 *  \brief Contains tests to verify the correct implementation of the Typex simulator.    
 */

#include<boost/scoped_ptr.hpp>
#include<rmsk_globals.h>
#include<typex.h>
#include<typex_test.h>
#include<configurator.h>

namespace test_typex {

/*! \brief A class that verifies the Typex simulator. The main component is a test that encrypts a string of all the
           characters in the letters and figures input alphabets and verifies that this produces the expected results.
 */
class typex_encryption_test : public test_case {
public:
    /*! \brief Construcor using an STL string to specify the name of the test.
     */ 
    typex_encryption_test(const string& n) : test_case(n) { ; }

    /*! \brief Construcor using a C-style zero terminated string to specify the name of the test.
     */ 
    typex_encryption_test(const char *n) : test_case(n) { ; }
    
    /*! \brief Performs the described tests.
     */     
    virtual bool test();
};

bool typex_encryption_test::test()
{
    bool result = false;
    // String containing all possible plaintext characters. BTW: > switches machine in figures mode and < switches it back.
    ustring plain = "qwertyuiopasdfghjkl cbnm>1234567890-/z%x£*() v,.<a";
    // Input to numeric decipherment test.
    ustring plain2 = "bbkddivxafwbkynnhtwdcpjhfnmmgz";
    typex typex_t(TYPEX_SP_02390_UKW, TYPEX_SP_02390_A, TYPEX_SP_02390_B, TYPEX_SP_02390_C, TYPEX_SP_02390_D, TYPEX_SP_02390_E);    
    typex typex_t2(TYPEX_SP_02390_UKW, TYPEX_SP_02390_A, TYPEX_SP_02390_B, TYPEX_SP_02390_C, TYPEX_SP_02390_D, TYPEX_SP_02390_E);        
    vector<pair<char, char> > inv;
    ustring enc_result, dec_result;
    // Expected encryption result of string contained in variable plain.
    ustring result_ref = "hvdngqylgghjokkioxpeqlfemxnwizaomssrmfsvvpuacykucn";
    
    inv.push_back(pair<char, char>('a', 'r'));
    inv.push_back(pair<char, char>('b', 'y'));
    inv.push_back(pair<char, char>('c', 'u'));
    inv.push_back(pair<char, char>('d', 'h'));    
    inv.push_back(pair<char, char>('e', 'q'));
    inv.push_back(pair<char, char>('f', 's'));
    inv.push_back(pair<char, char>('g', 'l'));
    inv.push_back(pair<char, char>('i', 'x'));
    inv.push_back(pair<char, char>('j', 'p'));
    inv.push_back(pair<char, char>('k', 'n'));
    inv.push_back(pair<char, char>('m', 'o'));
    inv.push_back(pair<char, char>('t', 'w'));
    inv.push_back(pair<char, char>('v', 'z'));
    
    typex_t.set_reflector(inv);
    typex_t2.set_reflector(inv);
    
    // Encryption test
    enc_result = typex_t.get_keyboard()->symbols_typed_encrypt(plain);
    
    result = (enc_result == result_ref);
    
    if (!result)
    {
        append_note(enc_result);
        append_note("Typex Encryption Test failed.");
        return result;
    }
    
    append_note(enc_result);
    
    // Numeric decryption test
    append_note("TypeX numeric code decipherment");
    
    dec_result = typex_t2.get_keyboard()->symbols_typed_decrypt(plain2);
    append_note(dec_result);
    
    result = (dec_result == "34872 42789 25470 21346 89035");
    
    return result;
}

}

/*! \brief Test to verify the Typex simulator by performing a test decryption.
 */     
decipherment_test typex_test_case("Proper TypeX Test");

/*! \brief Test case implementing the described test encryption.
 */ 
test_typex::typex_encryption_test typex_test_case_enc("TypeX Encryption Test");

void test_typex::register_tests(composite_test_case *container)
{
    // Reference values have been created using the Typex simulator available at http://www.hut-six.co.uk/typex/   
    rotor_machine *typex_t;
    typex *typex_t_load = new typex(TYPEX_SP_02390_UKW, TYPEX_SP_02390_E, TYPEX_SP_02390_D, TYPEX_SP_02390_A, TYPEX_SP_02390_C, TYPEX_SP_02390_B);
    
    map<string, string> typex_conf;
    typex_conf[KW_TYPEX_ROTORS] = "aNbNcRdNeN";
    typex_conf[KW_TYPEX_RINGS] = "aaaaa";
    typex_conf[KW_TYPEX_REFLECTOR] = "arbycudheqfsglixjpknmotwvz";
    boost::scoped_ptr<configurator> c(configurator_factory::get_configurator("Typex"));            
    typex_t = c->make_machine(typex_conf);
    
    
    ustring expected_plain = "qwertyuiopasdfghjkl cbnm1234567890-/z%x£*() v',.a";
    ustring spruch =  "ptwcichvmijbkvcazuschqyaykvlbswgqxrqujjnyqyqptrlaly";
    typex_test_case.set_test_parms(spruch, expected_plain, typex_t, typex_t_load);        

    container->add(&typex_test_case);
    container->add(&typex_test_case_enc);    
}

