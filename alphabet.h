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

/*! \file alphabet.h
 *  \brief Contains implementation of an alphabet in form of a class template.
 */ 
#ifndef __alphabet_h__
#define __alphabet_h__

#include<map>
#include<stdexcept>
#include<set>
#include<utility>
#include<iostream>
#include<string>
#include<permutation.h>
#include<transforms.h>


/*! \brief A template class that abstracts the notion of an alphabet.
 *
 *  An alphabet is a mapping of a vector of symbols or characters of size n to the numbers between 0 and n-1.
 *  This class provides the implementation of an alphabet in form of a class template. In rmsk all machines
 *  operate not on characters but on non-negative integers. This is done in order to decouple the machine implementations
 *  from the set of symbols used to represent input and output characters. This helps to keep the low level machine 
 *  implementations independent from any issues that may arise from displaying characters in graphical or text based
 *  user interfaces. Additionally it allows to implement the cryptographic core of each machine without paying attention
 *  to operational details like figure and letter shifting.   
 *  
 *  An alphabet is the means rmsk provides
 *  for mapping input characters to integers and integers to output characters. Machines can use different alphabets for input
 *  and output symbols or even several input alphabets. The template parameter T specifies the type of the symbols or characters.
 *
 */ 
template<class T> class alphabet {
public:
    /*! \brief Constructs an alphabet from a vector of type T. 
     */
    alphabet(const vector<T>& m);
    
    /*! \brief Constructs an alphabet from a C-style array of type T and length size. 
     */
    alphabet(const T* m, unsigned int size);
    
    /*! \brief Maps a symbol to an integer. 
     */
    unsigned int from_val(T val);
    
    /*! \brief Maps an integer to a symbol. 
     */
    T to_val(unsigned int c);
    
    /*! \brief Tests whether a given symbol is contained in the symbols of the alphabet. 
     */
    bool contains_symbol(T symbol);

    /*! \brief Tests whether all symbols in the stirng symbols are contained in the symbols of the alphabet. 
     */
    bool contains_symbols(basic_string<T>& symbols);
    
    /*! \brief Applies to_val to the elements of of the given vector and returns the results of this as a string. 
     */
    basic_string<T> to_string(vector<unsigned int> data);
    
    /*! \brief Applies from_val to the elements of of the given string and returns the results of this as a vector. 
     */    
    vector<unsigned int> to_vector(basic_string<T> data);

    /*! \brief Returns a string of random symbols of specified length. Uses the default urandom_generator. 
     */    
    basic_string<T> get_random_string(unsigned int length);
    
    /*! \brief Returns a string of random symbols of specified length. Uses the random_generator provided in parameter in. 
     */    
    basic_string<T> get_random_string(unsigned int length, random_generator& in);
    
    /*! \brief Returns a permutation of 0 .. get_size()-1 that maps each number to itself. 
     */        
    permutation get_identity();
    
    /*! \brief Returns a random permutation of 0 .. get_size()-1. Uses the default urandom_generator. 
     */        
    permutation get_random_permutation();
    
    /*! \brief Returns a random permutation of 0 .. get_size()-1. Uses the random_generator provided in parameter in. 
     */        
    permutation get_random_permutation(random_generator& in);
    
    /*! \brief Returns a random involution of 0 .. get_size()-1. Uses the default urandom_generator. 
     */            
    permutation get_random_involution();
    
    /*! \brief Returns a random involution of 0 .. get_size()-1. Uses the random_generator provided in parameter in. 
     */        
    permutation get_random_involution(random_generator& in);    
    
    /*! \brief Prints a textual representation of the given ::permutation to stdout using the symbols of this alphabet. 
     */        
    void print_perm(permutation& perm);
    
    /*! \brief Returns a textual representation of the given permutation using the symbols of this alphabet. 
     */        
    basic_string<T> perm_as_string(permutation& perm);
    
    /*! \brief Constructs a permutation from a vector that contains a permutation of the symbols of this alphabet. 
     */            
    permutation make_perm(const T *data);
    
    /*! \brief Convenience method that has the same functionality as make_perm but returns its result as a pointer to a new permutation. 
     */                
    permutation *make_perm_ptr(const T *data);
    
    /*! \brief Constructs a permutation from a vector of pairs of symbols of this alphabet that specify an involution of these symbols. 
     */ 
    permutation make_involution(vector<pair<T, T> >& commutations);
    
    /*! \brief Convenience method that has the same functionality as make_involution but returns its result as a pointer to a new permutation. 
     */                    
    permutation *make_involution_ptr(vector<pair<T, T> >& commutations);
    
    /*! \brief Returns the number of symbols in this alphabet. 
     */                
    unsigned int get_size();
    
    /*! \brief Returns a vector that represents the mapping of integers to symbols of this alphabet. 
     */
    vector<T> get_mapping();

private:
    /*! \brief Sets mapping and inverse_mapping from given vector. 
     */
    void set_mappings(const vector<T>& m);
    
    /*! \brief Helper method used by make_perm and make_perm_ptr. 
     */
    void fill_vector(vector<unsigned int>& m, const T *data);
    
    /*! \brief Helper method used by make_involution and make_involution_ptr. 
     */    
    void fill_involution(vector<unsigned int>& perm_data, vector<pair<T, T> >& commutations);
    
    /*! \brief Default random_generator. 
     */
    urandom_generator rand_gen;

    /*! \brief Mapping of integers to symbols. 
     */
    vector<T> mapping;
    
    /*! \brief Mapping of symbols to integers. 
     */
    map<T, unsigned int> inverse_mapping;
    
};

