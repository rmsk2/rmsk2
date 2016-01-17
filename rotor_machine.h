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

#ifndef __rotor_machine_h__
#define __rotor_machine_h__

/*! \file rotor_machine.h
 *  \brief Header file for the rotor_machine class.
 */ 

#include<vector>
#include<boost/shared_ptr.hpp>
#include<alphabet.h>
#include<transforms.h>
#include<stepping.h>
#include<rmsk_globals.h>
#include<rotor_set.h>
#include<printing_device.h>
#include<rotor_keyboard.h>

/*! \brief Name of the default set that has to be present in each rotor_machine object.
 */ 
#define DEFAULT_SET "defaultset"


class printing_device;
class rotor_keyboard;


/*! \brief A class implementing a generic rotor machine.
 *
 *  There are two classes which are important for the rotor machine class. One is the stepping_gear class and its children that
 *  provide any rotor machine with its stepping mechanism. The second is the rotor_set class, which acts as a factory that knows
 *  how to construct ::rotor and rotor_ring objects. 
 *
 *  Each rotor_machine knows in principle several rotor_sets which are stored as key
 *  value pairs in the member variable rotor_sets. Each rotor_machine has to know at least one rotor set which has the name 
 *  DEFAULT_SET. Additional sets can be added to any rotor_machine object. Whenever a method is called, that deals with the
 *  construction of rotors and/or rotor_rings and no rotor_set name is explicitly given, then the name of the rotor_set contained in the
 *  default_rotor_set_name member variable is implied. The value of this member can be changed by the set_default_set_name method.
 *
 *  Apart from the rotors a generic rotor_machine has two additional cryptographic elements. The input_transfrom and the output_transform.
 *  When encrypting the input_tansform processes the input value before it it sent through the rotor stack and the output_transform
 *  processes the value that is leaving the rotor stack. The output of the output_transform is the encryption result. When decrypting
 *  the process is reversed. It has to be noted that if the rotor stack is operated in reflecting mode, input_transform and 
 *  output_transform have to be equal. This is enforced when using the setter methods, i.e. with a reflecting rotor stack the setter
 *  methods set both transforms at the same time.
 *
 *  Another thing distinguishes one rotor machine from another. In some machines the rotors step first and then the en/decryption of
 *  a symbol takes place. Other machines do the en/decryption first and then step the rotors. In the first case we call the machine
 *  a "prestepping" machine. This attribute of a machine can be queried through the is_pre_stepper() method.
 *
 *  A rotor machine also makes use of a printing_device which knows how to translate the raw unsigned int output of the machine into
 *  characters. A printing device can implement figures or letters modes, but does not have to.  
 *
 *  A rotor machine also has a rotor_keyboard which knows how to translate characters into the raw unsigned int input the machine 
 *  implementation expects. Some rotor_keyboard children can be operated in figures and letters mode. Some others do not have this
 *  feature.
 *
 *  rmsk models keyboards and printig devices in entities separate from the raw implementation of the cryptographic core. It would
 *  therefore be easily possible to use rmsk to simulate a machine that has the cryptographic core of a SIGABA but the keyboard
 *  and printing device of a Typex. 
 */ 
class rotor_machine : public machine_transform {
public:
    /*! \brief Constructs a rotor machine from a given stepping_gear.
     */  
    rotor_machine(stepping_gear *step) { stepper = step; is_pre_step = false; machine_name = "Generic"; default_rotor_set_name = DEFAULT_SET; }

    /*! \brief Constructs a default rotor machine without a stepping_gear.
     */  
    rotor_machine() { stepper = NULL; is_pre_step = false; machine_name = "Generic"; default_rotor_set_name = DEFAULT_SET; }
    
    /*! \brief Encrypts the value given in in_char.
     */  
    virtual unsigned int encrypt(unsigned int in_char);

    /*! \brief Decrypts the value given in in_char.
     */  
    virtual unsigned int decrypt(unsigned int in_char);

