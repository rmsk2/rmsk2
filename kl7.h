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

#ifndef __kl7_h__
#define __kl7_h__

/*! \file kl7.h
 *  \brief This file contains the header for the classes that implement the KL7 simulator.
 */ 

#include<rotor_machine.h>

#define KL7_ROTOR_SIZE 36

#define KL7_ROT_1 "kl7_rotor_1"
#define KL7_ROT_2 "kl7_rotor_2"
#define KL7_ROT_3 "kl7_rotor_3"
#define KL7_ROT_4 "kl7_rotor_4"
#define KL7_ROT_5 "kl7_rotor_5"
#define KL7_ROT_6 "kl7_rotor_6"
#define KL7_ROT_7 "kl7_rotor_7"
#define KL7_ROT_8 "kl7_rotor_8"
#define LETTER_RING_POS "letter"

// Rotor identifiers
const unsigned int KL7_ROTOR_A = 0;
const unsigned int KL7_ROTOR_B = 1;
const unsigned int KL7_ROTOR_C = 2;
const unsigned int KL7_ROTOR_D = 3;
const unsigned int KL7_ROTOR_E = 4;
const unsigned int KL7_ROTOR_F = 5;
const unsigned int KL7_ROTOR_G = 6;
const unsigned int KL7_ROTOR_H = 7;
const unsigned int KL7_ROTOR_I = 8;
const unsigned int KL7_ROTOR_J = 9;
const unsigned int KL7_ROTOR_K = 10;
const unsigned int KL7_ROTOR_L = 11;
const unsigned int KL7_ROTOR_M = 12;

// Ring identifiers
const unsigned int KL7_RING_1 =  100;
const unsigned int KL7_RING_2 =  101;
const unsigned int KL7_RING_3 =  102;
const unsigned int KL7_RING_4 =  103;
const unsigned int KL7_RING_5 =  104;
const unsigned int KL7_RING_6 =  105;
const unsigned int KL7_RING_7 =  106;
const unsigned int KL7_RING_8 =  107;
const unsigned int KL7_RING_9 =  108;
const unsigned int KL7_RING_10 = 109;
const unsigned int KL7_RING_11 = 110;
const unsigned int KL7_RING_WIDE = 111;

/*! \brief A class that allows to retrieve a rotor_set object that contains the standard rotor set used by the KL7.
 *
 *  The returned rotor_set contains the data for the 13 rotors A-M as well as the rings 1-11.
 */
class kl7_rotor_factory {
public:
    /*! \brief Returns a reference to the default rotor_set that is currently in use.
     */
    static rotor_set& get_rotor_set();

    /*! \brief Allows to set the default rotor_set that is in use.
     */    
    static void set_rotor_set(rotor_set& r_set) { kl7_set = r_set; }
    
protected:
    /*! \brief Holds the default rotor_set.
     */
    static rotor_set kl7_set;
};

/*! \brief A class that provides a stepping_gear object that implements the stepping algorithm of the KL7.
 *
 *  Noteworthy about the KL7 stepping gear is that the rotors have a notch ring that can be chosen out of a set of
 *  11 rings *and* a letter ring. Notch rings and letter rings can be moved independently with respect to the wiring
 *  core. Therefore each rotor has a notch ring offset and a letter ring offset. The two offsets are modeled by adding
 *  corresponding mod_int_vals to the rotor_descriptor objects in use in the kl7_stepping_gear. Also worth mentioning
 *  is that the rotors of the KL7 have 36 contacts instead of the usual 26. The reason for that is that 10 contacts
 *  at the end of the rotor bank are looped back to the beginning of the bank. This increases the number of different
 *  rotor positions but still results in a permutation of the 26 letters of the latin alphabet.
 */
class kl7_stepping_gear : public stepping_gear {
public:
    /*! \brief Constructor. As usual the vector rotor_identifiers has to contain the names that are used
     *         to refer to the eight rotor slots of the KL7.
     */
    kl7_stepping_gear(vector<string>& rotor_identifiers);
    
    /*! \brief Implements the KL7 stepping motion.
     */        
    virtual void step_rotors();

    /*! \brief Sets the letter and notch ring offset of the rotor identified by the parameter identifier to the values indicated
     *         by the parameters letter_ring_offset and notch_ring_offset.
     */            
    virtual void set_kl7_rings(const char *identifier, unsigned int letter_ring_offset, unsigned int notch_ring_offset);

