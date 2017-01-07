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

#ifndef __configurator_h__
#define __configurator_h__

/*! \file configurator.h
 *  \brief Header file for ::configurator and its subclasses.
 */ 

#include<string>
#include<map>
#include<vector>
#include<rotor_machine.h>

// Keywords for SIGABA
#define KW_CIPHER_ROTORS "cipher"
#define KW_CONTROL_ROTORS "control"
#define KW_INDEX_ROTORS "index"
#define KW_CSP_2900_FLAG "csp2900"

// Keywords for Schluesselgeraet 39
#define KW_SG39_ROTOR_SET "rotorset"
#define KW_SG39_ROTORS "rotors"
#define KW_SG39_RING_POS "rings"
#define KW_SG39_REFLECTOR_PLUGS "reflector"
#define KW_SG39_ENTRY_PLUGS "plugs"
#define KW_SG39_PINS_WHEEL_1 "pinswheel1"
#define KW_SG39_PINS_WHEEL_2 "pinswheel2"
#define KW_SG39_PINS_WHEEL_3 "pinswheel3"
#define KW_SG39_PINS_ROTOR_1 "pinsrotor1"
#define KW_SG39_PINS_ROTOR_2 "pinsrotor2"
#define KW_SG39_PINS_ROTOR_3 "pinsrotor3"

// Keywords for Typex
#define KW_TYPEX_ROTORS "rotors"
#define KW_TYPEX_RINGS "rings"
#define KW_TYPEX_REFLECTOR "reflector"
#define KW_TYPEX_ROTOR_SET "rotorset"

// Keywords for KL7
#define KW_KL7_ROTORS "rotors"
#define KW_KL7_ALPHA_POS "alpharings"
#define KW_KL7_NOTCH_RINGS "notchselect"
#define KW_KL7_NOTCH_POS "notchrings"

// Keywords for Nema
#define KW_NEMA_ROTORS "rotors"
#define KW_NEMA_RINGS "ringselect"
#define KW_NEMA_WAR_MACHINE "warmachine"

using namespace std;

/*! \brief Nema training machine can be used with rotors a-d.  
 */
const char NEMA_UPPER_LIMIT_TRAINING = 'd';

/*! \brief Nema war machine can be used with rotors a-f.  
 */
const char NEMA_UPPER_LIMIT_WAR = 'f';

/*! \brief Used as return value that signifies that no errors occurred.
 */
const unsigned int CONFIGURATOR_OK = 0;

/*! \brief Used as return value that signifies that the configuration is syntactically correct
 *         but sematically inconsistent. A rotor sepcification which repeats a rotor would generate
 *         this repsonse when verified.
 */
const unsigned int CONFIGURATOR_INCONSISTENT = 1;

/*! \brief Used as return value that signifies that the configuration is syntactically inccorrect
 *         or is unausable due to other unspecified reasons.
 */
const unsigned int CONFIGURATOR_ERROR = 2;

/*! \brief An abstract class that embodies the concept of a thing that knows how to create and configure
 *         a rotor machine of a certain type. For each machine that is simulated by rotorvis a configurator
 *         subclass has to be implemented.
 * 
 *  The rotor machine's configuration parameters are described by a vector of key_word_info 
 *  structures. Creating or configuring the machine is done by using the rotor_set named by the
 *  instance variable rotor_set_name.
 */
class configurator {
public:    
    /*! \brief Default constructor. Sets rotor_set_name to DEFAULT_SET, which is known in all simulators.
     */    
    configurator() { rotor_set_name = DEFAULT_SET; }

    /*! \brief Returns a vector that contains the key_word_info elements that desribe the configuration
     *         of a specific type of rotor machine. Has to be reimplemented in children of this class.
     */        
    virtual void get_keywords(vector<key_word_info>& infos) = 0;

    /*! \brief Returns the configuration of the machine pointed to by the parameter configured_machine
     *         in the vector referenced by the parameter config_data. Has to be reimplemented in children 
     *         of this class.
     */        
    virtual void get_config(map<string, string>& config_data, rotor_machine *configured_machine) = 0;

