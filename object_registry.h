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

#ifndef __object_registry__
#define __object_registry__

/*! \file object_registry.h
 *  \brief Header file for the service_provider, registry_manager and object_registry classes.
 */ 

#include<string>
#include<utility>
#include<map>
#include<sigc++/functors/slot.h>
#include<tlv_stream.h>

const unsigned int ERR_METHOD_NOT_FOUND = 100;
const unsigned int ERR_CLASS_NOT_FOUND = 101;
const unsigned int ERR_NOT_IMPLEMENTED = 102;
const unsigned int ERR_SYNTAX_INPUT = 103;
const unsigned int ERR_OBJECT_CREATE = 104;
const unsigned int ERR_SEMANTICS_INPUT = 105;

using namespace std;

class object_registry;
/*! \brief All callbacks that implement TLV functionality have to be of this type.  
 */ 
typedef sigc::slot<unsigned int, tlv_entry&, tlv_stream *> tlv_callback;

/*! \brief An abstract TLV class. A service provider is a thing that knows how to create and manage objects
 *         which provide TLV functionality.
 *
 *  Each type of object made available over the TLV interface has to have an associated service provider.
 *  It can be used to create new objects, delete existing objects and return tlv_callback objects which 
 *  allow to call a specific object's methods.
 */ 
class service_provider {
public:
    /*! \brief Constructor. Any service provider has to know an object registry in order register objects
     *         it has created. The parameter obj_registry has to point to an appropriate object of type
     *         object_registry. 
     */
    service_provider(object_registry *obj_registry) { registry = obj_registry; counter = 0; }

    /*! \brief Implementations of this method are intended to return a tlv_callback object that can be used to
     *         create new TLV objects of type managed by this service provider. 
     */
    virtual tlv_callback *make_new_handler() = 0;    

    /*! \brief Implementations of this method have to create new objects of the type managed by the servide provider
     *         and register them with the object registry. Implementations have to return ERR_OK if a new object was
     *         successfully created.
     *
     *  Functors for implementations of this method are returned by make_new_handler(). The parameter params has to
     *  contain the parameters which are needed to create a new TLV object of the desired type. The parameter
     *  out_stream has to point to a tlv_stream that can be used to talk to the connected client. 
     */
    virtual unsigned int new_object(tlv_entry& params, tlv_stream *out_stream) = 0;

    /*! \brief Implementations of this method have to create and return a functor which allows to call the
     *         method which is named by the parameter method_name on the object specified by the parameter
     *         object.
     *
     *  In case of an error NULL is to be returned by implementations of this method.
     */
    virtual tlv_callback *make_functor(string& method_name, void *object) = 0;

    /*! \brief Implementations of this method have to delete the object specified in parameter object.
     */
    virtual void delete_object(void *obj_to_delete) = 0;

    /*! \brief Has to return a human readable description of the type of TLV objects which are managed by this
     *         service provider.
     */
    virtual string get_name() = 0;

    /*! \brief Destructor.
     */    
    virtual ~service_provider() { ; }

protected:
    /*! \brief Each object regsitered with the object regsitry is given a handle. This method generates a new handle
     *         and stores it in the string referenced by parameter new_handle.
     *
     *  The algorithm used by this method to create a new handle is extremely simple. Append a ':' and the current value
     *  of service_provider::counter to the string returned by get_name(). No checking for uniqeness of the handle
     *  is performed against the object regsitry. As the counter is a 64 bit unsiged int we should nerver see any repitition
     *  of a handle during the typical life time of a service_provider instance.
     */    
    virtual void make_handle(string& new_handle);

    /*! \brief Points to the object registry used by this service_provider instance. */    
    object_registry *registry;
    /*! \brief A pointer which is incremented each time a new object is created by this service_provider instance.
     *         The value of this counter becomes part of the created handle.
     */        
    unsigned long int counter;
};

class registry_manager;
typedef unsigned int (registry_manager::*manager_fun)(tlv_entry& params, tlv_stream *out_stream); 

/*! \brief A TLV class that implements generic TLV methods which are provided by all object registries and are
 *         therefore independent of any service_provider.
 *
 *  TLV methods provided by the registry_manager of an object_registry appear as methods of a special object
 *  with the handle "root".
 */ 
class registry_manager {
public:
    /*! \brief Constructor. The parameter obj_registry has to point to the object_registry that is serviced
     *         by this registry_manager instance. 
     */
    registry_manager(object_registry *rgstry);

    /*! \brief This method creates and returns tlv_callbacks for the root.listobjects(), root.clear() and
     *         root.listproviders() methods. The desired method name has to be specified through the method_name
     *         parameter.
     *
     *  In case of an error NULL is returned. 
     */    
    tlv_callback *get_handler(string& method_name);

