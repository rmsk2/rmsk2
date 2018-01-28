/***************************************************************************
 * Copyright 2018 Martin Grap
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

#ifndef __rotor_set_h__
#define __rotor_set_h__

/*! \file rotor_set.h
 *  \brief Header file for the rotor_set class.
 */ 

#include<vector>
#include<map>
#include<glibmm/keyfile.h>
#include<rotor_ring.h>
#include<rmsk_globals.h>

using namespace std;

/*! \brief A class that knows how to construct ::rotor and rotor_ring objects.
 *
 *  Apart from the desire for abstraction the main purpose of this class is to provide rotor machines with
 *  the possibility to use several sets of rotors. The rotor_machine base class offers methods that allow
 *  to add additional rotor_sets to be used when constructing rotors and rings. Each rotor and ring is
 *  assigned a numerical id. While all rotors and rings have to have different ids, it is normal to have
 *  a rotor and a ring of the same id.
 *
 *  One assumption when originally designing the interface of this class was that there is a one to one
 *  relationship between rotors and rings, as is for instance the case with the Enigma. The KL7 implementation
 *  broke that assumption because the KL7 has 13 rotors and 11 notch rings to choose from. In order to keep
 *  the class usable it is has been made possible to add "single" rotors or rings to the rotor_set. The price
 *  that has to be paid  for that is that a dummy rotor permutation (identity) or dummy ring data (all zeros) 
 *  is created for each single rotor or ring added to the set.
 *
 *  ToDo: This is a kludge that has to be removed. The main work lies in adapting the load and save methods.
 *  
 */ 
class rotor_set {
public:
    /*! \brief Creates an empty rotor set, that contains rotors which all have r_size contacts and rings
     *         of equal size. 
     */
    rotor_set(unsigned r_size) { rotor_size = r_size;  rset_name = DEFAULT_SET;}

    /*! \brief Default constructor. Creates an empty rotor set, that contains rotors which all have a size
     *         of zero. 
     */
    rotor_set() { rotor_size = 0; rset_name = DEFAULT_SET; }

    /*! \brief Allows to change the rotor set name. The parameter new_name has to contain the new name.
     */        
    void set_name(string& new_name) { rset_name = new_name; }

    /*! \brief Returns the name of the rotor set.
     */            
    string get_name() { return rset_name; }
    
    /*! \brief Returns a rotor/rotor_ring pair as specified by the supplied r_id parameter. If the rotor id
     *         is unknown an exception of type runtime_error is thrown.
     */    
    virtual pair<boost::shared_ptr<rotor>, boost::shared_ptr<rotor_ring> > make_rotor(rotor_id r_id);

    /*! \brief Returns a rotor_ring as specified by the supplied ring_id parameter. If the ring_id is unknown
     *         in the context of this rotor_set a ring with an all zero ring data is returned.
     */    
    virtual boost::shared_ptr<rotor_ring> make_ring(unsigned int ring_id);
    
    /*! \brief Returns all rotor ids known to this rotor_set. The vector ids is sorted in ascending order.
     */        
    virtual void get_ids(vector<unsigned int>& ids);

    /*! \brief Returns all ring ids known to this rotor_set. The vector ids is sorted in ascending order.
     */        
    virtual void get_ring_ids(vector<unsigned int>& ids);    
    
    /*! \brief Returns the number of permutations that are known to this rotor_set.
     */            
    virtual unsigned int get_num_rotors() { return perms.size(); }

    /*! \brief Returns the number of ring data vectors that are known to this rotor_set.
     */            
    virtual unsigned int get_num_rings() { return ring_data_vals.size(); }

    /*! \brief Returns the number of contacts the rotors/permutations have that are known to this rotor_set.
     */            
    virtual unsigned int get_rotor_size() { return rotor_size; }

    /*! \brief Adds a permutation to this rotor_set. The newly added permutation can be referenced by the value given
     *         in theparameter desired_id.
     */                
    virtual void add_rotor(unsigned int desired_id, vector<unsigned int> permutation);

    /*! \brief Adds a vector of ring data to this rotor_set. The newly added ring data can be referenced by the value given
     *         in the parameter desired_id.
     */                
    virtual void add_ring(unsigned int desired_id, vector<unsigned int> ring_data);    

    /*! \brief Combines add_rotor and add_ring.
     */                
    virtual void add_rotor_and_ring(unsigned int desired_id, vector<unsigned int> permutation, vector<unsigned int> ring_data);

    /*! \brief Replaces all information in this rotor_set with new, randomly chosen rotor permutations and ring data vectors. The
     *         parameter num_rotors specifies the number of rotors and rings to generate. The ids of these new entities are the
     *         values 0, ..., num_rotors-1. 
     *
     *  The contents of the created ring data vectors is implicitly determined by the rotor_ring_random_source object to which
     *  the parameter rand_source points.
     */    
    virtual void randomize(unsigned int num_rotors, rotor_ring_random_source *rand_source);

