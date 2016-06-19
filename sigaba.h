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

#ifndef __sigaba_h__
#define __sigaba_h__

/*! \file sigaba.h
 *  \brief This file contains the header for the classes that implement the SIGABA simulator.
 */

#include<set>

#include<rotor.h>
#include<rotor_machine.h>
#include<rotor_set.h>

// Names of rotor slots of driver machine
#define STATOR_L "stator_l"
#define STATOR_R "stator_r"
#define S_FAST "fast"
#define S_MIDDLE "middle"
#define S_SLOW "slow"

// Names of rotor slots of index machine
#define I_ZERO "i_zero"
#define I_ONE "i_one"
#define I_TWO "i_two"
#define I_THREE "i_three"
#define I_FOUR "i_four"

// Names of rotor slots of cipher rotors
#define R_ZERO "r_zero"
#define R_ONE "r_one"
#define R_TWO "r_two"
#define R_THREE "r_three"
#define R_FOUR "r_four"

// ids for the normal rotors
const unsigned int SIGABA_ROTOR_0 = 0;
const unsigned int SIGABA_ROTOR_1 = 1;
const unsigned int SIGABA_ROTOR_2 = 2;
const unsigned int SIGABA_ROTOR_3 = 3;
const unsigned int SIGABA_ROTOR_4 = 4;
const unsigned int SIGABA_ROTOR_5 = 5;
const unsigned int SIGABA_ROTOR_6 = 6;
const unsigned int SIGABA_ROTOR_7 = 7;
const unsigned int SIGABA_ROTOR_8 = 8;
const unsigned int SIGABA_ROTOR_9 = 9;

// ids for the index rotors
const unsigned int SIGABA_INDEX_0 = 0;
const unsigned int SIGABA_INDEX_1 = 1;
const unsigned int SIGABA_INDEX_2 = 2;
const unsigned int SIGABA_INDEX_3 = 3;
const unsigned int SIGABA_INDEX_4 = 4;


/*! \brief A class that allows to retrieve the default rotor_set objects for the normal and index
 *          rotors which are used by the SIGABA.
 */
class sigaba_rotor_factory {
public:
    /*! \brief Returns a rotor_set object which is currently in use as the default rotor set for the index machine.
     */
    static rotor_set& get_index_rotor_set();

    /*! \brief Sets the rotor_set object referenced in parameter r_set as the current default rotor set for the 
     *         index machine.
     */
    static void set_index_rotor_set(rotor_set& r_set) { index_set = r_set; }

    /*! \brief Returns a rotor_set object which is currently in use as the default rotor set for the driver machine
     *         and the cipher rotors.
     */
    static rotor_set& get_cipher_rotor_set();

    /*! \brief Sets the rotor_set object referenced in parameter r_set as the current default rotor set for the 
     *         driver machine and the cipher rotors.
     */
    static void set_cipher_rotor_set(rotor_set& r_set) { normal_set = r_set; }

protected:

    /*! \brief Holds the rotor_set object which stores the normal rotors that are used in the driver machine
     *         and as cipher rotors.
     */
    static rotor_set normal_set;

    /*! \brief Holds the rotor_set object which stores the index rotors that are used in the index machine.
     */
    static rotor_set index_set;
};

/*! \brief A class that serves as the base class for all three rotor machines that make up a SIGABA.
 */
class sigaba_base_machine : public rotor_machine {
public:
    /*! \brief Constructor.
     */
    sigaba_base_machine() : rotor_machine() { ; }

    /*! \brief Set the displacement of all rotors in this machine to such a value that the characters in new_positions
     *         appear in the rotor windows. The parameter alpha references the alphabet that specifies the allowed 
     *         characters and their mapping to the correct numerical value. If do_modify is false the machine state is
     *         not modified. This serves to test that the contents of the parameter new_positions is valid.
     *
     *  Returns true in case an error was encountered. Otherwise it returns false.
     */    
    bool move_all_sigaba_rotors(ustring& new_positions, alphabet<char>& alpha, bool do_modify);

protected:    
    /*! \brief Returns a string that specifies the character that can currently be seen in the rotor
     *         window of a cipher, driver or index rotor. The rotor is identified by the parameter rotor_identifier.
     *         The alphabet referenced by the parameter alpha is used to map the current rotor position to a
     *         character.
     *
     *  When used by the sigaba_index machine alpha references an alphabet that has ten elements. For the driver
     *  and cipher rotors an alphabet with 26 elements has to be used.
     */
    ustring visualize_sigaba_rotor_pos(string& rotor_identifier, alphabet<char>& alpha);
    
