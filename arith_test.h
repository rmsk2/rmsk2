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

#ifndef __arith_test_h__
#define __arith_test_h__

/*! \file arith_test.h
 *  \brief Header file for some classes which allow to test the TLV infrastructure. On top of that it contains
 *         the header for the TLV rotor machine functionality.
 */ 

#include<unordered_map>
#include<rotor_machine.h>
#include<sigaba.h>
#include<glibmm.h>
#include<object_registry.h>

/*! \brief A TLV class that allows to do arithmetic which integers. It is intended as simple test case for the
 *         TLV insfrastructure.
 */ 
class arithmetic {
public:
    /*! \brief This method handles requests for adding two integers. The parameter params has to contain the
     *         left and right summands in form of a TLV sequence that in turn contains two integers. The parameter
     *         out_stream is used to talk to the client.
     *
     *  In case of success this method returns ERR_OK (i.e. 0).
     */ 
    virtual unsigned int add_processor(tlv_entry& params, tlv_stream *out_stream);
    
    /*! \brief Destructor.
     */    
    virtual ~arithmetic() { ; }
};

/*! \brief typedef for a pointer to a member function of class arithmetic which is suitable as a tlv_callback. */    
typedef unsigned int (arithmetic::*arith_mem_fun)(tlv_entry& params, tlv_stream *out_stream); 

/*! \brief A TLV class that allows to manage TLV objects as implemented by the arithmetic class.
 */ 
class arithmetic_provider : public service_provider {
public:
    /*! \brief Constructor. The parameter obj_regsitry has to point to the object registry which is to be used by this
     *         arithmetic_provider instance.
     */    
    arithmetic_provider(object_registry *obj_registry) : service_provider(obj_registry) { add_proc = &arithmetic::add_processor; }

    /*! \brief Returns a callback for the arithmetic_provider::new_object() method.
     */    
    virtual tlv_callback *make_new_handler();

    /*! \brief Returns a callback for the arithmetic::add_processor method of the object specified in parameter object.
     *         In case the parameter method_name is not "add" NULL is returned.
     */    
    virtual tlv_callback *make_functor(string& method_name, void *object);

    /*! \brief Constructs a new arithemtic object and registers it with the object registry. The parameter params is
     *         ignored and can therefore reference an arbitrary tlv_entry object. The parameter out_stream is used to talk
     *         to the client.
     *
     *  Returns ERR_OK (i.e. 0) in case of success.
     */    
    virtual unsigned int new_object(tlv_entry& params, tlv_stream *out_stream);

    /*! \brief Deletes the object specified by parameter obj_to_delete.
     */    
    virtual void delete_object(void *obj_to_delete); 

    /*! \brief Returns the name of the type of objects managed by this arithmetic_provider instance.
     */        
    virtual string get_name() { return string("arithmetic"); }   

    /*! \brief Destructor.
     */    
    virtual ~arithmetic_provider() { ; }

protected:
    /*! \brief Holds a pointer to the arithmetic::add_processor member function. */    
    arith_mem_fun add_proc;
};

/* ------------------------------------------------------------------------------------------- */

/*! \brief A TLV class that returns a TLV structure sent by a client back to the client. It is intended as simple test
 *         case for the TLV infrastructure.
 */ 
class echo {
public:
    /*! \brief This method handles echo requests. The parameter params has to contain the TLV structure that is
     *         to be echo'ed back to the client. The parameter out_stream is used to talk to the client.
     *
     *  In case of success this method returns ERR_OK (i.e. 0).
     */ 
    virtual unsigned int echo_processor(tlv_entry& params, tlv_stream *out_stream);
    
    /*! \brief This method handles echo requests for dictionaries. The parameter params has to contain a TLV structure that
     *         represnets a dictionary. The dictionary is pased modified and sent back to the client.
     *
     *  In case of success this method returns ERR_OK (i.e. 0).
     */ 
    virtual unsigned int echo_dict_processor(tlv_entry& params, tlv_stream *out_stream);    

    /*! \brief Destructor.
     */    
    virtual ~echo() { ; }
};

/*! \brief typedef for a pointer to a member function of class echo which is suitable as a tlv_callback. */
typedef unsigned int (echo::*echo_mem_fun)(tlv_entry& params, tlv_stream *out_stream); 

/*! \brief A TLV class that allows to manage TLV objects as implemented by the echo class.
 */ 
class echo_provider : public service_provider {
public:
    /*! \brief Constructor. The parameter obj_regsitry has to point to the object registry which is to be used by this
     *         echo_provider instance.
     */
    echo_provider(object_registry *obj_registry) : service_provider(obj_registry) { echo_proc = &echo::echo_processor; echo_dict_proc = &echo::echo_dict_processor; }