    /*! \brief This method deletes all objects currently managed by the object_registry. The parameter
     *         params is not evaluated and can therfore reference any valid tlv_entry object. The parameter
     *         out_stream is used to talk to the client.
     *
     *  In case of success ERR_OK is returned. 
     */    
    virtual unsigned int clear_processor(tlv_entry& params, tlv_stream *out_stream);

    /*! \brief This method returns the handles of all objects objects currently managed by the object_registry to 
     *         the client. The parameter params is not evaluated and can therfore reference any valid tlv_entry object.
     *         The parameter out_stream is used to talk to the client.
     *
     *  In case of success ERR_OK is returned. 
     */    
    virtual unsigned int list_objects_processor(tlv_entry& params, tlv_stream *out_stream);

    /*! \brief This method returns the names of all service_providers currently known to the object_registry to 
     *         the client. The parameter params is not evaluated and can therfore reference any valid tlv_entry object.
     *         The parameter out_stream is used to talk to the client.
     *
     *  In case of success ERR_OK is returned. 
     */    
    virtual unsigned int list_providers_processor(tlv_entry& params, tlv_stream *out_stream);    
    
    /*! \brief Destructor.
     */        
    virtual ~registry_manager() { ; }
    
protected:
    /*! \brief Holds a pointer to the object_registry which is serviced by this registry_manager instance. */        
    object_registry *registry;
    /*! \brief Holds a map which maps the name of the method the user wants to call to a pointer to the
     *         member function which implements that method.
     */            
    map<string, manager_fun> method_pointers;
};

/*! \brief A TLV class that manages all TLV objects known to a TLV server. It delegates the creation of new objects and
 *         the construction of tlv_callback objects to the appropriate service_provider instance.
 */ 
class object_registry {
public:
    /*! \brief Constructor.
     */ 
    object_registry() : manager(this) { ; }

    /*! \brief This is the main method of an object_registry instance. It is called by the processor callback of the tlv_server.
     *         It is used to determine a callback that is capable to handle a call of the method (given in parameter
     *         method) on the object specified by the handle object_name.
     *
     *  NULL is returned in case of an error. The caller is responsible for deleting objects returned by this method. This
     *  method knows about the pseudo objects "root" and "new". Calls to the "root" object are delegated to object_registry::manager.
     *  Calls to the "new" object are forwarded to the service provider responsible for creating new objects of the type requested
     *  in the parameter method.
     */ 
    virtual tlv_callback *get_processor(string& object_name, string& method);

    /*! \brief This method can be used to register a newly created object with the object registry. The parameter name has
     *         to specify the handle of the newly created object. The parameter new_object specifies a pointer to the new
     *         object and a pointer to the service provider which can be used to manage it.
     */     
    virtual void add_object(string& name, pair<void *, service_provider *>& new_object);

    /*! \brief This method deletes the object with the handle object_name and removes the handle from the object registry.
     */     
    virtual void delete_object(string& object_name);

    /*! \brief This method deletes all objects currently knwon to this object_regsitry instance.
     */     
    virtual void clear();

    /*! \brief Returns a reference to the internal data structure which maps each of the object handles currently known to this
     *         object_regsitry instance to a std::pair containing a pointer to the actual object and a pointer to the associated
     *         service_provider instance.
     */     
    virtual map<string, pair<void *, service_provider *> >& get_objects() { return objects; } 

    /*! \brief Returns a reference to the internal data structure which maps the name of each service_provider known to this
     *         object registry instance to a pointer to the appropriate service_provider object.
     */     
    virtual map<string, service_provider *>& get_providers() { return func_factory; } 
    
    /*! \brief Adds the service_provider object referenced through parameter provider under the name given in parameter class_name to
     *         the object registry.
     *
     *  Important: The object_registry instance takes ownership of the object referenced through the parameter provider and deletes
     *  it when this becomes necessary.
     */
    virtual void add_service_provider(string& class_name, service_provider *provider);

    /*! \brief This method deletes the service_provider with name class_name from the object regsitry. It also deletes all objects
     *         that are managed by the service_provider which is to be deleted.
     */     
    virtual void delete_service_provider(string& class_name);

    /*! \brief Destructor. Deletes all objects and service providers.
     */            
    virtual ~object_registry();
    
protected:
    /*! Maps object handles to a std::pair containing a pointer to the actual object and the associated service_provider. */
    map<string, pair<void *, service_provider *> > objects;
    /*! Maps service provider names to the actual service_provider objects. */
    map<string, service_provider *> func_factory;
    /*! Holds the registry manager associated with this object_regsitry instance. */    
    registry_manager manager;
};


#endif /* __object_registry__ */
