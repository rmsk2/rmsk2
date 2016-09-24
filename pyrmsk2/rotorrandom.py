################################################################################
# Copyright 2016 Martin Grap
# 
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
################################################################################

## @package rotorrandom A Python3 interface to the C++ alphabet class used to generate random strings and permutations
#           
# \file pyrmsk2/rotorrandom.py
# \brief This file provides some Python3 classes that allow to generate random strings and
#        permutation using the C++ program tlv_object.

import pyrmsk2.tlvobject as tlvobject


## \brief This class allows access to objects provided by the random_provider and thereby makes
#         functionality to generate random strings and permutations available to the python3 side
#          of the TLV infrastructure.
#
#  We say that the rotorsim.RotorRandom instance proxies the random_proxy object on the C++ side of the
#  TLV infrastructure.
#
class RotorRandom(tlvobject.TlvProxy):
    ## \brief Constructor. Creates a new random_proxy object on the TLV server.
    #
    #  \param [alphabet] Is a string. It has to specifiy an alphabet which is used to generate random strings.
    #
    #  \param [server_address] Is a string. Has to contain the address of the TLV server that is to be used
    #         by this RotorRandom instance.
    #
    def __init__(self, alphabet, server_address):
        super().__init__(server_address)
        param = tlvobject.TlvEntry().to_string(alphabet)
        res = self.do_method_call('new', 'randomproxy', param)        
        self._handle = res[0]
    
    ## \brief Retrieves a random string.
    #
    #  \param [size] An integer. It has to contain the desired length of the random string. If it is < 1 a string
    #         of length 1 is returned.
    #
    #  \returns A string containing random symbols of the alphabet which was used to initialize the object.
    #
    def get_rand_string(self, size):
        param = tlvobject.TlvEntry().to_int(size)    
        res = self.do_method_call(self._handle, 'randstring', param)
        
        return res[0]
    
    ## \brief Retrieves a random permutation.
    #
    #  \returns A list of integers which specifies the generated random permutation.
    #
    def get_rand_permutation(self):
        param = tlvobject.TlvEntry().to_null()    
        res = self.do_method_call(self._handle, 'randpermutation', param)
        res = list(res[0])
        
        return res

