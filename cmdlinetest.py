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

import subprocess
import os
import rotorsimtest
import rotorsim
import simpletest

RESULT_PROCESSOR_FORMAT = 43

## \brief This class serves as the generic something went wrong exception.
#
class ProcessorException(Exception):
    def __init__(self, error_code):
        Exception.__init__(self, 'Processor: ' + str(error_code))


## \brief This class provides the simplest possible interface to the C++ rotorsim program and hides the
#         gory details of how to communicate with it
#
class Processor:
    ## \brief Constructor. 
    #
    # \param [state] Is a byte array. Has to specifiy the machine state with which this instance is to
    #         operate.
    # \param [binary_name] Is a string. Has to contain the path to the binary of the rotorsim C++
    #         program
    # \param [progress_state] Is a boolean. If it is true then the member self.__state is updated with
    #         the new state after a call to encrypt, decrypt or process.
    #
    def __init__(self, state, binary_name = './rotorsim', progress_state = True):
        self.__rotorsim_binary = binary_name
        self.__state = state
        self.__do_state_progression = progress_state
    
    ## \brief Returns the current machine state known to this instance.
    #        
    def get_state(self):
        return self.__state

    ## \brief Sets the current machine state known to this instance to the value given in the parameter new_state.
    #        
    #  \param [new_state] Is a byte array that represents the new machine state to use.
    #    
    def set_state(self, new_state):
        self.__state = new_state

    ## \brief Constructs a command line for the C++ program rotorsim and executes rotorsim.
    #
    #  \param [command] Is a string and has to specifiy the command which is used as the paraemter for the -c option
    #                  Allowed values at the moment are "encrypt" and "decrypt".
    #  \param [input_data] Is a string which has to contain the input data. This value must not contain characters
    #                     that can not be encoded in ASCII.
    #  \param [output_grouping] Is an int. Specifies the group size which is used when producing the output string.
    #
    #  \param [additional_params] Is a list of strings. Each element of the list is appended to the command line
    #                             when "calling" the C++ rotorsim program.
    #
    #  \returns A string containing the machine output. In case of an error an exception is thrown.
    #            
    def process(self, command, input_data, output_grouping = 0, additional_params = []):
        result = ''
        
        proc_arguments = [self.__rotorsim_binary, command, '-g', str(output_grouping)]
        
        if self.__do_state_progression:
            proc_arguments.append('--state-progression')
        
        proc_arguments = proc_arguments + additional_params
        
        # call rotorsim program
        p = subprocess.Popen(proc_arguments, cwd=os.getcwd(), stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE, close_fds=True)
        comm_result = p.communicate(self.__state + bytes([0xFF]) + input_data.encode())
        
        if p.returncode != 0:
            raise RotorSimException(p.returncode)
        
        if self.__do_state_progression:
            pos = 0
            
            # search for terminator (0xFF) between machine output and state information
            while (pos < len(comm_result[0])) and (comm_result[0][pos] != 255):
                pos += 1
            
            # At least a new line is returned as procssing result    
            if (pos < 1) or (pos == len(comm_result[0])):            
                raise ProcessorException(RESULT_PROCESSOR_FORMAT)          
            
            # output is everyting before the 0xFF            
            result = (comm_result[0][:pos]).decode().strip('\n')    
            # new state is contained in the bytes following the 0xFF
            self.__state = comm_result[0][pos + 1:]
        else:            
            result = comm_result[0].decode().strip()
                    
        return result

    ## \brief Simple wrapper for the process method that allows to decrypt the string value specified in paramter 
    #         input_data.
    #
    #  \param [input_data] Is a string which has to contain the input data. This value must not contain characters
    #                     that can not be encoded in ASCII.
    #
    #  \returns A string containing the decryption result.
    #            
    def decrypt(self, input_data):
        return self.process('decrypt', input_data, 0)

    ## \brief Simple wrapper for the process method that allows to encrypt the string value specified in paramter 
    #         input_data.
    #
    #  \param [input_data] Is a string which has to contain the input data. This value must not contain characters
    #                      that can not be encoded in ASCII.
    #  \param [out_grouping] Is an int. Specifies the group size which is used when producing the output string.
    #
    #  \returns A string containing the encryption result.
    #                    
    def encrypt(self, input_data, out_grouping = 0):
        return self.process('encrypt', input_data, out_grouping)

    ## \brief Simple wrapper for the process method that allows to step the rotor machine specified by self.__state.
    #
    #  \param [num_iterations] An int that specifies how many times the machine is to be stepped.
    #
    #  \returns A vector of strings specifying the rotor positions encountered while stepping the machine
    #                        
    def step(self, num_iterations = 1):
        help =  self.process('step', '', 0, ['--num-iterations', str(num_iterations)])
        return self._response_to_string_vector(help)

    ## \brief Simple wrapper for the process method that allows to "setup step" the rotor given in parameter rotor_num. If 
    #         the current state does not describe a SIGABA calling this method does not step any rotors.
    #
    #  \param [rotor_num] Is an int specifying the number of the rotor which is to "setup step", where the numbering
    #                     goes from left to right of the driver machine rotors (the five in the middle in rotorvis).
    #  \param [num_iterations] An int that specifies how many times the machine is to be stepped.
    #
    #  \returns A vector of strings string specifying the rotor positions generated.
    #                        
    def setup_step(self, rotor_num, num_iterations = 1):
        help =  self.process('sigabasetup', '', 0, ['--rotor-num', str(rotor_num), '--num-iterations', str(num_iterations)])
        return self._response_to_string_vector(help)

    ## \brief Simple wrapper for the process method that allows to retrieve the current rotor positions.
    #
    #  \returns A string specifying the current rotor positions.
    #                        
    def get_rotor_positions(self):
        return self.process('pos', '', 0)

    ## \brief Simple wrapper for the process method that allows to retrieve the current machine permutation.
    #
    #  \param [num_iterations] An int that specifies how many times the machine is to be stepped.
    #
    #  \returns A vector of vector of ints that specifies the permutations generated by the underlying machine.
    #                        
    def get_perm(self, num_iterations = 1):
        help = self.process('perm', '', 0, ['--num-iterations', str(num_iterations)])        
        return self._response_to_int_vectors(help)

    ## \brief Turns string parameter into a vector of vector of ints. Separator is '\n'.
    #
    #  \param [string_data] A string that contains the data which is to be transformed.
    #
    #  \returns A vector of vector of ints.
    #                                
    def _response_to_int_vectors(self, string_data):
        result = '[' + string_data.strip() + ']'
        result = result.replace('\n', ',')
        return eval(result)

    ## \brief Turns string parameter into a vector of strings. Separator is '\n'.
    #
    #  \param [string_data] A string that contains the data which is to be transformed.
    #
    #  \returns A vector of strings.
    #                                
    def _response_to_string_vector(self, string_data):
        result = "['" + string_data.strip() + "']"
        result = result.replace('\n', "','")
        return eval(result)