    /*! \brief Destructor.
     */    
    virtual ~sigaba_base_machine() { ; }    
};

/*! \brief A class that implements the stepping_gear which is in use in the SIGABA driver machine.
 *  
 *  Only the three rotors in the middle of the five rotors in the SIGABA driver machine do step. The three stepping rotors do so
 *  in an odometer like fashion. The stepping_gear of the driver machine can be uncoupled. When it is uncoupled the three moving
 *  rotors do not move even if step_rotors() is called. This is used for implementing sigaba_stepper::setup_step().
 */
class sigaba_driver_stepper : public stepping_gear {
public:
    /*! \brief Constructor. The parameter rotor_identifiers has to specify the names of the five rotor slots of the driver machine.
     */
    sigaba_driver_stepper(vector<string>& rotor_identifiers) : stepping_gear(rotor_identifiers, rmsk::std_alpha()->get_size()) { uncouple_stepper = false; }

    /*! \brief Implements the odometer stepping motion of the driver machine.
     */    
    virtual void step_rotors();

    /*! \brief Resets the positions of the five rotors of the driver machine to a value that makes the letter O appear
     *         in their rotor windows.
     */    
    virtual void reset();

    /*! \brief Returns whether this sigaba_driver_stepper is currently uncoupled (true) or not (false). 
     */    
    virtual bool get_uncouple_state() { return uncouple_stepper; }

    /*! \brief Marks this sigaba_driver_stepper as being currently uncoupled (true) or not (false). 
     */    
    virtual void set_uncouple_state(bool new_state) { uncouple_stepper = new_state; }

    /*! \brief Destructor. 
     */        
    virtual ~sigaba_driver_stepper() { ; }

protected:

    /*! \brief Is true if this sigaba_driver_stepper is currently uncoupled and false otherwise. 
     */        
    bool uncouple_stepper;
};

/*! \brief A class that implements the SIGABA driver rotor machine.
 */
class sigaba_driver : public sigaba_base_machine {
public:
    /*! \brief Constructor. The parameters specifiy the rotor_id to use for the left stator, the slow, fast and middle
     *         rotors and the right stator.
     */
    sigaba_driver(rotor_id stat_l_id, rotor_id slow_id, rotor_id fast_id, rotor_id middle_id, rotor_id stat_r_id);

    /*! \brief Calls sigaba_base_machine::visualize_sigaba_rotor_pos with the standard alphabet.
     */
    virtual ustring visualize_rotor_pos(string& rotor_identifier);

    /*! \brief Takes a textual specification of the new rotor positions and moves the (visible) rotors accordingly.
     *
     *  Returns true if an error was encountered else false.
     */
    virtual bool move_all_rotors(ustring& new_positions);

    /*! \brief Destructor. 
     */
    virtual ~sigaba_driver() { delete stepper; } 
};

/*! \brief A class that implements the SIGABA index rotor machine.
 */
class sigaba_index_machine : public sigaba_base_machine {
public:
    /*! \brief Constructor. The parameters specifiy the rotor_id to use for the rotors 0, 1, 2, 3, 4 of the index
     *         machine, where rotor 0 is the leftmost and rotor 4 the rightmost rotor when seen from the perspective
     *         of the user of the machine.
     */
    sigaba_index_machine(rotor_id null_id, rotor_id one_id, rotor_id two_id, rotor_id three_id, rotor_id four_id);
    
    /*! \brief Resets the position of all index rotors to the value 0.
     */    
    void reset();

    /*! \brief Calls sigaba_base_machine::visualize_sigaba_rotor_pos with a the alphabet consisting of the characters
     *         0123456789.
     */
    virtual ustring visualize_rotor_pos(string& rotor_identifier);

    /*! \brief Takes a textual specification of the new rotor positions and moves the (visible) rotors accordingly.
     *
     *  Returns true if an error was encountered else false.
     */
    virtual bool move_all_rotors(ustring& new_positions);

    /*! \brief Destructor. 
     */
    virtual ~sigaba_index_machine() { delete stepper; }
};

/*! \brief A class that implements the stepping_gear of a SIGABA. This is where the index and the driver machine live. 
 */