    /*! \brief Returns a callback for the echo_provider::new_object() method.
     */    
    virtual tlv_callback *make_new_handler();
    
    /*! \brief Returns a callback for the echo::echo_processor method of the object specified in parameter object.
     *         In case the parameter method_name is not "echo" NULL is returned.
     */    
    virtual tlv_callback *make_functor(string& method_name, void *object);

    /*! \brief Constructs a new echo object and registers it with the object registry. The parameter params is
     *         ignored and can therefore reference an arbitrary tlv_entry object. The parameter out_stream is used to talk
     *         to the client.
     *
     *  Returns ERR_OK (i.e. 0) in case of success.
     */    
    virtual unsigned int new_object(tlv_entry& params, tlv_stream *out_stream);
    
    /*! \brief Deletes the object specified by parameter obj_to_delete.
     */    
    virtual void delete_object(void *obj_to_delete); 

    /*! \brief Returns the name of the type of objects managed by this arithmetic_provider instance.
     */    
    virtual string get_name() { return string("echo"); }   

    /*! \brief Destructor.
     */    
    virtual ~echo_provider() { ; }

protected:
    /*! \brief Holds a pointer to the echo::echo_processor member function. */    
    echo_mem_fun echo_proc, echo_dict_proc;
};

/* ------------------------------------------------------------------------------------------- */

/*! \brief A TLV class that allows to access ::rotor_machine objects via the TLV infrastructure.
 */
class rotor_machine_proxy {
public:
    /*! \brief Constructor. The parameter m has to specify the rotor_machine instance which should be made accessible
     *         via the TLV infrastructure.
     *
     *  This rotor_machine_proxy instance takes ownership of m and deletes it when necessary.
     */
    rotor_machine_proxy(rotor_machine *m);
    
    /*! \brief This method handles encrypt requests sent by the client. The parameter params has to contain a string
     *         tlv_entry which holds the plaintext to be encrypted. The parameter out_stream is used to talk to the client.
     */     
    virtual unsigned int encrypt_processor(tlv_entry& params, tlv_stream *out_stream);

    /*! \brief This method handles decrypt requests sent by the client. The parameter params has to contain a string
     *         tlv_entry which holds the ciphertext to be decrypted. The parameter out_stream is used to talk to the client.
     *
     *  Returns ERR_OK (i.e. 0) in case of success.     
     */
    virtual unsigned int decrypt_processor(tlv_entry& params, tlv_stream *out_stream);

    /*! \brief This method returns the result of machine->save_ini() to the client. The parameter params is ignored.
     *         The parameter out_stream is used to talk to the client.
     *
     *  Returns ERR_OK (i.e. 0) in case of success.          
     */
    virtual unsigned int get_state_processor(tlv_entry& params, tlv_stream *out_stream);

    /*! \brief This method calls rmsk::restore_from_data() for the TLV string contained in parameter params. The parameter
     *         out_stream is used to talk to the client.
     *
     *  Returns ERR_OK (i.e. 0) in case of success.     
     */
    virtual unsigned int set_state_processor(tlv_entry& params, tlv_stream *out_stream);

    /*! \brief This method handles randomize requests sent by the client. The parameter params has to contain a string
     *         tlv_entry which holds the randomization parameter. The parameter out_stream is used to talk to the client.
     */     
    virtual unsigned int randomize_state_processor(tlv_entry& params, tlv_stream *out_stream);

    /*! \brief This method calls machine->step() a number of times specified by the TLV integer referenced by parameter params.
     *         For each call the result of machine->visualize_all_positions() is returned to the client. Communication with
     *         the client is done via the object to which parameter out_stream points.
     *
     *  Returns ERR_OK (i.e. 0) in case of success.     
     */
    virtual unsigned int step_processor(tlv_entry& params, tlv_stream *out_stream);

    /*! \brief This method returns the result of machine->get_description() to the client. The parameter params is ignored.
     *         The parameter out_stream is used to talk to the client.
     *
     *  Returns ERR_OK (i.e. 0) in case of success.     
     */
    virtual unsigned int get_description_processor(tlv_entry& params, tlv_stream *out_stream);
    
    /*! \brief This method performs a setup stepping of a SIGABA if the member machine is of the corresponding type. params has
     *         contain a TLV sequence which consists of two integers. The first has to specify the number (1-5) of the driver
     *         rotor which is to be stepped. And the second has to designate how often this rotor is to be stepped. For each
     *         stepping the result of machine->visualize_all_positions() is returned to the client. Communication with
     *         the client is done via the object to which parameter out_stream points.
     *
     *  Returns ERR_OK (i.e. 0) in case of success.     
     */    
    virtual unsigned int sigaba_setup_processor(tlv_entry& params, tlv_stream *out_stream);

