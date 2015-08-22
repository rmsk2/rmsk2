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

#ifndef __transforms_h__
#define __transforms_h__

/*! \file transforms.h
 *  \brief Header file for several abstract base classes/interfaces.
 */ 


/*! \brief An abstract class that describes the interface of a thing that can encrypt and decrypt unsigned ints in the
 *         range 0 to get_size()-1.
 */ 
class encryption_transform {
public:

    /*! \brief Encrypt the input value given in parameter in_char. 
     */     
    virtual unsigned int encrypt(unsigned int in_char) = 0;

    /*! \brief Decrypt the input value given in parameter in_char. 
     */     
    virtual unsigned int decrypt(unsigned int in_char) = 0;

    /*! \brief Returns the number of input symbols that this encryption_transform instance can handle.
     *         In the context of rmsk this normally tells the caller the number of contacts in the underlying
     *         rotor_machine. 
     */     
    virtual unsigned int get_size() = 0;

    /*! \brief Destructor.
     */ 
    virtual ~encryption_transform() { ; }
};

/*! \brief An abstract class describing an encryption_transform that also knows how to step rotors
 */ 

class machine_transform : public encryption_transform {
public:

    /*! \brief Steps the rotors of the underlying rotor machine. 
     */     
    virtual void step_rotors() = 0;

    /*! \brief Destructor.
     */ 
    virtual ~machine_transform() { ; }
};

#endif /* __transforms_h__ */

