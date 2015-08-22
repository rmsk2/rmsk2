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

#ifndef __rotor_keyboard_h__
#define __rotor_keyboard_h__

/*! \file rotor_keyboard.h
 *  \brief Header file for the rotor_keyboard class and its children.
 */ 

#include<rotor_machine.h>

using namespace Glib;

/*! \brief Used in the shifting_keyboard class to define a dummy character for machines that have
 *         no key for switching the keyboard into letters mode while being in letters mode.
 */
const unsigned int UNDEFINED_SHIFT_CHAR = 0x8000;

class rotor_machine;

/*! \brief An abstract class that provides an interface for mapping input key codes and symbols to each other
 *         in the context of keyboards that have a letters/figures mode and/or differing input alphabets 
 *         for encryption and decryption.
 *
 *  As the rotor_machine implementations in rmsk only operate on integers and not on symbols there has
 *  to be a way to map input symbols to integers (called key codes) and vice versa. In the simplest case this
 *  can be done via a single instance of the alphabet template. But in some cases rotor machines employ 
 *  several input alphabets and it depends on the state of the keyboard and the operation desired (encrypt or
 *  decrypt) which of these alphabets is used to interpret an input symbol or a key code.
 *  
 */
class rotor_keyboard {
public:
    /*! \brief Default constructor.
     */
    rotor_keyboard() { machine = NULL; }

    /*! \brief This method allows to map a symbol to a key code while the underlying machine is doing
     *         encryptions. 
     *
     *  To avoid misunderstandings: The symbol in is not passed through the machine when calling this
     *  method. The parameter f_add_to_state controls whether the input symbol is used to update the 
     *  rotor_keyboard's state. If this method is called with f_add_to_state = false a call to 
     *  add_to_state(unsigned int) has to be made (directly or indirectly) at a later time.
     */    
    virtual unsigned int get_key_code_encrypt(gunichar in, bool f_add_to_state) = 0;

    /*! \brief This method allows to map a symbol to a key code while the underlying machine is doing
     *         decryptions. 
     *
     *  To avoid misunderstandings: The symbol in is not passed through the machine when calling this
     *  method. 
     */    
    virtual unsigned int get_key_code_decrypt(gunichar in) = 0;

    /*! \brief This method allows to encrypt an input symbol. The output symbol is produced in the same
     *         way as if it would have been if the symbol would have been typed on the keyboard.
     *
     *  add_to_state(unsigned int) is called by this method.
     */        
    virtual ustring symbol_typed_encrypt(gunichar in);

    /*! \brief This method allows to decrypt an input symbol. The output symbol is produced in the same
     *         way as if it would have been if the symbol would have been typed on the keyboard.
     *
     *  add_to_state(unsigned int) is called by this method.     
     */        
    virtual ustring symbol_typed_decrypt(gunichar in);    

    /*! \brief Applies symbol_typed_encrypt(gunichar) to each character in the parameter in and returns
     *         the resulting output symbols.
     */    
    virtual ustring symbols_typed_encrypt(ustring& in);

    /*! \brief Applies symbol_typed_decrypt(gunichar) to each character in the parameter in and returns
     *         the resulting output symbols.
     */    
    virtual ustring symbols_typed_decrypt(ustring& in);

    /*! \brief This method returns true if the symbol in parameter to_test is contained in the input
     *         encryption alphabet that is active in the current state.
     */    
    virtual bool is_valid_input_encrypt(gunichar to_test) = 0;

    /*! \brief This method returns true if the symbol in parameter to_test is contained in the input
     *         decryption alphabet that is active in the current state.
     */    
    virtual bool is_valid_input_decrypt(gunichar to_test) = 0;

    /*! \brief This method allows to process a key code by the keyboard without producing an output
     *         value. The input value is also not processed by the underlying rotor machine. This method
     *         only influences the rotor_keybaord's state (letters mode/figures mode).
     *
     *  This method allows to separate the creation of the output symbol from switching the state of the
     *  keyboard. It is for instance used to switch a rotor_keybaord into the correct mode only after a
     *  key has been released. The output symbol has been produced when pressing the key.
     */        
    virtual void add_to_state(unsigned int key_code) { ; }        