    /*! \brief Same as randomize(unsigned int, rotor_ring_random_source), but with this method the desired ids can be specified
     *         by the contents of the vector desired_rotor_ids. 
     */                    
    virtual void randomize(vector<unsigned int> desired_rotor_ids, rotor_ring_random_source *rand_source);

    /*! \brief This method replaces all permutations in this rotor set by random ones. The parameter rand_gen points to the random
     *         generator object used for generating the new permutations. If rand_gen is NULL, then a urandom_generator object is utilized.
     */                        
    virtual void replace_permutations(random_generator *rand_gen = NULL);    

    /*! \brief Replaces the ring data of all the rings whose ids are contained in the ids_to_change vector with the data specified in
     *         the new_ring_data_parameter.
     */                        
    virtual void replace_ring_data(vector<unsigned int>& ids_to_change, vector<unsigned int>& new_ring_data);    

    /*! \brief Copies part of this rotor set into the rotor_set specified by parameter new_set. The parameters rotor_ids_to_copy and
     *         ring_ids_to_copy specify the rotors and rings to copy into the new set.
     */                        
    virtual void slice_rotor_set(rotor_set& new_set, vector<unsigned int>& rotor_ids_to_copy, vector<unsigned int>& ring_ids_to_copy);    

    /*! \brief This method changes the ids of the rotors and rings in this set to new values. The parameter rotor_id_mapping maps the
     *         old rotor ids to the new values. The parameter ring_id_mapping does the same for the ring ids.
     */                            
    virtual void change_ids(map<unsigned int, unsigned int>& rotor_id_mapping, map<unsigned int, unsigned int>& ring_id_mapping);

    /*! \brief Empties this rotor_set.
     */                        
    virtual void clear() { perms.clear(); ring_data_vals.clear(); }

    /*! \brief Replaces the set of rotor ids that must not be changed by the replace_permutations() method.
     */                            
    virtual void set_const_ids(set<unsigned int>& new_const_ids) { const_ids = new_const_ids; }

    /*! \brief Restores a saved state contained in the given KeyFile object into this rotor_set object. This method returns true 
     *         if an error is encountered while restoring the state.
     */       
    virtual bool load_ini(Glib::KeyFile& ini_file);
    
    /*! \brief Restores a saved state contained in a file that is named by the file_name parameter into this rotor_set object.
     *         This method returns *true* if an error is encountered while restoring the state.
     */            
    virtual bool load(string& file_name);
    
    /*! \brief Same as load(string&), but the file name is given in a C-style zero terminated string instead of an STL string.
     */        
    virtual bool load(const char* file_name) { string temp(file_name); return load(temp); }
         
    /*! \brief Saves the state of this rotor_set in the KeyFile object given in parameter ini_file.
     */             
    virtual void save_ini(Glib::KeyFile& ini_file);

    /*! \brief Saves the state of this rotor_set in the file system using a file the name of which is given by the parameter
     *         file_name. The method returns *true* in case an error is encountered while writing the state into the file.
     */        
    virtual bool save(string& file_name);
    
    /*! \brief Same as save(string&), but file name is given in a C style zero terminated string instead of an STL string.
     */        
    virtual bool save(const char* file_name) { string temp(file_name); return save(temp); }    

    /*! \brief Destructor.
     */        
    virtual ~rotor_set() { ; } 
    
protected:

    /*! \brief Holds the size (i.e. the number of contacts) of the rotors which are known to this rotor_set.
     */    
    unsigned int rotor_size;
    

    /*! \brief Fills the array referenced by the parameter new_vals whith random ring data as determined by the
     *         object to which rand_source points.
     */        
    virtual void get_rand_ring_data(vector<unsigned int>& new_vals, rotor_ring_random_source *rand_source);

    /*! \brief Fills the array referenced by the parameter new_perm_vals whith the vector representation of a
     *         permutation randomly chosen under the influence of the random_generator to which the parameter 
     *         rand_gen points.
     */        
    virtual void get_rand_perm_data(vector<unsigned int>& new_perm_vals, random_generator *rand_gen);    

    /*! \brief Holds the known rotor permutations and rotor ids as well as the mapping of rotor id to
     *         permutations.
     */            
    map<unsigned int, vector<unsigned int> > perms;

    /*! \brief Holds the ids of the rotors that must not be changed by the replace_permutations() method.
     */                
    set<unsigned int> const_ids;    

    /*! \brief Holds the known ring data vectors and ring ids as well as the mapping of ring id to
     *         ring data vector.
     */            
    map<unsigned int, vector<unsigned int> > ring_data_vals;

    /*! \brief Holds the name of the rotor set by which it can be referred to in a ::rotor_machine object.
     */                    
    string rset_name;  
};

#endif /* rotor_set_h__ */

