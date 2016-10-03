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

## @package pytester implements a program that performs all tests defined for the python3 interface.
#   
# \file pytester.py
# \brief This file contains a function that aggregates and subsequently performs all the tests defined
#        for the modules that make up the python3 interface.

import tlvtest
import rotorsimtest
import cmdlinetest
import rotorrandomtest
import simpletest
import subprocess
import shlex
import os
import sys

## \brief This class wraps calling any test program (for instance rmsk) in a test case derived from 
#         simpletest.SimpleTest.
#
class RmskCmdLineTest(simpletest.SimpleTest):
    ## \brief Constructor. 
    #
    # \param [name] Is a string. Has to specifiy a human readable description of the test.
    #
    # \param [command_line] Is a string. Has to contain the command line of the test program that is to
    #        be called. The test program's exit code has to be set to 0 in case of a scuccessfull test.
    #
    def __init__(self, name, command_line):
        super().__init__(name)
        self._args = shlex.split(command_line)

    ## \brief Performs the test.
    #
    #  \returns A boolean. A value of True indicates a successfull test.
    #    
    def test(self):
        result = super().test()        
        
        try:
            p = subprocess.Popen(self._args, cwd=os.getcwd(), stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE, close_fds=True)
            comm_result = p.communicate()        
            result = (p.returncode == 0)
            output = comm_result[0].decode('UTF-8')
            lines = output.split('\n')
            
            for i in lines:
                message = i.strip()            
                if message != '':
                    self.append_note(i)
        except:
            self.append_note("EXCEPTION!!")
            result = False
        
        return result
        

## \brief Gathers all defined test cases.
#         
#  \returns A simpletest.CompositeTest object. This object represents all defined tests.
#                
def get_all_tests():
    all_tests = simpletest.CompositeTest('Module tests')
    all_tests.add(RmskCmdLineTest('rmsk', './rmsk'))
    all_tests.add(tlvtest.get_module_test())
    all_tests.add(rotorsimtest.get_module_test())    
    all_tests.add(cmdlinetest.get_module_test())    
    all_tests.add(rotorrandomtest.get_module_test())
    return all_tests

## \brief Performs all the tests named in parameter test_names.
#
#  \param [test_names] Is a sequence of strings. Specifies the names of the tests that are
#         to be performed.
#         
#  \returns Nothing.
#                
def perform_some_tests(test_names):
    all_tests_ok = True
    test_dict = {}
    
    for i in get_all_tests().test_cases:
        test_dict[i.name] = i
    
    for i in test_names:
        if i in test_dict.keys():
            all_tests_ok = all_tests_ok and test_dict[i].test()
            test_dict[i].print_notes()
        else:
            print('Unknown test {}'.format(i))
    
    if not all_tests_ok:
        print('Some tests FAILED!!')


if __name__ == "__main__":

    if len(sys.argv) == 1:
        print('usage: ./unittest all | test1 test2 ...\n')
        print('Known tests:')
        print('============')        
        for i in get_all_tests().test_cases:
            print(i.name)
        print()
    elif len(sys.argv) == 2:
        if sys.argv[1] == 'all':
            get_all_tests().execute()
        else:
            perform_some_tests(sys.argv[1:])
    else:
        perform_some_tests(sys.argv[1:])

