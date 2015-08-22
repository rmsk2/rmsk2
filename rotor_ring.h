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

#ifndef __rotor_ring_h__
#define __rotor_ring_h__

/*! \file rotor_ring.h
 *  \brief Header file for the rotor_ring class.
 */

#include<vector>
#include<rotor.h>

using namespace std;


/*! \brief A class that implements the concept of a ring that can be attached to or associated with a wired ::rotor.
 *         
 *   Associated with a rotor_ring is so the called ring data. For each contact of the wired rotor there is an unsigned
 *   int of ring data that correspnds to this contact. On rotor rings of real cipher machines the ring data often
 *   takes the form of notches that are used to control the rotor stepping mechanism. Rotor rings can be rotated with
 *   respect to the wired ::rotor to which they are attached. The number of positions by which the ring is rotated is
 *   called the offset. The implementation of this class makes use of C-style arrays for efficiency.
 *   
 */
class rotor_ring {
public:
    /*! \brief Copy constructor. 
     */    
    rotor_ring(const rotor_ring& r) { ring_data = NULL; copy(r); }
    
    /*! \brief Constructs a rotor ring given a ::rotor and ring data in form of a vector of unsigned ints.
     *  
     *  The ring data will be set even if the ::rotor r is NULL. If r is non-NULL then the rotor_ring is attached
     *  to the rotor to which r points.
     */    
    rotor_ring(rotor *r, vector<unsigned int>& data);

    /*! \brief Constructs a rotor ring only from the desired ring data. 
     */    
    rotor_ring(vector<unsigned int>& data);

    /*! \brief Set the offset by which the ring should be rotated with respect to the wired rotor it is attached to.
     *
     *  The displacement of the wired rotor is not changed by this method. Only the ring is rotated to a new offset. 
     */  
    virtual void set_offset(unsigned int new_offset) { offset = new_offset; }

    /*! \brief Returns the current offset by which the ring has been rotated with respect to the wired rotor. 
     */    
    virtual unsigned int get_offset() { return offset; }

    /*! \brief This method moves the ring to the position given in new_pos. The displacement of the rotor is adapted
               so that the offset remains constant.
     *
     *  Calling this method has the effect of moving the ring together with the rotor. After calling this method new_pos
     *  would be shown in the rotor window of a rotor to which this ring is attached when it is assumed that the positions 
     *  0 ... rot_size-1 would be inscribed on the ring's circumfence. 
     */  
    virtual void set_pos(unsigned int new_pos);
    
    /*! \brief Returns the current position that would be shown in the rotor window of a rotor to which this ring is attached when
     *         it is assumed that the positions 0 ... rot_size-1 would be inscribed on the ring's circumfence. 
     */        
    inline unsigned int get_pos()
    {
        unsigned int result = rot->get_displacement();
        
        result += offset;
        
        if (result >= rot_size)
        {
            result -= rot_size;
        }
        
        return result;
    }

    /*! \brief Returns the ring data at the position which is as many steps ahead as specified by the parameter offset 
     *         from the position returned by get_pos().
     *
     *  This method allows to read the ring data at a position different from the current ring position (which is returned 
     *  by get_pos()).
     */    
    virtual unsigned int get_current_data(unsigned int offset);

    /*! \brief Returns the ring data at the position returned by get_pos().
     */
    inline  unsigned int get_current_data() { return ring_data[get_pos()];}
    
    /*! \brief Sets new ring data on this rotor_ring.
     */
    virtual void set_ring_data(vector<unsigned int>& r_dat) { set_ring_data_priv(r_dat); }

    /*! \brief Updates r_dat with the ring data of this rotor_ring.
     */
    virtual void get_ring_data(vector<unsigned int>& r_dat) { get_ring_data_priv(r_dat); }
    
    /*! \brief Returns the ::rotor to which this ring is attached.
     */    
    virtual rotor *get_rotor() { return rot; }
    
    /*! \brief Attaches this rotor_ring to the ::rotor specified by the parameter r.
     */        
    virtual void set_rotor(rotor *r) { rot = r; if (rot != NULL) { rot_size = rot->get_size(); } }

    /*! \brief Assignment operator.
     */            
    virtual rotor_ring& operator=(const rotor_ring& r) { copy(r); return *this; }
    
    /*! \brief Destructor. The ::rotor to which this rotor_ring is attched is not deleted by the desctructor.
     */            
    virtual ~rotor_ring() { delete[] ring_data; }

protected:

    /*! \brief Helper method which is used by copy constructor and assignment operator.
     */            
    void copy(const rotor_ring& p);
    
    /*! \brief Helper method for setting the ring data.
     */            
    void set_ring_data_priv(vector<unsigned int>& r_dat);

    /*! \brief Helper method for reading the ring data.
     */            
    void get_ring_data_priv(vector<unsigned int>& r_dat);    

    /*! \brief Points to the C-style array which holds the ring data of this rotor_ring.
     */            
    unsigned int *ring_data;
    
    /*! \brief Points to the ::rotor to which this rotor_ring is attached.
     */                
    rotor *rot;
    
    /*! \brief Holds the size of the ring data. Has to match the size of the rotor.
     */    
    unsigned int rot_size;
    
    /*! \brief Holds the number of steps this rotor_ring has been rotated with respect to the ::rotor to
     *         which it is attached.
     */                
    unsigned int offset;
};

#endif /* __rotor_ring_h__ */