    /*! \brief This method returns the result of machine->visualize_all_positions() to the client. The parameter params
     *         is ignored. The parameter out_stream is used to talk to the client.
     *
     *  Returns ERR_OK (i.e. 0) in case of success.     
     */
    virtual unsigned int get_positions_processor(tlv_entry& params, tlv_stream *out_stream);

    /*! \brief This method returns the configuration of the underlying rotor machine in the format used by configurator::get_config. 
     *         The parameter params is ignored. The parameter out_stream is used to talk to the client.
     *
     *  Returns ERR_OK (i.e. 0) in case of success.     
     */
    virtual unsigned int get_config_processor(tlv_entry& params, tlv_stream *out_stream);
    
    /*! \brief This method configures the underlying rotor machine according to the dictionary sent by the client in the parameter 
     *         param. The parameter out_stream is used to talk to the client.
     *
     *  Returns ERR_OK (i.e. 0) in case of success.     
     */
    virtual unsigned int set_config_processor(tlv_entry& params, tlv_stream *out_stream);    

    /*! \brief This method returns the result of machine->move_all_rotors("") to the client. The parameter params
     *         has to contain a string which specifies the positions to which the visible rotors are to be moved.
     *         The parameter out_stream is used to talk to the client.
     *
     *  Returns ERR_OK (i.e. 0) in case of success.     
     */
    virtual unsigned int set_positions_processor(tlv_entry& params, tlv_stream *out_stream);

    /*! \brief This method calls machine->step() a number of times specified by the TLV integer referenced by parameter params.
     *         For each stepping the result of machine->get_current_perm() is returned to the client. If the value of param is 0 
     *         then no stepping is performed but only the result of machine->get_current_perm() is returned to the client. 
     *         Communication with the client is done via the object to which parameter out_stream points.
     *
     *  Returns ERR_OK (i.e. 0) in case of success.     
     */
    virtual unsigned int get_permutations_processor(tlv_entry& params, tlv_stream *out_stream);        

    /*! \brief Replaces rotor_machine_proxy::machine with the value specified by the paramter new_machine. The object to
     *         which rotor_machine_proxy::machine points when this method is called is deleted.
     */        
    virtual void set_new_machine(rotor_machine *new_machine);

    /*! \brief Destructor.
     */    
    virtual ~rotor_machine_proxy() { delete machine; }
    
protected:
    /*! \brief This method handles encrypt and decrypt requests sent by the client. The parameter params has to contain a string
     *         tlv_entry which holds the text to be processed. The parameter do_enc determines if an encryption or a decryption is
     *         performed. If do_enc is true an encryption is performed. The parameter out_stream is used to talk to the client.
     *
     *  Returns ERR_OK (i.e. 0) in case of success.     
     */
    virtual unsigned int encdec_processor(tlv_entry& params, tlv_stream *out_stream, bool do_enc);

    /*! \brief This method converts the vector given in parameter perm into a TLV byte array. The resulting TLV byte array is
     *         stored in the object referenced by parameter byte_array.
     */    
    virtual void perm_to_byte_array(vector<unsigned int>& perm, tlv_entry& byte_array);

    /*! \brief Holds the rotor_machine object which is used to perform the cryptographic operations offered the methods of this object. */    
    rotor_machine *machine;
    
};

/*! \brief typedef for a pointer to a member function of class rotor_machine_proxy which is suitable as a tlv_callback. */
typedef unsigned int (rotor_machine_proxy::*rotor_proxy_fun)(tlv_entry& params, tlv_stream *out_stream); 

/*! \brief A TLV class that allows to manage TLV objects as implemented by the rotor_machine_proxy class.
 */ 
class rotor_machine_provider : public service_provider {
public:
    /*! \brief Constructor. The parameter obj_regsitry has to point to the object registry which is to be used by this
     *         rotor_machine_provider instance.
     */
    rotor_machine_provider(object_registry *obj_registry);

    /*! \brief Returns a callback for the rotor_machine_provider::new_object() method.
     */
    virtual tlv_callback *make_new_handler();
    
    /*! \brief Returns a callback for an appropriate (as specified by parameter method_name) method of a rotor_machine_proxy instance.
     *         Allowed method names are: encrypt, decrypt, getstate, setstate, step, getdescription, sigabasetup, getpositions and
     *         getpermutations.
     *         
     *  In case the value of parameter method_name is not contained in the above enumeration NULL is returned.
     */        
    virtual tlv_callback *make_functor(string& method_name, void *object);