    /*! \brief Configures the machine pointed to by the parameter machine_to_configure with the data referenced
     *         by the parameter config_data. Has to be reimplemented in children of this class.
     *
     *  Returns CONFIGURATOR_OK in case of success.
     */        
    virtual unsigned int configure_machine(map<string, string>& config_data, rotor_machine *machine_to_configure) = 0;

    /*! \brief Creates a new machine and configures it with the data referenced by the parameter config_data.
     *         Has to be reimplemented in children of this class.
     */        
    virtual rotor_machine *make_machine(map<string, string>& config_data) = 0;

    /*! \brief Returns the rotor set name that is currently used by this instance.
     */    
    virtual string get_rotor_set_name() { return rotor_set_name; }

    /*! \brief Allows to set the rotor set name that is currently used by this instance.
     */    
    virtual void set_rotor_set_name(string& new_value) { rotor_set_name = new_value; }

    /*! \brief Default constructor.
     */        
    virtual ~configurator() { ; }
    
    /*! \brief This method can be used to retrieve the permutation currently implemented by the encryption_transform
     *         pointed to by parameter t. It is mainly intended to retrieve the current plugboard permutation of a 
     *         rotor_machine. The result is a string that represents this permutation in the usual way.
     *
     *  Assumes that there are exactly 26 input characters.
     */            
    static string get_entry_plugboard(encryption_transform *t);

    /*! \brief This method can be used to retrieve the permutation currently implemented by the encryption_transform
     *         pointed to by parameter t. It is mainly intended to retrieve the current reflector permutation of a 
     *         rotor_machine. Reflectors always specify an involution. The string returned by this method is to be
     *         interpreted as the sequence of two element cycles that make up this involution.
     *
     *  Assumes that there are exactly 26 input characters.
     */            
    static string get_reflector(encryption_transform *t);

    /*! \brief Returns a string consisting of the characters '0' and '1' that depends on the contents of the vector vec.
     *         Each 0 in the vector referenced by parameter vec creates a '0' in the output string, each nonzero value
     *         produces a '1' in the output.
     */            
    static string vec_to_bool(vector<unsigned int>& vec);

    /*! \brief Returns a string consisting of the characters a-z, where a letter is included iff the position that 
     *         corresponds to this letter in vec is a one. Only up to the first 26 elements of vec are relevant.
     */            
    static string bool_to_string(vector<unsigned int>& vec);

    /*! \brief Sets all the positions in vec to 1 which correspond to a letter in pin_spec. If a letter in pin_spec
     *         is not between a-z and/or the position of the letter in the standard alphabet is bigger than vec.size() it
     *         is ignored.
     */                
    static void string_to_bool(vector<unsigned int>& vec, string& pin_spec);

    /*! \brief Returns true if the string referenced by parameter to_test is of the length specified in parameter
     *         desired_length. When true is returned the vector referenced by parameter data is filled with 0s and 1s
     *         where each '0' character in to_test prodcues a 0 and all other characters produce a 1.
     */        
    static bool check_bool(string& to_test, unsigned int desired_length, vector<unsigned int>& data); 

    /*! \brief Returns true if the string referenced by parameter to_test has length 26 and contains 26 different
     *         characters.
     */        
    static bool check_for_perm(string& to_test);  

    /*! \brief Returns true if the string referenced by parameter data contains only characters between '0' and '9' 
     *         or ' ' characters and if it contains at least one character from the range '0'-'9'. If true is returned
     *         the vector referenced by parameter parse_result is filled with the numbers that result from converting the
     *         numeric strings separated by the ' ' characters into numbers.
     */        
    static bool parse_numeric_vector(string& data, vector<unsigned int>& parse_result);

