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

#ifndef __stepping_h__
#define __stepping_h__

/*! \file stepping.h
 *  \brief Header files for the classes that make up the stepping gear of a rotor machine.
 */ 

#include<stdint.h>
#include<string>
#include<vector>
#include<map>
#include<boost/shared_ptr.hpp>
#include<glibmm.h>

#include<transforms.h>
#include<rotor.h>
#include<rotor_ring.h>
#include<rmsk_globals.h>

using namespace std;

/*! \brief Maximum number of rotors that can be placed in a rotor stack. 
 */
const unsigned int MAX_ROTORS = 50;

class stepping_gear;

/*! \brief A class encapsulating the concept of a stack of rotors in a rotor machine.
 *
 *  The variable rotor_stack::stack references the rotors that make up the stack. When encrypting the signal travels from the rotor 
 *  at position 0 to the rotor on position stack_size-1. In the default case the signal then leaves the rotor stack
 *  at a certain position which is the result of the encryption. The signal path is reversed when decrypting.
 *    
 *  If the flag rotor_stack::is_reflecting_stack is true, the signal is reflected back through the rotor stack. This is for 
 *  instance used in the Enigma simulator. The rotor on the last position in rotor_stack::stack is made the reflecting rotor. 
 *  In a reflecting stack there is no difference between encryption and decryption. A reflecting stack must contain at least two
 *  rotors. It is noted that the reflecting rotor does not have to be an involution. If one chooses a permutation that maps
 *  one character to itself, one could even have the simulated signal travel the same way back that it came from. If this happens
 *  a letter can represent itself in ciphertext. In a real physical rotor stack this is of course impossible even though a similar
 *  effect can be acheived with additional circuitry.
 *
 *  If the set rotor_stack::feedback_points contains elements it is checked whether the position where the signal would leave
 *  the stack is contained in the set feedback_points. If yes the signal is sent again through the stack in the same
 *  direction as before. This is repeated until an exit position is "hit" that is not contained in the set of feedback points.
 *  When decrypting the signal path is reversed. This is used in the KL7 implementation.
 *  
 */ 
class rotor_stack : public encryption_transform {
public:   
    /*! \brief Copy constructor. 
     */ 
    rotor_stack(const rotor_stack& s) { copy(s); }
    
    /*! \brief Constructs a rotor stack from the rotors given in all_rotors. The parameter is_reflecting determines whether the new stack
     *         is operated as a reflecting stack. 
     */     
    rotor_stack(vector<rotor *>& all_rotors, bool is_reflecting = false) { is_reflecting_stack = is_reflecting; set_rotors_help(all_rotors); }
    
    /*! \brief Default constructor. 
     */     
    rotor_stack() { is_reflecting_stack = false; stack_size = 0; }

    /*! \brief Encrypt the input value given in in_char. 
     */         
    virtual unsigned int encrypt(unsigned int in_char);

    /*! \brief Decrypt the input value given in in_char. 
     */         
    virtual unsigned int decrypt(unsigned int in_char);

    /*! \brief Returns the number of contacts the stack has. It is identicial to the size of the rotors used in the stack.
     */         
    virtual unsigned int get_size() { return stack[0]->get_size(); }

    /*! \brief Resets the contents of rotor_stack::stack to the values given in new_rotors.
     */         
    virtual void set_rotors(vector<rotor *>& new_rotors) { set_rotors_help(new_rotors); }
    
    /*! \brief Sets a new feedback permutation.
     */             
    virtual void set_feedback_permutation(permutation& new_permutation) { feedback_permutation = new_permutation; }

    /*! \brief The contacts given in the parameter new_feedback_points are used to update the feedback points in use in 
     *         this rotor stack. This method sets the feedback permutation to the identity permutation.
     */             
    virtual void set_feedback_points(set<unsigned int>& new_feedback_points);

    /*! \brief Update the feedback points using a C-style array instead of an STL vector.
     */             
    virtual void set_feedback_points(const unsigned int *new_feedback_points, unsigned int num_feedback_points);

    /*! \brief Helper method. Returns a pointer to a rotor stack that is identical to this one.
     */             
    virtual rotor_stack *clone();

    /*! \brief Returns true if the stack is operated in reflecting mode. False otherwise.
     */                 
    virtual bool get_reflecting_flag() { return is_reflecting_stack; }