    /*! \brief Returns the size of the permutation this machine generates. Normally this is equal to the number of
     *         contacts of the rotors in this machine.
     */      
    virtual unsigned int get_size() { return stepper->get_size(); }
    
    /*! \brief Steps the rotors in this machine.
     */      
    virtual void step_rotors() { stepper->step_rotors(); }
    
    /*! \brief Returns the stepping_gear in use in this machine.
     */      
    stepping_gear *get_stepping_gear() {return stepper; }

    /*! \brief Sets the stepping_gear in use in this machine.
     */      
    void set_stepping_gear(stepping_gear *s) { stepper = s; }

    /*! \brief Adds the given rotor_set to this rotor_machine. The rotor_set can subsequently be referred to by the name given in the
     *         parameter name.
     */              
    void add_rotor_set(string& name, rotor_set& set);

    /*! \brief The same as add_rotor_set(string&, rotor_set&) but the name of the rotor_set is contained in a C-style zero terminated
     *         string.
     */
    void add_rotor_set(const char *name, rotor_set& set) { string temp(name); add_rotor_set(temp, set); }

    /*! \brief Returns the names of all rotor_sets known to this rotor_machine.
     */      
    virtual vector<string> get_rotor_set_names();

    /*! \brief Sets the default rotor set name in use in this rotor_machine.
     */          
    virtual void set_default_set_name(string default_set_name) { default_rotor_set_name = default_set_name; }

    /*! \brief Returns the default rotor set name in use in this rotor_machine.
     */          
    virtual string get_default_set_name() { return default_rotor_set_name; }

    /*! \brief Returns a reference to the rotor_set identified by the parameter name. If a rotor_set of that name can not be found
     *         in this rotor_machine an exception of type runtime_exception is thrown.
     */          
    virtual rotor_set& get_rotor_set(string& name);

    /*! \brief The same as get_rotor_set(string&) but the name of the rotor_set is contained in a C-style zero terminated
     *         string.
     */
    virtual rotor_set& get_rotor_set(const char *name) { string temp(name); return get_rotor_set(temp); }    
    
    /*! \brief Deletes the rotor_set identified by the parameter name from this rotor_machine.
     *         string.  If a rotor_set of that name can not be found
     *         in this rotor_machine an exception of type runtime_exception is thrown.
     */    
    virtual void delete_rotor_set(string& name);

    /*! \brief The same as delete_rotor_set(string&) but the name of the rotor_set is contained in a C-style zero terminated
     *         string.
     */
    virtual void delete_rotor_set(const char *name) { string temp(name); delete_rotor_set(temp); }

    /*! \brief Changes the name of this rotor_machine. The name of a machine is piece of control information, that is not
     *         intended to be displayed to the user.
     */              
    virtual void set_name(string new_name) { machine_name = new_name; }

    /*! \brief Returns the name of this rotor_machine. The name of a machine is piece of control information, that is not
     *         intended to be displayed to the user.
     */              
    virtual string get_name() { return machine_name;}

    /*! \brief Returns the description of this rotor_machine. The description of the rotor machine is a piece of information
     *         that is intended to be displayed to the user.
     *
     *  The default implementation of get_description() also returns the machine name but there are simulators in rmsk where
     *  this is not true. In the SIGABA simulator the machine name is SIGABA, whereas the description is either CSP889 or 
     *  CSP2900.
     */
    virtual string get_description() { return machine_name; }

    /*! \brief Changes the description of this rotor_machine. The description of the rotor machine is a piece of information
     *         that is intended to be displayed to the user.
     */                  
    virtual void set_description(string new_decription) { ; }    

    /*! \brief This method instructs the rotor machine object to randomize its state. It returns false if the randomization was
     *         successfull. If this method returns true an error occurred and the object's state is unchanged.
     *
     *  The parameter param can be used to influence the randomization. How this is done depends on the implementation of
     *  this method in the corresponding sunclass.
     */
    virtual bool randomize(string& param) { return false; }

