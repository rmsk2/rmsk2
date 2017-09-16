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

#ifndef __enigma_sim_h__
#define __enigma_sim_h__

/*! \file enigma_sim.h
 *  \brief This file contains the header for the classes that implement all the Enigma variants
 *         known to this software.
 */ 

#include<utility>
#include<boost/shared_ptr.hpp>
#include<alphabet.h>
#include<rotor.h>
#include<rotor_machine.h>
#include<transforms.h>
#include<stepping.h>
#include<enigma_rotor_set.h>
#include<enigma_uhr.h>

#define FAST "fast"
#define MIDDLE "middle"
#define SLOW "slow"
#define UMKEHRWALZE "umkehrwalze"
#define GRIECHENWALZE "griechenwalze"


/*! \brief A class that allows to retrieve a rotor_set object that contains the standard rotor set
 *         common to all simulated Enigma variants.
 *
 *  The rotor_set object managed by this class is a singleton.
 */
class enigma_rotor_factory {
public:
    /*! \brief Returns the default rotor_set object.
     *
     *  If this method is called the first time it checks whether enigma_set contains any data.
     *  If not it constructs an object that contains the data contained in enigma_rotor_set.cpp.
     */  
    static rotor_set *get_rotor_set();
    
    /*! \brief Allows to set the default rotor_set object.
     */      
    static void set_rotor_set(rotor_set *r_set) { enigma_set = r_set; }

protected:

    /*! \brief The actual rotor_set object to be used.
     */      
    static rotor_set enigma_set_data;

    /*! \brief Points to the actual rotor_set object to be used.
     */      
    static rotor_set *enigma_set;

};

/*! \brief A class intended as the base class for the stepping_gear objects used by the simulated Enigma variants.
 *
 *  It provides methods that allow to set and get the Ringstellung as well as the rotor positions indicated through
 *  the rotor windows (which in turn are depending on the Ringstellung).
 */
class enigma_stepper_base : public stepping_gear {
public:
    /*! \brief Constructor that hast be called with a vector of strings that contains the symbolic names of the
     *         rotors inserted into the machine.
     */  
    enigma_stepper_base(vector<string>& rotor_identifiers);
    
    /*! \brief Pure virtual method that implements the actual stepping in subclasses inheriting from this class.
     */               
    virtual void step_rotors() = 0;
    
    /*! \brief Resets the Enigma stepping gear to default values.
     */                   
    virtual void reset();

    /*! \brief Sets the Ringstellung of the rotor named by the string identifier to the value given in the parameter new_pos.
     *
     *  Allowed values for new_pos are the characters a-z. The value must not be specified by capital letters.
     */                       
    virtual void set_ringstellung(string& identifier, char new_pos);

    /*! \brief Sets the Ringstellung of the rotor named by the C-string identifier to the value given in the parameter new_pos.
     *
     *  Allowed values for new_pos are the characters a-z. The value must not be specified by capital letters.
     */                       
    virtual void set_ringstellung(const char *identifier, char new_pos) { string temp(identifier); set_ringstellung(temp, new_pos); }

    /*! \brief Returns the Ringstellung of the rotor named by the string identifier.
     */                       
    virtual char get_ringstellung(string& identifier);

    /*! \brief Returns the Ringstellung of the rotor named by the C-string identifier.
     */                       
    virtual char get_ringstellung(const char *identifier) { string temp(identifier); return get_ringstellung(temp); }

    /*! \brief This method changes the rotor displacement of the rotor named by the string identifier in such a way that the
     *         letter indicated by the value of the parameter new_pos appears in the rotor window.
     *
     *  Allowed values for new_pos are the characters a-z. The value must not be specified by capital letters.
     */                        
    virtual void set_rotor_pos(string& identifier, char new_pos);

    /*! \brief This method changes the rotor displacement of the rotor named by the C-string identifier in such a way that the
     *         letter indicated by the value of the parameter new_pos appears in the rotor window.
     *
     *  Allowed values for new_pos are the characters a-z. The value must not be specified by capital letters.
     */                        
    virtual void set_rotor_pos(const char *identifier, char new_pos) { string temp(identifier); set_rotor_pos(temp, new_pos); }


