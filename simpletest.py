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

class SimpleTest:
    def __init__(self, name):
        self._name = name
        self._notes = []
    
    @property
    def name(self):
        return self._name
    
    def test(self):
        self._notes = []
        return True
        
    def append_note(self, note):
        self._notes.append(note)
    
    @property
    def notes(self):
        return self._notes
    
    def print_notes(self):
        for i in self._notes:
            print(i)
    
    def execute(self):
        result = self.test()
        self.print_notes()        
        
        if result:
            print("Test OK")
        else:
            print("Test FAILED")

class CompositeTest(SimpleTest):
    def __init__(self, name):
        super().__init__(name)
        self._test_cases = []
    
    def add(self, test_case):
        self._test_cases.append(test_case)
    
    def test(self):
        result = super().test()
        
        for i in self._test_cases:
            self.append_note(self.name + " -> " + i.name + " start")
            result_of_last_test = i.test()
            notes_of_last_test = i.notes
            
            for j in notes_of_last_test:
                self.append_note(self.name + " -> " + j)
            
            result = result and result_of_last_test            
            if not result:
                self.append_note(self.name + " -> " + i.name + " Has FAILED")
                break
            else:
                self.append_note(self.name + " -> " + i.name + " OK")
        
        if result:
            self.append_note('All tests in test case "{}" succeeded'.format(self.name))
        else:
            self.append_note('At least one test in test case "{}" FAILED'.format(self.name))
                
        return result 
        
        
