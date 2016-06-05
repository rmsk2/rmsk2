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

## @package rotorrandomtest contains code that allows to test the RotorRandom class contained in the rotorrandom module.
#   
# \file rotorrandomtest.py
# \brief rotorrandomtest contains code that allows to test the RotorRandom class contained in the rotorrandom module.
#

import tlvobject
import simpletest
import rotorrandom

## \brief This class implements tests which perform basic consistency checks of the data returned by rotorrandom.Random objects. It
#         does not test in any way whether the data returned is really random. 
#        
class RandomTest(simpletest.SimpleTest):
    ## \brief Constructor. 
    #
    # \param [name] Is a string. It specifies an explanatory text which serves as the name of the test which is to
    #        be performed.   
    #   
    # \param [alphabet] Is a string. It has to specify the characters that make up an alphabet which is used to test
    #        the generation of random strings and permutations. 
    #
    def __init__(self, name, alphabet):
        super().__init__(name)
        self._alpha = alphabet
    
    ## \brief Performs the test.
    #
    #  \returns A boolean. A return value of True means that the test was successfull.
    #    
    def test(self):
        result = super().test()
        
        with tlvobject.TlvServer(server_address='sock_fjsdhfjshdkfjh') as server, rotorrandom.RotorRandom(self._alpha, server.address) as rand_obj:
            try:
                rand_string = rand_obj.get_rand_string(len(self._alpha))
                
                # Verify length of retrieved string
                if not len(rand_string) == len(self._alpha):
                    self.append_note("Random data has wrong length: {} instead of {}".format(len(rand_string), len(self._alpha)))
                    result = False
                
                self.append_note("Retrieved random string: {}".format(rand_string))

                # Verify that retrieved string only contains characters from self._alpha
                for i in rand_string:
                    if not(i in self._alpha):
                        self.append_note('Illegal string value: {}'.format(i))
                        result = False

                rand_permutation = rand_obj.get_rand_permutation()
                
                # Verify that retrieved permutation contains exactly len(self._alpha) elements
                if not len(rand_permutation) == len(self._alpha):
                    self.append_note("Random permutation has wrong length: {} instead of {}".format(len(rand_permutation), len(self._alpha)))
                    result = False
                
                self.append_note("Retrieved random permutation: {}".format(rand_permutation))
                
                # Verify that retrieved permutation only contains values between 0 and len(self._alpha) - 1
                perm_test = set()
                for i in rand_permutation:
                    perm_test.add(i)
                    if not((i >= 0) and (i < len(self._alpha))):
                        self.append_note('Illegal permutation value: {}'.format(i))
                        result = False                        
                
                # Verify that retrieved permutation contains each possible value exactly once
                if len(perm_test) != len(self._alpha):                    
                    self.append_note('Wrong number of unique elements in permutation: {} instead of {}'.format(len(perm_test), len(self._alpha)))
                    result = False
                
            except:
                self.append_note("EXCEPTION!!!!")
                result = False                
                
        return result


## \brief Returns a rotorrandom.RandomTest object that allows to perform all the tests defined in this module.
#
#  \returns A RandomTest object.
#                
def get_module_test():
    return RandomTest('random', 'abcdefghijklmnopqrstuvwxyz')

## \brief Performs all the tests defined in this module.
#
#  \returns Nothing.
#                
def execute_tests(num_iterations):
    tests = get_module_test()
    test_result = tests.test()
    tests.print_notes()
