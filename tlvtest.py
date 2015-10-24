################################################################################
# Copyright 2015 Martin Grap
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

import simpletest
from tlvobject import *

class TlvFuncTest(simpletest.SimpleTest):
    def __init__(self, name, left_argument, right_argument):
        super().__init__(name)
        self._left_summand = left_argument
        self._right_summand = right_argument

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

    def test(self):
        result = super().test()
        with TlvServer('./tlv_object', 'sock_tmpjffdfkdfgj') as s:
            with TestArithmetic(s.address) as a, TestEcho(s.address) as e:
                try:
                    add_result = a.add(self._left_summand, self._right_summand)
                    self.append_note("Arithmetic add test {} + {} = {}".format(self._left_summand, self._right_summand, add_result))
                    last_result = (add_result == (self._left_summand + self._right_summand))
                    result = result and last_result
                    
                    if not last_result:
                        self.append_note("Add test FAILED")
                                        
                    echo_result = e.echo(TlvEntry().to_sequence(self.get_test_sequence()))
                    echo_result = echo_result[0]
                    
                    last_result = (echo_result[0] == 1254.6553) and (echo_result[2] == 'hollaraedulioe') and (echo_result[3] == -123456) and (echo_result[6][0] == 'komp1')                    
                    result = result and last_result
                    self.append_note("Echo result: " + str(echo_result))
                    
                    if not last_result:
                        self.append_note("Echo test FAILED")                                        
                    
                    obj_list = s.list_objects()                    
                    self.append_note("Objects returned by server: " + str(obj_list))
                    last_result = (len(obj_list) == 2)
                    result = result and last_result
                    
                    if not last_result:
                        self.append_note("Unexpected number of objects returned")

                    self.append_note("Providers known to server: " + str(s.list_providers()))
                except:
                    self.append_note("EXCEPTON!!!!")
                    result = False
        
        return result


class TlvPerfTest(simpletest.SimpleTest):
    def __init__(self, name, num_iterations = 22000):
        super().__init__(name)
        self._iterations = num_iterations
            
    def test(self):
        result = super().test()

        with TlvServer('./tlv_object', 'sock_tmpjffdfkdfgj') as s, TestArithmetic(s.address) as a:
            try:
                jetzt = datetime.datetime.now()
                
                for i in range(self._iterations):
                    res = a.add(1, 1)

                spaeter = datetime.datetime.now()
                self.append_note("Time needed for {} iterations: {}".format(self._iterations, str(spaeter - jetzt)))
            except:
                self.append_note("EXCEPTON!!!!")
                result = False        
        
        return result

def get_module_test(num_iterations = 22000):
    functional_test = TlvFuncTest('TLV functional test', 17, 4)
    performance_test = TlvPerfTest('TLV performance test', num_iterations)
    all_tests = simpletest.CompositeTest('All TLV tests')
    all_tests.add(functional_test)
    all_tests.add(performance_test)
    
    return all_tests

def execute_tests(num_iterations):
    tests = get_module_test(num_iterations)
    test_result = tests.test()
    tests.print_notes()
