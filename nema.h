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

#ifndef __nema_h__
#define __nema_h__

/*! \file nema.h
 *  \brief This file contains the header for the classes that implement the Nema simulator.
 */ 

#include<utility>
#include<boost/shared_ptr.hpp>
#include<alphabet.h>
#include<rotor.h>
#include<rotor_ring.h>
#include<rotor_machine.h>
#include<rotor_set.h>

#define DRIVE_RED_1 "drive1"
#define CONTACT_2   "contact2"
#define DRIVE_3     "drive3"
#define CONTACT_4   "contact4"
#define DRIVE_5     "drive5"
#define CONTACT_6   "contact6"
#define DRIVE_7     "drive7"
#define CONTACT_8   "contact8"
#define DRIVE_9     "drive9"
#define CONTACT_UKW_10 "contact10"

// ids of rotors/contact wheels
const unsigned int NEMA_ROTOR_A = 0;
const unsigned int NEMA_ROTOR_B = 1;
const unsigned int NEMA_ROTOR_C = 2;
const unsigned int NEMA_ROTOR_D = 3;
const unsigned int NEMA_ROTOR_E = 4;
const unsigned int NEMA_ROTOR_F = 5;
const unsigned int NEMA_UKW = 6;
const unsigned int NEMA_ETW = 7;

// ids of rings
const unsigned int NEMA_DRIVE_WHEEL_1 = 100;
const unsigned int NEMA_DRIVE_WHEEL_2 = 101;
const unsigned int NEMA_DRIVE_WHEEL_12 = 102;
const unsigned int NEMA_DRIVE_WHEEL_13 = 103;
const unsigned int NEMA_DRIVE_WHEEL_14 = 104;
const unsigned int NEMA_DRIVE_WHEEL_15 = 105;
const unsigned int NEMA_DRIVE_WHEEL_16 = 106;
const unsigned int NEMA_DRIVE_WHEEL_17 = 107;
const unsigned int NEMA_DRIVE_WHEEL_18 = 108;
const unsigned int NEMA_DRIVE_WHEEL_19 = 109;
const unsigned int NEMA_DRIVE_WHEEL_20 = 110;
const unsigned int NEMA_DRIVE_WHEEL_21 = 111;
const unsigned int NEMA_DRIVE_WHEEL_22 = 112;
const unsigned int NEMA_DRIVE_WHEEL_23 = 113;

/*! \brief A class that allows to retrieve a rotor_set object that contains the standard rotor set used by the Nema.
 *
 *  In contrast to most other machines the Nema makes use of two different types of rotors. First there are the contact
 *  rotors that are used to scramble (or unscramble) the input data and then there are the drive wheels that have no
 *  electrical contacts but are used to control the stepping of the contact wheels through notches which are placed on
 *  their circumfence. Drive wheels are modeled as rotor objects which use the identity permutation and a rotor_ring
 *  that contains the notch data.
 */
class nema_rotor_factory {
public:
    /*! \brief Returns a reference to the default rotor_set that is currently in use.
     */
    static rotor_set& get_rotor_set();

    /*! \brief Allows to set the default rotor_set that is in use.
     */
    static void set_rotor_set(rotor_set& r_set) { nema_set = r_set; }

    /*! \brief Returns the standard alphabet that is used by the Nema.
     *
     *  An odd thing about the Nema is, that the contact in position zero is mapped to the letter I and not A as in all other
     *  machines. Therefore this method returns a pointer to an alphabet<char> object that was initialized with the data
     *  "ijklmnopqrstuvwxyzabcdefgh".
     */
    static alphabet<char> *get_nema_alpha(); 

    /*! \brief This method allows to create ring data from a zero terminated string. The positions of the notches are spcified by
     *         letters from the alphabet returned by get_nema_alpha(). In other words an 'i' specifies a noth on position
     *         0, a 'j' on position 1 and so on.
     */    
    static vector<unsigned int> create_ring_data(const char *notches);

    /*! \brief Combines the ring data given in the parameters data_left and data_right and stores the result in the vector referenced by
     *         ring_data. 
     *
     *  The combined data is created at each position by multiplying the corresponding value from the vector data_left by two and then
     *  adding the corresponding value from data_right.
     */    
    static void create_ring_data_double(vector<unsigned int>& ring_data, vector<unsigned int>& data_left, vector<unsigned int>& data_right);
    
protected:
    /*! \brief Holds the default rotor_set.
     */
    static rotor_set nema_set;
};

/*! \brief A class that provides a stepping_gear object that implements the stepping algorithm of the Nema.
 */
class nema_stepper : public stepping_gear {
public:
    /*! \brief Constructor. The vector rotor_identifiers has to specify the names of the rotor slots.
     */
    nema_stepper(vector<string>& rotor_identifiers) : stepping_gear(rotor_identifiers, 26) { stack->set_reflecting_flag(true); }

    /*! \brief Implments the stepping algorithm used by the Nema.
     */
    virtual void step_rotors();
    
    /*! \brief Resets all contact and drive wheels to their default positions.
     *
     *  Drive and contact wheels are moved to that position that makes an 'a' appear in the window of the rotor. The offset of the ring
     *  with respect to the drive wheel or contact rotor core is set to 2.
     */    
    virtual void reset();

