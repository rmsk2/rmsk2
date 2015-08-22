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

#ifndef __printing_device_h__
#define __printing_device_h__

/*! \file printing_device.h
 *  \brief Header file for the printing_device class and its children.
 */ 

#include<boost/shared_ptr.hpp>
#include<glibmm.h>
#include<alphabet.h>
#include<rotor_machine.h>

using namespace Glib;

class rotor_machine;

/*! \brief An abstract class that provides an interface for producing output symbols from integer values
 *         (key codes) in the context that some machines have a letters/figures mode and/or differing
 *         output alphabets for encryption and decryption. It mirrors the rotor_keyboard class which
 *         does the same on the input side of a rotor_machine.
 *
 *  As the rotor_machine implementations in rmsk only operate on integers and not on symbols there has
 *  to be a way produce output symbols from integer values or key codes. In the simplest case this
 *  can be done via a single instance of the alphabet template. But in some cases rotor machines employ 
 *  several output alphabets and it depends on the state of the printing_device and the operation desired
 *  (encrypt or decrypt) which of these alphabets is used to create an output symbol.
 *  
 */
class printing_device {
public:

    /*! \brief Default constructor.
     */    
    printing_device() { machine = NULL;  }    

    /*! \brief This method encrypts the input value or key code contained in parameter in using the
     *         underlying rotor_machine and produces an output symbol.
     */        
    virtual ustring print_encrypt(unsigned int in) = 0;

    /*! \brief This method decrypts the input value or key code contained in parameter in using the
     *         underlying rotor_machine and produces an output symbol.
     *
     *  The output symbol can be the empty string as some input values do not produce visible output
     *  upon decryption.
     */
    virtual ustring print_decrypt(unsigned int in) = 0;

    /*! \brief This method encrypts the input values contained in the parameter vector in by applying 
     *          print_encrypt(unsigned int) to each of its elements.
     */            
    virtual ustring print_encrypt(vector<unsigned int>& in);    

    /*! \brief This method decrypts the input values contained in the parameter vector in by applying 
     *          print_decrypt(unsigned int) to each of its elements.
     */            
    virtual ustring print_decrypt(vector<unsigned int>& in);        

    /*! \brief Allows to set the rotor_machine that is using this printing_device.
     */            
    virtual void set_machine(rotor_machine *new_machine) { machine = new_machine; }
    
    /*! \brief Returns the rotor_machine this printing_device is associated with.
     */        
    virtual rotor_machine *get_machine() { return machine; }

    /*! \brief Helper method to translate the ustring value given in parameter in to a vector
     *         of gunichars which is referened by the parameter vec.
     */        
    static void ustr_to_vec(ustring& str, vector<gunichar>& vec);
    
    /*! \brief Returns the current state of this printing_device.
     *
     *  FUNC_FIGURE_SHIFT = Keyboard is in figures mode
     *  FUNC_LETTER_SHIFT = Keyboard is in letters mode
     *  FUNC_NONE         = Keyboard has no letters or figures mode
     */            
    virtual unsigned int get_state() { return FUNC_NONE; }
    
    /*! \brief Sets the current state in which this printing_device is.
     */            
    virtual void set_state(unsigned int new_state) { ; }

    /*! \brief Returns the last en- or decryption result in form of a keycode.
     */    
    virtual unsigned int get_last_cipher_result() { return last_cipher_result; }


    /*! \brief Destructor. Does not delete the machine menber.
     */    
    virtual ~printing_device() { ; }

protected:

    /*! \brief Points to the rotor_machine with which this rotor_keyboard is associated.
     */            
    rotor_machine *machine;

    /*! \brief Holds the last en- or decryption result
     */    
    unsigned int last_cipher_result;
};

/*! \brief A class that implements the printing_device interface for machines that have a letters/figures
 *         mode and use different alphabets for output during encryption and decryption.
 *
 *  Used by the Typex and the KL7. These machines use a ciphertext ::alphabet of output symbols when doing 
 *  encryptions and an output letters alphabet as well as an output figures alphabet when doing decryptions.
 *  Switching between the two possible output alphabets during decryption occurs when decryption by the
 *  underlying rotor machine results in certain special integer values.
 */
