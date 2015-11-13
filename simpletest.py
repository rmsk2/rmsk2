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

## @package simpletest Contains classes that together make up a primitive python3 unit testing facility.
#   
# \file simpletest.py
# \brief simpletest contains classes that together make up a primitive unit test facility.
#

## \brief This class serves as the base class for all unit tests.
#
#  Each test can be annotated with so called notes that can be used to convey information about
#  what happened when the test was performed. 
#
class SimpleTest:
    ## \brief Constructor. 
    #
    #  \param [name] Is a string. It has to specifiy a textual description for the test.
    #
    def __init__(self, name):
        self._name = name
        self._notes = []

    ## \brief Returns the name given to this SimpleTest instance. 
    #
    #  \returns A string.
    #
    @property
    def name(self):
        return self._name

    ## \brief Changes the name of this SimpleTest instance. 
    #
    #  \param [new_value] A string. Contains the new name.
    #
    #  \returns Nothing.
    #    
    @name.setter
    def name(self, new_value):
        self._name = new_value

    ## \brief Performs the test defined for this instance of SimpleTest 
    #
    #  \returns A boolean. Return value True means that the test were successfull.
    #    
    def test(self):
        self._notes = []
        return True

    ## \brief Add a note to this SimpleTest instance. 
    #
    #  \param [note] A string. This parameter has to contain a message that is to be associated with this
    #         SimpleTest instance.
    #
    #  \returns Nothing.
    #        
    def append_note(self, note):
        self._notes.append(note)

    ## \brief Returns the notes that have been appended to this SimpleTest instance. 
    #
    #  \returns A sequence of strings.
    #    
    @property
    def notes(self):
        return self._notes

    ## \brief Prints all notes that are associated with this SimpleTest instance. 
    #
    #  \returns Nothing.
    #        
    def print_notes(self):
        for i in self._notes:
            print(i)

    ## \brief Performs the test implemented by this SimpleTest instance ans subsequently prints all notes that
    #         have been generated. 
    #
    #  \returns Nothing.
    #            
    def execute(self):
        result = self.test()
        self.print_notes()        
        
        if result:
            print("Test OK")
        else:
            print("Test FAILED")

## \brief This class serves as a sort of container for test cases. Test cases can be added to it.
#         Its test method simply calls all the test methods of the subordinate test cases that
#         have been added to a CompositeTest instance.
#
class CompositeTest(SimpleTest):
    ## \brief Constructor. 
    #
    #  \param [name] Is a string. It has to specifiy a textual description for the test.
    #
    def __init__(self, name):
        super().__init__(name)
        self._test_cases = []

    ## \brief Returns the sequence of test cases. 
    #
    #  \returns A sequence of objects which implement the interface defined by SimpleTest
    #    
    @property    
    def test_cases(self):
        return self._test_cases

    ## \brief Add a subordinate test case to this CompositeTest instance. 
    #
    #  \param [test_case] An object implementing the interface defined in SimpleTest.
    #
    #  \returns Nothing.
    #            
    def add(self, test_case):
        self.test_cases.append(test_case)

    ## \brief Performs all subordinate test cases that have been added to this CompositeTest instance. 
    #
    #  \returns A boolean. Return value True means that all subordinate test cases were successfull.
    #        
    def test(self):
        result = super().test()
        
        for i in self.test_cases:
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