    /*! \brief This method allows to map a key code to a symbol while the underlying machine is doing
     *         encryptions. 
     *
     *  To avoid misunderstandings: The symbol in is not passed through the machine when calling this
     *  method. The parameter f_add_to_state controls whether the input symbol is used to update the 
     *  rotor_keyboard's state. If this method is called with f_add_to_state = false a call to 
     *  add_to_state(unsigned int) has to be made (directly or indirectly) at a later time.
     */        
    virtual gunichar map_key_code_encrypt(unsigned int key_code, bool f_add_to_state) = 0;

    /*! \brief This method allows to map a key code to a symbol while the underlying machine is doing
     *         decryptions. 
     *
     *  To avoid misunderstandings: The symbol in is not passed through the machine when calling this
     *  method.
     */        
    virtual gunichar map_key_code_decrypt(unsigned int key_code) = 0;   

    /*! \brief Allows to set the rotor_machine that is using this rotor_keyboard.
     */    
    virtual void set_machine(rotor_machine *new_machine) { machine = new_machine; }

    /*! \brief Returns the rotor_machine this rotor_keyboard is associated with.
     */    
    virtual rotor_machine *get_machine() { return machine; }
    
    /*! \brief Returns the current state of this rotor keyboard.
     *
     *  FUNC_FIGURE_SHIFT = Keyboard is in figures mode
     *  FUNC_LETTER_SHIFT = Keyboard is in letters mode
     *  FUNC_NONE         = Keyboard has no letters or figures mode
     */        
    virtual unsigned int get_state() { return FUNC_NONE; }  
    
    /*! \brief Sets the current state in which this rotor_keyboard is.
     */        
    virtual void set_state(unsigned int new_state) { ; }      

    /*! \brief Destructor. Does not delete the machine menber.
     */            
    virtual ~rotor_keyboard() { ; }
    
protected:

    /*! \brief Points to the rotor_machine with which this rotor_keyboard is associated.
     */            
    rotor_machine *machine;    
};

/*! \brief A class that implements the rotor_keyboard interface for machines that have a letters/figures
 *         mode and use different alphabets for input during encryption and decryption.
 *
 *  Used by the Typex and the KL7. These machines use a ciphertext ::alphabet of input symbols when doing 
 *  decryptions and an input letters alphabet as well as an input figures alphabet when doing encryptions.
 *  Switching between the two possible input alphabets during enryption occurs when certain keys are
 *  pressed on the keyboard.
 */
class shifting_keyboard : public rotor_keyboard {
public:

    /*! \brief Constructor that defines the key codes which make this rotor_keyboard switching to another state.
     */            
    shifting_keyboard(unsigned int letter_code, unsigned int figure_code);

    /*! \brief Constructor that defines the key codes which make this rotor_keyboard switching to another state. In contrast
     *         to the shifting_keyboard(unsigned int, unsigned int) constructor this constructor allows to define
     *         a different set of codes for letters mode and figures mode.
     */            
    shifting_keyboard(unsigned int ltr_letter_code, unsigned int ltr_figure_code, unsigned int fig_letter_code, unsigned int fig_figure_code);    

    virtual unsigned int get_key_code_encrypt(gunichar in, bool f_add_to_state);
    virtual unsigned int get_key_code_decrypt(gunichar in);

    /*! \brief Allows to set the ciphertext alphabet of this shifting_keyboard.
     */                    
    virtual void set_cipher_alphabet(boost::shared_ptr<alphabet<gunichar> > alpha) { out_alpha = alpha; }

