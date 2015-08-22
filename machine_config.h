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

#ifndef __machine_config_h__
#define __machine_config_h__

/*! \file machine_config.h
 *  \brief Header file for a class that describes the characteristics of the Enigma variants simulated by this software.
 */ 

#include<utility>
#include<set>
#include<map>
#include<vector>
#include<permutation.h>
#include<gtkmm.h>
#include<enigma_sim.h>

using namespace std;

/*! \brief A structure which describes the characteristics of the rotors that can be inserted in a specific slot in the rotor bank
 *         of an Enigma machine.
 */
struct rotor_family_descriptor {
    /*! \brief Name of the rotor position or slot to which descriptor relates.
     */
    string wheel_identifier;
    
    /*! \brief A map that assigns a numeric rotor identifier to a human readable desription for each of the rotors that can be used
     *         in the slot named by wheel_identifier. The numeric rotor identifiers are defined in enigma_rotor_set.h. 
     */    
    map<Glib::ustring, unsigned int> available_rotors;
    
    /*! \brief A vector that contains all the keys from the map available_rotors.
     */        
    vector<string> rotor_names;
    
    /*! \brief Holds the ring setting of the rotor currently inserted in the slot identified by wheel_identifier.
     *
     *  Must be from the range A-Z in capital letters.
     */        
    char ring_setting;

    /*! \brief Holds the rotor displacement of the rotor currently inserted in the slot identified by wheel_identifier. The displacement
     *         is determined by the letter shown in the rotor window of the slot, where rotor_pos contains that letter.
     *
     *  Must be from the range A-Z in capital letters.
     */        
    char rotor_pos;

    /*! \brief Determines which rotor is currently used in the slot named by wheel_identifier by specifying the index/position of
     *         the name of the selected rotor in the vector rotor_names. 
     */        
    int rotor_index_active;

    /*! \brief Is true if the slot named by wheel_identfier is actually in use in a specific Enigma variant.
     */        
    bool rotor_selection_state;
    
    /*! \brief Is true if the rotors that can be inserted in the slot named by wheel_identfier have a settable ring.
     */            
    bool ring_selection_state;

    /*! \brief Is true if the slot named by wheel_identfier has a rotor window.
     */            
    bool has_rotor_window;    
};

/*! \brief A class that is intended to capture the complete state of a simulated Enigma machine. It also fills
 *         the gap between the purely numeric state information of the underlying rotor_machine object and
 *         its graphical representation in the simulator's GUI.  
 */
class machine_config {
public:
    /*! \brief Default constructor. Recreates the state of an M4 Enigma machine.     
     */
    machine_config();

    /*! \brief Returns a reference to a vector that contains all the rotor_family_descriptor objects which
               are required to describe the specific Enigma variant that is currently simulated.     
     */
    virtual vector<rotor_family_descriptor>& get_all_descriptors() { return all_descriptors; }

    /*! \brief Returns the rotor_family_descriptor that describes the rotor on position pos. The fastet moving rotor
     *         is always on position 0, the UKW always on position 4. The greek wheel is on position 3 but is only active in
     *         the M4. The postions 1-2 are occupied by the middle (1) and slow (2) rotors.
     */
    virtual rotor_family_descriptor& get_desc_at(unsigned int pos) {return all_descriptors[pos]; }

    /*! \brief Returns true if the simulated Enigma variant had a plugboard and false otherwise. 
     */
    virtual bool& get_has_plugboard() { return has_plugboard; }

    /*! \brief Returns true if the simulated Enigma variant is currently configured to use the Schreibmax and false otherwise. 
     */
    virtual bool& get_uses_schreibmax() { return uses_schreibmax; }

    /*! \brief Returns true if the simulated Enigma variant is currently configured to use the Uhr and false otherwise. 
     */
    virtual bool& get_uses_uhr() { return uses_uhr; }

    /*! \brief Returns true if the simulated Enigma variant is able to use the Uhr and false otherwise. 
     */
    virtual bool get_uhr_capable() { return uhr_capable; }

    /*! \brief Returns a reference to the instance variable that holds the current dial pos of the Uhr.
     */
    virtual unsigned int& get_uhr_dial_pos() { return uhr_dial_pos; }

    /*! \brief Returns true if the rotors of the simulated Enigma have numeric values on the rings. This is true for
     *         the Services Enigma only. 
     */
    virtual bool& get_wheels_are_numeric() { return wheels_are_numeric; }

    /*! \brief Returns a reference to an instance variable that holds the current plugs inserted into the plugboard. 
     */
    virtual vector<pair<char, char> >& get_inserted_plugs() { return inserted_plugs; }

    /*! \brief Returns a reference to an instance variable that holds the current UKW D permutation. 
     */
    virtual permutation& get_ukw_d_perm() {return ukwd_perm; }

    /*! \brief Returns the name of the Enigma variant that is currently simulated. 
     */
    virtual Glib::ustring& get_machine_type() { return machine_type; }

    /*! \brief Randomizes the current configuration, where the machine type is left unchanged.
     *
     *  false is returned if no errors occurred during randomization. 
     */
    virtual bool randomize();

