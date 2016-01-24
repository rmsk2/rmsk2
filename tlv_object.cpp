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

    // Create test arithmetic provider
    arithmetic_provider *arith_provider = new arithmetic_provider(&registry);
    string provider_name = arith_provider->get_name();

    // Create test echo provider
    echo_provider *echo_prov = new echo_provider(&registry);
    string echo_provider_name = echo_prov->get_name();

    // Create the rotor machine provider
    rotor_machine_provider *rotor_machine_prov = new rotor_machine_provider(&registry);
    string rotor_machine_provider_name = rotor_machine_prov->get_name();

    // Create the rotor machine provider
    random_provider *random_prov = new random_provider(&registry);
    string random_provider_name = random_prov->get_name();
    
    string address(argv[1]);
    
    // Add providers to registry
    registry.add_service_provider(provider_name, arith_provider);
    registry.add_service_provider(echo_provider_name, echo_prov);
    registry.add_service_provider(rotor_machine_provider_name, rotor_machine_prov);
    registry.add_service_provider(random_provider_name, random_prov);    
    
    // Create the server object and start the server
    uxdomain_socket_server tlv_server(address);
    service_result = tlv_server.start(sigc::mem_fun(tlv_server, &uxdomain_socket_server::on_connect), &registry);
        
    if (service_result != ERR_OK)
    {
        cout << "Error " << service_result << " executing service" << endl;
    }

    return 0;
}

