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

## @package rotorsim a Python3 interface to the C++ rotor machines implemented by rmsk2.
#           
# \file rotorsim.py
# \brief This file provides some Python3 classes that allow to en/decipher texts using the
#        C++ program rotorsim. On top of that these classes provide the functionality to
#        create rotor machine state files which then can be used with the rotorsim program.

import sys
import subprocess
from gi.repository import GLib
import enigrotorset as es
import os

RESULT_OK = 0
RESULT_ERROR = 42
RESULT_ROTORSIM_FORMAT = 43

# SIGABA rotor ids
SIGABA_ROTOR_0 = 0
SIGABA_ROTOR_1 = 1
SIGABA_ROTOR_2 = 2
SIGABA_ROTOR_3 = 3
SIGABA_ROTOR_4 = 4
SIGABA_ROTOR_5 = 5
SIGABA_ROTOR_6 = 6
SIGABA_ROTOR_7 = 7
SIGABA_ROTOR_8 = 8
SIGABA_ROTOR_9 = 9

SIGABA_INDEX_0 = 0
SIGABA_INDEX_1 = 1
SIGABA_INDEX_2 = 2
SIGABA_INDEX_3 = 3
SIGABA_INDEX_4 = 4

# SG39 rotor ids
SG39_ROTOR_0 = 0
SG39_ROTOR_1 = 1
SG39_ROTOR_2 = 2
SG39_ROTOR_3 = 3
SG39_ROTOR_4 = 4
SG39_ROTOR_5 = 5
SG39_ROTOR_6 = 6
SG39_ROTOR_7 = 7
SG39_ROTOR_8 = 8
SG39_ROTOR_9 = 9
ID_SG39_UKW = 100

# KL7 rotor and ring ids
KL7_ROTOR_A = 0
KL7_ROTOR_B = 1
KL7_ROTOR_C = 2
KL7_ROTOR_D = 3
KL7_ROTOR_E = 4
KL7_ROTOR_F = 5
KL7_ROTOR_G = 6
KL7_ROTOR_H = 7
KL7_ROTOR_I = 8
KL7_ROTOR_J = 9
KL7_ROTOR_K = 10
KL7_ROTOR_L = 11
KL7_ROTOR_M = 12

KL7_RING_1 =  100
KL7_RING_2 =  101
KL7_RING_3 =  102
KL7_RING_4 =  103
KL7_RING_5 =  104
KL7_RING_6 =  105
KL7_RING_7 =  106
KL7_RING_8 =  107
KL7_RING_9 =  108
KL7_RING_10 = 109
KL7_RING_11 = 110
KL7_RING_WIDE = 111

# Nema rotor and ring ids
NEMA_ROTOR_A = 0
NEMA_ROTOR_B = 1
NEMA_ROTOR_C = 2
NEMA_ROTOR_D = 3
NEMA_ROTOR_E = 4
NEMA_ROTOR_F = 5
NEMA_UKW = 6
NEMA_ETW = 7

NEMA_DRIVE_WHEEL_1 = 100
NEMA_DRIVE_WHEEL_2 = 101
NEMA_DRIVE_WHEEL_12 = 102
NEMA_DRIVE_WHEEL_13 = 103
NEMA_DRIVE_WHEEL_14 = 104
NEMA_DRIVE_WHEEL_15 = 105
NEMA_DRIVE_WHEEL_16 = 106
NEMA_DRIVE_WHEEL_17 = 107
NEMA_DRIVE_WHEEL_18 = 108
NEMA_DRIVE_WHEEL_19 = 109
NEMA_DRIVE_WHEEL_20 = 110
NEMA_DRIVE_WHEEL_21 = 111
NEMA_DRIVE_WHEEL_22 = 112
NEMA_DRIVE_WHEEL_23 = 113


## \brief This class serves as the generic something went wrong exception.
#
class RotorSimException(Exception):
    def __init__(self, error_code):
        Exception.__init__(self, 'RotorSim: ' + str(error_code))