    /*! \brief Sets the operating mode of this stack. If new_val is true the rotor stack is operated in reflecting mode.
     */                 
    virtual void set_reflecting_flag(bool new_val);
    
    /*! \brief Assignment operator.
     */                     
    virtual rotor_stack& operator=(const rotor_stack& s) { copy(s); return *this; }    
    
    /*! \brief Destructor. The rotors of this stack are not deleted by the destructor.
     */                     
    virtual ~rotor_stack() { ; }
    
protected:
    // Last rotor is defined to be the (only) reflecting rotor
    
    /*! \brief Helper method that copies the contents of the vector new_rotors to the rotor_stack::stack variable.
     */      
    void set_rotors_help(vector<rotor *>& new_rotors);
    
    /*! \brief Copies the contents of the instance variables in the stack given in parameter s to this stack.
     */      
    void copy(const rotor_stack& s);    
    
    /*! \brief Number of rotors used in rotor_stack::stack.
     */      
    unsigned int stack_size;

    /*! \brief Contains pointers to the rotors that make up this stack. There are rotor_stack::stack rotors that are
     *         actually used.
     */      
    rotor *stack[MAX_ROTORS];
    
    /*! \brief Is true when the stack is to be operated in reflecting mode.
     */          
    bool is_reflecting_stack;
    
    /*! \brief Holds the feedback_points in use in this stack.
     */          
    set<unsigned int> feedback_points;
    
    /*! \brief Holds the feedback permutation.
     *
     *  It is noted that the feedback permutation has to have the same size (the same number of contacts) as the rotors in this stack.
     *  It is intended to permute the feedback points among themselves and therefore must be equal to the identity permutation on
     *  non feedback positions.
     */          
    permutation feedback_permutation;    
};

/*! \brief A struct that binds together the rotor, its ring, its state and some additional (mostly optional) information.
 *
 *  In addition to the information given above, a rotor_descriptor contains key value pairs for different types of information.
 *  This is intended to associate metainformation with the rotor and its ring. It is for instance used to implement the notch
 *  wheels in the simulator of the Schluesselgeraet 39.
 */ 
struct rotor_descriptor {

    /*! \brief Default constructor.
     */      
    rotor_descriptor() { current_displacement = 0; }

    /*! \brief Restores the rotor permutation, the rotor_id, and the ring data from the given
     *         ini_file. The section of the ini file to read is determined by the parameter identifier.
     */          
    bool load_from_config(string& identifier, Glib::KeyFile& ini_file);

    /*! \brief Saves the contents of this rotor descriptor in the KeyFile referenced through the parameter ini_file.
     *  
     *  Currently only saves the rotor permutation, the rotor id, the ring id and the ring data. The key value pairs
     *  are not saved. The section to write to is determined by the parameter identifier.
     */      
    void save_to_config(string& identifier, Glib::KeyFile& ini_file);

    /*! \brief References the rotor that is contained in this rotor_descriptor.
     */          
    boost::shared_ptr<rotor> r;

    /*! \brief References the ring which is attached to the rotor rotor_descriptor::r.
     */          
    boost::shared_ptr<rotor_ring> ring;

    /*! \brief ::rotor_id that describes ring id and rotor id of the rotor and ring contained in this rotor_descriptor.
     */          
    rotor_id id;

    /*! \brief Displacement variable for the rotor referenced through rotor_descriptor::r.
     */          
    unsigned int current_displacement;

    /*! \brief Key value pairs where the value is of type unsigned int.
     */          
    map<string, unsigned int> int_vals;

    /*! \brief Key value pairs where the value is of type string.
     */          
    map<string, string> string_vals;

    /*! \brief Key value pairs where the value is of type vector<unsigned int>.
     */           
    map<string, vector<unsigned int> > binary_vals;

    /*! \brief Key value pairs where the value is of ::simple_mod_int.
     */          
    map<string, simple_mod_int> mod_int_vals;        

    /*! \brief Key value pairs where the value is of type char.
     */          
    map<string, char> char_vals;

    /*! \brief Key value pairs where the value is of type void *. This intended to provide a mechanism for associating
     *         pointers to arbitraty objects with this rotor_descriptor.
     */          
    map<string, void *> object_vals;
};