class shifting_printing_device : public printing_device {
public:

    /*! \brief Constructor that defines the key codes which make this shifting_printing_device switching to 
     *         another state and thereby to another plaintext output alphabet.
     */          
    shifting_printing_device(unsigned int letter_char, unsigned int figure_char);
    
    /*! \brief Constructor that defines the key codes which make this shifting_printing_device switching to another state. In contrast
     *         to the shifting_printing_device(unsigned int, unsigned int) constructor this constructor allows to define
     *         a different set of "switching codes" for letters mode and figures mode.
     */                
    shifting_printing_device(unsigned int ltr_letter_char, unsigned int ltr_figure_char, unsigned int fig_letter_char, unsigned int fig_figure_char);    

    /*! \brief This method allows to specifiy the alphabet (the ciphertext alphabet) that is used to produce output symbols
     *         while performing encryptions.
     */                  
    virtual void set_cipher_alphabet(boost::shared_ptr<alphabet<gunichar> > alpha) { out_alpha = alpha; }

    /*! \brief This method allows to specifiy the alphabet (the plaintext letters alphabet) that is used to produce
     *         output symbols while performing decryptions is letters mode.
     */                  
    virtual void set_letters_alphabet(boost::shared_ptr<alphabet<gunichar> > alpha) { letters_alpha = alpha; }

    /*! \brief This method allows to specifiy the alphabet (the plaintext figures alphabet) that is used to produce
     *         output symbols while performing decryptions is figures mode.
     */                  
    virtual void set_figures_alphabet(boost::shared_ptr<alphabet<gunichar> > alpha) { figures_alpha = alpha; } 

    /*! \brief This method allows to specifiy the output that is to be generated when the decryption results in
     *         the value that causes this shifting_printing_device to switch to letters mode.
     */              
    virtual void set_ltr_key_output(ustring out_ltr_state, ustring out_fig_state);

    /*! \brief This method allows to specifiy the output that is to be generated when the decryption results in
     *         the value that causes this shifting_printing_device to switch to figures mode.
     */              
    virtual void set_fig_key_output(ustring out_ltr_state, ustring out_fig_state);   

    /*! \brief This method encrypts the input value or key code contained in parameter in using the
     *         underlying rotor_machine and produces a ciphertext output symbol using the out_alpha.
     */
    virtual ustring print_encrypt(unsigned int in);
    
    /*! \brief This method decrypts the input value or key code contained in parameter in using the
     *         underlying rotor_machine and produces a plaintext output symbol. Depending on get_state()
     *         either the letters_alpha or the figures_alpha is used to produce the output symbol.
     */
    virtual ustring print_decrypt(unsigned int in);

    virtual unsigned int get_state() { return current_state; }
    virtual void set_state(unsigned int new_state) { current_state = new_state; }

    /*! \brief Destructor. Does not delete the machine member.
     */   
    virtual ~shifting_printing_device() { ; }
    
protected:
    
    /*! \brief Helper method that sets the special_outputs member to default values, i.e. the empty string.
     */       
    void init_special_outputs();

    /*! \brief Holds the letters alphabet that is used when doing decryptions.
     */       
    boost::shared_ptr<alphabet<gunichar> > letters_alpha;

    /*! \brief Holds the figures alphabet that is used when doing decryptions.
     */       
    boost::shared_ptr<alphabet<gunichar> > figures_alpha;

    /*! \brief Holds the alphabet that is used when doing encryptions.
     */       
    boost::shared_ptr<alphabet<gunichar> > out_alpha;

    /*! \brief Holds the state this shifting_printing_device is currently in.
     */   
    unsigned int current_state;

    /*! \brief This member maps the current state to the "switching codes" that are valid in that state. The
     *         first component of the pair specifies the code that causes this shifting_printing_device to
     *         switch into letters mode and the second component the code that causes switching into letters
     *         mode.
     */       
    map<unsigned int, pair<unsigned int, unsigned int> > code_contacts;

