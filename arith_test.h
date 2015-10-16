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

#include<unordered_map>
#include<rotor_machine.h>
#include<sigaba.h>
#include<glibmm.h>
#include<object_registry.h>

class arithmetic {
public:
    virtual unsigned int add_processor(tlv_entry& params, tlv_stream *out_stream);
    virtual ~arithmetic() { ; }
};

typedef unsigned int (arithmetic::*arith_mem_fun)(tlv_entry& params, tlv_stream *out_stream); 

class arithmetic_provider : public service_provider {
public:
    arithmetic_provider(object_registry *obj_registry) : service_provider(obj_registry) { add_proc = &arithmetic::add_processor; }

    virtual tlv_callback *make_new_handler();
    virtual tlv_callback *make_functor(string& method_name, void *object);

    virtual unsigned int new_object(tlv_entry& params, tlv_stream *out_stream);
    virtual void delete_object(void *obj_to_delete); 
    
    virtual string get_name() { return string("arithmetic"); }   
    
    virtual ~arithmetic_provider() { ; }

protected:
    arith_mem_fun add_proc;
};

/* ------------------------------------------------------------------------------------------- */

class echo {
public:
    virtual unsigned int echo_processor(tlv_entry& params, tlv_stream *out_stream);
    virtual ~echo() { ; }
};

typedef unsigned int (echo::*echo_mem_fun)(tlv_entry& params, tlv_stream *out_stream); 

class echo_provider : public service_provider {
public:
    echo_provider(object_registry *obj_registry) : service_provider(obj_registry) { echo_proc = &echo::echo_processor; }

    virtual tlv_callback *make_new_handler();
    virtual tlv_callback *make_functor(string& method_name, void *object);

    virtual unsigned int new_object(tlv_entry& params, tlv_stream *out_stream);
    virtual void delete_object(void *obj_to_delete); 
    
    virtual string get_name() { return string("echo"); }   
    
    virtual ~echo_provider() { ; }

protected:
    echo_mem_fun echo_proc;
};

/* ------------------------------------------------------------------------------------------- */

class rotor_machine_proxy {
public:
    rotor_machine_proxy(rotor_machine *m);
    virtual unsigned int encrypt_processor(tlv_entry& params, tlv_stream *out_stream);
    virtual unsigned int decrypt_processor(tlv_entry& params, tlv_stream *out_stream);
    
    virtual void set_new_machine(rotor_machine *new_machine);
    
    virtual ~rotor_machine_proxy() { delete machine; }
    
protected:
    virtual unsigned int encdec_processor(tlv_entry& params, tlv_stream *out_stream, bool do_enc);

    rotor_machine *machine;
    
};

typedef unsigned int (rotor_machine_proxy::*rotor_proxy_fun)(tlv_entry& params, tlv_stream *out_stream); 

class rotor_machine_provider : public service_provider {
public:
    rotor_machine_provider(object_registry *obj_registry);

    virtual tlv_callback *make_new_handler();
    virtual tlv_callback *make_functor(string& method_name, void *object);

    virtual unsigned int new_object(tlv_entry& params, tlv_stream *out_stream);
    virtual void delete_object(void *obj_to_delete); 
    
    virtual string get_name() { return string("rotorproxy"); }   

    virtual ~rotor_machine_provider() { ; }

protected:
    unordered_map<string, rotor_proxy_fun> rotor_proxy_proc;
};

#endif /* __arith_test_h__ */
