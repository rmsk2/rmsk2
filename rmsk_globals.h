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

#ifndef __rmsk_globals_h__
#define __rmsk_globals_h__

/*! \file rmsk_globals.h
 *  \brief Header files for general helper methods, global data structures and constants.
 */ 

#include<glibmm.h>
#include<alphabet.h>

class rotor_machine;

// Machine names used in INI files that contain saved rotor_machine states.
#define MNAME_ENIGMA_I "Enigma"
#define MNAME_RAILWAY_ENIGMA "RailwayEnigma"
#define MNAME_TIRPITZ_ENIGMA "TirpitzEnigma"
#define MNAME_KD_ENIGMA "KDEnigma"
#define MNAME_ABWEHR_ENIGMA "AbwehrEnigma"
#define MNAME_M4_ENIGMA "M4Enigma"
#define MNAME_KL7 "KL7"
#define MNAME_NEMA "Nema"
#define MNAME_SG39 "SG39"
#define MNAME_SIGABA "SIGABA"
#define MNAME_TYPEX "Typex"

/*! \brief Contains the section name that is used when saving machine specific information (in contrast for 
 *         instance to information about a rotor_descriptor) in a gnome settings file.
 */ 
#define MACHINE_SECTION "machine"

/*! \brief Contains the name of the key in the section MACHINE_SECTION where the machine name is stored.
 */ 
#define KEY_MACHINE_NAME "name"

/*! \brief Contains the name of the key in the section MACHINE_SECTION where the machinetype is stored for Enigma variants.
 */ 
#define KEY_MACHINE_TYPE "machinetype"

/*! \brief Used in the ::configurator class and its children to represent the boolean value true.  
 */
#define CONF_TRUE "true"

/*! \brief Used in the ::configurator class and its children to represent the boolean value false.  
 */
#define CONF_FALSE "false"

/*! \brief Constant that is used to define an alphabet that describes the symbols on the circumfence 
 *         of a KL7 rotor. 
 * 
 *  Where there are empty spaces on the real rotors a number is used. If the spaces
 *  were used instead it would not be possible to use this string to specify a valid ::alphabet.
 */
#define KL7_RING_CIRCUMFENCE_HELP "ab1cde2fg3hij4klm5no6pqr7st8uvw9xyz0"

/*! \brief Alphabet that specifies the mapping of contacts on a UKW D to the letters on its circumfence
 */
#define UKWD_ALPHA_CIRCUMFENCE "yzxwvutsrqponjmlkihgfedcba"

/*! \brief Holds the contact number of the upper fixed contact in a UKW D plugable reflector. */      
const unsigned int UKWD_FIXED_CONTACT_Y = 0;

/*! \brief Holds the contact number of the lower fixed contact in a UKW D plugable reflector. */      
const unsigned int UKWD_FIXED_CONTACT_J = 13;

/*! \brief Constant which specifies that a printing_device or rotor_keyboard is in FIGURE state.  
 */
const unsigned int FUNC_FIGURE_SHIFT = 0xFFFFFFFF;

/*! \brief Constant which specifies that a printing_device or rotor_keyboard is in LETTER state.  
 */
const unsigned int FUNC_LETTER_SHIFT = 0xFFFFFFFE;

/*! \brief A printing_device that has no LETTER of FIGURE mode is always in state NONE.  
 */
const unsigned int FUNC_NONE = 0xFFFFFFFD;

/*! \brief Type that specifies whether a key_word_info element describes a string or boolean keyword.  
 */
typedef enum _keyword_type {KEY_STRING, KEY_BOOL} keyword_type; 


/*! \brief A struct that consists of a numeric identifier for a rotor and a ring in relation to a rotor_set.
 *         On top of that it stores whether the rotor has to be inserted in the normal way or in reverse.
 *         
 *   rotor_id instances are used in the implementation of several machines to specify which rotors
 *   and rings are to be inserted in which way into the simulated machine.
 */