/*! \brief A class that implements the stepping mechanism of a rotor machine.
 *
 *  The stepping_gear moves the rotors in the rotor_stack using the information contained in the rotor_descriptors. In many ways the
 *  stepping_gear of a simulated rotor machine *is* the rotor machine because it is mainly the stepping mechanism that distinguishes
 *  one machine from the other. It is also the main component that determines the cryptographic strength of a rotor machine. Errors
 *  are reported by exceptions of type runtime_exception.
 *
 *  Rotor descriptors in a stepping_gear must be identified by a string, the rotor identifier, or optionally by a number, the rotor number, 
 *  where the rotor number is equal to the index of the corresponding rotor identifier in the stepping_gear::rotor_positions member.
 *
 *  This class does not do anyting useful in itself. It is intended as a base class for the implementation of stepping mechanisms.
 */ 
class stepping_gear {
public:
    /*! \brief Copy constructor. 
     */
    stepping_gear(const stepping_gear& s) { copy(s); }
    
    /*! \brief Constructing a stepping_gear from a vector of rotor identifiers and the number of contacts the rotor stack has. 
     */    
    stepping_gear(vector<string>& rotor_identifiers, unsigned int alpha_size, rotor_stack *r_stack = NULL);

    /*! \brief Adds the ::rotor given by the parameter rotor_to_insert to the rotor_descriptor named by the value of parameter identifier.  
     */
    virtual void insert_rotor(string& identifier, boost::shared_ptr<rotor> rotor_to_insert);

    /*! \brief Same as insert_rotor(string&, boost::shared_ptr<rotor>), but identifier has to be specified by a C-style
     *         zero terminated string.  
     */
    virtual void insert_rotor(const char *identifier, boost::shared_ptr<rotor> rotor_to_insert) { string temp(identifier); insert_rotor(temp, rotor_to_insert); }

    /*! \brief Adds the rotors given by the parameter new_rotors to the rotor descriptors in this stepping gear. 
     *
     *  The size of the vector new_rotors has to be equal to the value returned by get_num_rotors(). I.e. for each
     *  rotor descriptor new_rotors has to contain a rotor. The rotors are inserted in the sequence given by new_rotors.
     */
    virtual void insert_all_rotors(vector<boost::shared_ptr<rotor> >& new_rotors);

    /*! \brief Adds the ::rotor_ring given by the parameter ring to the rotor_descriptor named by the value of parameter identifier.  
     */
    virtual void attach_ring(string& identifier, boost::shared_ptr<rotor_ring> ring);

    /*! \brief Same as attach_ring(string&, boost::shared_ptr<rotor_ring>), but identifier has to be specified by a C-style
     *         zero terminated string.  
     */
    virtual void attach_ring(const char *identifier, boost::shared_ptr<rotor_ring> ring) { string temp(identifier); attach_ring(temp, ring); }

    /*! \brief Same as attach_ring(string&, boost::shared_ptr<rotor_ring>), but instead of a string identifier the rotor number is used
     *         to identify the rotor_descriptor to which the rotor_ring has to be added.  
     */
    virtual void attach_ring(unsigned int rotor_num, boost::shared_ptr<rotor_ring> ring) { attach_ring(rotor_positions[rotor_num % num_rotors], ring); }

    /*! \brief Sets the offsets of all rotor rings in the rotor descriptors of this stepping gear to the values given in parameter new_offsets.
     *
     *  The size of the vector new_offsets has to be equal to the value returned by get_num_rotors(). I.e. for each
     *  rotor descriptor new_offsets has to contain an offset value. The offsets are applied to the rings in the sequence given by new_offsets.
     */
    virtual void set_all_ring_offsets(vector<unsigned int>& new_offsets);

    /*! \brief Permutes the rotor_rings which are currently attached to rotors in this stepping_gear among themselves and reattaches them to the rotors. 
     *         The size of the permutation given in the parameter ring_permutation has to be equal to stepping_gear::num_rotors.  
     */
    virtual void permute_rings(permutation& ring_permutation);

    /*! \brief Combines the functionality of insert_rotor and attach_ring. The rotor_descriptor is determined by the value of parameter identifier.  
     */
    virtual void insert_rotor_and_ring(string& identifier, pair<boost::shared_ptr<rotor>, boost::shared_ptr<rotor_ring> >& assembly);