class sigaba_stepper : public stepping_gear {
public:
    /*! \brief Constructor. The parameter rotor_identifiers has to specify the names of the five slots of the 
     *         cipher rotors. The parameter csp_2900_flag has to be true if the stepping_gear is to be operated
     *         in CSP 2900 mode and false otherwise. I.e. false means CSP 889 mode.
     */
    sigaba_stepper(vector<string>& rotor_identifiers, bool csp_2900_flag = false);

    /*! \brief Implements the stepping motion for the cipher rotors.
     */        
    virtual void step_rotors();

    /*! \brief Performs a setup_setp operation on the driver rotor which is identified by the parameter rotor_name.
     *
     *  When setting up a SIGABA machine all cipher, driver and index rotors have to be moved to a defined initial position. 
     *  This method advances the driver rotor specified by rotor_name by one position. Additionally the cipher rotors are
     *  stepped upon each call of this method. In the real machine this has been used to generate the starting position of 
     *  the cipher rotors from a given starting position of the control rotors. On the one hand this reduces the number of
     *  possible machine settings but on the other hand it guarantees that the starting position of the cipher rotors is chosen
     *  in a clean pseudorandom way.
     */        
    virtual void setup_step(string& rotor_name);

    /*! \brief Performs a setup_step operation on the driver rotor which is identified by the zero terminated string
     *         rotor_name.
     */        
    virtual void setup_step(const char *rotor_name) { string temp(rotor_name); setup_step(temp); }

    /*! \brief Performs a num_steps setup_step operations on the driver rotor which is identified by the parameter
     *         rotor_name.
     */        
    virtual void setup_step(string& rotor_name, unsigned int num_steps);

    /*! \brief Performs a num_steps setup_step operations on the driver rotor which is identified by the zero terminated string
     *         rotor_name.
     */        
    virtual void setup_step(const char *rotor_name, unsigned int num_steps) { string temp(rotor_name); setup_step(temp, num_steps); }

    /*! \brief Resets cipher, driver and index rotors to default positions. For the driver rotors this is the positon
     *         makrked by the letter O.
     */        
    virtual void reset();

    /*! \brief Sets the sigaba_driver object given in parameter d to be used as the driver machine. The destructor deletes the
     *         object to which d points.
     */            
    virtual void set_driver_machine(sigaba_driver *d) { driver = d; }

    /*! \brief Returns a pointer to the driver machine currently in use with this object.
     */            
    virtual sigaba_driver *get_driver_machine() { return driver; }

    /*! \brief Sets the sigaba_index_machine object given in parameter i to be used as the index machine. The destructor
     *         deletes the object to which i points.
     */            
    virtual void set_index_bank(sigaba_index_machine *i) { index = i; }

    /*! \brief Returns a pointer to the index machine currently in use with this object.
     */            
    virtual sigaba_index_machine *get_index_bank() { return index; }

    /*! \brief Returns true if this sigaba_stepper is operated in CSP 2900 mode. When in CSP 889 mode false is returned.
     */                
    virtual bool is_2900() { return is_csp_2900; }

    /*! \brief Sets up the state variables (energized contacts, backstepping rotors) that allow the code to differentiate
     *         between CSP 2900 and CSP 889 mode.
     */                
    void prepare_machine_type(bool csp_2900_flag);    

    /*! \brief Destructor. Deletes driver and index machine.
     */                    
    virtual ~sigaba_stepper() { delete index; delete driver;  }
    
protected:

    /*! \brief This method simulates current that is applied to the input contact identified by parameter
     *         in_char of the driver rotors. It returns a number between 0-9 or the special value N. When
     *         a number < 10 is returned this signifies that the corresponding output contact of the index machine
     *         is energized. If the value N is returned no output contact of the index machine is energized. 
     */                    
    virtual unsigned int produce_control_output(unsigned int in_char);    

    /*! \brief Points to the driver machine.
     */                            
    sigaba_driver *driver;

    /*! \brief Points to the index machine.
     */                            
    sigaba_index_machine *index;
    
    /*! \brief Contains the contacts of the driver machine that are energized to produce the control
     *         output, i.e. the stepping information for the cipher rotors. These are different between
     *         CSP 889 and CSP 2900.
     */                                
    string energized_contacts;

    /*! \brief Contains the numbers of the cipher rotors that step in the opposite direction. IN CSP 889
     *         all cipher rootrs step in the same direction.
     */                            
    set<unsigned int> backstepping_rotors;
    