    /*! \brief Allows to set the letters mode plaintext alphabet of this shifting_keyboard. The alphabet has to use
     *         the character '>' for signifying the symbol that causes shfiting into figures mode and (where applicable)
     *         the character '<' for signifying the symbol that causes shfiting into letters mode 
     */                    
    virtual void set_letters_alphabet(boost::shared_ptr<alphabet<gunichar> > alpha) { letters_alpha = alpha; }

    /*! \brief Allows to set the figures mode plaintext alphabet of this shifting_keyboard. The alphabet has to use
     *         the character '<' for signifying the symbol that causes shfiting into letters mode and (where applicable)
     *         the character '>' for signifying the symbol that causes shfiting into figures mode.
     */                    
    virtual void set_figures_alphabet(boost::shared_ptr<alphabet<gunichar> > alpha) { figures_alpha = alpha; }    

    virtual gunichar map_key_code_encrypt(unsigned int key_code, bool f_add_to_state);
    virtual gunichar map_key_code_decrypt(unsigned int key_code); 

    virtual bool is_valid_input_encrypt(gunichar to_test);
    virtual bool is_valid_input_decrypt(gunichar to_test);
    
    virtual void add_to_state(unsigned int key_code);  

    /*! \brief Convenience method that switches into figures mode when the parameter symbol is equal to '>' and changes
     *         to letters mode, when the parameter symbol is equal to '<'.
     */                    
    virtual void add_char_to_state(gunichar symbol);

    virtual unsigned int get_state() { return current_state; }
    virtual void set_state(unsigned int new_state) { current_state = new_state; }

    /*! \brief Destructor.
     */                
    ~shifting_keyboard() { ; }
        
protected:

    /*! \brief References the plaintext letters mode alphabet in use in this shifting_keyboard.
     */            
    boost::shared_ptr<alphabet<gunichar> > letters_alpha;

    /*! \brief References the plaintext figures mode alphabet in use in this shifting_keyboard.
     */            
    boost::shared_ptr<alphabet<gunichar> > figures_alpha;
    
    /*! \brief References the ciphertext alphabet in use in this shifting_keyboard.
     */                
    boost::shared_ptr<alphabet<gunichar> > out_alpha;
    
    /*! \brief Holds the current state of this shifting_keyboard
     */                
    unsigned int current_state;
    
    /*! \brief Holds the key codes that cause state switching. These can be different depending
     *         on whether the keyboard is in lettes mode or in figures mode.
     */                    
    map<unsigned int, pair<unsigned int, unsigned int> > code_contacts;
};

/*! \brief A class that implements the rotor_keyboard interface for machines that have no letters/figures
 *         mode and use the same alphabet (the symbol alphabet) for input during encryption and decryption.
 *
 *  Used by all Enigma variants, the SG39 and the Nema.
 */
class symmetric_keyboard : public rotor_keyboard {
public:
    /*! \brief Default constructor.
     */            
    symmetric_keyboard() { ; }
    
    /*! \brief Constructs a symmetric_keyboard from the ustring parameter alpha_chars which in turn is
     *         used to create an ::alphabet object that is stored in the the member variable sym_alpha.
     */                
    symmetric_keyboard(ustring alpha_chars);

    /*! \brief This method allows to map a symbol to a key code while the underlying machine is doing
     *         encryptions. Simply queries sym_alpha.
     */            
    virtual unsigned int get_key_code_encrypt(gunichar in, bool f_add_to_state = true);

    /*! \brief This method allows to map a symbol to a key code while the underlying machine is doing
     *         decryptions. Simply queries sym_alpha.
     */            
    virtual unsigned int get_key_code_decrypt(gunichar in);

    /*! \brief This method allows to map a key code to a symbol while the underlying machine is doing
     *         encryptions. Simply queries sym_alpha.
     */            
    virtual gunichar map_key_code_encrypt(unsigned int key_code, bool f_add_to_state = true);

    /*! \brief This method allows to map a key code to a symbol while the underlying machine is doing
     *         decryptions. Simply queries sym_alpha.
     */            
    virtual gunichar map_key_code_decrypt(unsigned int key_code);
        