    /*! \brief Returns true if the vector referenced by parameter to_test has length desired_length and contains only
     *         numbers between range_start and range_end. If the parameter require_unique is true it is aditionally checked 
     *         if each number appears at most once.
     */        
    static bool check_vector(vector<unsigned int>& to_test, unsigned int range_start, unsigned int range_end, unsigned int desired_length, bool require_unique = false);    

    /*! \brief Returns true if the vector referenced by parameter to_test has length desired_length and contains only
     *         numbers from the set referenced by parameter ref_values. If the parameter require_unique is true it is
     *         aditionally checked  if each number appears at most once.
     */        
    static bool check_vector(vector<unsigned int>& to_test, set<unsigned int>& ref_values, unsigned int desired_length, bool require_unique = false);

    /*! \brief Returns true if the string referenced by parameter rotor_spec has length desired_length and contains only
     *         characters between start_char and end_char. If the parameter require_unique is true it is aditionally checked 
     *         if each character appears at most once.
     */        
    static bool check_rotor_spec(string& rotor_spec, char start_char, char end_char, unsigned int desired_length, bool require_unique = true);

    /*! \brief Returns true if the string referenced by parameter pin_spec has length at most max_length and contains only
     *         characters between start_char and end_char. Each character must appears at most once.
     */        
    static bool check_pin_spec(string& pin_spec, char start_char, char end_char, unsigned int max_length);    

protected:
    /*! \brief Holds the name of the rotor_set name currently in use in this object.
     */        
    string rotor_set_name;  

    /*! \brief Pure virtual method that has to be reimplemented in subclasses. This method is intended to parse and
     *         verify the configuration referenced by the parameter config_data.
     */            
    virtual unsigned int parse_config(map<string, string>& config_data) = 0;

    /*! \brief Returns true if the configuration data referenced through parameter config_data contains an entry for
     *         each of the keywords specified by the get_keywords() method.
     */            
    virtual bool check_for_completeness(map<string, string>& config_data);


};

/*! \brief A class that knows how to create ::configurator objects for the machines simulated by rotorvis.
 */
class configurator_factory {
public:
    /*! \brief Returns a ::configurator object for the machine type specified by parameter machine_name. NULL is returned
     *         if the machine type is unknown.
     *
     *  Currently the following machine names are valid: SIGABA, SG39, Typex, KL7 and Nema.
     */   
    static configurator *get_configurator(string& machine_name);

    /*! \brief Returns a ::configurator object for the machine type specified by the zero terminated string machine_name.
     *         NULL is returned if the machine type is unknown.
     */   
    static configurator *get_configurator(const char *machine_name) { string help(machine_name); return get_configurator(help); }
};

/*! \brief A class that knows how to create and configure ::sigaba objects.
 */
class sigaba_configurator : public configurator {
public:

    /*! \brief Default constructor.
     */    
    sigaba_configurator() { csp_2900_flag = false; }

    /*! \brief Returns the key_word_info structures that pertain to the SIGABA simulator.
     */    
    virtual void get_keywords(vector<key_word_info>& infos);

    /*! \brief Reads config from ::sigaba object to which the parameter configured_machine points and stores it in
     *         the map referenced by config_data. If configured_machine is of wrong type, i.e. does not point to a ::sigaba
     *         object, an exception is thrown.
     */    
    virtual void get_config(map<string, string>& config_data, rotor_machine *configured_machine);

    /*! \brief Configures the ::sigaba pointed to by the parameter machine_to_configure with the data referenced
     *         by the parameter config_data.
     *
     *  Returns CONFIGURATOR_OK if no errors occurred.
     */        
    virtual unsigned int configure_machine(map<string, string>& config_data, rotor_machine *machine_to_configure);

    /*! \brief Creates a new ::sigaba object and configures it with the data referenced by the parameter config_data.
     *
     *  Returns NULL if errors occurred.
     */        
    virtual rotor_machine *make_machine(map<string, string>& config_data);

    /*! \brief Destructor.
     */        
    virtual ~sigaba_configurator() { ; }

protected:

    /*! \brief Returns CONFIGURATOR_OK if parsing and verifying the configuration referenced through config_data
     *         succeded.  
     *
     *  If CONFIGURATOR_OK is returned this method fills parsed_config and sets csp_2900_flag as a side effect.
     */        
    virtual unsigned int parse_config(map<string, string>& config_data);

    /*! \brief Parses and verifies the configuration string specified by parameter rotor_spec and stores the result
     *         in the vector referenced by parameter parsed_ids. If index_rotors is true it is assumed that rotor_sepc
     *         spcifies configuration information for the index rotors. Else rotor_spec has to contain config information
     *         for driver or cipher rotors.
     *
     *  rotor_spec has to contain ten characters. Two for each rotor in the rotor bank. The first character has to specify
     *  the rotor number ('0'-'9' cipher/driver rotors, '0'-'4' index rotors) and the second signifies whether the rotor
     *  should be inserted in reverse ('R') or not ('N').
     *
     *  Caveat: This method assumes that the unsigned int constants SIGABA_ROTOR_0, ..., SIGABA_ROTOR_9 from the file sigaba.h
     *  have the values 0-9 and the constants SIGABA_INDEX_0, ..., SIGABA_INDEX_4 the values 0-4.
     */        
    unsigned int parse_rotor_bank(string& rotor_spec, vector<rotor_id>& parsed_ids, bool index_rotors = false);

    /*! \brief Returns a string representation of the rotor_id object referenced by parameter id. The representation
     *         is a two character string. The first character is the rotor number ('0'-'9') and the second is 'R' if
     *         the rotor has been inserted in reverse and 'N' otherwise.
     *
     *  Caveat: This method assumes that the unsigned int constants SIGABA_ROTOR_0, ..., SIGABA_ROTOR_9 from the file sigaba.h
     *  have the values 0-9 and the constants SIGABA_INDEX_0, ..., SIGABA_INDEX_4 the values 0-4.
     */        
    string transform_rotor_info(rotor_id& id);

    /*! \brief Holds the rotor_id objects needed to create a ::sigaba instance.
     */        
    vector<rotor_id> parsed_config;

    /*! \brief Determines if the ::sigaba object to be creatd is configured to be a CSP 889 or CSP 2900.
     */        
    bool csp_2900_flag;
};

/*! \brief A class that knows how to create and configure ::schluesselgeraet39 objects.
 */
class sg39_configurator : public configurator {
public:
    /*! \brief Default constructor.
     */    
    sg39_configurator() { ; }

    /*! \brief Returns the key_word_info structures that pertain to the Schluesselgeraet 39 simulator.
     */     
    virtual void get_keywords(vector<key_word_info>& infos);
    
    /*! \brief Reads config from the ::schluesselgeraet39 object to which the parameter configured_machine points and stores it in
     *         the map referenced by config_data. If configured_machine is of wrong type, i.e. does not point to a ::schluesselgeraet39
     *         object, an exception is thrown.
     */       
    virtual void get_config(map<string, string>& config_data, rotor_machine *configured_machine);
    
    /*! \brief Configures the ::schluesselgeraet39 pointed to by the parameter machine_to_configure with the data referenced
     *         by the parameter config_data.
     *
     *  Returns CONFIGURATOR_OK if no errors occurred.
     */            
    virtual unsigned int configure_machine(map<string, string>& config_data, rotor_machine *machine_to_configure);

    /*! \brief Creates a new ::schluesselgeraet39 object and configures it with the data referenced by the parameter config_data.
     *
     *  Returns NULL if errors occurred.
     */        
    virtual rotor_machine *make_machine(map<string, string>& config_data);

    /*! \brief Destructor.
     */             
    virtual ~sg39_configurator() { ; }
    
protected:
    /*! \brief Holds pin data for wheel 1.
     */   
    vector<unsigned int> wheel_1_pins;

    /*! \brief Holds pin data wheel 2.
     */   
    vector<unsigned int> wheel_2_pins;
    