    /*! \brief Returns the current value shown in the rotor window of the rotor named by the string identifier.
     */ 
    virtual char get_rotor_pos(string& identifier);

    /*! \brief Returns the current value shown in the rotor window of the rotor named by the C-string identifier.
     */ 
    virtual char get_rotor_pos(const char *identifier) { string temp(identifier); return get_rotor_pos(temp); }

    /*! \brief Destructor.
     */     
    virtual ~enigma_stepper_base() { ; }
    
};

/*! \brief A class that implements classic Enigma stepping. It is used by all Enigma variants except Abwehr Enigma.
 */
class enigma_stepper : public enigma_stepper_base {
public:
    /*! \brief Constructor that hast be called with a vector of strings that contains the symbolic names of the
     *         rotors inserted into the machine.
     */  
    enigma_stepper(vector<string>& rotor_identifiers) : enigma_stepper_base(rotor_identifiers) { ; }

    /*! \brief Implements Enigma stepping including double stepping of the middle rotor.
     */      
    virtual void step_rotors();

    /*! \brief Destructor.
     */         
    virtual ~enigma_stepper() { ; }
    
};

/*! \brief A class that implements Abwehr Enigma stepping.
 */
class abwehr_stepper : public enigma_stepper_base {
public:
    /*! \brief Constructor that hast be called with a vector of strings that contains the symbolic names of the
     *         rotors inserted into the machine.
     */  
    abwehr_stepper(vector<string>& rotor_identifiers) : enigma_stepper_base(rotor_identifiers) { ; }

    /*! \brief Implements Abwehr-Enigma stepping.
     *
     *  The rotors of the Abwehr-Enigma stepped in a truly cyclometric fashion and therefore there was no double
     *  stepping.
     */          
    virtual void step_rotors();

    /*! \brief Destructor.
     */         
    virtual ~abwehr_stepper() { ; }

};

/*! \brief A class intended as the base class for all Enigma variants including the Typex
 *
 *  It adds a method to the rotor_machine class that allows to retrieve the stepping_gear object as 
 *  an enigma_stepper_base object.
 */
class enigma_family_base : public rotor_machine {
public:
    /*! \brief Default constructor.
     */ 
    enigma_family_base();
    
    /*! \brief Allows to retrieve a pointer to the enigma_stepper_base object in use as the stepping_gear. Mainly
     *         a convenience method that eliminates the necessity for type casts in client code. 
     */     
    virtual enigma_stepper_base *get_enigma_stepper()  { return dynamic_cast<enigma_stepper_base *>(stepper); }
    
    /*! \brief The string new_positions has to specify the new positions of the rotors. Depending on the model it
     *         has to contain three (Services, M3, KD), four (M4, Tirpitz, Abwehr, Railway) or five (Typex) characters
     *         from the range a-z. The sequence in which the characters appear in the string new_positions has to
     *         match the sequence of rotors as seen by the user in the graphical simulator.
     *
     *  Returns true if an error was encountered else false. If true is returned the state of the underlying machine has not been changed.
     */
    virtual bool move_all_rotors(ustring& new_positions);

    /*! \brief Does the same as move_all_rotors(ustring& new_positions) but new_positions is a zero terminated C-style string.
     *
     *  Returns true if an error was encountered else false. If true is returned the state of the underlying machine has not been changed.
     */
    virtual bool move_all_rotors(const char *new_positions) { ustring pos_help(new_positions); return move_all_rotors(pos_help); }
    
    /*! \brief Destructor.
     */         
    virtual ~enigma_family_base() { delete stepper; }    
};

/*! \brief A class intended as the base class for all german Enigma variants.
 *
 *  It adds methods that allow to randomize the machine state and to store additional data along with the machine
 *  state.
 */
class enigma_base : public enigma_family_base {
public:
    /*! \brief Default constructor.
     */ 
    enigma_base() : enigma_family_base() { ; }
        
    /*! \brief This method instruct the rotor machine object to randomize its state. It returns false if the randomization was
     *         successfull. If this method returns true an error occurred and the object's state is unchanged.
     *
     *  Currently the parameter param is ignored.
     */
    virtual bool randomize(string& param);

