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

#ifndef __rotor_h__
#define __rotor_h__

/*! \file rotor.h
 *
 *  \brief Header file for the rotor class
 *
 */ 

#include<set>
#include<boost/shared_ptr.hpp>
#include<transforms.h>
#include<permutation.h>
#include<simple_mod_int.h>

/*! \brief A class that abstracts the notion of a rotor in a rotor machine. 
 *
 *  A rotor has a permutation, the rotor permutation, and a displacement. The displacement together with the rotor permutation
 *  (referenced through perm) determine the resulting permutation the rotor implements at the current displacement. The displacement
 *  of the rotor is the number of positions it has been rotated with respect to the zero position. A rotor is the most basic
 *  cryptographic element of any rotor machine. In order to allow to capture all relevant information about a rotor in one place
 *  (the rotor_descriptor) the rotor displacement is external to the rotor and only referenced through a pointer.
 *  
 */   
class rotor : public encryption_transform {
public:
    /*! \brief Constructs a rotor using a ::permutation p and a displacement variable d. 
     */
    rotor(boost::shared_ptr<permutation> p, unsigned int *d);
    
    /*! \brief Constructs a rotor using only a displacement variable d. The rotor permutation remains uninitialized. 
     */    
    rotor(unsigned int *d);
    
    /*! \brief Default constructor sets displacement variable to a dummy value. The rotor permutation remains uninitialized. 
     */    
    rotor();
    
    /*! \brief Sets the rotor permutation to a new value p. 
     */    
    virtual void set_perm(boost::shared_ptr<permutation> p) { perm = p; perm_size = p->get_size(); }
    
    /*! \brief Returns the current rotor permuation as a "naked" pointer to a permutation. 
     */    
    virtual permutation *get_perm() { return perm.get(); }
    
    /*! \brief Sets the displacement variable to a new pointer given in d. 
     */        
    virtual void set_displacement_var(unsigned int *d) { displacement = d; }
    
    /*! \brief Getter for the displacement variable. 
     */    
    virtual unsigned int *get_displacement_var() { return displacement; }
    
    /*! \brief Returns the rotor size, which is equal to the size of the rotor permutation. 
     */    
    virtual unsigned int get_size() { return perm->get_size(); }
    
    /*! \brief Encrypt using the rotor permutation and the value of the displacement variable. 
     */    
    virtual unsigned int encrypt(unsigned int c) { return rot_enc(c); }
    
    /*! \brief Decrypt using the rotor permutation and the value of the displacement variable. 
     */        
    virtual unsigned int decrypt(unsigned int c) { return rot_dec(c); }
    
    /*! \brief Returns the value of the unsigned int the displacement variable currently points to. 
     */        
    virtual unsigned int get_displacement() { return *displacement; }

    /*! \brief Encrypt using the rotor permutation and the value of the displacement variable. 
     *         This implementation strives for maximum efficiency and therefore is inlined and avoids
     *         division or modulo operations.
     */    
    inline unsigned int rot_enc(unsigned int c)
    {   
        c += *displacement;
        
        if (c >= perm_size)
        {
            c -= perm_size;
        }
        
        c = perm->permute(c);
        
        c -= *displacement;
        
        if (c & 0x80000000)
        {
            c += perm_size; 
        }
        
        return c;
    }

    /*! \brief Decrypt using the rotor permutation and the value of the displaceent variable. 
     *         This implementation strives for maximum efficiency and therefore is inlined and avoids
     *         division or modulo operations.
     */        
    inline unsigned int rot_dec(unsigned int c)    
    {
        c += *displacement;
        
        if (c >= perm_size)
        {
            c -= perm_size;
        }
        
        c = perm->inv(c);
        
        c -= *displacement;
        
        if (c & 0x80000000)
        {
            c += perm_size; 
        }
        
        return c;
    }

    /*! \brief Destructor. 
     */        
    virtual ~rotor();
    
protected:
    
    /*! \brief Dummy used when no displacement variable is available. 
     */        
    unsigned int dummy;
    
    /*! \brief Shared pointer referencing the rotor permutation. 
     */        
    boost::shared_ptr<permutation> perm;
    
    /*! \brief Displacement variable. 
     */            
    unsigned int *displacement;
    
    /*! \brief Size of the rotor permutation. Set during construction used during en/decryption for efficiency. 
     */            
    unsigned int perm_size;
};

#endif /* __rotor_h__ */