    /*! \brief Holds pin data wheel 3.
     */   
    vector<unsigned int> wheel_3_pins;

    /*! \brief Holds pin data rotor 1.
     */       
    vector<unsigned int> rotor_1_pins;

    /*! \brief Holds pin data rotor 2.
     */       
    vector<unsigned int> rotor_2_pins;

    /*! \brief Holds pin data rotor 3.
     */       
    vector<unsigned int> rotor_3_pins;

    /*! \brief Holds the rotor_id objects that describe the rotors which are to be inserted into the machine.
     */ 
    vector<rotor_id> rotors;

    /*! \brief Holds the ring position for each of the wired rotors.
     */ 
    vector<unsigned int> ring_positions;

    /*! \brief Holds the cycles that make up the permutation of the pluggable reflector of the Schluesselgeraet 39.
     */     
    vector<pair<char, char> > reflector;

    /*! \brief Holds the permutation that is to be used as the plugboard permutation of the Schluesselgeraet 39.
     */     
    vector<unsigned int> entry_perm;

    /*! \brief Returns CONFIGURATOR_OK if parsing and verifying the configuration referenced through config_data
     *         succeded.  
     *
     *  If CONFIGURATOR_OK is returned this method fills the variables wheel_1_pins, wheel_2_pins, wheel_3_pins, rotor_1_pins
     *  rotor_2_pins, rotor_3_pins, rotors, reflector and entry_perm as a side effect.
     */         
    virtual unsigned int parse_config(map<string, string>& config_data); 
};

/*! \brief A class that knows how to create and configure ::typex objects.
 */
class typex_configurator : public configurator {
public:
    /*! \brief Default constructor.
     */   
    typex_configurator();

    /*! \brief Returns the key_word_info structures that pertain to the Typex simulator.
     */     
    virtual void get_keywords(vector<key_word_info>& infos);
    
    /*! \brief Reads config from the ::typex object to which the parameter configured_machine points and stores it in
     *         the map referenced by config_data. If configured_machine is of wrong type, i.e. does not point to a ::typex
     *         object, an exception is thrown.
     */     
    virtual void get_config(map<string, string>& config_data, rotor_machine *configured_machine);
    
    /*! \brief Configures the ::typex pointed to by the parameter machine_to_configure with the data referenced
     *         by the parameter config_data.
     *
     *  Returns CONFIGURATOR_OK if no errors occurred.
     */        
    virtual unsigned int configure_machine(map<string, string>& config_data, rotor_machine *machine_to_configure);
    
    /*! \brief Creates a new ::typex object and configures it with the data referenced by the parameter config_data.
     *
     *  Returns NULL if errors occurred.
     */            
    virtual rotor_machine *make_machine(map<string, string>& config_data);

    /*! \brief Destructor.
     */                 
    virtual ~typex_configurator() { ; }
    
protected:
    /*! \brief Holds the ringstellung that is to be used with this machine.
     */         
    string ringstellung;

    /*! \brief Holds the rotor id for the UKW to use in the context of the current rotor set
     */      
    unsigned int typex_ukw_id;

    /*! \brief Holds the rotor id f the "a" rotor to use in the context of the current rotor set
     */      
    unsigned int typex_base_id;

    /*! \brief Holds the identifier (a-z) of the last rotor in the current set
     */          
    char typex_max_rotor;
    
    /*! \brief Holds the rotor_id objects that describe the rotors which are to be inserted into the machine.
     */     
    vector<rotor_id> rotors;

    /*! \brief Holds the cycles that make up the permutation of the pluggable reflector of the Typex.
     */       
    vector<pair<char, char> > reflector;

    /*! \brief Returns a string representation of the rotor_id object referenced by parameter id. The representation
     *         is a two character string. The first character is the rotor name ('a'-'g') and the second is 'R' if
     *         the rotor has been inserted in reverse and 'N' otherwise.
     */            
    string transform_typex_rotor_info(rotor_id& id);
    