class AllEnigmaCLITests(simpletest.CompositeTest):
    def __init__(self, name):
        super().__init__(name)
        self._r_set = rotorsimtest.EnigmaRotorSet()
        self.add(rotorsimtest.M4EnigmaTest(self._r_set))
        self.add(rotorsimtest.M3UhrTest(self._r_set))
        self.add(rotorsimtest.KDTest(self._r_set))
        self.add(rotorsimtest.TirpitzTest(self._r_set))
        self.add(rotorsimtest.AbwehrTest(self._r_set))
        self.add(rotorsimtest.RailwayTest(self._r_set))
    
    def set_processor(self, proc):
        for i in self._test_cases:
            i.set_processor(proc)
    
    def test(self):
        result = True
        m4_state = rotorsim.RotorMachine.load_machine_state('reference/Enigma M4 Test 1.ini')
        machine = Processor(m4_state)
        try:
            result = self._r_set.load('reference/enigma_rotor_set.ini')
            
            if not result:
                self.append_note('Unable to load Enigma rotor set data')
            else:                    
                self.set_processor(machine)
                result = super().test()
        except:
            self.append_note("EXCEPTON!!!!")
            result = False
        
        return result


def get_module_test():
    verification_test = AllEnigmaCLITests("CLI Enigma verification test")    
    return verification_test

def execute_tests(num_iterations):
    tests = get_module_test()
    test_result = tests.test()
    tests.print_notes() 