struct rotor_id
{
    /*! \brief Default constructor. Sets r_id and ring_id to 0 and insert_inverse to false.  
     */
    rotor_id() { r_id = 0; ring_id = 0; insert_inverse = false; }

    /*! \brief Constructs a rotor_id using a rotor identifier and flag.
     *
     *  \param i [in] rotor identifer
     *  \param j [in] Specifies whether rotor is inserted in reverse (true) or in the normal way (false)
     */
    rotor_id(unsigned int i, bool j) { r_id = i; ring_id = i; insert_inverse = j; }
    
    /*! \brief Constructs a rotor_id using a rotor identifier. insert_inverse is set to false. ring_id is
     *         also set to i.
     *
     *  \param i [in] rotor identifer
     */    
    rotor_id(unsigned int i) { r_id = i; ring_id = i; insert_inverse = false; }

    /*! \brief Constructs a rotor_id using a rotor and a ring identifier. rotor_id::insert_inverse is set to false.
     *
     *  \param i [in] rotor identifer
     *  \param k [in] ring identifier
     */
    rotor_id(unsigned int i, unsigned int k) { r_id = i; ring_id = k; insert_inverse = false; }    

    /*! \brief Constructs a rotor_id using a rotor identifier, a ring identifier and an insertion flag.
     *
     *  \param i [in] rotor identifer
     *  \param k [in] ring identifier
     *  \param j [in] insertion flag (true = insert inverse)     
     */
    rotor_id(unsigned int i, unsigned int k, bool j) { r_id = i; ring_id = k; insert_inverse = j; }    
    
    /*! \brief Numeric identifier of rotor to use.  
     */    
    unsigned int r_id;

    /*! \brief Numeric identifier of ring to use.  
     */    
    unsigned int ring_id;
    
    /*! \brief Specifies whether rotor is to be inserted in reverse (= true) or not (= false)  
     */        
    bool insert_inverse;
};

/*! \brief A struct used in ::nema implementation to specifiy a rotor assembly that consists of a contact rotor and
 *         a drive wheel.
 *
 *  Due to the unique stepping mechanism of the Nema. The Nema simulator can not use the rotor_id struct.
 */
struct rotor_assembly {
    /*! \brief Constructs a rotor_assembly using the given contact_id and drive_id.  
     */    
    rotor_assembly(unsigned int drive_id, unsigned int contact_id) { contact_rotor_id = contact_id; drive_wheel_id = drive_id; }
    
    /*! \brief Numeric identifier of the contact rotor to use.  
     */        
    unsigned int contact_rotor_id;

    /*! \brief Numeric identifier of the drive wheel to use.  
     */        
    unsigned int drive_wheel_id;
};

/*! \brief A struct which is used in ::configurator to specifiy the name/identifer (a.k.a key word) of a configuration             
 *         parameter and its type.
 *
 *  Currently there are only two types of key word: STRING and BOOLEAN. See ::keyword_type.
 */
struct key_word_info {
    /*! \brief Constructs a key_word_info instance using the given key word and type.  
     */   
    key_word_info(string kw, keyword_type t, string d) { keyword = kw; type = t; descriptive_text = d; }

    /*! \brief Name/identifer of the configuration key word.  
     */       
    string keyword;

    /*! \brief Human understandable description.  
     */           
    string descriptive_text;
    
    /*! \brief Type: STRING or BOOLEAN.  
     */    
    keyword_type type;
};

/*! \brief A class which provides several static utility functions for mapping permutations to alphanumeric UKW D plugs
 *         specifications and vice versa.
 */
class ukw_d_wiring_helper {
public:
    /*! \brief Method that can be used to parse the permutation given in parameter perm and which specifies an involution into
     *         the character pairs that make up that involution. 
     */    
    static vector<pair<char, char> > perm_to_plugs(permutation& perm);