    /*! \brief Same as insert_rotor_and_ring(string&, pair<boost::shared_ptr<rotor>, boost::shared_ptr<rotor_ring> >&), but 
     *         identifier has to be specified by a C-style zero terminated string.  
     */
    virtual void insert_rotor_and_ring(const char *identifier, pair<boost::shared_ptr<rotor>, boost::shared_ptr<rotor_ring> >& assembly) { string temp(identifier); insert_rotor_and_ring(temp, assembly); }

    /*! \brief This is the "main" method of any stepping_gear. It determines how the rotors in the rotor_stack are stepped. 
     *
     *  This method has to be reimplemented by all children of stepping_gear* in order to achieve the rotor motion, that is required for
     *  the implemented machine.
     */
    virtual void step_rotors() { char_count++; }

    /*! \brief Resets the stepping gear and the rotors in it to a neutral position. 
     *
     *  This method has to be reimplemented by all children of stepping_gear.
     */
    virtual void reset() { char_count = 0; }

    /*! \brief Resets char_count to zero.  
     */
    virtual void reset_counter() { char_count = 0; }
    
    /*! \brief Returns the number of rotors/rotor descriptors that are managed by this stepping_gear.  
     */    
    virtual unsigned int get_num_rotors() { return num_rotors; }

    /*! \brief Returns the rotor identifiers of the rotors that are managed by this stepping_gear.  
     */    
    virtual void get_rotor_identifiers(vector<string>& ids);

    /*! \brief Sets the rotor displacement for the rotor identified by the parameter identifier. The new rotor position is given by the value of 
     *         the parameter new_pos.
     */        
    virtual void set_rotor_displacement(string& identifier, unsigned int new_pos);

    /*! \brief Same as set_rotor_displacement(string&, unsigned int) but the rotor is identified by a C-style zero terminated string instead of
     *         an STL string.
     */        
    virtual void set_rotor_displacement(const char *identifier, unsigned int new_pos) { string temp(identifier); set_rotor_displacement(temp, new_pos); }

    /*! \brief Same as set_rotor_displacement(string&, unsigned int) but the rotor is identified by a rotor number instead of an STL string.
     */        
    virtual void set_rotor_displacement(unsigned rotor_num, unsigned int new_pos);

    /*! \brief Sets the rotor displacement for the rotor identified by the parameter identifier. The new rotor position is calculated by adding
     *         the value of the parameter distance to the current displacement of the rotor.
     */        
    virtual void move_rotor(string& identifier, unsigned int distance);

    /*! \brief Same as move_rotor(string&, unsigned int) but the rotor is identified by a C-style zero terminated string instead of
     *         an STL string.
     */  
    virtual void move_rotor(const char *identifier, unsigned int distance) { string temp(identifier); move_rotor(temp, distance); }    

    /*! \brief Same as move_rotor(string&, unsigned int) but the rotor is identified by a rotor number.
     */  
    virtual void move_rotor(unsigned rotor_num, unsigned int distance);
    
    /*! \brief Shorthand for move_rotor with distance 1. This means the rotor is stepped one position ahead.
     */      
    virtual void advance_rotor(string& identifier);

    /*! \brief Shorthand for move_rotor with distance 1. This means the rotor is stepped one position ahead.
     */      
    virtual void advance_rotor(const char *identifier) { string temp(identifier); advance_rotor(temp); }    

    /*! \brief Shorthand for move_rotor with distance get_size()-1. This means the rotor is stepped one position backwards.
     */      
    virtual void step_rotor_back(string& identifier);

    /*! \brief Shorthand for move_rotor with distance get_size()-1. This means the rotor is stepped one position backwards.
     */      
    virtual void step_rotor_back(const char *identifier) { string temp(identifier); step_rotor_back(temp); }    

    /*! \brief Returns the rotor displacement for the rotor identified by the parameter identifier.
     */        
    virtual unsigned int get_rotor_displacement(string& identifier);

    /*! \brief Same as get_rotor_displacement(string&) but the rotor is identified by a C-style zero terminated string instead of an STL string.
     */        
    virtual unsigned int get_rotor_displacement(const char *identifier) { string temp(identifier); return get_rotor_displacement(temp); }

    /*! \brief Same as get_rotor_displacement(string&) but the rotor is identified by a rotor number.
     */  
    virtual unsigned int get_rotor_displacement(unsigned int rotor_num);

