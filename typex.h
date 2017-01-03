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

#ifndef __typex_h__
#define __typex_h__

/*! \file typex.h
 *  \brief Header file for a collection of classes that implement a simulator for the Typex machine.
 */

#include<glibmm.h>
#include<enigma_sim.h>
#include<enigma_rotor_set.h>

#define STATOR1 "stator1"
#define STATOR2 "stator2"
#define Y269 "Y269"


using namespace Glib;

/*! \brief A class that allows to retrieve the rotor sets as they have been defined for the Typex.
 */
class typex_rotor_sets {
public:
    /*! \brief This method allows to retrieve a Typex rotor set by name. The name is specified as a zero terminated
     *         C-string through the parameter set_name.
     */
    static rotor_set& get_rotor_set(const char* set_name);

protected:
    /*! \brief Holds the known Typex rotor sets */
    static map<string, rotor_set*> typex_sets;
};

/*! \brief A class that implements the stepping_gear of the Typex.
 */
class typex_stepper : public enigma_stepper {
public:
    /*! \brief Constructor. The vector rotor_identifiers has to specifiy the symbolic names of the rotor positions.     
     */
    typex_stepper(vector<string>& rotor_identifiers) : enigma_stepper(rotor_identifiers) { ; }

    /*! \brief Resets the positions and ring settings of all rotors in this simulator. This method calls the reset()
     *         method of the parent class and additionally sets the positions of Typex' two stators to 'a'. It also
     *         resets the rings to position 'a'.     
     */    
    virtual void reset();    
    
     /*! \brief Destructor.     
     */   
    virtual ~typex_stepper() { ; }
};

/*! \brief A class that implements a simulator for the Typex machine.
 *
 *  This class shows very clearly that the Typex is essentially an Enigma (as can be inferred from its full name RAF Enigma
 *  with Type-X extensions), because nearly no additional code has to be added. 
 */
class typex : public enigma_family_base {
public:
    /*! \brief Constructor. The parameters of this method use the rotor_id structure to specify which rotors have to placed in
     *         the machine in what positions.
     *
     *  The biggest cryptographic difference between the Typex and the Eigma is that its rotors can be placed in reverse in
     *  the machine. This makes it necessary to use the rotor_id structure instead of simple unsigned ints to specify the
     *  individual rotors.
     */
    typex(unsigned int ukw_id, rotor_id slow_id, rotor_id middle_id, rotor_id fast_id, rotor_id stat2_id, rotor_id stat1_id);

    /*! \brief Allows to change the involution used in the reflector. The vector has to contain exactly 13 pairs of lower case
     *         chars from the range a-z that specify an involution.
     */
    virtual void set_reflector(vector<pair<char, char> >& data);
    
    /*! \brief This method randomizes the state of this typex object. If this method returns true an error occurred and the
     *         object's state is unchanged.
     *
     *  The parameter param is ignored.
     */
    virtual bool randomize(string& param);

     /*! \brief Destructor.     
     */       
    virtual ~typex() { ; }        
};

#endif /* __typex_h__ */

