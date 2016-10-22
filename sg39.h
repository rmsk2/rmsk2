/***************************************************************************
 * Copyright 2016 Martin Grap
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

#ifndef __sg39_h__
#define __sg39_h__

/*! \file sg39.h
 *  \brief This file contains the header for the classes that implement the Schluesselgeraet 39
 */ 

#include<utility>
#include<boost/shared_ptr.hpp>
#include<rotor_machine.h>
#include<rotor_set.h>

#define ROTOR_1 "rotor_1"
#define ROTOR_2 "rotor_2"
#define ROTOR_3 "rotor_3"
#define ROTOR_4 "rotor_4"
#define UKW_SG39 "umkehrwalze"

const unsigned int SG39_ROTOR_0 = 0;
const unsigned int SG39_ROTOR_1 = 1;
const unsigned int SG39_ROTOR_2 = 2;
const unsigned int SG39_ROTOR_3 = 3;
const unsigned int SG39_ROTOR_4 = 4;
const unsigned int SG39_ROTOR_5 = 5;
const unsigned int SG39_ROTOR_6 = 6;
const unsigned int SG39_ROTOR_7 = 7;
const unsigned int SG39_ROTOR_8 = 8;
const unsigned int SG39_ROTOR_9 = 9;

const unsigned int ID_SG39_UKW = 100;

/*! \brief A class that allows to retrieve a rotor_set object that contains the standard rotor set
 *         used by the Schluesselgeraet 39.
 *
 *  The rotor_set object (sg39_set) managed by this class is a singleton.
 */
class sg39_rotor_factory {
public:
    /*! \brief Returns the default rotor_set object.
     *
     *  If this method is called the first time it checks whether sg39_set contains any data.
     *  If not it constructs a corresponding object.
     */  
    static rotor_set& get_rotor_set();
    
    /*! \brief Allows to set the default rotor_set object.
     */       
    static void set_rotor_set(rotor_set& r_set) { sg39_set = r_set; }
    
protected:
    /*! \brief The actual rotor_set object to be used.
     */
    static rotor_set sg39_set;
};

/*! \brief A class that implements the stepping_gear used by the Schluesselgeraet 39.
 *
 *  The Schluesselgeraet 39, or short SG39, used three Haegelin style pinwheels as well as additional pins on 
 *  the rotors to control rotor stepping. Most of the methods that are added to the base class by this class
 *  deal with imlementing and managing the functionality of these pinwheels.   
 */
class sg39_stepping_gear : public stepping_gear {
public:
    /*! \brief Constructor. As usual the vector rotor_identifiers has to contain the names that are used
     *         to refer to the five rotor slots of the SG39.
     */
    sg39_stepping_gear(vector<string>& rotor_identifiers) : stepping_gear(rotor_identifiers, 26) { stack->set_reflecting_flag(true); }

    /*! \brief Implements the stepping motion. Stepping depends on the pins on the pinwheels and the rotors.
     */    
    virtual void step_rotors();
    
    /*! \brief Moves all rotors and wheels to the 0 position.
     */  
    virtual void reset();
    
    /*! \brief Moves the wheel identified by the zero terminated string rotor_name to the position specified in parameter
     *         new_pos.
     *
     *  In SG39 the three pinwheels are named by the rotors they control. I.e. valid values for rotor_name are ROTOR_1, ROTOR_2
     *  and ROTOR_3. 
     */      
    virtual void set_wheel_pos(const char *rotor_name, unsigned int new_pos);

    /*! \brief Returns the current position of the the wheel identified by the zero terminated string rotor_name.
     *
     *  In SG39 the three pinwheels are named by the rotors they control. I.e. valid values for rotor_name are ROTOR_1, ROTOR_2
     *  and ROTOR_3. 
     */  
    virtual unsigned int get_wheel_pos(const char *rotor_name);
    
    /*! \brief Sets the wheel data on the wheel identified by the zero terminated string rotor_name. The wheel data 
     *         to be used is given in parameter new_data. Each nonzero element in new_data represents an active pin.
     *         For ROTOR_1 21 values are needed, for ROTOR_2 23 and for ROTOR_3 25.
     *
     *  In SG39 the three pinwheels are named by the rotors they control. I.e. valid values for rotor_name are ROTOR_1, ROTOR_2
     *  and ROTOR_3. 
     */       
    virtual void set_wheel_data(const char *rotor_name, vector<unsigned int>& new_data);

    /*! \brief Returns the wheel data on the wheel identified by the zero terminated string rotor_name. This method modifies  
     *         the vector referenced by parameter data in such a way that it contains the wheel data of desired wheel. 
     *
     *  In SG39 the three pinwheels are named by the rotors they control. I.e. valid values for rotor_name are ROTOR_1, ROTOR_2
     *  and ROTOR_3. 
     */     
    virtual void get_wheel_data(const char *rotor_name, vector<unsigned int>& data);

    /*! \brief Destructor.
     */      
    virtual ~sg39_stepping_gear() { ; }
    
protected:
    /*! \brief Returns true if the pin on the current position of the wheel named by identifier is active.
     */     
    bool wheel_is_at_notch(const char *identifier);

    /*! \brief Increments the position of the wheel named by identifier.
     */ 
    void advance_wheel(const char *identifier);