    /*! \brief This member maps the current state to the output that is produced when a "switching code" 
     *         is encountered in that state during decryption. The first component of the pair specifies
     *         the output that is produced when the letters code is encountered and the second component gives
     *         the output that is generated for the figures code.
     */       
    map<unsigned int, pair<ustring, ustring> > special_outputs;    
};

/*! \brief A class that implements the printing_device interface for machines that have no letters/figures
 *         mode and use the same alphabet (the symbol alphabet) for output during encryption and decryption.
 *
 *  Used by all Enigma variants, the SG39 and the Nema.
 */
class symmetric_printing_device : public printing_device  {
public:

    /*! \brief Default constructor.
     */   
    symmetric_printing_device() : printing_device() { ; }
    
    /*! \brief Constructs a symmetric printing device from a ustring which is used to create an alphabet that
     *         is assigend to the member sym_alpha.
     */       
    symmetric_printing_device(ustring alpha_chars);

    /*! \brief Sets the value of the member sym_alpha.
     */       
    virtual void set_alphabet(boost::shared_ptr<alphabet<gunichar> > alpha) { sym_alpha = alpha; }    

    /*! \brief This method encrypts the input value or key code contained in parameter in using the
     *         underlying rotor_machine and produces an output symbol.
     */
    virtual ustring print_encrypt(unsigned int in);
    
    /*! \brief This method decrypts the input value or key code contained in parameter in using the
     *         underlying rotor_machine and produces an output symbol.
     */    
    virtual ustring print_decrypt(unsigned int in);

    /*! \brief Destructor. Does not delete the machine member.
     */   
    virtual ~symmetric_printing_device() { ; }
    
protected:

    /*! \brief Holds the alphabet that is used to map integervalues to output symbols during en- and
     *         decryptions.
     */       
    boost::shared_ptr<alphabet<gunichar> > sym_alpha;
};

/*! \brief A class that implements the printing_device interface for machines that have no letters/figures
 *         mode but use different alphabets for output during encryption and decryption.
 *
 *  Used by the SIGABA variants. The output alphabet used during encryption is called ciphertext alphabet 
 *  and the output alphabet used when decrypting is called the plaintext alphabet.
 *
 */
class asymmetric_printing_device : public printing_device {
public:

    /*! \brief Default constructor.
     */   
    asymmetric_printing_device() : printing_device() { ; }
    
    /*! \brief Sets the dec_alpha member, that is used to produce plaintext symbols.
     */       
    virtual void set_plain_alphabet(boost::shared_ptr<alphabet<gunichar> > alpha) { dec_alpha = alpha; }

    /*! \brief Sets the enc_alpha member, that is used to produce ciphertext symbols.
     */       
    virtual void set_cipher_alphabet(boost::shared_ptr<alphabet<gunichar> > alpha) { enc_alpha = alpha; }    

    /*! \brief This method encrypts the input value or key code contained in parameter in using the
     *         underlying rotor_machine and produces an output ciphertext symbol using the enc_alpha.
     */
    virtual ustring print_encrypt(unsigned int in);
    
    /*! \brief This method decrypts the input value or key code contained in parameter in using the
     *         underlying rotor_machine and produces an output plaintext symbol using the dec_alpha.
     */        
    virtual ustring print_decrypt(unsigned int in);

    /*! \brief Destructor. Does not delete the machine member.
     */   
    virtual ~asymmetric_printing_device() { ; }

protected:

    /*! \brief Holds the alphabet that is used to map integer values to output symbols during
     *         encryptions (ciphertext alphabet).
     */       
    boost::shared_ptr<alphabet<gunichar> > enc_alpha;

    /*! \brief Holds the alphabet that is used to map integer values to output symbols during
     *         decryptions (plaintext alphabet).
     */       
    boost::shared_ptr<alphabet<gunichar> > dec_alpha;
};


#endif /* __printing_device_h__ */