    /*! \brief This method returns true if the symbol in parameter to_test is contained in the symbol
     *         alphabet.
     */        
    virtual bool is_valid_input_encrypt(gunichar to_test);

    /*! \brief This method returns true if the symbol in parameter to_test is contained in the symbol
     *         alphabet.
     */        
    virtual bool is_valid_input_decrypt(gunichar to_test);        

    /*! \brief Explcit method to change the symbol alphabet in use in this symmteric_keyboard.
     */                
    virtual void set_alphabet(boost::shared_ptr<alphabet<gunichar> > alpha) { sym_alpha = alpha; }

    /*! \brief Destructor.
     */                
    virtual ~symmetric_keyboard() { ; }
    
protected:

    /*! \brief References the symbol alphabet in use in this symmetric_keyboard.
     */            
    boost::shared_ptr<alphabet<gunichar> > sym_alpha;    
};

/*! \brief A class that implements the rotor_keyboard interface for machines that have no letters/figures
 *         mode but use different alphabets for input during encryption and decryption.
 *
 *  Used by the SIGABA variants. The SIGABA allows the blank or space character as an input symbol during
 *  encryption but sacrifices the Z character. On decryption it is the other way round: Z is allowed but
 *  space/blank is not. The input alphabet used during encryption is called plaintext alphabet and the 
 *  input alphabet used when decrypting is called the ciphertext alphabet.
 *
 */
class asymmetric_keyboard : public rotor_keyboard {
public:

    /*! \brief Default constructor.
     */                
    asymmetric_keyboard() { ; }

    /*! \brief This method allows to map a symbol to a key code while the underlying machine is doing
     *         encryptions. Simply queries the plaintext alphabet.
     */            
    virtual unsigned int get_key_code_encrypt(gunichar in, bool f_add_to_state = true);

    /*! \brief This method allows to map a symbol to a key code while the underlying machine is doing
     *         decryptions. Simply queries ciphertext alphabet.
     */            
    virtual unsigned int get_key_code_decrypt(gunichar in);

    /*! \brief Allows to set the plaintext alphabet of this asymmetric_keyboard.
     */                    
    virtual void set_plain_alphabet(boost::shared_ptr<alphabet<gunichar> > alpha) { dec_alpha = alpha; }

    /*! \brief Allows to set the ciphertext alphabet of this asymmetric_keyboard.
     */                    
    virtual void set_cipher_alphabet(boost::shared_ptr<alphabet<gunichar> > alpha) { enc_alpha = alpha; } 

    /*! \brief This method allows to map a key code to a symbol while the underlying machine is doing
     *         encryptions. Simply queries the plaintext alphabet.
     */                
    virtual gunichar map_key_code_encrypt(unsigned int key_code, bool f_add_to_state = true);

    /*! \brief This method allows to map a key code to a symbol while the underlying machine is doing
     *         decryptions. Simply queries the ciphertext alphabet.
     */                
    virtual gunichar map_key_code_decrypt(unsigned int key_code);
    
    /*! \brief This method returns true if the symbol in parameter to_test is contained in the plaintext
     *         alphabet.
     */            
    virtual bool is_valid_input_encrypt(gunichar to_test);

    /*! \brief This method returns true if the symbol in parameter to_test is contained in the ciphertext
     *         alphabet.
     */            
    virtual bool is_valid_input_decrypt(gunichar to_test);           
    
    /*! \brief Destructor.
     */                    
    virtual ~asymmetric_keyboard() { ; }
    
protected:

    /*! \brief References the ciphertext alphabet. I.e. the alphabet which contains the symbols that are
     *         generated by an encryption.
     */                    
    boost::shared_ptr<alphabet<gunichar> > enc_alpha;

    /*! \brief References the plaintext alphabet. I.e. the alphabet which contains the symbols that are
     *         generated by an decryption.
     */                    
    boost::shared_ptr<alphabet<gunichar> > dec_alpha;
};


#endif /* __rotor_keyboard_h__ */