    /*! \brief Does the inverse of perm_to_plugs(). Transforms the vector of character pairs referenced by
     *         parameter plugs into a permutation.
     */    
    static permutation plugs_to_perm(vector<pair<char, char> >& plugs);
    
protected:
    /*! \brief Helper method to compare two character pairs by their first element.
     */        
    static bool less_than(const pair<char, char>& l, const pair<char, char>& r);
};


/*! \brief A class which provides several static utility functions. 
 */
class rmsk {
public:
    /*! \brief Returns the standard alphabet a-z consisting of conventional chars. The returned object is
     *         a singleton.
     */    
    static alphabet<char> *std_alpha();

    /*! \brief Returns the standard alphabet a-z consisting of gtk wide characters (gunichar). The returned object is
     *         a singleton.
     */    
    static alphabet<gunichar> *std_uni_alpha();
    
    /*! \brief Converts the given string consisting of chars '0's and '1's into a vector of 0s and 1s.
     *
     *  \param str [in] Input string.
     *  \param len [in] Length of input string.
     *  \param bool_vec [out] Conversion result. Contains a 0 for every '0' in str, a 1 otherwise.
     */        
    static void str_to_bool_vec(const char *str, unsigned int len, vector<unsigned int>& bool_vec);
    
    /*! \brief Converts a C-Style array of unsigned ints into an STL vector and returns the conversion result.
     */            
    static vector<unsigned int> uint_arr_to_vec(unsigned int *arr, unsigned int len);
    
    /*! \brief Should be called at the end of all programs that use rmsk. Deletes the created singletons.
     */        
    static void clean_up();
    
    /*! \brief Returns the path where the help files can be found.
     */            
    static string get_doc_path();
    
    /*! \brief Returns the XML formatted GUI description that is needed to construct all the dialogs.
     */            
    static string get_glade_data();
    
    /*! \brief Replacement for assert macro. In contrast to the macro this method allows to specifiy 
     *         the exception message.
     *
     *  An exception of type runtime_error is thrown, when the given condition is true.
     */            
    static void simple_assert(bool condition, const char *exception_message);

    /*! \brief Returns a default instance of the rotor machine specified in parameter machine_name.
     *
     *  In case of an error NULL is returned.
     */
    static rotor_machine *make_default_machine(string& machine_name);

    /*! \brief Returns the string which is to be used to construct a configurator object for this machine via a
     *         call to configurator_factory::get_configurator().
     */
    static string get_config_name(rotor_machine *machine);

    /*! \brief Restores the state stored in the file named by parameter file_name into a rotor_machine object
     *         and returns that object.
     *
     *  In case of an error NULL is returned.
     */
    static rotor_machine *restore_from_file(string& file_name);

    /*! \brief Restores the state contained in the keyfile referenced by parameter machine_state into a
     *         rotor_machine object and returns that object.
     *
     *  In case of an error NULL is returned.
     */
    static rotor_machine *restore_from_ini(Glib::KeyFile& machine_state);

    /*! \brief Restores the state contained in the string referenced by parameter machine_state into a
     *         rotor_machine object and returns that object.
     *
     *  In case of an error NULL is returned.
     */
    static rotor_machine *restore_from_data(string& machine_state);

    /*! \brief Reads the ini file specified by the parameter file_name and uses the callback given in the 
     *         parameter processor to process the data contained in the resulting KeyFile object.
     */                
    static bool settings_file_load(string& file_name, sigc::slot<bool, Glib::KeyFile&> processor);

    /*! \brief Uses the callback given in the parameter generator to fill a KeyFile object and then stores it
     *         in the file specified by parameter file_name.
     */                    
    static bool settings_file_save(string& file_name, sigc::slot<void, Glib::KeyFile&> generator);    

protected:
    /*! \brief Holds the object returned by std_alpha().  
     */        
    static alphabet<char> *classical_alpha;

    /*! \brief Holds the object returned by std_uni_alpha().  
     */
    static alphabet<gunichar> *classical_uni_alpha;    
};

#endif /* __rmsk_globals_h__ */