    /*! \brief Returns the result of rotor_ring::get_pos() for the ring that is attached to the rotor specified by the identifier given
     *         in parameter identifier.
     *
     *  In case there is no ring attached to the specified rotor this method returns the same value as the corresponding version of
     *  get_rotor_displacement().
     */  
    virtual unsigned int get_ring_pos(string& identifier);

    /*! \brief Same as get_ring_pos(string&) but the rotor is identified by a C-style zero terminated string instead of an STL string.
     */        
    virtual unsigned int get_ring_pos(const char *identifier) { string temp(identifier); return get_ring_pos(temp); }

    /*! \brief Same as get_ring_pos(string&) but the rotor is identified by a rotor number.
     */  
    virtual unsigned int get_ring_pos(unsigned int rotor_num);

    /*! \brief Returns a reference to the rotor_descriptor that is identified by the value of the parameter identifier.
     */  
    virtual rotor_descriptor& get_descriptor(string& identifier);

    /*! \brief Same as get_descriptor(string&) but the rotor descriptor is identified by a C-style zero terminated string instead of an STL string.
     */
    virtual rotor_descriptor& get_descriptor(const char *identifier) { string temp(identifier); return get_descriptor(temp); }

    /*! \brief Same as get_descriptor(string&) but the rotor descriptor is identified by a rotor number instead of an STL string.
     */
    virtual rotor_descriptor& get_descriptor(unsigned int rotor_num);

    /*! \brief Sets the displacements of all rotors in this stepping_gear in one go to the values given in the parameter new_positions.
     */    
    virtual void set_all_displacements(vector<unsigned int>& new_positions);

    /*! \brief Fills the vector specified by the parameter current_positions with the the displacements of all rotors in this stepping_gear.
     */    
    virtual void get_all_displacements(vector<unsigned int>& current_positions);

    /*! \brief Restores a saved state contained in the given KeyFile object into this stepping_gear object. This method returns true 
     *         if an error is encountered while restoring the state.
     */        
    virtual bool load_ini(Glib::KeyFile& ini_file);

    /*! \brief Restores a saved state contained in a file that is named by the file_name parameter into this stepping_gear object.
     *         This method returns *true* if an error is encountered while restoring the state.
     */        
    virtual bool load(string& file_name);

    /*! \brief Same as load(string&), but the file name is given in a C-style zero terminated string instead of an STL string.
     */    
    virtual bool load(const char* file_name) { string temp(file_name); return load(temp); }

    /*! \brief Saves the state of this stepping_gear in the KeyFile object given in parameter ini_file.
     */        
    virtual void save_ini(Glib::KeyFile& ini_file);

    /*! \brief Saves the state of this stepping_gear in a KeyFile which is stored in the file system under the name given by 
     *         the parameter file_name. The method returns *true* in case an error is encountered while writing the state into
     *         the file.
     */    
    virtual bool save(string& file_name);

    /*! \brief Same as save(string&), but file name is given in a C style zero terminated string instead of an STL string.
     */    
    virtual bool save(const char* file_name) { string temp(file_name); return save(temp); }

    /*! \brief Returns the number of characters that have been processed by this stepping_gear object.
     */        
    virtual uint64_t get_char_count() { return char_count; }

    /*! \brief Sets the char_count member to zero.
     */    
    virtual void set_char_count(uint64_t new_char_count) { char_count = new_char_count; }

    /*! \brief Returns the number of contacts the rotors have, which are managed by this stepping_gear.
     */        
    virtual unsigned int get_size() { return alphabet_size; }

    /*! \brief Returns a reference to the rotor_stack managed by this stepping_gear.
     */            
    virtual rotor_stack& get_stack() { return *stack; }

    /*! \brief Assignment operator.
     */            
    virtual stepping_gear& operator=(const stepping_gear& s) { copy(s); return *this; }        

    /*! \brief Destructor.
     */            
    virtual ~stepping_gear();

protected:
    /*! \brief Returns true if the ring data of the ring that is attached to the rotor identified by the value of the parameter identifier
     *         is not zero. If the value of the parameter offset is different from zero then not the ring data of the current position is
     *         checked but the ring data that is offset steps ahead from the current position.
     *
     *  This is a helper method that is useful when a notch on a rotor ring of a real machine is modeled by a non zero value in the ring data
     *  of the corresponding ::rotor_ring object. This is used in the implementation of all Enigma variants, the Typex, the Schluesselgeraet 39,
     *  the Nema and the KL7.
     */        
    virtual bool rotor_is_at_notch(string& identifier, unsigned int offset = 0);