    /*! \brief Moves the contact rotor or drive wheel specified by identifier to that position that makes the letter given in parameter
     *         pos appear in the rotor window. The letter has to be lower case and in the the range a-z.
     */    
    virtual void set_rotor_to_pos(string& identifier, char pos);

    /*! \brief Moves the contact rotor or drive wheel specified by rotor_num to that position that makes the letter given in parameter
     *         pos appear in the rotor window. The letter has to be lower case and in the the range a-z.
     */    
    virtual void set_rotor_to_pos(unsigned int rotor_num, char pos);

    /*! \brief Moves the contact rotor or drive wheel specified by the zero terminated string identifier to that position that makes
     *         the letter given in parameter pos appear in the rotor window. The letter has to be lower case and in the the range a-z.
     */    
    virtual void set_rotor_to_pos(const char *identifier, char pos) { string temp(identifier); set_rotor_to_pos(temp, pos); }

    /*! \brief Returns the letter currently shown in the window of the rotor in the slot named by the STL string identfier as a
     *         lower case letter in the range a-z.
     */
    virtual char get_rotor_pos(string& identifier);

    /*! \brief Returns the letter currently shown in the window of the rotor in the slot named by the C-style string identfier as a
     *         lower case letter in the range a-z.
     */
    virtual char get_rotor_pos(const char *identifier) { string temp(identifier); return get_rotor_pos(temp); }

    /*! \brief Returns the string that you get when you read the characters that are currently shown in the windows of all the rotors
     *         (contact and drive wheel) when read from left to right.
     */
    virtual string get_all_positions();

    /*! \brief Allows to set the positions of all rotors through the string specified by the parameter new_positions. The string specifies
     *         the new positions in form of 10 lower case letters in the range a-z. The letter on position 0 determines the position
     *         of the leftmost contact or drive wheel and the letter on position 9 the position of the rightmost (red) wheel. 
     */
    virtual void set_all_positions(string& new_positions);

    /*! \brief Does the same as set_all_positions(string&) but uses a zero terminated C-style string to specify the new positions.
     */
    virtual void set_all_positions(const char *new_positions) { string temp(new_positions); set_all_positions(temp); }

    /*! \brief Destructor.
     */    
    virtual ~nema_stepper() { ; }
    
protected:
    /*! \brief Helper method that returns the notch data of the red wheel at the current position.
     *
     *  In left_notch the notch information on the left side of the red wheel is returned and right_notch contains the
     *  notch information from the right side. Interestingly enough the offset with respect to the current drive wheel position
     *  where the notch information is sensed is different for the left and right side.
     */    
    virtual void red_notches(bool& left_notch, bool& right_notch);
};

/*! \brief A class implementing a simulator for the Nema.
 */
class nema : public rotor_machine {
public:
    /*! \brief Constructor. The rotors used by a Nema are configured by specifiying a vector (parameter rotor_settings) containing
     *         four rotor_assembly objects each of which specifies a contact and a drive wheel. Additionally one has to specify the
     *         drive wheels that will be used as the left and the right side of the red wheel (parameters left_red_drive_wheel and
     *         right_red_drive_wheel). As there is only one UKW it has not to be explicitly passed to this method.
     */
    nema(vector<rotor_assembly>& rotor_settings, unsigned int left_red_drive_wheel, unsigned int right_red_drive_wheel);

    /*! \brief Returns the letter currently shown in the window of the rotor in the slot named by the STL string identfier as a
     *         ustring of length one. The letter at position zero is lower case and in the range a-z.
     */
    virtual ustring visualize_rotor_pos(string& rotor_identifier);

    /*! \brief Allows to create a special drive wheel that combines the notches of two other drive wheels.
     *
     *  The rightmost drive wheel in a Nema is special in that it is coloured red and has notches on its left and right side. The
     *  parameters rotor_id_l and rotor_id_r have to specify the drive wheels, which are to be placed on
     *  the left and the right side of the newly constructed drive wheel. The rotor that is returned in the first component of the
     *  pair has its permutation set to the identity. The ring or notch data of the returned red wheel consists of two bits on each
     *  position: The most siginificant bit is the ring data from rotor_id_l the least significant bit is the ring data from
     *  rotor_id_r.
     */
    pair<boost::shared_ptr<rotor>, boost::shared_ptr<rotor_ring> > make_red_wheel(unsigned int rotor_id_l, unsigned int rotor_id_r);    

    /*! \brief Constructs the red wheel using the parameters left_red_drive_wheel and right_red_drive_wheel by calling 
     *         make_red_wheel(unsigned int, unsigned int) and places the newly created drive wheel into the machine at the 
     *         rightmost slot DRIVE_RED_1.
     */        
    void prepare_red_wheel(unsigned int left_red_drive_wheel, unsigned int right_red_drive_wheel);

    /*! \brief Convenience method that returns the stepping_gear object in use in this nema instance and casts it to the correct type
     *         nema_stepper.
     */    
    nema_stepper *get_nema_stepper() { return dynamic_cast<nema_stepper *>(stepper); }
    
    /*! \brief This method randomizes the state of this nema object. If this method returns true an error occurred and the
     *         object's state is unchanged.
     *
     *  The parameter param is ignored.
     */
    virtual bool randomize(string& param);        

    /*! \brief Destructor.
     */  
    virtual ~nema() { delete stepper; } 
};

#endif /* __nema_h__ */

