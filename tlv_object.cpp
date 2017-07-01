/***************************************************************************
 * Copyright 2017 Martin Grap
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

/*! \file tlv_object.cpp
 *  \brief Contains the main function of the TLV server.
 */ 

#include<iostream>
#include<tlv_server.h>
#include<object_registry.h>
#include<arith_test.h>

/*! \brief This function is the main function for the TLV server provided by the tlv_rotorsim program.
 */
int main(int argc, char *argv[])
{
    unsigned int service_result = ERR_OK;
    object_registry registry;

    if (argc < 2)
    {
        printf("Missing argument\n");
        exit(0);
    }
    
    // Create providers
    arithmetic_provider *arith_provider = new arithmetic_provider(&registry); // Create test arithmetic provider    
    echo_provider *echo_prov = new echo_provider(&registry); // Create test echo provider    
    rotor_machine_provider *rotor_machine_prov = new rotor_machine_provider(&registry); // Create the rotor machine provider    
    random_provider *random_prov = new random_provider(&registry); // Create the rotor random provider

    // Create pseudo objects
    registry_manager *root_obj = new registry_manager(&registry); // Create the root pseudo object    
    rmsk_pseudo_object *rmsk2_obj = new rmsk_pseudo_object(); // Create the rmsk2 pseudo object
    
    string address(argv[1]);
    
    // Add pseudo objects to registry
    registry.add_pseudo_object(root_obj);
    registry.add_pseudo_object(rmsk2_obj);    
    
    // Add providers to registry
    registry.add_service_provider(arith_provider);
    registry.add_service_provider(echo_prov);
    registry.add_service_provider(rotor_machine_prov);
    registry.add_service_provider(random_prov);    
    
    // Create the server object and start the server
    uxdomain_socket_server tlv_server(address);
    service_result = tlv_server.start(sigc::mem_fun(tlv_server, &uxdomain_socket_server::on_connect), &registry);
        
    if (service_result != ERR_OK)
    {
        cout << "Error " << service_result << " executing service" << endl;
    }

    return 0;
}

