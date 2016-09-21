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

#ifndef __tlv_data_struct_h__
#define __tlv_data_struct_h__

/*! \file tlv_data_struct.h
 *  \brief Header file for helper classes that use on the lower level tlv_entry class to implement more complex data structures.
 */ 

#include<map>
#include<tlv_stream.h>

using namespace std;

/*! \brief A TLV class that allows to en/decode objects of type map<string, string> to/from TLV format.
 */
class tlv_map {
public:
    /*! \brief This method can be used to set the state of tlv_map objects in such a way that they represent
     *         the map object referenced through the parameter data_struct.
     */    
    virtual void to_tlv_entry(map<string, string>& data_struc);

    /*! \brief This method modifies the map object which is referenced by the parameter data_struct in such a way
     *         that it reflects the current state of this tlv_map object.
     */    
    virtual void tlv_convert(map<string, string>& data_struc);

    /*! \brief This method can be used to retrieve the current state of this tlv_map object.
     */    
    virtual tlv_entry& get_elements() {return elements; }

    /*! \brief This method can be used to set the current state of this tlv_map object. The parameter new_elements
     *         references a TLV encoded vector of tlv_entry objects that make up the new state. This method checks
     *         whether new_elements contains a valid encoding for a tlv_map object.
     *
     *  The method returns true, in case new_elements does not contain a valid encoding.  
     */    
    virtual bool set_elements(tlv_entry& new_elements);

    /*! \brief Destructor.
     */        
    virtual ~tlv_map() { ; }

protected:    
    /*! \brief Holds the current state of this tlv_map object. */    
    tlv_entry elements;
};


#endif /* __tlv_data_struct_h__ */