    /*! \brief Creates and returns an enigma_base object of the machine type given in the parameter subtype. 
     *
     *  subtype can assume the following values: M3, M4, Services, Abwehr, Tirpitz, Railway, KD. The object returned
     *  is *not configured* according to values set in this machine_config object.        
     */
    virtual enigma_base *make_machine(Glib::ustring& subtype);
    
    /*! \brief Loads a configuration from an ini file. In case no error was encountered, false is returned.
     */       
    virtual bool load_settings(const Glib::ustring& file_name);

    /*! \brief Saves the state of the machine given in parameter enigma to an ini file. In case no error was 
     *         encountered, false is returned.
     */       
    virtual bool save_settings(const Glib::ustring& file_name, enigma_base *enigma);    

    /*! \brief Destructor. 
     */    
    virtual ~machine_config() { ; }

    /*! \brief Modifies the state of this object so that it contains a default configuration for the type of
     *         machine specified via the machine_name parameter.
     *
     *  machine_name can assume the following values: M3, M4, Services, Abwehr, Tirpitz, Railway, KD. 
     */        
    virtual void make_config(Glib::ustring& machine_name);
    
protected:

    /*! \brief Helper method that extracts the keys of the map source into the string vector extracted_names. 
     */ 
    void extract_rotor_names(map<Glib::ustring, unsigned int>& source, vector<string>& extracted_names);

    /*! \brief Returns a random 5 character strings consisting of values from the range A-Z. 
     */       
    string get_random_quintuple();

    /*! \brief Modifies the state of this object so that it contains a default configuration for an M4 enigma
     */        
    void make_m4_config();    

    /*! \brief Returns a numeric vector that has five elements. The first three contain random values in the range 0..num_rotors - 1
     *         but no value is repeated. The 4th contains a one or a zero, the fifth a value in the range of 0...num_ukws - 1.
     */      
    vector<unsigned int> make_random_wheel_order(unsigned int num_ukws, unsigned int num_rotors, unsigned int num_greeks);

    /*! \brief Returns a random set of plugs for the Enigma plugboard. The parameter num_plugs specifies how many plugs are
     *         to be inserted. The machine config is not changed by this method.
     */ 
    void make_random_involution(vector<pair<char, char> >& plugs, unsigned int num_plugs = 10);

    /*! \brief Returns a random involution that can be used as an UKW D wiring. I.e. the contacts 1 and 14 are always mapped
     *         to each other.  The machine config is not changed by this method.
     */ 
    void make_random_ukw_d_wiring();

    /*! \brief This method can be used to map the rotor id from enigma_rotor_set.h as specified in search_val to the index
     *         of the corresponding rotor having that id in the rotor_family_descriptor given in parameter to_search.
     *
     *  false is returned if the rotor id was not found in the available_rotors mapping and true otherwise. If true is returned
     *  the index_found parameter is set to the desired value. 
     */        
    bool find_rotor_index(rotor_family_descriptor& to_search, unsigned int search_val, unsigned int& index_found);    

    /*! \brief Holds all the rotor_family_descriptor objects which are needed to describe the possible settings of the simulated
     *         Enigma variant.
     */      
    vector<rotor_family_descriptor> all_descriptors;

    /*! \brief Holds the plugs currently in use on the plugboard of the simulated machine.
     */      
    vector<pair<char, char> > inserted_plugs;

    /*! \brief Instance variables that hold the state information returned by get_uhr_capable, get_uses_uhr, get_has_plugboard ....
     */
    bool has_plugboard;

    /*! \brief Instance variables that hold the state information returned by get_uhr_capable, get_uses_uhr, get_has_plugboard ....
     */
    bool wheels_are_numeric;

    /*! \brief Instance variables that hold the state information returned by get_uhr_capable, get_uses_uhr, get_has_plugboard ....
     */
    bool uses_uhr;

    /*! \brief Instance variables that hold the state information returned by get_uhr_capable, get_uses_uhr, get_has_plugboard ....
     */
    bool uhr_capable;

    /*! \brief Instance variables that hold the state information returned by get_uhr_capable, get_uses_uhr, get_has_plugboard ....
     */
    bool uses_schreibmax;

    /*! \brief The default rotor_family_descriptors used by make_m4_config()
     */
    rotor_family_descriptor ukw;

    /*! \brief The default rotor_family_descriptors used by make_m4_config()
     */
    rotor_family_descriptor greek;

    /*! \brief The default rotor_family_descriptors used by make_m4_config()
     */
    rotor_family_descriptor slow;

    /*! \brief The default rotor_family_descriptors used by make_m4_config()
     */
    rotor_family_descriptor middle;

    /*! \brief The default rotor_family_descriptors used by make_m4_config()
     */
    rotor_family_descriptor fast;

     /*! \brief Holds the current dial position of the Enigma Uhr.
     */    
    unsigned int uhr_dial_pos;

     /*! \brief Holds the current UKW D permutation.
     */    
    permutation ukwd_perm;

     /*! \brief Holds the current machine type. A reference to this value is returned by get_machine_type()
     */    
    Glib::ustring machine_type;
};

#endif /* __machine_config_h__ */