template<class T> vector<T> alphabet<T>::get_mapping()
{
    return mapping;
}

/*! 
 *  \param commutations [in] Has to specify the pairs of symbols that define the involution.  
 * 
 *  An involution is a special type of permutation that is the inverse of itself. This method
 *  can be used to construct an involution through a vector of pairs of symbols. Each
 *  pair specifies that the first element of the pair maps to the second and vice versa. Such
 *  involutions are used to implement the Steckerbrett of certain Enigma variants and the reflectors
 *  of all rotor machines that make use of them. If less than get_size() / 2 pairs are given
 *  the symbols not contained in the pairs are mapped to themselves. If more than get_size() / 2 
 *  pairs are given or if symbols are contained in more than one pair the resulting permutation
 *  may not specify an involution. If symbols that are not contained in the alphabet are part
 *  of any pair an exception is thrown.
 *  
 */ 
template<class T> permutation alphabet<T>::make_involution(vector<pair<T, T> >& commutations)
{
    vector<unsigned int> perm_data(get_size());
    
    fill_involution(perm_data, commutations);
    
    return permutation(perm_data);
}

/*! 
 *  \param symbol [in] Symbol which is tested whether it is contained in the alphabet.  
 * 
 *  Returns true if symbol is contained in the alphabet and false in case it is not.  
 */ 
template<class T> bool alphabet<T>::contains_symbol(T symbol)
{
    bool result = false;
    typename vector<T>::iterator iter;
    
    for (iter = mapping.begin(); (iter != mapping.end()) and (!result); ++iter)
    {
        result = (*iter == symbol);
    }
    
    return result;
}

template<class T> bool alphabet<T>::contains_symbols(basic_string<T>& symbols)
{
    bool result = true;
    
    for (unsigned int count = 0; (count < symbols.length()) and (result); count++)
    {
        result = result && this->contains_symbol(symbols[count]);
    }
    
    return result;
}

/*! 
 *  See documentation for make_involution(). The caller of this method is responsible for deleting the returned permutation.
 *
 */ 
template<class T> permutation *alphabet<T>::make_involution_ptr(vector<pair<T, T> >& commutations)
{
    vector<unsigned int> perm_data(get_size());
    
    fill_involution(perm_data, commutations);
    
    return new permutation(perm_data);
}

template<class T>  basic_string<T> alphabet<T>::get_random_string(unsigned int length)
{
    urandom_generator urand;
    
    return get_random_string(length, urand);
}

template<class T>  basic_string<T> alphabet<T>::get_random_string(unsigned int length, random_generator& in)
{
    vector<unsigned int> res_temp;
    
    in.read_symbols(res_temp, length, get_size());    
        
    return to_string(res_temp);
}

template<class T> permutation alphabet<T>::get_random_permutation()
{
    return this->get_random_permutation(this->rand_gen);
}

/*! 
 *  See get_random_involution(random_generator& in) for more details.  
 *
 */ 
template<class T> permutation alphabet<T>::get_random_involution()
{
    return this->get_random_involution(this->rand_gen);
}

/*! 
 *  \param in [in] random_generator object that is used to generate the needed random numbers.  
 * 
 *  This method generates a random involution by interpreting a random permutation as a sequence of random commutations.
 *  This is only possible if the alphabet contains an even number of symbols. If this method is called on an alphabet
 *  containing an odd number of symbols an excepetion is thrown.   
 *
 */ 