    /*! \brief Returns CONFIGURATOR_OK if parsing and verifying the configuration referenced through config_data
     *         succeded.  
     *
     *  If CONFIGURATOR_OK is returned this method fills the instance variables ringstellung, rotors and reflector
     *  as a side effect.
     */           
    virtual unsigned int parse_config(map<string, string>& config_data);

    /*! \brief This method sets the members typex_ukw_id, typex_base_id and typex_max_rotor accroding on the rotor 
     *         set name.
     */    
    void change_rotor_set(string& rotor_set_name);
};

/*! \brief A class that knows how to create and configure ::kl7 objects.
 */
class kl7_configurator : public configurator {
public:
    /*! \brief Default constructor.
     */     
    kl7_configurator() { ; }

    /*! \brief Returns the key_word_info structures that pertain to the KL7 simulator.
     */   
    virtual void get_keywords(vector<key_word_info>& infos);
    
    /*! \brief Reads config from the ::kl7 object to which the parameter configured_machine points and stores it in
     *         the map referenced by config_data. If configured_machine is of wrong type, i.e. does not point to a ::kl7
     *         object, an exception is thrown.
     */       
    virtual void get_config(map<string, string>& config_data, rotor_machine *configured_machine);
    
    /*! \brief Configures the ::kl7 pointed to by the parameter machine_to_configure with the data referenced
     *         by the parameter config_data.
     *
     *  Returns CONFIGURATOR_OK if no errors occurred.
     */            
    virtual unsigned int configure_machine(map<string, string>& config_data, rotor_machine *machine_to_configure);
    
    /*! \brief Creates a new ::kl7 object and configures it with the data referenced by the parameter config_data.
     *
     *  Returns NULL if errors occurred.
     */                
    virtual rotor_machine *make_machine(map<string, string>& config_data);

    /*! \brief The notch ring position is specified through the the letters 'a'-'z' and '+'. Because KL7 rotors have
     *         36 contacts some positions are named by a single letter between 'a' and 'z' and 10 others are named
     *         by a letter followed by a '+' sign. This method transforms the position specified in parameter pos
     *         into such a string representation.
     */             
    static string transform_notch_ring_pos(unsigned int pos);

    /*! \brief Destructor.
     */         
    virtual ~kl7_configurator() { ; }

protected:    
    /*! \brief Describes the rotors that are to be inserted into the KL7. The rotor_id at position 0 holds
     *         information about the rotor in slot KL7_ROT_1, the one on position 1 refers to KL7_ROT_2 and
     *         so on up to position 7 and KL7_ROT_8.
     */         
    vector<rotor_id> rotors;
    
    /*! \brief Holds a sting of exactly 8 letters, where each of the letters is between 'a' and 'm' and no letter
     *         is repeated. This string represents the rotors that are to be inserted into the KL7. 'a' stands 
     *         for KL7_ROTOR_A, 'b' for KL7_ROTOR_B and so on.
     */             
    string rotor_names;

    /*! \brief Holds the letter ring offsets of the rotors inserted into the machine. 
     *
     *  Numbering is the same as in the instance variable rotors. The stationary rotor KL7_ROT_4 also has a 
     *  letter ring, therefore this vector holds 8 offsets.
     */         
    vector<unsigned int> letter_ring_offsets;

    /*! \brief Holds the notch ring offsets of the rotors inserted into the machine. 
     *
     *  Numbering is the same as in the instance variable rotors but the stationary rotor KL7_ROT_4 does not have
     *  a notch ring and therefore this vector only holds 7 offsets.
     */         
    vector<unsigned int> notch_ring_offsets;

    /*! \brief Holds the numeric id of the notch rings that are attached to the rotors inserted into the machine. 
     *  
     *  Numbering is the same as in the instance variable rotors but the stationary rotor KL7_ROT_4 does not have 
     *  a notch ring and therefore this vector only holds 7 ids.
     */         
    vector<unsigned int> notch_rings;
    