## \brief This class allows to read a rotor set file that can be obtained by selecting
#         the 'Save rotor set data ...' in the Help menu of rotorvis or enigma.
#
class RotorSet:
    ## \brief Constructor.
    #    
    def __init__(self):
        ## \brief Vector of ints that specifies the rotor ids contained in this set.
        #    
        self.ids = []

        ## \brief A dictionary that maps rotor ids to dictionaries that have the keys
        #         permutation and ringdata.
        #    
        self.data = {}
        self.__key_file = GLib.KeyFile()

    ## \brief Loads the rotor set from the file which is specified in parameter file_name.
    #
    #  \param [file_name] Is a string holding the name of the rotor set file to read.
    #
    #  \returns A boolean that is either False when loading was unsuccessful and True otherwise 
    #    
    def load(self, file_name):
        result = True
    
        try:
            result = self.__key_file.load_from_file(file_name, 0)
            
            # read rotor ids
            self.ids = self.__key_file.get_integer_list('general', 'ids')
            
            # read permutation and ring data for each of the ids
            for i in self.ids:
                section_name = 'rotorid_' + str(i)
                
                perm = self.__key_file.get_integer_list(section_name, 'permutation')
                ring_data = self.__key_file.get_integer_list(section_name, 'ringdata')
                
                self.data[i] = {'permutation': perm, 'ringdata':ring_data}            
        except:
            self.__key_file = GLib.KeyFile()
            self.ids = []
            self.data = {}
            result = False
                
        return result