    /*! \brief Sets the letter and notch ring offset of the rotor identified by the parameter identifier to the values indicated
     *         by the parameters letter_ring_offset and notch_ring_offset. Additionally the displacement of the rotor is set
     *         in such a way that the rotor window shows the letter at position new_pos.
     *
     *  The rotor KL7_ROT_4 is stationary and therefore can not be moved. Calling this method with identifier set to KL7_ROT_4
     *  results in an exception.
     */            
    virtual void set_kl7_rings_and_pos(const char *identifier, unsigned int letter_ring_offset, unsigned int notch_ring_offset, unsigned int new_pos = 0);    
    
    /*! \brief Returns the notch ring offest that has been set on the rotor specified by the parameter identifier.
     */            
    virtual unsigned int get_notch_offset(const char *identifier);

    /*! \brief Returns the letter ring offest that has been set on the rotor specified by the parameter identifier.
     */            
    virtual unsigned int get_letter_offset(const char *identifier);
    
    /*! \brief Sets the displacement of the rotor named by the parameter identifier in such a way that the rotor window shows
     *         the letter at position new_pos.
     *
     *  The rotor KL7_ROT_4 is stationary and therefore can not be moved. Calling this method with identifier set to KL7_ROT_4
     *  results in an exception.
     */       
    virtual void move_to_letter_ring_pos(const char *identifier, unsigned int new_pos);    

    /*! \brief The stationary rotor KL7_ROT_4 has a letter ring position but no ring. This method can be used to set the
     *         letter ring position of the stationary rotor.
     */ 
    virtual void set_stationary_rotor_ring_pos(unsigned int letter_ring_offset);
    
    /*! \brief Returns the current letter ring position of the stationary rotor KL7_ROT_4.
     */          
    virtual unsigned int get_stationary_rotor_ring_pos();

    /*! \brief Sets all rings to the offset 0 and moves all rotors to the 0 position.
     */      
    virtual void reset();

    /*! \brief Destructor.
     */      
    virtual ~kl7_stepping_gear() { ; }
    
protected:
    /*! \brief Saves the letter ring and notch ring offsets of the rotor identified by the parameter identifier in
     *         the KeyFile object referenced by the parameter ini_file.
     */ 
    virtual void save_additional_components(string& identifier, Glib::KeyFile& ini_file);

    /*! \brief Restores the letter ring and notch ring offsets of the rotor identified by the parameter identifier
     *         from the KeyFile object referenced by the parameter ini_file.
     *
     *  Returns false in case no errors were encountered.
     */ 
    virtual bool load_additional_components(string& identifier, Glib::KeyFile& ini_file);
};

/*! \brief A class implementing a simulator for the KL7.
 *
 *  The KL7 simulator has been implemented using information from the the help file of Mr. Rijmenants KL7 simulator
 *  available at http://users.telenet.be/d.rijmenants/KL-7%20Help.pdf .
 */
class kl7 : public rotor_machine {
public:
    /*! \brief Constructor. The vector r_ids has to specify a rotor_id object for each of the eight rotors that are
     *         to be placed in the machine. Even though technically possible it is historically incorrect to place
     *         a rotor in reverse in the machine by setting rotor_id.insert_inverse to true.
     *
     *  The rotor slots are numbered from left to right as seen by the user of the machine.
     */
    kl7(vector<rotor_id>& r_ids);
    
    /*! \brief Convenience method that returns the stepping_gear object currently in use after casting it to
     *         the correct type kl7_stepping_gear.
     */    
    virtual kl7_stepping_gear *get_kl7_stepper() { return dynamic_cast<kl7_stepping_gear *>(stepper); }
    
    /*! \brief Even though the rotors of the KL7 have 36 contacts, the machine still only generates a permutation of
     *         size 26. This is due to the feedback feature of the rotor stack. 
     */            
    virtual unsigned int get_size() { return 26; }    

    /*! \brief Returns the letter currently visible in the rotor window of the rotor identified by the parameter
     *         rotor_identifier. 
     *
     *  In the real machine ten positons on the letter ring of each rotor are empty. When the rotor is at one of these
     *  empty positions this method nontheless returns a non empty value. In that case characters from the range 0-9
     *  are returned for the first, second, third, ... empty position.
     */    
    virtual ustring visualize_rotor_pos(string& rotor_identifier);
    
    /*! \brief Returns a string that contains a visualization of the rotor positions as shown in the rotor windows
     *         of the machine.
     *
     *  This overloaded version reverses the character sequence. 
     */
    virtual ustring visualize_all_positions();    
    

    /*! \brief Destructor.
     */          
    virtual ~kl7() { delete stepper; }
};

#endif /* __kl7_h__ */