    /*! \brief Returns the input transform in use in this rotor_machine.
     */
    virtual boost::shared_ptr<encryption_transform> get_input_transform() {return input_transform; }

    /*! \brief Sets the input transform in use in this rotor_machine.
     *
     *  In a machine with a reflecting rotor_stack this method also sets the output transform.          
     */
    virtual void set_input_transform(boost::shared_ptr<encryption_transform> t);

    /*! \brief Returns the output transform in use in this rotor_machine.
     */
    virtual boost::shared_ptr<encryption_transform>get_output_transform() {return output_transform; }

    /*! \brief Sets the output transform in use in this rotor_machine. 
     *
     *  In a machine with a reflecting rotor_stack this method also sets the input transform.     
     */
    virtual void set_output_transform(boost::shared_ptr<encryption_transform> t);

    /*! \brief Returns true if this machine is a prestepping machine. Returns false, if that is not the case.
     */    
    virtual bool is_pre_stepper() { return is_pre_step; }

    /*! \brief Setter method for the prestepping flag. Setting this flag changes the behaviour of the machine
     *         accordingly, i.e. after setting the prestepping flag the machine is operated in prestepping mode.
     */    
    virtual void set_pre_stepper_flag(bool f) { is_pre_step = f; }
    
    /*! \brief Helper method that is used to translate the numerical ring position of the rotor identified by
     *         rotor_identifier to a character. Using ustring as a return type leaves open the possibility to
     *         have "nothing" (i.e. the empty string) visualize the ring position.
     */        
    virtual ustring visualize_rotor_pos(string& rotor_identifier);
    
    /*! \brief The same as visualize_rotor_pos(string&) but the name of the rotor is contained in a C-style
     *         zero terminated string.
     */    
    virtual ustring visualize_rotor_pos(const char *rotor_identifier);
    
    /*! \brief Returns a string that contains a visualization of the rotor positions as shown in the rotor windows
     *         of the machine. Uses rotor_machine::visualize_rotor_pos() to visualize each position and 
     *         rotor_machine::unvisualized_rotor_names to determine which rotors are not to be visualized at all.
     */
    virtual ustring visualize_all_positions();

    /*! \brief This method changes the vector which is referenced through the parameter current_perm in such a way that
     *         it reflects the permutation that is produced by the rotor machine in its current state.
     */        
    virtual void get_current_perm(vector<unsigned int>& current_perm);

    /*! \brief Setter method for the printing_device in use in this rotor_machine.
     */        
    void set_printer(boost::shared_ptr<printing_device> new_printer);

    /*! \brief Returns the printing_device in use in this rotor_machine.
     */        
    boost::shared_ptr<printing_device> get_printer() { return printer; }

    /*! \brief Setter method for the rotor_keyboard in use in this rotor_machine.
     */        
    void set_keyboard(boost::shared_ptr<rotor_keyboard> new_keyboard);

    /*! \brief Returns the rotor_keyboard in use in this rotor_machine.
     */        
    boost::shared_ptr<rotor_keyboard> get_keyboard() { return keyboard; }

    /*! \brief Restores the state of this rotor_machine from the KeyFile object referenced in the ini_file parameter.
     *         
     *  The method returns true if an error was encountered while restoring the machine's state.
     */            
    virtual bool load_ini(Glib::KeyFile& ini_file);

    /*! \brief Restores the state of this rotor_machine from a gnome settings file the name of which has to be provided in
     *         in the file_name parameter.
     */            
    virtual bool load(string& file_name);
    
    /*! \brief Same as load(string&), but the file_name is to be specified by a C-style zero terminated string.
     */                
    virtual bool load(const char* file_name) { string temp(file_name); return load(temp); }

    /*! \brief Saves the state of this rotor_machine in the KeyFile object referenced in the ini_file parameter.
     */                
    virtual void save_ini(Glib::KeyFile& ini_file);