## \brief This class implements a set of transformations for permutations, where a permutation is
#         a vector of a certain length containing ints, in which each of the values 0 ... length-1
#         appears exactly once.
#
class Permutation:
    ## \brief Constructor. 
    #
    # \param [alphabet] Is a string. Has to specifiy the characters that are used to map the ints of the
    #        permutation to actual symbols.
    #
    def __init__(self, alphabet = "abcdefghijklmnopqrstuvwxyz"):
        self.__alphabet = alphabet
        self.__val = list(map(lambda x: x, range(len(self.__alphabet))))
        self.__inv_alpha = {}
        
        for i in range(len(alphabet)):
            self.__inv_alpha[self.__alphabet[i]] = i

    ## \brief Sets the permutation in use in this instance.
    #
    #  \param [value] Is a vector of ints that specifies the permutation in use in this instance.
    #
    #  \returns Nothing.
    #                    
    def from_int_vector(self, value):
        self.__val = value


    ## \brief Sets the permutation in use in this instance.
    #
    #  \param [s] Is a string of symbols that specifies the permutation in use in this instance.
    #         self.__alphabet is used to map the symbols of value to ints.
    #
    #  \returns Nothing.
    #                    
    def from_string(self, s):
        self.__val = list(map(lambda x: 0, range(len(self.__alphabet))))
        
        for i in range(len(self.__alphabet)):
            self.__val[i] = self.from_val(s[i])

    ## \brief Returns the permutation in use in this instance.
    #
    #  \returns The int vector that defines the permutaton in use in this instance.
    #                        
    def to_int_vector(self):
        return self.__val

    ## \brief Sets the permutation in use in this instance.
    #
    #  \param [letter_pairs] Is a string of characters that specifies the permutation in use in this instance.
    #         self.__alphabet is used to map the symbols of value to ints. This parameter is interpreted as a
    #         sequence of letter pairs each of which specifies one cycle that makes up the involution.
    #
    #  \returns Nothing.
    #                        
    def involution_from_pairs(self, letter_pairs):
        self.__val = list(map(lambda x: x, range(len(self.__alphabet))))
        
        for i in range(len(letter_pairs) // 2):
            self.__val[self.from_val(letter_pairs[2 * i])] = self.from_val(letter_pairs[(2 * i) + 1])
            self.__val[self.from_val(letter_pairs[(2 * i) + 1])] = self.from_val(letter_pairs[2 * i])

    ## \brief Returns the inverse permutation of the one in use in this instance.
    #
    #  \returns The int vector that defines the inverse of the permutaton that us in use in this instance.
    #                                    
    def to_inverse(self):
        result = list(map(lambda x: 0, range(len(self.__val))))
        
        for i in range(len(self.__val)):
            result[self.__val[i]] = i
            
        return result

    ## \brief Returns the reverse permutation of the one in use in this instance. Where the reverse of a 
    #         permutation p is defined as the  permutation that results from inserting a rotor that is wired 
    #         according to p in reverse in the rotor machine.
    #
    #  \returns The int vector that defines the reverse of the permutaton that us in use in this instance.
    #                                        
    def to_reverse(self):
        result = list(map(lambda x: 0, range(len(self.__val))))    
        inv_val = self.to_inverse()
        
        for i in range(len(self.__val)):
            result[i] = self.neg(inv_val[self.neg(i)])
        
        return result

    ## \brief Uses self.__alphabet to return the numeric value that corresponds to the symbol specified in
    #         parameter char.
    #
    #  \param [char] Is a character the value of which is mapped to an int.
    #
    #  \returns An int corresponding to the numeric value of char in alphabet self.__alphabet.
    #                                
    def from_val(self, char):
        return self.__inv_alpha[char]

    ## \brief Returns the additive inverse of v modulo len(self.__val).
    #
    #  \param [v] Is an int whose additive inverse is to be calculated.
    #
    #  \returns An int value that is the additive inverse of parameter v.
    #                                    
    def neg(self, v):
        perm_len = len(self.__val)
        return (perm_len - v) % perm_len

    ## \brief Returns the length (i.e. the number of elements) of this permutation.
    #
    #  \returns An int value specifying the length of the permutation.
    #                                        
    def get_len(self):
        return len(self.__val)

class UnsteckeredEnigmaState:
    def __init__(self, machine_name, rotor_set):
        self.__name = machine_name
        self.__rotor_set = rotor_set
        self.__slot_names = ['rotor_eintrittswalze', 'rotor_fast', 'rotor_middle', 'rotor_slow', 'rotor_umkehrwalze']
        self.__config = {}

    def get_slot_names(self):
        return self.__slot_names

    def insert_rotor(self, slot_id, rotor_id, ring_offset, rotor_pos, use_inverse = False):
        help = Permutation()
        self.__config[self.__slot_names[slot_id]] = {'rid': rotor_id, 'ringoffset':help.from_val(ring_offset), 'rotorpos':help.from_val(rotor_pos), 'inverse':use_inverse}
        
    def render_state(self):
        result = GLib.KeyFile()
        
        result.set_string('machine', 'name', self.__name)
        result.set_string('machine', 'rotorsetname', 'defaultset')
        
        for i in self.__slot_names:
            conf = self.__config[i]
            p = Permutation()
            p.from_int_vector(self.__rotor_set.data[conf['rid']]['permutation'])
            
            if conf['inverse']:
                result.set_integer_list(i, 'permutation', p.to_inverse())
            else:
                result.set_integer_list(i, 'permutation', p.to_int_vector())
            
            result.set_integer_list(i, 'ringdata', self.__rotor_set.data[conf['rid']]['ringdata'])
            result.set_integer(i, 'rid', conf['rid'])
            result.set_integer(i, 'ringid', conf['rid'])            
            result.set_boolean(i, 'insertinverse', False)
            result.set_integer(i, 'ringoffset', conf['ringoffset'])
            result.set_integer(i, 'rotordisplacement', (conf['rotorpos'] + p.neg(conf['ringoffset'])) % p.get_len()) 
        
        return result.to_data()[0].encode()        

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
    def __init__(self, state, binary_name = './rotorsim', progress_state = False):
        self.__rotorsim_binary = binary_name
        self.__state = state
        self.__do_state_progression = progress_state
    
    ## \brief Returns the current machine state known to this instance.
    #        
    def get_state(self):
        return state

    ## \brief Sets the current machine state known to this instance to the value given in the parameter new_state.
    #        
    #  \param [new_state] Is a byte array that represnets the new machine state to use.
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
                raise RotorSimException(RESULT_ROTORSIM_FORMAT)          
            
            # output is everyting before the 0xFF            
            result = (comm_result[0][:pos]).decode().strip()    
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
    #  \returns A string specifyng the new rotor positions.
    #                        
    def step(self):
        return self.process('step', '', 0)

    ## \brief Simple wrapper for the process method that allows to "setup step" the rotor given in parameter rotor_num.
    #         If self.__state does not describe a SIGABA calling this method does not step any rotors.
    #
    #  \param [rotor_num] Is an int specifying the number of the rotor which is to "setup step", where the numbering
    #                     goes from left to right of the driver machine rotors (the five in the middle in rotorvis).
    #
    #  \returns A string specifyng the new rotor positions.
    #                        
    def setup_step(self, rotor_num):
        return self.process('step', '', 0, ['--sigaba-setup', str(rotor_num)])


def test():
    f = open("Enigma M4 Test 1.ini", "rb")
    s = f.read()
    f.close()
    
    machine = Processor(s, progress_state = True)
    help = Permutation()
    
    print(machine.encrypt('nczwvusx'))
    print(help.to_int_vector())

