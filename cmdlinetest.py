################################################################################
# Copyright 2017 Martin Grap
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

## @package cmdlinetest contains tests for the rotorsim command line simulator implemented in python3.
#   
# \file cmdlinetest.py
# \brief This file contains classes that implement tests which can be used to verify that
#        the command line simulator implemented by the programs rotorsim and rotorstate is functional
#        and correct.

import subprocess
import os
import re
import rotorsimtest
import pyrmsk2.rotorsim as rotorsim
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
    #        operate.
    # \param [binary_name] Is a string. Has to contain the path to the binary of the rotorsim C++
    #        program
    # \param [progress_state] Is a boolean. If it is true then the member self.__state is updated with
    #        the new state after a call to encrypt, decrypt or process.
    #
    def __init__(self, state, binary_name = './rotorsim', progress_state = True):
        self.__rotorsim_binary = binary_name
        self.__state = state
        self.__do_state_progression = progress_state
        exp = '^.+\((.+)\): (.+)$'
        self._exp_c = re.compile(exp)
        
    
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
    #  \param [command] Is a string and has to specifiy the command which is used as the paraemter for the -c option.
    #  \param [input_data] Is a string which has to contain the input data. This value must not contain characters
    #                      that can not be encoded in ASCII.
    #  \param [output_grouping] Is an int. Specifies the group size which is used when producing the output string.
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
            raise rotorsim.RotorSimException(p.returncode)
        
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
    #  \param [new_rotor_pos] Is a string which specifies the new positions to which the visible rotors are set
    #         before the input data is processed. If this string is empty the rotor positions remain unchanged.
    #
    #  \returns A string containing the decryption result.
    #            
    def decrypt(self, input_data, new_rotor_pos = ''):
        additional_params = []
        
        if new_rotor_pos != '':
            additional_params = ['-p', new_rotor_pos]
    
        return self.process('decrypt', input_data, 0, additional_params)

    ## \brief Simple wrapper for the process method that allows to encrypt the string value specified in paramter 
    #         input_data.
    #
    #  \param [input_data] Is a string which has to contain the input data. This value must not contain characters
    #                      that can not be encoded in ASCII.
    #  \param [out_grouping] Is an int. Specifies the group size which is used when producing the output string.
    #
    #  \param [new_rotor_pos] Is a string which specifies the new positions to which the visible rotors are set
    #         before the input data is processed. If this string is empty the rotor positions remain unchanged.
    #
    #  \returns A string containing the encryption result.
    #                    
    def encrypt(self, input_data, out_grouping = 0, new_rotor_pos = ''):
        additional_params = []
        
        if new_rotor_pos != '':
            additional_params = ['-p', new_rotor_pos]
    
        return self.process('encrypt', input_data, out_grouping, additional_params)

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
    def sigaba_setup(self, rotor_num, num_iterations = 1):
        help =  self.process('sigabasetup', '', 0, ['--rotor-num', str(rotor_num), '--num-iterations', str(num_iterations)])
        return self._response_to_string_vector(help)

    ## \brief Simple wrapper for the process method that allows to retrieve the current rotor positions.
    #
    #  \returns A string specifying the current rotor positions.
    #                        
    def get_rotor_positions(self):
        return self.process('getpos', '', 0)

    ## \brief Simple wrapper for the process method that allows to retrieve the current rotor machine configuration in form
    #         of a string to string dictionary. The keys are the same values as accepted by rotorstate as command line
    #         parameters.
    #
    #  \returns A string to string dictionary holding the configuration information of the underlying rotor machine state.
    #                        
    def get_config(self):
        raw_string = self.process('getconfig', '', 0)
        lines = raw_string.split('\n')
        # drop line with machine name
        lines = lines[1:]
        result = {}
        
        for line in lines:
            match = self._exp_c.search(line)
            if match != None:
                result[match.group(1)] = match.group(2)
        
        return result                

    ## \brief Simple wrapper for the process method that allows to set the current rotor positions.
    #
    #  \param [new_rotor_positions] Is a string which specifies the new positions to which the visible rotors are set.
    #    
    #  \returns Nothing.
    #                        
    def set_rotor_positions(self, new_rotor_positions):
        self.process('encrypt', '', 0, ['-p', new_rotor_positions])

    ## \brief Simple wrapper for the process method that allows to retrieve the current machine permutation.
    #
    #  \param [num_iterations] An int that specifies how many times the machine is to be stepped.
    #
    #  \returns A vector of vector of ints that specifies the permutations generated by the underlying machine.
    #                        
    def get_permutations(self, num_iterations = 1):
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