    /*! \brief Returns CONFIGURATOR_OK if parsing and verifying the configuration referenced through config_data
     *         succeded.  
     *
     *  If CONFIGURATOR_OK is returned this method fills the instance variables letter_ring_offsets, notch_rings,
     *  rotor_names, notch_ring_offsets and rotors as a side effect.
     */               
    virtual unsigned int parse_config(map<string, string>& config_data);
};

/*! \brief A class that knows how to create and configure ::nema objects.
 */
class nema_configurator : public configurator {
public:
    /*! \brief Default constructor.
     */     
    nema_configurator();

    /*! \brief Returns the key_word_info structures that pertain to the Nema simulator.
     */   
    virtual void get_keywords(vector<key_word_info>& infos);

    /*! \brief Reads config from the ::nema object to which the parameter configured_machine points and stores it in
     *         the map referenced by config_data. If configured_machine is of wrong type, i.e. does not point to a ::nema
     *         object, an exception is thrown.
     */           
    virtual void get_config(map<string, string>& config_data, rotor_machine *configured_machine);

    /*! \brief Configures the ::nema pointed to by the parameter machine_to_configure with the data referenced
     *         by the parameter config_data.
     *
     *  Returns CONFIGURATOR_OK if no errors occurred.
     */
    virtual unsigned int configure_machine(map<string, string>& config_data, rotor_machine *machine_to_configure);
    
    /*! \brief Creates a new ::nema object and configures it with the data referenced by the parameter config_data.
     *
     *  Returns NULL if errors occurred.
     */    
    virtual rotor_machine *make_machine(map<string, string>& config_data);

    /*! \brief Destructor.
     */        
    virtual ~nema_configurator() { ; }

protected:
    /*! \brief This instance variable holds the four rotor_assembly objects that are to be inserted into the Nema. 
     *
     *  The 0th element holds information about DRIVE_9 and CONTACT_8, the first specifies the information for DRIVE_7
     *  and CONTACT_6, the 2nd for DRIVE_5 and CONTACT_4, the 3rd for DRIVE_3 and CONTACT_2.
     */ 
    vector<rotor_assembly> rotor_settings;
    
    /*! \brief Specifies whether the Nema to be configured is of the war (war_machine = true) or training (war_machine = false)
     *         type.
     */     
    bool war_machine;

    /*! \brief A set that contains the ids of the notch rings that can be used with the war machine.
     */        
    set<unsigned int> notch_ring_war;

    /*! \brief A set that contains the ids of the notch rings that can be used with the training machine.
     */        
    set<unsigned int> notch_ring_training;
    
    /*! \brief Contains information about the four rings that are part of the rotor assemblies which are to be inserted
     *         into the Nema.
     *
     *  Numbering is the same as in instance variable rotor_settings.
     */            
    vector<unsigned int> ring_ids;

    /*! \brief Contains information about the four rotors that are part of the rotor assemblies which are to be inserted
     *         into the Nema. 
     *
     *  Each character of rotor_names has to have a value between 'a' - 'f' (war machine) or 'a' - 'd' (training machine).
     *  Numbering is the same as in instance variable rotor_settings.
     */            
    string rotor_names;
    
    /*! \brief Specifies the id of the ring used on the left side of the red wheel.
     */            
    unsigned int left_red_drive_wheel;

    /*! \brief Specifies the id of the ring used on the right side of the red wheel.
     */            
    unsigned int right_red_drive_wheel;

    /*! \brief Returns CONFIGURATOR_OK if parsing and verifying the configuration referenced through config_data
     *         succeded.  
     *
     *  If CONFIGURATOR_OK is returned this method fills the instance variables left_red_drive_wheel, right_red_drive_wheel,
     *  rotor_names, ring_ids, war_machine and rotor_settings as a side effect.
     */               
    virtual unsigned int parse_config(map<string, string>& config_data);    
};

#endif /* __configurator_h__ */