template<class T> permutation alphabet<T>::get_random_involution(random_generator& in)
{
    permutation temp_perm;
    vector<unsigned int> help(get_size());
    unsigned int plug1, plug2;    
    
    if ((this->get_size() % 2) == 0)
    {
        temp_perm = permutation::get_random_permutation(in, this->get_size());
        
        for (unsigned int count = 0; count < (this->get_size() / 2); count++)
        {
            plug1 = temp_perm.permute(2 * count);
            plug2 = temp_perm.permute((2 * count) + 1);
            help[plug1] = plug2;
            help[plug2] = plug1;
        }
    }
    else
    {
        throw runtime_error("alphabet has to have an even size");
    }
    
    return permutation(help);
}

template<class T> permutation alphabet<T>::get_random_permutation(random_generator& in)
{    
    return permutation::get_random_permutation(in, get_size());
}

/*! 
 *  \param data [in] A C-style array of symbols that is to be interpreted as a representation of a permutation 
 *                   of 0 .. get_size()-1.  
 * 
 *  If the symbol array contains symbols that are not part of the aplphabet an exception is thrown. If the array
 *  contains less than get_size() symbols or if some symbols occur more than once, the resulting permutation object
 *  does not implement a valid permutation. If the array contains more than get_size() symbols, the excess symbols will
 *  be ignored.
 *
 */ 
template<class T> permutation alphabet<T>::make_perm(const T *data)
{
    vector<unsigned int> help(get_size());
    
    fill_vector(help, data);
    
    return permutation(help);
}

/*! 
 *  See documentation for make_perm(). The caller of this method is responsible for deleting the returned permutation.
 *
 */
template<class T> permutation *alphabet<T>::make_perm_ptr(const T *data)
{
    vector<unsigned int> help(get_size());
    
    fill_vector(help, data);    
    
    return (new permutation(help));
}

template<class T> void alphabet<T>::fill_involution(vector<unsigned int>& perm_data, vector<pair<T, T> >& commutations)
{
    typename vector<pair<T, T> >::iterator iter;
    unsigned int from, to, temp;
    
    for (unsigned int count = 0; count < get_size(); count++)
    {
        perm_data[count] = count;
    }
    
    for (iter = commutations.begin(); iter != commutations.end(); ++iter)
    {
        from = from_val(iter->first);
        to = from_val(iter->second);
        temp = perm_data[to];
        perm_data[to] = perm_data[from];
        perm_data[from] = temp;        
    }    
}

template<class T> void alphabet<T>::fill_vector(vector<unsigned int>& m, const T *data)
{
    for (unsigned int count = 0; count < get_size(); count++)
    {
        m[count] = inverse_mapping[data[count]];
    }    
}

template<class T> void alphabet<T>::set_mappings(const vector<T>& m)
{
    mapping = m;
    
    for (unsigned int count = 0; count < get_size(); count++)
    {
        inverse_mapping[mapping[count]] = count;
    }    
}

template<class T> basic_string<T> alphabet<T>::perm_as_string(permutation& perm)
{
    basic_string<T> result;
    
    for (unsigned int count = 0; count < get_size(); count++)
    {
        result += to_val(perm.permute(count));
    }    
    
    return result;
}

template<class T> void alphabet<T>::print_perm(permutation& perm)
{
    for (unsigned int count = 0; count < get_size(); count++)
    {
        cout << to_val(perm.permute(count));
    }
    
    cout << endl;
}

template<class T> vector<unsigned int> alphabet<T>::to_vector(basic_string<T> data)
{
    vector<unsigned int> result;
    
    for (unsigned int count = 0; count < data.size(); count++)
    {
        result.push_back(from_val(data[count]));
    }
    
    return result;
}

template<class T> basic_string<T> alphabet<T>::to_string(vector<unsigned int> data)
{
   basic_string<T> result;
    
    for (unsigned int count = 0; count < data.size(); count++)
    {
        result.push_back(to_val(data[count]));
    }
    
    return result;
}

template<class T> permutation alphabet<T>::get_identity()
{
    return permutation::get_identity(get_size());
}

template<class T> unsigned int alphabet<T>::from_val(T val)
{
    return inverse_mapping[val];
}

template<class T> T alphabet<T>::to_val(unsigned int c)
{
    return mapping[c];
}