## \brief This class allows to "call" to the C++ rotorstate program.
#
class CLIRotorState:
    ## \brief Constructor. 
    #
    # \param [binary_name] Is a string. Has to contain the path to the binary of the rotorstate C++
    #        program
    #
    def __init__(self, binary_name = './rotorstate'):
        self.__rotorstate_binary = binary_name

    ## \brief Constructs a command line for the C++ program rotorstate and executes rotorstate.
    #
    #  \param [machine_name] Is a string and has to specifiy the name of the machine for which a state is to be created.
    #  \param [configuration_params] Is a string to string dictionary that specifies the command line parameters and their
    #                                values which are used to determine the machine state.
    #  \param [rotor_positions] Is a string. It has to specify the rotor positions which are to be used in the newly created
    #                            state.
    #  \param [additional_params] Is a list of strings. Each element of the list is appended to the command line
    #                             when "calling" the C++ rotorstate program.    
    #
    #  \returns A byte array which contains the generated state.
    #            
    def process(self, machine_name, configuration_params, rotor_positions, additional_params = []):
        result = ''
        empty_input_data = ''
        
        proc_arguments = [self.__rotorstate_binary, machine_name]
        
        if rotor_positions != '':        
            proc_arguments = proc_arguments + ['-p', rotor_positions]
        
        for i in configuration_params.keys():
            proc_arguments.append("--" + i)            
            proc_arguments.append(configuration_params[i])
            
        proc_arguments = proc_arguments + additional_params            
        
        # call rotorstate program
        p = subprocess.Popen(proc_arguments, cwd=os.getcwd(), stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE, close_fds=True)
        comm_result = p.communicate(empty_input_data.encode())
        
        if p.returncode != 0:
            raise rotorsim.RotorSimException(p.returncode)
        
        pos = 0
        
        # search terminator (0xFF) for state output
        while (pos < len(comm_result[0])) and (comm_result[0][pos] != 255):
            pos += 1
        
        # A correct output is not empty and it has to contain 0xFF    
        if (pos < 1) or (pos == len(comm_result[0])):            
            raise ProcessorException(RESULT_PROCESSOR_FORMAT)          
        
        # new state is everyting before the 0xFF            
        result = comm_result[0][:pos]    
                    
        return result

    ## \brief Creates a new rotor machine state.
    #
    #  \param [machine_name] Is a string and has to specifiy the name of the machine for which a state is to be created.
    #  \param [configuration_params] Is a string to string dictionary that specifies the command line parameters and their
    #                                values which are used to determine the machine state.
    #  \param [rotor_positions] Is a string. It has to specify the rotor positions whcih are to be used in the newly created
    #                            state.
    #
    #  \returns A byte array which contains the generated state.
    #                
    def make_state(self, machine_name, configuration_params, rotor_positions = ''):
        return self.process(machine_name, configuration_params, rotor_positions)

    ## \brief Creates a new random rotor machine state.
    #
    #  \param [machine_name] Is a string and has to specifiy the name of the machine for which a state is to be created.
    #  \param [rotor_positions] Is a string. It has to specify the rotor positions which are to be used in the newly created
    #                            state.
    #
    #  \returns A byte array which contains the generated state.
    #                
    def make_random_state(self, machine_name, rotor_positions = ''):
        return self.process(machine_name, {}, rotor_positions, ['--random'])
        
    ## \brief Creates a new random rotor machine state constrained by a randomizer parameter
    #
    #  \param [machine_name] Is a string and has to specifiy the name of the machine for which a state is to be created.
    #  \param [randomizer_param] Is a string. It has to specify the randomizer parameter which is to be used.
    #  \param [rotor_positions] Is a string. It has to specify the rotor positions which are to be used in the newly created
    #                            state.
    #
    #  \returns A byte array which contains the generated state.
    #                
    def make_rand_parm_state(self, machine_name, randomizer_param, rotor_positions = ''):
        return self.process(machine_name, {}, rotor_positions, ['--randparm', randomizer_param])


