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

#ifndef __decipherment_test_h__
#define __decipherment_test_h__

/*! \file decipherment_test.h
 *  \brief Header file for the decipherment_test class.
 */ 


#include<simple_test.h>
#include<rotor_machine.h>

/*! \brief A class that can be used to test decipherment as well loading and saving state of a rotor_machine.
 */ 
class decipherment_test : public test_case {
public:

    /*! \brief Constucts a dicpherment_test test_case. The parameter n has to specifiy the name of the test.
     */ 
    decipherment_test(const string& n) : test_case(n) { machine_enc = NULL; machine_dec = NULL; }

    /*! \brief Constucts a dicpherment_test test_case. The parameter n has to specifiy the name of the test as
     *         a C-style zero terminated string.
     */ 
    decipherment_test(const char *n) : test_case(n) { ; }
    
    /*! \brief Implements the test case.
     *
     *  This method does the following things: Saving the state of machine_enc, decryption of the ciphertext
     *  with machine_enc, verification that the decryption result matches the expected plaintext, loading the
     *  previously saved state of machine_enc into machine_dec, use machine_dec for a second decipherment test.
     */     
    virtual bool test();
    
    /*! \brief Sets the parameters that are needed for the test.
     *
     *  \param cipher_text [in] Has to contain the ciphertext, that is to be decrypted during the execution
     *                          of this test.
     *  \param expected_plain_text [in] Has to contain the expected plain text that the machine produces when
     *                                  decrypting the ciphertext given in the first parameter.
     *  \param enc [in] Points to the rotor_machine that is used to decrypt the text given in cipher_text
     *  \param dec [in] Points to another rotor_machine that is used for a second decipherment of the ciphertext.
     *
     *  The two rotor_machines that are handed over to this decipherment_test are deleted by the destructor!
     */         
    virtual void set_test_parms(ustring& cipher_text, ustring& expected_plain_text, rotor_machine *enc, rotor_machine *dec);

    /*! \brief Deletes machine_enc and machine_dec.
     */     
    virtual ~decipherment_test() { delete machine_enc; delete machine_dec; }
    
protected:

    /*! \brief Holds the machine that is used for the first decipherment.
     */     
    rotor_machine *machine_enc;

    /*! \brief Holds the machine that is used for the second decipherment.
     */     
    rotor_machine *machine_dec;

    /*! \brief Ciphertext that is to be decrypted.
     */         
    ustring spruch;

    /*! \brief Plaintext that is expected to be the result of the test decipherment.
     */         
    ustring plain;
};

#endif /* __decipherment_test_h__ */