    /*! \brief Constructs a new rotor_machine_proxy object and registers it with the object registry. The parameter params
     *         has to contain a valid machine state in the form of a TLV string. The parameter out_stream is used to talk
     *         to the client.
     *
     *  Returns ERR_OK (i.e. 0) in case of success.
     */
    virtual unsigned int new_object(tlv_entry& params, tlv_stream *out_stream);

    /*! \brief Deletes the object specified by parameter obj_to_delete.
     */
    virtual void delete_object(void *obj_to_delete); 
    
    /*! \brief Returns the name of the type of objects managed by this rotor_machine_provider instance.
     */    
    virtual string get_name() { return string("rotorproxy"); }   

    /*! \brief Destructor.
     */
    virtual ~rotor_machine_provider() { ; }

protected:
    /*! \brief Maps each allowed method name to a pointer of type rotor_proxy_fun, where that pointer points to a method of 
     *         rotor_machine_proxy that knows how to perform the requested method call.
     */
    unordered_map<string, rotor_proxy_fun> rotor_proxy_proc;
};

/* ------------------------------------------------------------------------------------------- */

/*! \brief A TLV class that allows to generate random strings and permutations in relation to an alphabet.
 */
class random_proxy {
public:
    /*! \brief Constructor. The parameter a has to point to an appropriate alphabet<char> object. This random_proxy
     *         instance takes ownership of the alphabet object and deletes it when necessary.
     */        
    random_proxy(alphabet<char> *a) { alpha = a; }

    /*! \brief This method calls alpha->get_random_string(). The parameter params has to contain an integer which has to be 1
     *         or bigger. In case its value is < 1 it is set to 1. Communication with the client is done via the object
     *         to which parameter out_stream points. 
     *
     *  Returns ERR_OK (i.e. 0) in case of success. Sends an appropriate tlv_object of type TAG_STRING back to the client.   
     */    
    virtual unsigned int random_string_processor(tlv_entry& params, tlv_stream *out_stream);

    /*! \brief This method calls alpha->get_random_permutation(). The parameter params is ignored. Communication with the client
     *         is done via the object to which parameter out_stream points. 
     *
     *  Returns ERR_OK (i.e. 0) in case of success. Sends an appropriate tlv_object of type TAG_BYTE_ARRAY back to the client.   
     */    
    virtual unsigned int random_permutation_processor(tlv_entry& params, tlv_stream *out_stream);

    /*! \brief Destructor.
     */    
    virtual ~random_proxy() { delete alpha; }
    
protected:
    /*! \brief Holds the alphabet which is used to generate the random data. */    
    alphabet<char> *alpha;
    
};

/*! \brief typedef for a pointer to a member function of class random_proxy which is suitable as a tlv_callback. */
typedef unsigned int (random_proxy::*random_proxy_fun)(tlv_entry& params, tlv_stream *out_stream); 

/*! \brief A TLV class that allows to manage TLV objects as implemented by the random_proxy class.
 */ 
class random_provider : public service_provider {
public:
    /*! \brief Constructor. The parameter obj_regsitry has to point to the object registry which is to be used by this
     *         random_provider instance.
     */
    random_provider(object_registry *obj_registry);

    /*! \brief Returns a callback for the random_provider::new_object() method.
     */
    virtual tlv_callback *make_new_handler();
    
    /*! \brief Returns a callback for an appropriate (as specified by parameter method_name) method of a random_proxy instance.
     *         Allowed method names are: randstring, randpermutation.
     *         
     *  In case the value of parameter method_name is not contained in the above enumeration NULL is returned.
     */        
    virtual tlv_callback *make_functor(string& method_name, void *object);

    /*! \brief Constructs a new random_proxy object and registers it with the object registry. The parameter params
     *         has to contain a string that sepcifies an alphabet.
     *
     *  Returns ERR_OK (i.e. 0) in case of success.
     */
    virtual unsigned int new_object(tlv_entry& params, tlv_stream *out_stream);

    /*! \brief Deletes the object specified by parameter obj_to_delete.
     */
    virtual void delete_object(void *obj_to_delete); 
    
    /*! \brief Returns the name of the type of objects managed by this random_provider instance.
     */    
    virtual string get_name() { return string("randomproxy"); }   

    /*! \brief Destructor.
     */
    virtual ~random_provider() { ; }

protected:
    /*! \brief Maps each allowed method name to a pointer of type random_proxy_fun, where that pointer points to a method of 
     *         rotor_machine_proxy that knows how to perform the requested method call.
     */
    unordered_map<string, random_proxy_fun> random_proxy_proc;
};


#endif /* __arith_test_h__ */