    /*! \brief This method returns the machine type as used by the machine_config class.
     */    
    virtual string get_machine_type() { return machine_type; }

    /*! \brief Destructor.
     */         
    virtual ~enigma_base() { ; } 
    
protected:

    /*! \brief This method saves the machine_type variable and a value for the UKW D wiring. ini_file has to specify a Glib::KeyFile
     *         object into which this data is written.
     */                      
    virtual void save_additional_components(Glib::KeyFile& ini_file);

    /*! \brief This method test whether the machinetype stored in ini_file matches the value of the machine_type varible. It returns
     *         false if no error occurred and true otherwise.
     */         
    virtual bool load_additional_components(Glib::KeyFile& ini_file);

    /*! \brief Contains the machine type as needed by machine_config. */    
    string machine_type;       
};


/*! \brief A class that implements a simulator for the Abwehr Enigma.
 */
class abwehr_enigma : public enigma_base {
public:
    /*! \brief Constructor. The parameters have to contain the numeric rotor ids of the rotors that are to be
     *         inserted into the machine.
     *
     *  Historically correct values are WALZE_ABW_I, WALZE_ABW_II, WALZE_ABW_III. All other values in enigma_rotor_set.h
     *  are allowed.
     */ 
    abwehr_enigma(unsigned int slow_id, unsigned int middle_id, unsigned int fast_id);

    /*! \brief Destructor.
     */             
    virtual ~abwehr_enigma() { ; }
};

/*! \brief A class that is intended as base class for all the Enigma variants without a plugboard, except Abwehr, i.e. 
 *         Tirpitz, KD, Railway.
 */
class unsteckered_enigma : public enigma_base {
public:
    /*! \brief Constructor. The parameters have to contain the numeric rotor ids of the rotors that are to be
     *         inserted into the machine.
     */ 
    unsteckered_enigma(unsigned int ukw_id, unsigned int slow_id, unsigned int middle_id, unsigned int fast_id, unsigned int etw_id);

    /*! \brief Destructor.
     */    
    virtual ~unsteckered_enigma() { ; }
};

/*! \brief A class that implements a simulator for the Railway Enigma.
 */
class railway_enigma : public unsteckered_enigma {
public:

    /*! \brief Constructor. The parameters have to contain the numeric rotor ids of the rotors that are to be
     *         inserted into the machine.
     *
     *  Historically correct values are WALZE_RB_I, WALZE_RB_II, WALZE_RB_III. All other values in enigma_rotor_set.h
     *  are allowed.     
     */ 
    railway_enigma(unsigned int slow_id, unsigned int middle_id, unsigned int fast_id);

    /*! \brief Destructor.
     */
    virtual ~railway_enigma() { ; }
};

/*! \brief A class that implements a simulator for the Tirpitz Enigma.
 */
class tirpitz_enigma : public unsteckered_enigma {
public:
    /*! \brief Constructor. The parameters have to contain the numeric rotor ids of the rotors that are to be
     *         inserted into the machine.
     *
     *  Historically correct values are WALZE_T_I, WALZE_T_II, ... , WALZE_T_VIII. All other values in enigma_rotor_set.h
     *  are allowed.     
     */ 
    tirpitz_enigma(unsigned int slow_id, unsigned int middle_id, unsigned int fast_id);
    
    /*! \brief Destructor.
     */        
    virtual ~tirpitz_enigma() { ; }
};

/*! \brief A class that implements a simulator for the KD Enigma.
 */
class kd_enigma : public unsteckered_enigma {
public:
    /*! \brief Constructor. The parameters have to contain the numeric rotor ids of the rotors that are to be
     *         inserted into the machine.
     *
     *  Historically correct values are WALZE_KD_I, WALZE_KD_II, ... , WALZE_KD_VI. All other values in enigma_rotor_set.h
     *  are allowed.     
     */ 
    kd_enigma(unsigned int slow_id, unsigned int middle_id, unsigned int fast_id);

    /*! \brief Destructor.
     */        
    virtual ~kd_enigma() { ; }
};

/*! \brief A class that is intended as base class for all the Enigma variants with a plugboard. I.e. 
 *         Services, M3 and M4.
 */