    /*! \brief Saves the state of this rotor_machine into a gnome settings file the name of which has to be provided in
     *         in the file_name parameter.
     *
     *  The method returns true if an error was encountered while saving the machine's state.
     */            
    virtual bool save(string& file_name);

    /*! \brief Same as save(string&), but the file_name is to be specified by a C-style zero terminated string.
     */                
    virtual bool save(const char* file_name) { string temp(file_name); return save(temp); }    

    /*! \brief This method uses the rotor_set with the name given in the parameter rotor_set_name to construct a rotor and a rotor_ring. 
     *         These entities are then inserted into the stepping_gear of this machine to replace the rotor and rotor_ring which are currently
     *         in use in the rotor_descriptor identified by the value of the parameter rotor_name. If the parameter reverse is true then
     *         the inverse of the retrieved rotor permutation is used.
     *
     *  Using the reverse rotor permutation is different from inserting the rotor in reverse. Inserting a rotor in reverse
     *  involves a more complex transformation than simply using the inverse rotor permutation. See rotor_set::make_rotor(rotor_id).
     */                
    void prepare_rotor(string& rotor_set_name, rotor_id r_id, string& rotor_name, bool reverse = false);
    
    /*! \brief Same as prepare_rotor(string&, rotor_id, string&, bool), just the parameter types are different to allow the use of
     *         C-style zero terminated strings.
     */                    
    void prepare_rotor(const char *rotor_set_name, rotor_id r_id, const char *rotor_name, bool reverse = false);

    /*! \brief Same as prepare_rotor(string&, rotor_id, string&, bool), but the explicit rotor_set_name is omitted. The value of
     *         the member variable default_rotor_set_name is used to determine the rotor_set name to use.
     */                    
    void prepare_rotor(rotor_id r_id, string& rotor_name, bool reverse = false) { prepare_rotor(default_rotor_set_name, r_id, rotor_name, reverse); }

    /*! \brief Same as prepare_rotor(rotor_id, string&, bool), just the parameter types are different to allow the use of
     *         C-style zero terminated strings.
     */                    
    void prepare_rotor(rotor_id r_id, const char *rotor_name, bool reverse = false) { string help1(rotor_name); prepare_rotor(default_rotor_set_name, r_id, help1, reverse); }    

    /*! \brief Same as prepare_rotor(string&, rotor_id, string&, bool), but the rotor_set to use is not determined by its name but by a reference to
     *         actual rotor_set object.
     */                    
    void prepare_rotor(rotor_set& r_set, rotor_id r_id, string& rotor_name, bool reverse = false);    

    /*! \brief Same as prepare_rotor(rotor_set&, rotor_id, string&, bool), just the parameter types are different to allow the use of
     *         C-style zero terminated strings.
     */                    
    void prepare_rotor(rotor_set& r_set, rotor_id r_id, const char *rotor_name, bool reverse = false) { string help(rotor_name); prepare_rotor(r_set, r_id, help, reverse); }
    
    /*! \brief This method returns the rotor and rotor_ring which are identified by the parameter id. The rotor_set used for construction of these
     *         objects is determined by the value of the member variable default_rotor_set_name. The parameter insert_inverse can be set to true, if
     *         the inverse rotor permutaion is to be used.
     *
     *  Beware: Setting the insert_inverse flag to true is *not the same* as inserting a rotor in reverse. Setting the flag only has the effect
     *  that the inverse permutation is used to construct the rotor.
     */                    
    pair<boost::shared_ptr<rotor>, boost::shared_ptr<rotor_ring> > make_rotor(const unsigned int id, bool insert_inverse = false); 

    /*! \brief This method creates and returns the ::rotor and rotor_ring which are identified by the parameter r_id. The rotor_set used for construction of these
     *         objects is determined by the value of the member variable default_rotor_set_name.
     */                    
    pair<boost::shared_ptr<rotor>, boost::shared_ptr<rotor_ring> > make_rotor_rid(rotor_id r_id);               