template<class T> alphabet<T>::alphabet(const vector<T>& m)
{
    set_mappings(m);
}

template<class T> alphabet<T>::alphabet(const T* m, unsigned int size)
{
    vector<T> help(size);
    
    for (unsigned int count = 0; count < size; count++)
    {
        help[count] = m[count];
    }    
    
    set_mappings(help);
}

template<class T> unsigned int alphabet<T>::get_size()
{
    return mapping.size();
}

/*! \brief A template class which provides a simple facility for encrypting or decrypting a string of symbols using a 
 *         given cryptographic transform and an appropriate ::alphabet. 
 *
 *  This class is a little helper that is used in some parts of the code for the implementation of automated tests. Objects of this
 *  class do not delete their members alpha or machine during destruction.
 *
 */ 
template<class T> class alphabet_transformer {
public:
    
    /*! \brief Constructs an alphabet_transformer from an alphabet over T and an object that implements the encryption_transform interface. 
     */    
    alphabet_transformer(alphabet<T> *a, encryption_transform *m) { alpha = a; machine = m; }
    
    /*! \brief Encrypts one symbol. 
     */
    T encrypt(T in_char);
    
    /*! \brief Decrypts one symbol. 
     */
    T decrypt(T in_char);
    
    /*! \brief Applies encrypt() to all symbols in parameter in and stores the result in parameter out. 
     */
    void encrypt(basic_string<T>& in, basic_string<T>& out);
    
    /*! \brief Applies decrypt() to all symbols in parameter in and stores the result in parameter out. 
     */    
    void decrypt(basic_string<T>& in, basic_string<T>& out);
    
    /*! \brief Getter method for the current alphabet object. 
     */
    alphabet<T> *get_alphabet() { return alpha; }
    
    /*! \brief Setter method for the current alphabet object. 
     */    
    void set_alphabet(alphabet<T> *a) { alpha = a; }
    
    /*! \brief Convenience method for printing the string given in parameter text. 
     */
    void print_text(basic_string<T>& text);
    
protected:
    /*! \brief A pointer to the current alphabet object. 
     */
    alphabet<T> *alpha;
    
    /*! \brief A pointer to the current encryption_transform object. 
     */
    encryption_transform *machine;
};

/*!
 *  \param in_char [in] Symbol to encrypt
 *
 *  Uses the alphabet to turn the symbol in_char into an integer, then calls the encryption_transform and finally uses the alphabet again
 *  to turn the resulting integer into a symbol which is then returned.  
 */
template<class T> T alphabet_transformer<T>::encrypt(T in_char)
{
    unsigned int temp = alpha->from_val(in_char);
    temp = machine->encrypt(temp);
    
    return alpha->to_val(temp);   
}

/*!
 *  \param in_char [in] Symbol to decrypt
 *
 *  Uses the alphabet to turn the symbol in_char into an integer, then calls the encryption_transform and finally uses the alphabet again
 *  to turn the resulting integer into a symbol which is then returned.  
 */
template<class T> T alphabet_transformer<T>::decrypt(T in_char)
{
    unsigned int temp = alpha->from_val(in_char);
    temp = machine->decrypt(temp);
    
    return alpha->to_val(temp);   
}

/*!
 *  \param in [in] Symbols to encrypt
 *
 *  \param out [out] Encrypted symbols
 */
template<class T> void alphabet_transformer<T>::encrypt(basic_string<T>& in, basic_string<T>& out)
{
    out.clear();
    typename basic_string<T>::iterator iter;
    
    for (iter = in.begin(); iter != in.end(); ++iter)
    {
        out.push_back(encrypt(*iter));
    }    
}

/*!
 *  \param in [in] Symbols to decrypt
 *
 *  \param out [out] Decrypted symbols
 */
template<class T> void alphabet_transformer<T>::decrypt(basic_string<T>& in, basic_string<T>& out)
{
    out.clear();
    typename basic_string<T>::iterator iter;
    
    for (iter = in.begin(); iter != in.end(); ++iter)
    {
        out.push_back(decrypt(*iter));
    }    
}

template<class T> void alphabet_transformer<T>::print_text(basic_string<T>& text)
{
    typename basic_string<T>::iterator iter;
    
    for (iter = text.begin(); iter != text.end(); ++iter)
    {
        cout << *iter;
    }    
    
    cout << endl;
}

#endif /* __alphabet_h__ */