class steckered_enigma : public enigma_base {
public:

    /*! \brief Default constructor.
     */
    steckered_enigma();
    
    /*! \brief This method allows to specify the plugs that have been inserted into the plugboard. For each plug
     *         there has to be a corresponding element in the vector that is referenced through the parameter stecker.
     *         The parameter force_uhr determines whether the plugs are interpreted as an uhr cabling or a normal
     *         stecker setting.
     *  
     *  The following things hold true:
     *
     *  1. force_uhr == False, uses_uhr() == false -> input transform is set to a normal permutation.
     *  2. force_uhr == False, uses_uhr() == True -> input transform is set to a normal permutation. Uhr object discarded.
     *  3. force_uhr == True, uses_uhr() == False -> New Uhr object constructed and configured with cabling.
     *  4. force_uhr == True, uses_uhr() == True -> Existing Uhr object is configured with new cabling.
     */    
    virtual void set_stecker_brett(vector<pair<char, char> >& stecker, bool force_uhr);

    /*! \brief This method allows to retrieve the plugs that have been inserted into the plugboard. For each plug
     *         a corresponding element is placed in the vector that is referenced through the parameter stecker.
     *         
     *  When the machine is configured to use the Enigma Uhr this method returns the Uhr cabling.
     */    
    virtual void get_stecker_brett(vector<pair<char, char> >& stecker);
    
    /*! \brief Returns true if the steckered_enigma is currently configured to make use of the Enigma Uhr.
     */    
    virtual bool uses_uhr();

    /*! \brief Returns a pointer to the enigma_uhr object that is currently in use by this machine. If uses_uhr()
     *         is false then this method returns NULL.
     */    
    virtual enigma_uhr *get_uhr();    

    /*! \brief Destructor.
     */        
    virtual ~steckered_enigma() { ; }
    
protected:
    /*! \brief This method stores the components that are specific to a steckered Enigma in the Glib::KeyFile object
     *         referenced through the parameter ini_file.
     */    
    virtual void save_additional_components(Glib::KeyFile& ini_file);

    /*! \brief This method loads the components that are specific to a steckered Enigma from the Glib::KeyFile object
     *         referenced through the parameter ini_file and configures this unsteckered_enigma to use them.
     */    
    virtual bool load_additional_components(Glib::KeyFile& ini_file);
};


/*! \brief A class that implements a simulator for the Services and M3 Enigma.
 */
class enigma_I : public steckered_enigma {
public:

    /*! \brief Constructor. The parameters have to contain the numeric rotor ids of the rotors that are to be
     *         inserted into the machine.
     *
     *  Historically correct values are WALZE_I, WALZE_II, ... , WALZE_VIII for slow_id, middle_id, fast_id and 
     *  UKW_A, UKW_B and UKW_C for ukw_id. All other values in enigma_rotor_set.h are allowed. The flag type_m3 specifies
     *  whether the machine is of type M3 or Services. This is purely informational but allows to keep historical accuracy
     *  in some situations.    
     */ 
    enigma_I(unsigned int ukw_id, unsigned int slow_id, unsigned int middle_id, unsigned int fast_id, bool type_m3 = false);

    /*! \brief Destructor.
     */        
    virtual ~enigma_I() { ; }
};

/*! \brief A class that implements a simulator for the M4 Enigma.
 */
class enigma_M4 : public steckered_enigma {
public:

    /*! \brief Constructor. The parameters have to contain the numeric rotor ids of the rotors that are to be
     *         inserted into the machine.
     *
     *  Historically correct values are WALZE_I, WALZE_II, ... , WALZE_VIII for slow_id, middle_id, fast_id; 
     *  UKW_B_DN, UKW_C_DN for ukw_id and WALZE_BETA, WALZE_GAMMA for griechen_id. All other values in 
     *  enigma_rotor_set.h are allowed.
     */ 
    enigma_M4(unsigned int ukw_id, unsigned int griechen_id, unsigned int slow_id, unsigned int middle_id, unsigned int fast_id);

    /*! \brief Destructor.
     */    
    virtual ~enigma_M4() { ; }
};


#endif /* __enigma_sim_h__ */

