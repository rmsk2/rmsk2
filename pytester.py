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

import tlvtest
import rotorsimtest
import cmdlinetest
import simpletest

def perform_all_tests():
    all_tests = simpletest.CompositeTest("All Tests")
    all_tests.add(tlvtest.get_module_test())
    all_tests.add(rotorsimtest.get_module_test())    
    all_tests.add(cmdlinetest.get_module_test())    
    all_tests.execute()


perform_all_tests()


