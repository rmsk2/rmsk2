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

#ifndef __simple_mod_int_h__
#define __simple_mod_int_h__

/*! \file simple_mod_int.h
 *
 *  \brief Header file for the simple_mod_int class
 *
 */ 

#include<permutation.h>

/*! \brief A class that abstracts the notion of numbers which can be added and subtracted using modular arithmetic. 
 *
 *  It is intended to be a drop in replacement for normal unsigned ints and therefore can be used
 *  wherever one would use a normal unsigned int but without having to worry about modular arithmetic.
 *  Makes heavy use of inline methods for improved efficiency. Unfortunately it is still not efficient enough. 
 *  Therefore rmsk uses vanilla unsigned ints in a lot of places where a simple_mod_int would be more elegant. 
 *  
 */   
class simple_mod_int {
public:
    /*! \brief Constructs a simple_mod_int using a modulus and a value. The given value is mod-reduced by the constructor. 
     */    
    simple_mod_int(unsigned int val, unsigned int mod);
    
    /*! \brief Constructs a simple_mod_int using only a modulus. The value is set to 0. 
     */        
    simple_mod_int(unsigned int mod) { value = 0; modulus = mod; }
    
    /*! \brief Default constructor. Sets modulus to 1 and value to 0. 
     */    
    simple_mod_int() { value = 0; modulus = 1; }
    
    /*! \brief Conversion operator to convert a simple_mod_int to an unsgned int. Simply returns the current value. 
     */    
    inline operator unsigned int() { return value; }
    
    /*! \brief Operator + for modular addition. 
     */    
    inline simple_mod_int operator+(simple_mod_int& other) { simple_mod_int result(modulus); result.value = reduce(value + other.value); return result; }
    
    /*! \brief Operator - for modular subtraction. 
     */    
    inline simple_mod_int operator-(simple_mod_int& other) { simple_mod_int result(modulus); unsigned int temp = modulus - other.value; result.value = reduce(value + temp); return result;}
    
    /*! \brief Unary operator -. Returns the additive inverse of this simple_mod_int. 
     */    
    simple_mod_int operator-();
    
    /*! \brief Assignment operator. 
     */        
    inline simple_mod_int& operator=(const simple_mod_int& other) {  value = other.value; modulus = other.modulus; return *this; }
    
    /*! \brief Allows a normal unsigned int to be assigned to a simple_mod_int. The value is mod-reduced before it is used. 
     */        
    inline simple_mod_int& operator=(unsigned int val) { value = val % modulus; return *this; }
    
    /*! \brief Apply a given permutation to this simple_mod_int. 
     */        
    inline simple_mod_int permute(permutation* perm) { simple_mod_int result(modulus); result.value = perm->permute(value); return result; }
    
    /*! \brief Apply the inverse of a given permutation to this simple_mod_int. 
     */        
    inline simple_mod_int inv(permutation* perm) { simple_mod_int result(modulus); result.value = perm->inv(value); return result; }
    
    /*! \brief Increments this simple_mod_int. 
     */            
    inline void increment() { value = reduce(value + 1); }
    
    /*! \brief Decrements this simple_mod_int. 
     */                
    void decrement();
    
    /*! \brief Get the modulus of this simple_mod_int. 
     */                
    inline unsigned int get_mod() { return modulus; }
    
    /*! \brief Set the value of this simple_mod_int. new_value is mod-reduced before it is used. 
     */                
    void set_val(unsigned int new_value) { value = new_value % modulus; }
    
private:
    /*! \brief Mod-reduces the given value with respect to the current modulus. 
     */                
    inline unsigned int reduce(unsigned int data) { if (data >= modulus) { data -= modulus; } return data;}
    
    /*! \brief Current value of this simple_mod_int. 
     */
    unsigned int value;
    
    /*! \brief Current modulus of this simple_mod_int. 
     */    
    unsigned int modulus;
};


#endif /* __simple_mod_int_h__ */

