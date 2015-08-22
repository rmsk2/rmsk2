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

#ifndef __enigma_uhr_h__
#define __enigma_uhr_h__

#include<map>
#include<string>
#include<rmsk_globals.h>
#include<rotor.h>

/*! \file enigma_uhr.h
 *  \brief This file contains the header for the class that implements the Enigma Uhr
 */ 
 

/*! \brief A class that implements the Enigma Uhr.
 *
 *  Instances of this class are intended to be used as input transforms in Enigma simulators. See 
 *  rotor_machine::set_input_transform(boost::shared_ptr<encryption_transform> t). For a nice description
 *  of the Uhr see for instance http://www.cryptomuseum.com/crypto/enigma/uhr/index.htm .
 *
 *  The main cryptographic element of the Enigma Uhr is a scrambler disc that has 40 outer contacts and
 *  40 inner contacts, which are connected in a permuted way. 
 *  
 *  The connection of the Uhr to the Enigma is provided by 20 cables each of which contains two wires and 
 *  ends in a plug with two pins to which the wires are connected. One pin is thicker than the other. 
 *  These plugs have to be placed in 20 of the 26 correponding sockets on the Enigma plugboard.
 *
 *  The plugs come in two groups (1a, 2a, ...., 10a coloured red) and (1b, 2b, ..., 10b couloured black or white).
 *  Inside the Uhr the 20 wires belonging to the red plugs are connected to the outer contacts of the scrambler disc
 *  and the 20 wires of the black/white ones to the inner contacts of the scrambler disc.
 *
 *  The wire of the thick pin on plug 1a is connected to the outer contact 0 of the scrambler disc and the
 *  wire of 1a's thin pin is connected to contact 2 of the scrambler disc. For plug 2a outer contacts 4 (thick)
 *  and 6 (thin) are used and so on. So for the red plugs the wire of the thick pin of plug k_a is connected to 
 *  outer contact 4 * (k - 1) and the the wire of the thin pin to outer contact (4 * (k - 1)) + 2 of the 
 *  scrambler disc.
 *
 *  A similar thing is done with the black/white plugs but here things are a little more complicated. The 
 *  corresponding formulas for determining the number of the inner contact of the scrambler disc to which
 *  the wires emanating from the thick and thin pins of plug k_b are connected is as follows: 4 * pi(k - 1)
 *  [thick pins] and (4 * pi(k - 1)) + 2  [thin pins]. Where pi is the permutation 1, 4, 7, 9, 6, 3, 0, 2, 5, 8.
 *
 *  When operating the Enigma current passes through the Uhr two times for each key press. Once "on the way in"
 *  before entering the rotor bank and once "on the way back" after having been reflected back through the
 *  rotor bank. On the way "in" current enters the Uhr through the wires which are connected to the thick
 *  pins and leaves it through a wire belonging to a thin pin. On the way back current enters the Uhr through
 *  a wire belonging to a thin pin and leaves it again on a wire belonging to a thick pin.
 *
 *  A key setting (or cabling) is applied to the Uhr in a straightforward way. It is easily explained by an
 *  example. Let adcnetflgijvkzpuqywx be Stecker setting for the day, then plug 1a is placed in socket a of the
 *  plugboard, plug 1b in socket d, plug 2a in socket c, plug 2b in socket n and so on.
 */ 
class enigma_uhr : public encryption_transform {
public:
    /*! \brief Default constructor.
     */                   
    enigma_uhr();

    /*! \brief Transforms in_char on the way in before entering the rotor bank.
     */                           
    virtual unsigned int encrypt(unsigned int in_char);

    /*! \brief Transforms in_char on the way back after leaving the rotor bank.
     */                           
    virtual unsigned int decrypt(unsigned int in_char);
    virtual unsigned int get_size() { return rmsk::std_alpha()->get_size(); }
    
    /*! \brief Returns the current dial pos.
     */                       
    virtual unsigned int get_dial_pos() { return dial_pos; }
    
    /*! \brief Sets the current dial pos. 
     *
     *  Values given in parameter dial_pos are reduced mod 40.
     */                           
    virtual void set_dial_pos(unsigned int new_dial_pos) { dial_pos = new_dial_pos % 40; }

    /*! \brief Sets the current cabling. The format used is a 20 character C-style zero terminated string
     *         where each character represents an Uhr plug inserted into the plugboard. The characters on the 
     *         even indices determine the sockets to which outer red plugs have been steckered and the
     *         characters on the odd indices do the same for the black/white plugs.
     *         
     *  All characters have to be lower case characters in the range from a-z. Upper case characters must not be used.
     */                           
    virtual void set_cabling(const char *cable_spec) { string temp(cable_spec); set_cabling(temp); }

    /*! \brief Sets the current cabling. The parameter plugs has to specify exactly ten lower case character pairs.
     *         The first element of each pair determines the character to which the red plug is steckered and the
     *         second element the character to which the black/white plug is steckered.
     */                           
    virtual void set_cabling(vector<pair<char, char> >& plugs);
    
    /*! \brief Sets the current cabling. The format used is a 20 character STL string
     *         where each character represents an Uhr plug inserted into the plugboard. The characters on the 
     *         even indices determine the sockets to which outer red plugs have been steckered and the
     *         characters on the odd indices do the same for the black/white plugs.
     *
     *  All characters have to be lower case characters in the range from a-z. Upper case characters must not be used.
     */                               
    virtual void set_cabling(string& cable_spec);
    
    /*! \brief Returns the current cabling. The format used is a 20 character STL string consisting of 
     *         lower case characters where each two consecutive characters represent a plug inserted into the Uhr.
     */                                   
    virtual void get_cabling(string& cable_spec);

    /*! \brief Destructor.
     */                   
    virtual ~enigma_uhr() { ; }
    
protected:
    /*! \brief Holds the current dial position.
     */                   
    unsigned int dial_pos;
    
    /*! \brief The 40 contact scrambler disk is in essence a rotor with dial_pos as its displacement variable.
     */                       
    rotor scrambler_disk;
    
    /*! \brief Holds the permutation 1, 4, 7, 9, 6, 3, 0, 2, 5, 8 as described above
     */                           
    permutation inner_contacts;
    
    /*! \brief Stores the current cabling
     */                               
    string cable_settings;

    /*! \brief Maps the plug number 0, ..., 9 of the inner plug (black/white) to the
     *         plugboard socket into which it has been inserted
     */                                   
    map<unsigned int, unsigned int> inner_plugs;

    /*! \brief Maps the plug number 0, ..., 9 of the outer plug (red) to the
     *         plugboard socket into which it has been inserted
     */                                   
    map<unsigned int, unsigned int> outer_plugs;

    /*! \brief Inverse mapping of inner_plugs
     */                                       
    map<unsigned int, unsigned int> inner_plugs_inv;

    /*! \brief Inverse mapping of outer_plugs
     */                                       
    map<unsigned int, unsigned int> outer_plugs_inv;    
};


#endif /* __enigma_uhr_h__ */