    /*! \brief Same as rotor_is_at_notch(string&, unsigned int), but rotor identifier is given in a C style zero terminated string instead of
     *         an STL string.
     */
    virtual bool rotor_is_at_notch(const char *identifier, unsigned int offset = 0);

    /*! \brief This method is intended to be reimplemented by those children of stepping_gear that have to save additional state information
     *         for each of the rotor_descriptors that it controls. 
     *
     *  The rotor descriptor for which additional data is to be saved is specified by the value of the parameter identifier. The additional state
     *  information to be saved could for instance be contained in the key value pairs of the rotor descriptor. This method is called for each
     *  of the rotor identifiers known to this stepping gear when any of the save methods is called.
     */              
    virtual void save_additional_components(string& identifier, Glib::KeyFile& ini_file) { ; }

    /*! \brief This method is intended to be reimplemented by those children of stepping_gear that have to load additional state information
     *         for each of the rotor_descriptors that it controls. 
     *
     *  The rotor descriptor for which additional data is to be loaded is specified by the value of the parameter identifier. The additional state
     *  information to be loaded could for instance be written to the key value pairs of the rotor descriptor. This method is called for each
     *  of the rotor identifiers known to this stepping gear when any of the load methods is called.
     */     
    virtual bool load_additional_components(string& identifier, Glib::KeyFile& ini_file) { return false; } 
    
    /*! \brief Helper method used by the copy constructor and the assignment operator.
     */            
    void copy(const stepping_gear& s);       

    /*! \brief Holds pointers to the rotor descriptors in use in this stepping_gear.
     */          
    rotor_descriptor *descriptors[MAX_ROTORS];

    /*! \brief Holds the actual rotor descriptors in use in this stepping_gear. Each rotor_descriptor is placed in an STL map and 
     *         referenced by a symbolic name, the rotor identifier.
     */          
    map<string, rotor_descriptor> rotors;

    /*! \brief Holds the symbolic rotor identifiers for the rotor_descritptors in use in this stepping gear. The sequence of the
     *         identifiers in this vector is such that it matches the sequence of pointers in stepping_gear::descriptors
     */          
    vector<string> rotor_positions;

    /*! \brief Holds the number of rotor_descriptors/rotors in use in this stepping_gear.
     */          
    unsigned int num_rotors;

    /*! \brief Holds the number of times the step_rotors() method has been called on this stepping_gear. This may be used to count
     *         the number of characters that have been processed by this machine.
     */          
    uint64_t char_count;

    /*! \brief Holds the number of contacts the rotors in this machine have.
     */          
    unsigned int alphabet_size;

    /*! \brief Holds the rotor_stack that is controlled by this stepping_gear.
     */          
    rotor_stack *stack;
};

/*! \brief A class that implements a stepping_gear that steps its rotors in the fashion of an odometer.
 *
 *  As this special kind of stepping_gear is not used in the implementation of any real machine it is intended to serve as an 
 *  example for the simplest kind of stepping gear and is used in test cases. With the odometer_stepping_gear the rotor at
 *  index 0 steps always and the other rotors step if the one next to it (with lower index) reaches position zero.
 */ 
class odometer_stepping_gear : public stepping_gear {
public:

    /*! \brief Constructing an odometer_stepping_gear from a vector of rotor identifiers and the number of contacts the rotor stack has. 
     */ 
    odometer_stepping_gear(vector<string>& rotor_identifiers, unsigned int alpha_size) : stepping_gear(rotor_identifiers, alpha_size) { ; }

    /*! \brief Implements stepping as described in the class overview.
     */     
    virtual void step_rotors();

    /*! \brief Sets the displacement of all rotors and the offsets of their (optional) rings to zero.
     */     
    virtual void reset();
    
    /*! \brief Prints the displacement of all the rotors to stdout.
     */         
    virtual void print_displacements();

    /*! \brief Destructor. Does nothing.
     */                
    virtual ~odometer_stepping_gear() { ; }
};

#endif /* __stepping_h__ */