    /*! \brief Saves wheeldata and wheelpos of the wheel identified by the parameter identifier in the KeyFile object
     *         referenced by the parameter ini_file.
     */     
    void save_additional_components(string& identifier, Glib::KeyFile& ini_file);

    /*! \brief Retrieves wheeldata and wheelpos of the wheel identified by the parameter identifier from the KeyFile object
     *         referenced by the parameter ini_file.
     *
     *  Returns false in case no errors were encountered.
     */  
    bool load_additional_components(string& identifier, Glib::KeyFile& ini_file);
};

/*! \brief A struct which is intended as a helper for the randomize method. It contains a pointer to the pin
 *         specification and the maximum number of pins on the corresponding rotor/wheel.
 */
struct randomize_help {
    /*! \brief Constructor. The parameter sp is used for setting spec and sz is used for setting size.
     */
    randomize_help(string *sp, unsigned int sz) { spec = sp; size = sz; }
    /*! \brief Holds a pointer to the pin specification. */
    string *spec;
    /*! \brief Holds the maximum number of pins on the corresponding wheel/rotor. */    
    unsigned int size;
};

/*! \brief A class that implements a simulator for the Schlüsselgerät 39.
 *
 *  Information about the SG39 can be found in this document (Link goes to NSA. Use at your own risk): 
 *  https://www.nsa.gov/about/_files/cryptologic_heritage/publications/wwii/german_cipher.pdf .
 */
class schluesselgeraet39 : public rotor_machine {
public:
    /*! \brief Constructor. The parameters specify the rotor ids of the rotors that are to be inserted in the machine.
     *
     *  rotor_1_id specifies the rightmost rotor and rotor_4_id the leftmost rotor which does not step
     *  during operation of the machine.             
     */
    schluesselgeraet39(unsigned int rotor_1_id, unsigned int rotor_2_id, unsigned int rotor_3_id, unsigned int rotor_4_id);

    /*! \brief Convenience method that returns a pointer to the sg39_stepping_gear object currently used by the simulator.
     */
    sg39_stepping_gear *get_sg39_stepper() { return dynamic_cast<sg39_stepping_gear *>(stepper); }

    /*! \brief Changes the involution currently in use in the reflector of this machine. The parameter data has to specifiy
     *         an involution where each pair of lower case characters defines one cycle. There have to be exactly 13 pairs.
     */
    virtual void set_reflector(vector<pair<char, char> >& data);

    /*! \brief Sets the permutation which is to be used as the wiring of the reflector. It has to be noted that the parameter
     *         involution should specify an actual involution but it is possible to use any permutation.
     */
    virtual void set_reflector(boost::shared_ptr<permutation> involution);   

    /*! \brief Returns a string that contains a visualization of the rotor positions as shown in the rotor windows
     *         of the machine.
     *
     *  This overloaded version includes the pinwheel positions in the output. 
     */
    virtual ustring visualize_all_positions();
    
    /*! \brief This method randomizes the state of this schluesselgeraet39 object. If this method returns true an error
     *         occurred and the object's state is unchanged.
     *
     *  The parameter param is ignored.
     */
    virtual bool randomize(string& param);
    
    /*! \brief The parameter new_positions has to contain 7 lowercase characters. The first four specify the rotor positions.
     *         The last three the wheel positions. The rotor positions have to be from the range a-z. The leftmost wheel position
     *         has to be from the range a-y, the middle wheel position from the range a-w and the rightmost wheel positions from
     *         the range a-u. The sequence in which the characters appear in the string new_positions has to match the sequence
     *         of rotors/wheels as seen by the user in the graphical simulator.
     *
     *  Returns true if an error was encountered else false.
     */
    virtual bool move_all_rotors(ustring& new_positions);

    /*! \brief Does the same as move_all_rotors(ustring& new_positions) but new_positions is a zero terminated C-style string.
     *
     *  Returns true if an error was encountered else false. If true is returned the state of the underlying machine has not been changed.
     */
    virtual bool move_all_rotors(const char *new_positions) { ustring help(new_positions); return move_all_rotors(help); }
    
    /*! \brief Destructor.
     */             
    virtual ~schluesselgeraet39() { delete stepper; }
    
protected:
    /*! \brief Saves the SG39 specific state variables of this instance of schluesselgeraet39 in the KeyFile object referenced 
     *         by the ini_file parameter.
     */      
    virtual void save_additional_components(Glib::KeyFile& ini_file);

    /*! \brief Restores the SG39 specific state variables contained in the KeyFile object referenced by the parameter
     *         ini_file into this instance of schluesselgeraet39.
     *
     *  false is returned if no errors have been encountered.
     */      
    virtual bool load_additional_components(Glib::KeyFile& ini_file);

    /*! \brief This method returns true if the two given wheel specs do not overlap too much.
     */          
    virtual bool set_test(string& wheel_spec1, string& wheel_spec2, unsigned int max_overlap);

    /*! \brief This method returns true if the distance between each of two consecutive pins of wheel spec
     *         wheel_spec_candidate + new_pin_pos is bigger than one.
     */          
    virtual bool diff_test(string& wheel_spec_candidate, char new_pin_pos);

    /*! \brief This method ensures that the string to which wheel_spec points contains num_ones '1' characters.
     */          
    virtual void fill_wheel_spec(randomize_help wheel_spec, unsigned int num_ones);
};

#endif /* __sg39_h__ */