## \brief This class implements a verification test for the rotorstate program.
#
class RotorStateTest(simpletest.SimpleTest):
    ## \brief Constructor.
    #
    def __init__(self):
        super().__init__('CLI RotorState test')

    ## \brief Performs the actual test. Creates Enigma M4 state an does a trial decryptions.
    #    
    #  \returns A boolean. Is True when the trial decryptions were successfull.
    #                    
    def test(self):
        result = super().test()
        
        try:
            r_state = CLIRotorState()
            
            # Check whether decryption with a known state produces expected result
            state = r_state.make_state('M4', {'rotors':'11241', 'rings':'aaav', 'plugs':'atbldfgjhmnwopqyrzvx'}, 'vjna')
            p = Processor(state)
            
            dec_result = p.decrypt('nczwvusx')
            self.append_note('CLI rotor state decryption result: ' + dec_result)
            
            result = result and (dec_result == 'VONVONJL')
            
            # Check whether decryption with a random state produces a consistent result
            plain = 'diesisteintest'.upper()
            state = r_state.make_random_state('KL7')
            p.set_state(state)
            enc_result = p.encrypt(plain)
            self.append_note('KL7 random state encryption result ' + enc_result)
            p.set_state(state)
            dec_result = p.decrypt(enc_result)
            self.append_note('KL7 random state decryption result ' + dec_result)
            
            result = result and (dec_result == plain)            

            # Check whether randomization parameter works as intended
            state = r_state.make_rand_parm_state('SIGABA', 'csp2900')
            self.append_note('Checking randomizer parameter usage')
            p.set_state(state)
            test_conf = p.get_config()
            self.append_note('Is generated machine state a CSP2900? {}'.format(test_conf['csp2900']))
            
            result = result and (test_conf['csp2900'] == 'true')
            
            # Check whether rotorsim and rotorstate interoperate with respect to configuration information
            state = r_state.make_rand_parm_state('Services', 'fancy')
            p.set_state(state)
            config_dict = p.get_config()
            rotor_pos = p.get_rotor_positions()
            enc_string = p.encrypt('diesisteintest')
            new_state = r_state.make_state('Services', config_dict, rotor_pos)
            p.set_state(new_state)
            dec_string = p.decrypt(enc_string)
            self.append_note('Rotorstate/rotorsim iterop test. Decrypted message {}'.format(dec_string))            
            
            result = result and (dec_string == 'diesisteintest'.upper())            
            
        except:
            self.append_note('------------ EXCEPTION ------------')
            result = False
        
        return result

## \brief This function serves as the context "object" for verification tests using the command line program.
#
def cli_context(inner_test):
    state_helper = CLIRotorState()
    m4_default_conf = rotorsim.M4EnigmaState.get_default_config()
    m4_state = state_helper.make_state('M4', m4_default_conf.config, m4_default_conf.rotor_pos)
    machine = Processor(m4_state)
    result = inner_test(machine, state_helper)
    
    return result
    

## \brief Returns a simpletest.SimpleTest object that allows to perform all the tests defined in this module.
#
#  \returns A simpletest.CompositeTest object.
#                
def get_module_test():
    all_tests = rotorsimtest.get_module_test(context=cli_context, verification_only=True)
    all_tests.add(RotorStateTest())        
    all_tests.name = 'CLI'
    
    return all_tests

## \brief Performs all the tests defined in this module.
#
#  \returns Nothing.
#                
def execute_tests(num_iterations):
    tests = get_module_test()
    test_result = tests.test()
    tests.print_notes() 
