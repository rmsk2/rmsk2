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

## @package tlvtest Contains tests for the python3 side of the generic TLV interface.
#   
# \file tlvtest.py
# \brief This file contains classes the implement tests for the python3 side of the generic TLV interface.
#

import datetime
import simpletest
from tlvobject import *

## \brief This class implements tests for the TLV infrastructure by calling the add method of test arithmetic 
#         provider and the echo method of the echo provider.
#
class TlvFuncTest(simpletest.SimpleTest):
    ## \brief Constructor. 
    #
    #  \param [name] Is a string. It has to specifiy a textual description for the test.
    #
    #  \param [left_argument] Is an integer. It specifies the left argument of the add operation that is part
    #         of the test.
    #
    #  \param [right_argument] Is an integer. It specifies the right argument of the add operation that is part
    #         of the test.    
    #
    def __init__(self, name, left_argument, right_argument):
        super().__init__(name)
        self._left_summand = left_argument
        self._right_summand = right_argument
        
    ## \brief Returns a structure of TlvEntry objects which are to be used in the echo test.
    #
    #  \returns A sequence of TlvEntry objects. 
    #
    @staticmethod
    def get_test_sequence():
        double_test = TlvEntry().to_double(1254.6553)
        null_test = TlvEntry().to_null()
        string_test = TlvEntry().to_string('hollaraedulioe')
        int_test = TlvEntry().to_int(-123456)
        non_empty_byte_array = TlvEntry().to_byte_array(b'\x00\x01\x02\x03\x04\x05\x06')
        uint = TlvEntry().to_result(0xFFFFFFFF)
        string_1_in_seq = TlvEntry().to_string('komp1')
        string_2_in_seq = TlvEntry().to_string('komp2')
        test_seq = TlvEntry().to_sequence([string_1_in_seq, string_2_in_seq])
        return [double_test, null_test, string_test, int_test, non_empty_byte_array, uint, test_seq]                

    ## \brief Performs the test.
    #
    #  \returns A boolean. True is returned in case of a successfull test. 
    #
    def test(self):
        result = super().test()
        with TlvServer('./tlv_rotorsim', 'sock_tmpjffdfkdfgj') as s:
            with TestArithmetic(s.address) as a, TestEcho(s.address) as e:
                try:
                    # Perform add operation
                    add_result = a.add(self._left_summand, self._right_summand)
                    self.append_note("Arithmetic add test {} + {} = {}".format(self._left_summand, self._right_summand, add_result))
                    last_result = (add_result == (self._left_summand + self._right_summand))
                    result = result and last_result
                    
                    if not last_result:
                        self.append_note("Add test FAILED")
                    
                    # Call echo method
                    echo_result = e.echo(TlvEntry().to_sequence(self.get_test_sequence()))

                    # Test structure which was returned by the echo method
                    echo_result = echo_result[0]                    
                    last_result = (echo_result[0] == 1254.6553) and (echo_result[2] == 'hollaraedulioe') and (echo_result[3] == -123456) and (echo_result[6][0] == 'komp1')                    
                    result = result and last_result
                    self.append_note("Echo result: " + str(echo_result))
                    
                    if not last_result:
                        self.append_note("Echo test FAILED")
                    
                    test_dict = {'k1':'v1', 'k2':'v2'}
                    echo_dict = e.echo_dict(test_dict)
                    
                    last_result = (echo_dict['k1'] == 'v1 echo') and (echo_dict['k2'] == 'v2 echo')                                         
                    result = result and last_result
                    self.append_note("Echo dict result: " + str(echo_dict))
                    
                    if not last_result:
                        self.append_note("Echo dictionary test FAILED")                    
                    
                    # Ask server for list of known objects
                    obj_list = s.list_objects()                    
                    self.append_note("Objects returned by server: " + str(obj_list))
                    # There have to be two of these objects
                    last_result = (len(obj_list) == 2)
                    result = result and last_result
                    
                    if not last_result:
                        self.append_note("Unexpected number of objects returned")
                    
                    # Retrieve the list of providers known to the server and append it to the notes of this test
                    self.append_note("Providers known to server: " + str(s.list_providers()))
                except:
                    self.append_note("EXCEPTON!!!!")
                    result = False
        
        return result


## \brief This class has the purpose to measure the performance of the TLV infrastructure by repeating an add operation
#         a given number of times.
#
class TlvPerfTest(simpletest.SimpleTest):
    ## \brief Constructor. 
    #
    #  \param [name] Is a string. It has to specifiy a textual description for the test.
    #
    #  \param [num_iterations] Is an integer. It specifies how many add operations should be performed in the context
    #         of this test.
    #
    def __init__(self, name, num_iterations = 22000):
        super().__init__(name)
        self._iterations = num_iterations

    ## \brief Performs the test. This test should never fail. Its purpose is to add a note that specifies how long
    #         it took to perform the desired number of additions.
    #
    #  \returns A boolean. True is returned in case of a successfull test. 
    #            
    def test(self):
        result = super().test()

        with TlvServer('./tlv_rotorsim', 'sock_tmpjffdfkdfgj') as s, TestArithmetic(s.address) as a:
            try:
                jetzt = datetime.datetime.now()
                
                for i in range(self._iterations):
                    res = a.add(1, 1)

                spaeter = datetime.datetime.now()
                self.append_note("Time needed for {} iterations: {}".format(self._iterations, spaeter - jetzt))
            except:
                self.append_note("EXCEPTON!!!!")
                result = False        
        
        return result
        

## \brief This function returns a test case which aggregates the tests that were designed to verify the functionality
#         implemented in the tlvobject module.
#
#  \param [num_iterations] Is an integer. It has to specifiy the number of additions that are to be performed for the
#         performance measurement.
#
#  \returns A simpletest.CompositeTest object. It contains all defined test cases.
#
def get_module_test(num_iterations = 22000):
    functional_test = TlvFuncTest('TLV functional test', 17, 4)
    performance_test = TlvPerfTest('TLV performance test', num_iterations)
    all_tests = simpletest.CompositeTest('TLV')
    all_tests.add(functional_test)
    all_tests.add(performance_test)
    
    return all_tests

## \brief This function executes the test cases returned by the get_module_test() function. 
#
#  \param [num_iterations] Is an integer. It has to specifiy the number of additions that are to be performed for the
#         performance measurement.
#
#  \returns Nothing.
#
def execute_tests(num_iterations):
    tests = get_module_test(num_iterations)
    test_result = tests.test()
    tests.print_notes()