    /*! \brief Specifies how the 26 output contacts of the driver rotors are mapped to the ten input contacts
     *         of the index rotors.
     */                                
    unsigned int *contact_mapping;

    /*! \brief True if this sigaba_stepper is operated in CSP 2900 mode.
     */                                
    bool is_csp_2900;
};

/*! \brief A class that implements a simulator for the SIGABA. Both the CSP889 and the CSP2900 variants are
 *         implemented.
 *
 *  The SIGABA is in essence not a single rotor machine but three rotor machines that have been nested into each other.
 *  The simplest is the index machine, which consists of the five index rotors and has no stepping mechanism. The second
 *  is the driver machine which has five normal rotors, three of which step in an odometer like fashion. The index and
 *  the driver machine generate the stepping information for the third machine, which contains the five pseudorandomly
 *  stepping cipher rotors.
 */
class sigaba : public sigaba_base_machine {
public:
    /*! \brief Constructor. The vector r_ids has to specify a rotor_id object for each of the 15 rotors that are
     *         to be placed in the machine. If the parameter csp_2900_flag is true the machine is operated in
     *         CSP 2900 mode.
     *
     *  The elements 0-4 of r_ids specify the cipher rotors, elements 5-9 designate the driver rotors and elements
     *  10-14 determine the index rotors.
     */
    sigaba(vector<rotor_id>& r_ids, bool csp_2900_flag = false);

    /*! \brief Convenience method that returns the stepping_gear object currently in use after casting it to
     *         the correct type sigaba_stepper.
     */    
    virtual sigaba_stepper *get_sigaba_stepper() { return dynamic_cast<sigaba_stepper *>(stepper); }

    /*! \brief Changes the name of the default rotor set to be used in this SIGABA instance to the value specified
     *         by parameter default_set_name. This method calls the corresponding methods of the index and driver machine
     *         as well.
     */    
    virtual void set_default_set_name(string default_set_name);
    
    /*! \brief Calls sigaba_base_machine::visualize_sigaba_rotor_pos with the standard alphabet.
     */    
    virtual ustring visualize_rotor_pos(string& rotor_identifier);
    
    /*! \brief Returns a string that contains a visualization of the rotor positions as shown in the rotor windows
     *         of the machine.
     *
     *  This overloaded version includes the position of the driver and index rotors and additionally reverses their
     *  character sequence.
     */
    virtual ustring visualize_all_positions();
    
    /*! \brief This method randomizes the state of this sigaba object. If this method returns true an error occurred and the
     *         object's state is unchanged.
     *
     *  The parameter param is ignored.
     */
    virtual bool randomize(string& param);    

    /*! \brief Returns a description that differentiates between CSP 2900 and CSP 889 mode.
     */    
    virtual string get_description();
    
    /*! \brief The parameter new_positions has to contain 15 characters: The first five have to be from the range 0-9 and specify the index
     *         rotor positions. The next five specify the driver rotor positions and the last five the cipher rotor positions. The
     *         cipher and driver rotor positions have to be lowercase characters from the range a-z. The sequence in which the characters
     *         appear in the string new_positions has to match the sequence of rotors as seen by the user in the graphical simulator.
     *
     *  Returns true if an error was encountered else false. If true is returned the state of the underlying machine has not been changed.
     */
    virtual bool move_all_rotors(ustring& new_positions);    

    /*! \brief Does the same as move_all_rotors(ustring& new_positions) but new_positions is a zero terminated C-style string.
     *
     *  Returns true if an error was encountered else false. If true is returned the state of the underlying machine has not been changed.
     */
    virtual bool move_all_rotors(const char *new_positions) { ustring help(new_positions); return move_all_rotors(help); }

    /*! \brief Destructor.
     */        
    virtual ~sigaba() { delete stepper; }   
    
protected:
    /*! \brief Saves the value get_sigaba_stepper()->is_2900() in the KeyFile referenced through parameter ini_file.
     */        
    virtual void save_additional_components(Glib::KeyFile& ini_file);
    
    /*! \brief Retrieves the operating mode CSP 2900/CSP 889 from the KeyFile referenced through parameter ini_file.
     *         Return false if no errors occured.
     */        
    virtual bool load_additional_components(Glib::KeyFile& ini_file);
     
};

#endif /* __sigaba_h__ */