    /*! \brief Same as make_rotor(const unsigned int, bool), but in addition the rotor_set to use can be specified by the parameter rotor_set_name.
     */                    
    pair<boost::shared_ptr<rotor>, boost::shared_ptr<rotor_ring> > make_rotor(const char *rotor_set_name, const unsigned int id, bool insert_inverse = false);

    /*! \brief Same as make_rotor(const unsigned int, bool), but in addition the rotor_set to use can be specified by the parameter rotor_set_name.
     */                    
    pair<boost::shared_ptr<rotor>, boost::shared_ptr<rotor_ring> > make_rotor(string& rotor_set_name, const unsigned int id, bool insert_inverse = false);     

    /*! \brief Same as make_rotor_rid(rotor_id), but in addition the rotor_set to use can be specified by the parameter rotor_set_name.
     */                    
    pair<boost::shared_ptr<rotor>, boost::shared_ptr<rotor_ring> > make_rotor_rid(string& rotor_set_name, rotor_id r_id);

    /*! \brief Same as make_rotor_rid(rotor_id), but in addition the rotor_set to use can be specified by the parameter rotor_set_name.
     */                    
    pair<boost::shared_ptr<rotor>, boost::shared_ptr<rotor_ring> > make_rotor_rid(const char *rotor_set_name, rotor_id r_id);                   

    /*! \brief Destructor.
     */                
    virtual ~rotor_machine() { ; }
    
protected:

    /*! \brief Helper method. Calls t->encrypt(in_char) if t is not NULL.
     */                
    unsigned int safe_encrypt(encryption_transform *t, unsigned int in_char);

    /*! \brief Helper method. Calls t->decrypt(in_char) if t is not NULL.
     */                
    unsigned int safe_decrypt(encryption_transform *t, unsigned int in_char);

    /*! \brief This method is intended to be reimplemented by those children of rotor_machine that have to save additional state information
     *         when save_ini(Glib::KeyFile&) is called.
     */                      
    virtual void save_additional_components(Glib::KeyFile& ini_file) { ; }

    /*! \brief This method is intended to be reimplemented by those children of rotor_machine that have to load additional state information
     *         when restoring state from a KeyFile object. This method is called by load_ini(Glib::KeyFile&). Its default implementation does nothing.
     *         Reimplementations have to return false if no errors occured while restoring the state. true is to be returned otherwise.
     */         
    virtual bool load_additional_components(Glib::KeyFile& ini_file) { return false; }

    /*! \brief This method encrypts the symbol specified in parameter symbol but does *not* step the machine.
     */                        
    virtual unsigned int get_enc_symbol(unsigned int symbol);        

    /*! \brief Holds the stpping_gear in use in this rotor_machine.
     */                    
    stepping_gear *stepper;
    
    /*! \brief Holds the printing_device in use in this rotor_machine.
     */                    
    boost::shared_ptr<printing_device> printer;

    /*! \brief Holds the rotor_keyboard in use in this rotor_machine.
     */                    
    boost::shared_ptr<rotor_keyboard> keyboard;
    
    /*! \brief Holds the flag that determines whether the machine is operated in prestepping mode or not.
     */                        
    bool is_pre_step;

    /*! \brief Holds the input transform in use in this rotor_machine.
     */                        
    boost::shared_ptr<encryption_transform> input_transform;

    /*! \brief Holds the output transform in use in this rotor_machine.
     */                        
    boost::shared_ptr<encryption_transform> output_transform;

    /*! \brief Holds the rotor_sets known to this machine.
     */                            
    map<string, rotor_set> rotor_sets;
    
    /*! \brief Holds the machine's name.
     */                            
    string machine_name;
    
    /*! \brief Holds the name of the default rotor set.
     */                            
    string default_rotor_set_name;

    /*! \brief Holds the symbolic rotor identifiers which are excluded from visualization by rotor_machine::visualize_all_positions.
     */                                
    set<string> unvisualized_rotor_names;
};

#endif /* __rotor_machine_h__ */

