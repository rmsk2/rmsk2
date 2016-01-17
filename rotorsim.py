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

## @package rotorsim A Python3 interface to the C++ rotor machines implemented by rmsk2.
#           
# \file rmsk2/rotorsim.py
# \brief This file provides some Python3 classes that allow to en/decipher texts using the
#        C++ program tlv_object. On top of that these classes provide the functionality to
#        create rotor machine state files which then can be used with the rotorsim program.

import sys
from gi.repository import GLib
import enigrotorset as es
import os
import datetime
import tlvobject
import rotorsetdata
import binascii

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

INSERT_NORMAL = 0
INSERT_INVERSE = 1
INSERT_REVERSE = 2

## \brief This class serves as the generic something went wrong exception.
#
class RotorSimException(Exception):
    ## \brief Constructor.
    #   
    #  \param [error_code] An integer. It has to specify an error code that is to be conveyed to the 
    #         recipient of an exception.
    #
    def __init__(self, error_code):
        Exception.__init__(self, 'RotorSim: ' + str(error_code))

## \brief This class allows to read, modify and save a rotor set file that otherwise can be obtained 
#         by selecting the 'Save rotor set data ...' in the Help menu of rotorvis or enigma.
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

    ## \brief Saves the rotor set data of this object into a file the name of which is specified in parameter file_name.
    #
    #  \param [file_name] Is a string holding the name of the rotor set file to save data to.
    #
    #  \returns A boolean that is either False when saving was unsuccessful and True otherwise 
    #    
    def save(self, file_name):        
        result = True
        key_file = GLib.KeyFile()
        try:        
            key_file.set_integer_list('general', 'ids', self.ids)
        
            for i in self.ids:
                section_name = 'rotorid_' + str(i)
                key_file.set_integer_list(section_name, 'permutation', self.data[i]['permutation'])
                key_file.set_integer_list(section_name, 'ringdata', self.data[i]['ringdata'])
            
            data_to_save = key_file.to_data()[0].encode()
            with open(file_name, 'wb') as f:
                f.write(data_to_save)
                f.close()                        
        except:
            result = False
        
        return result        

    ## \brief Adds or replaces the permutation and ring data for a rotor
    #
    #  \param [new_id] An integer. Has to specify the id of the rotor which is to add/the data of which is to be replaced
    #
    #  \param [new_perm] An integer sequence. Specifies the permutation data for the rotor.
    #
    #  \param [new_ring_data] An integer sequence. Specifies the ring data for the rotor.
    #
    #  \returns Nothing
    #                
    def add_rotor(self, new_id, new_perm, new_ring_data):
        self.data[new_id] = {}
        self.data[new_id]['permutation'] =  new_perm
        self.data[new_id]['ringdata'] =  new_ring_data
        
        self.ids = list(self.data.keys())

    ## \brief Deletes a rotor from this rotor set
    #
    #  \param [rotor_id] Is an integer which holds the id of the rotor which is to be deleted. If rotor_id does not specify
    #         a valid id this method does nothing
    #
    #  \returns Nothing.
    #    
    def delete_rotor(self, rotor_id):
        if rotor_id in self.data.keys():
            del self.data[rotor_id]
            self.ids = list(self.data.keys())

    ## \brief Loads the rotor set from the string which is specified in parameter data.
    #
    #  \param [data] Is a string holding the data contained in a rotor set ini file.
    #
    #  \returns A boolean that is either False when loading was unsuccessful and True otherwise 
    #    
    def load_from_string(self, data):
        result = True
        
        try:
            key_file = GLib.KeyFile()
            result = key_file.load_from_data(data, len(data), 0)
            
            if result:
                result = self.parse_key_file(key_file)            
        except:
            result = False
        
        return result

    ## \brief Loads the rotor set from the file which is specified in parameter file_name.
    #
    #  \param [file_name] Is a string holding the name of a rotor set ini file to read.
    #
    #  \returns A boolean that is either False when loading was unsuccessful and True otherwise 
    #    
    def load(self, file_name):
        result = True
        
        try:
            key_file = GLib.KeyFile()
            result = key_file.load_from_file(file_name, 0)
            
            if result:
                result = self.parse_key_file(key_file)            
        except:
            result = False
        
        return result
    
    ## \brief Extracts rotor set information from a GLib.KeyFile() object specified as parameter key_file.
    #
    #  \param [key_file] Is a GLib.KeyFile() object which represents the contents of a rotor set ini file.
    #
    #  \returns A boolean that is either False when loading was unsuccessful and True otherwise 
    #        
    def parse_key_file(self, key_file):
        result = True
    
        try:
            self.ids = []
            self.data = {}
            
            # read rotor ids
            self.ids = key_file.get_integer_list('general', 'ids')
            
            # read permutation and ring data for each of the ids
            for i in self.ids:
                section_name = 'rotorid_' + str(i)
                
                perm = key_file.get_integer_list(section_name, 'permutation')
                ring_data = key_file.get_integer_list(section_name, 'ringdata')
                
                self.data[i] = {'permutation': perm, 'ringdata':ring_data}            
        except:
            self.ids = []
            self.data = {}
            result = False
                
        return result

    ## \brief Changes the permutation of the specified rotor.
    #
    #  \param [rotor_id] Is a integer holding the id of the rotor the permutation of which is to be changed.
    #
    #  \param [new_perm] Is an integer list specifying the new permutation.
    #
    #  \returns Nothing
    #    
    def change_perm(self, rotor_id, new_perm):
        if rotor_id in self.data.keys():
            self.data[rotor_id]['permutation'] = new_perm

    ## \brief Changes the ring data of the specified rotor.
    #
    #  \param [rotor_id] Is a integer holding the id of the rotor the ring data of which is to be changed.
    #
    #  \param [new_ring_data] Is an integer list specifying the new ring data.
    #
    #  \returns Nothing
    #    
    def change_ring_data(self, rotor_id, new_ring_data):
        if rotor_id in self.data.keys():
            self.data[rotor_id]['ringdata'] = new_ring_data
    
    ## \brief Changes to permutation of the specified rotor to an involution specified by letter pairs.
    #
    #  \param [reflector_id] Is a integer holding the id of the rotor the permutation of which is to be changed.
    #
    #  \param [new_perm] Is a string. Each two consecutive letters speify a cycle of the involution.
    #
    #  \returns Nothing
    #        
    def change_reflector(self, reflector_id, new_perm):
        help = Permutation()
        help.involution_from_pairs(new_perm)
        self.change_perm(reflector_id, help.to_int_vector())

    ## \brief Returns the standard rotor set for a specified machine family
    #
    #  \param [set_name] Is a string. Has to specifiy the machine family for which the default RotorSet object is to
    #         be returned. Allowed values are: enigma, kl7, nema, sg39, sigaba, sigaba_index. In case the specified value
    #         does not match any of these strings the standard Enigma rotor set is returned
    #
    #  \returns An object of type RotorSet or an appropriate child of RotorSet, i.e. EnigmaRotorSet.
    #            
    @staticmethod
    def get_std_set(set_name):
        known_sets = {}
        known_sets['enigma'] = rotorsetdata.enigma_rotor_set
        known_sets['kl7'] = rotorsetdata.kl7_rotor_set        
        known_sets['nema'] = rotorsetdata.nema_rotor_set
        known_sets['sg39'] = rotorsetdata.sg39_rotor_set
        known_sets['sigaba'] = rotorsetdata.sigaba_rotor_set        
        known_sets['sigaba_index'] = rotorsetdata.sigaba_rotor_set_index
        
        if not (set_name in known_sets.keys()):
            set_name = 'enigma'
        
        if set_name != 'enigma':
            result = RotorSet()
        else:
            result = EnigmaRotorSet()
        
        data = binascii.a2b_base64(known_sets[set_name].encode()).decode()
        result.load_from_string(data)
        
        return result


## \brief This class implements a set of transformations for permutations. 
#
#  Here a permutation is a vector of a certain length containing ints, in which each of the values 0 
#  .. length-1 appears exactly once.
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


## \brief This class allows to create an INI file which represents a rotor machine's state.
#
class GenericRotorMachineState:
    ## \brief Constructor.
    #
    #  \param [machine_name] Is a string. It has to specify the name of the type of machine for which a state
    #         file is to be generated.
    #
    #  \param [slot_names] Is a sequence of strings. This squence has to specify the names of the rotor
    #         slots which are used by the machine type specified through parameter machine_name.
    #
    #  \param [rotor_set] Is an object with the same interface as RotorSet. It has to specify the rotor
    #         set whch is to be used to determine rotor permutations and ring data.
    #
    def __init__(self, machine_name, slot_names, rotor_set):
        self._name = machine_name
        self._rotor_set = rotor_set
        self._slot_names = slot_names
        ## \brief Data structure which holds the machine state
        self._config = {}
        ## \brief Holds the alphabet which is ued to map alphanumerical rotor positions to ints
        self._default_alpha = 'abcdefghijklmnopqrstuvwxyz'
        
        for slot in self._slot_names:
            self.insert_rotor(slot, rotor_set.ids[0], rotor_set.ids[0], 0, 0)

    ## \brief Returns the names of the rotor slots which are in use in a machine of type self._name
    #
    #  \returns A sequence of strings.
    #                                        
    def get_slot_names(self):
        return self._slot_names

    ## \brief This method is intended to save additional data for each rotor slot.
    #
    #  \param [slot_name] Is a string. It specifies the slot name for which additional data is to be saved.
    #
    #  \param [ini_file] Is an object of type GLib.KeyFile(). It specifies the INI file object into which the
    #         additional data is to be saved.
    #
    #  \returns Nothing.
    #                                                
    def _save_additional_rotor_data(self, slot_name, ini_file):
        pass

    ## \brief This method is intended to save additional data which is indpendent of individual rotor slots.
    #
    #  \param [ini_file] Is an object of type GLib.KeyFile(). It specifies the INI file object into which the
    #         additional data is to be saved.
    #
    #  \returns Nothing.
    #                                                    
    def _save_additional_data(self, ini_file):
        pass

    ## \brief This method adds a rotor to the data structure held by self._config.
    #
    #  \param [slot_name] Is a string. It specifies the name of the rotor slot into which a new rotor is to be
    #         inserted.
    #
    #  \param [rotor_id] Is an int. It specifies the id of the permutation which is to be used in the new rotor.
    #         The id has to be part of the data contained in self._rotor_set
    #
    #  \param [ring_id] Is an int. It specifies the id of the ring data which is to be used in the new rotor.
    #         The id has to be part of the data contained in self._rotor_set
    #
    #  \param [ring_offset] Is an int. It specifies the ring offset which is set for the new rotor. Has to be in
    #         in the range 0,...,rotor size (usually 25).
    #
    #  \param [rotor_pos] Is an int. It specifies the rotor position of the new rotor. Has to be in
    #         in the range 0,...,rotor size (usually 25).
    #
    #  \param [insert_type] Is an int. Currently the following three values are allowed: INSERT_NORMAL, 
    #         INSERT_INVERSE, INSERT_REVERSE. Inserting a rotor in inverse means that the inverse of the rotor
    #         permutation is used. Inserting a rotor in reverse means that the rotor is inserted backwards or 
    #         "the other way round".
    #
    #  \returns Nothing.
    #                                                    
    def insert_rotor(self, slot_name, rotor_id, ring_id, ring_offset, rotor_pos, insert_type = INSERT_NORMAL):
        self._config[slot_name] = {'rid': rotor_id, 'ringid':ring_id, 'ringoffset':ring_offset, 'rotorpos':rotor_pos, 'inserttype':insert_type}

    ## \brief This method transforms the data stored in self._config into an INI file.
    #
    #  \param [ini_file_object] Is an object of type GLib.KeyFile(). It specifies the INI file object into which the
    #         additional data is to be saved.
    #
    #  \returns An object of type GLib.KeyFile().
    #                                                    
    def fill_ini(self, ini_file_object):
        result = ini_file_object
        
        # Store machine name and name of default rotor set in INI file
        result.set_string('machine', 'name', self._name)
        result.set_string('machine', 'rotorsetname', 'defaultset')
        
        # Iterate over rotor slots
        for i in self._slot_names:
            section_name = 'rotor_' + i
            conf = self._config[i]
            # Save permutation data in INI file
            p = Permutation(self._default_alpha)
            p.from_int_vector(self._rotor_set.data[conf['rid']]['permutation'])
            
            # Transform original permutation data as defined by conf['inserttype']
            if conf['inserttype'] == INSERT_INVERSE:
                result.set_integer_list(section_name, 'permutation', p.to_inverse())
            elif conf['inserttype'] == INSERT_REVERSE:
                result.set_integer_list(section_name, 'permutation', p.to_reverse())
            else:
                result.set_integer_list(section_name, 'permutation', p.to_int_vector())
            
            # Save ring data
            result.set_integer_list(section_name, 'ringdata', self._rotor_set.data[conf['ringid']]['ringdata'])
            # Save other information
            result.set_integer(section_name, 'rid', conf['rid'])
            result.set_integer(section_name, 'ringid', conf['ringid'])            
            result.set_boolean(section_name, 'insertinverse', conf['inserttype'] == INSERT_REVERSE)
            result.set_integer(section_name, 'ringoffset', conf['ringoffset'])
            # Calculate and save actual rotor displacement as determined by rotor position and ring offset
            result.set_integer(section_name, 'rotordisplacement', (conf['rotorpos'] + p.neg(conf['ringoffset'])) % p.get_len()) 
            
            # Save additional "per rotor" data as defined
            self._save_additional_rotor_data(i, result)
        
        # Save additional "global" data as defined        
        self._save_additional_data(result)
        
        return result

    ## \brief This method transforms the data stored in self._config into a byte array
    #
    #  \returns A byte array.
    #                                                            
    def render_state(self):
        result = GLib.KeyFile()        
        result = self.fill_ini(result)
                
        return result.to_data()[0].encode()

    ## \brief Returns a RotorMachine object that has been initialized with this machine state
    #
    #  \param [server_obj] An object of type tlvobject.TlvServer. This object is used to create the RotorMachine object.
    #
    #  \returns A RotorMachine object.
    #                                                                    
    def as_rotor_machine(self, server_obj):
        state = self.render_state()        
        return RotorMachine(state, server_obj.address)

    ## \brief This method saves the machine state stored in self._config in a file which is named by parameter file_name.    
    #
    #  \param [file_name] Is a string. Has to contain the name of the file in which the state is stored.
    #    
    #  \returns A boolean. True means saving the state was successfull.
    #                                                                
    def save(self, file_name):
        result = True
        try:
            data_to_save = self.render_state()
            with open(file_name, 'wb') as f:
                f.write(data_to_save)
                f.close()
        except:
            result = False
        
        return result


## \brief This class allows to create an INI file which represents the state of a SIGABA driver, index or cipher
#         machine.
#
class SigabaSubMachineState(GenericRotorMachineState):
    ## \brief Constructor.
    #
    #  \param [slot_names] Is a sequence of strings. This squence has to specify the names of the rotor
    #         slots which are used by the machine type specified through parameter machine_name.
    #
    #  \param [rotor_set] Is an object with the same interface as RotorSet. It has to specify the rotor
    #         set which is to be used to determine rotor permutations and ring data.
    #
    #  \param [default_alpha] Is a string. It has to contain a permutation which is used to map rotor positions
    #         to numerical value
    #
    def __init__(self, slot_names, rotor_set, default_alpha = 'abcdefghijklmnopqrstuvwxyz'):
        super().__init__('SIGABA', slot_names, rotor_set)
        self._default_alpha = default_alpha

    ## \brief This method adds a rotor to the data structure held by self._config.
    #
    #  \param [slot_name] Is a string. It specifies the name of the rotor slot into which a new rotor is to be
    #         inserted.
    #
    #  \param [rotor_id] Is an int. It specifies the id of the permutation which is to be used in the new rotor.
    #         The id has to be part of the data contained in self._rotor_set
    #
    #  \param [rotor_pos_as_char] Is a char, i.e. a string of length one. It specifies the rotor position of
    #         the new rotor.
    #
    #  \param [insert_type] Is an int. Currently the following three values are allowed: INSERT_NORMAL, 
    #         INSERT_INVERSE, INSERT_REVERSE. Inserting a rotor in inverse means that the inverse of the rotor
    #         permutation is used. Inserting a rotor in reverse means that the rotor is inserted backwards or 
    #         "the other way round".
    #
    #  \returns Nothing.
    #                                                            
    def insert_sigaba_rotor(self, slot_name, rotor_id, rotor_pos_as_char, insert_type = INSERT_NORMAL):
        self.insert_rotor(slot_name, rotor_id, rotor_id, 0, 0, insert_type)
        self._config[slot_name]['rotorpos'] = self.get_rotor_pos(slot_name, rotor_pos_as_char)

    ## \brief This method can be used to map a rotor position in form of a character to a numerical value. It
    #         takes into account, if the rotor for which this mapping is to be performed, has been inserted in
    #         reverse.    
    #    
    #  \param [slot_name] Is a string. It specifies the name of the rotor slot for which the mapping is to be
    #         performed.
    #
    #  \param [pos_as_char] Is a char, i.e. a string of length one. It specifies the rotor position which is
    #         to be mapped to an integer value.
    #
    #  \returns An integer. It contains the mapping result.
    #
    def get_rotor_pos(self, slot_name, pos_as_char):
        result = 0
        perm_help = Permutation(self._default_alpha)
        
        result = perm_help.from_val(pos_as_char)
        if self._config[slot_name]['inserttype'] == INSERT_REVERSE:
            result = perm_help.neg(result)
            
        return result


## \brief This class allows to create an INI file which represents the state of a whole SIGABA machine which consists
#         of three submachines: The driver, the index and the cipher or crypt machine.
#            
class SigabaMachineState:
    ## \brief Constructor.
    #
    #  \param [normal_rotor_set] Is an object with the same interface as RotorSet. It has to specify the rotor
    #         set whch is to be used to determine rotor permutations and ring data for the driver and cipher rotors
    #
    #  \param [index_rotor_set] Is an object with the same interface as RotorSet. It has to specify the rotor
    #         set whch is to be used to determine rotor permutations and ring data for the index rotors
    #
    def __init__(self, normal_rotor_set, index_rotor_set):
        self._driver_machine = SigabaSubMachineState(['stator_l', 'slow', 'fast', 'middle', 'stator_r'], normal_rotor_set)
        self._crypt_machine = SigabaSubMachineState(['r_zero', 'r_one', 'r_two', 'r_three', 'r_four'], normal_rotor_set)
        self._index_machine = SigabaSubMachineState(['i_zero', 'i_one', 'i_two', 'i_three', 'i_four'], index_rotor_set, '0123456789')
        self._is_csp_2900 = False

    ## \brief This static method returns the default machine state for the SIGABA
    #
    #  \param [normal_rotor_set] Is an object with the same interface as RotorSet. It has to specify the SIGABA rotor
    #         set which is to be used to determine rotor permutations for the cipher and driver rotors. If normal_rotor_set
    #          is None then the default SIGABA rotor set is used.
    #
    #  \param [index_rotor_set] Is an object with the same interface as RotorSet. It has to specify the SIGABA rotor
    #         set which is to be used to determine rotor permutations for the index rotors. If index_rotor_set
    #          is None then the default SIGABA index rotor set is used. 
    #
    #  \returns A SigabaMachineState object. 
    #
    @staticmethod
    def get_default_state(normal_rotor_set = None, index_rotor_set = None):        
        if normal_rotor_set == None:
            normal_rotor_set = RotorSet.get_std_set('sigaba')
            
        if index_rotor_set == None:
            index_rotor_set = RotorSet.get_std_set('sigaba_index')        
    
        csp889_state = SigabaMachineState(normal_rotor_set, index_rotor_set)
        csp889_state.crypt.insert_sigaba_rotor('r_zero', SIGABA_ROTOR_0, 'o')
        csp889_state.crypt.insert_sigaba_rotor('r_one', SIGABA_ROTOR_1, 'o')        
        csp889_state.crypt.insert_sigaba_rotor('r_two', SIGABA_ROTOR_2, 'm', INSERT_REVERSE)        
        csp889_state.crypt.insert_sigaba_rotor('r_three', SIGABA_ROTOR_3, 'o')
        csp889_state.crypt.insert_sigaba_rotor('r_four', SIGABA_ROTOR_4, 'o')        

        csp889_state.driver.insert_sigaba_rotor('stator_l', SIGABA_ROTOR_5, 'o')
        csp889_state.driver.insert_sigaba_rotor('slow', SIGABA_ROTOR_6, 'o')        
        csp889_state.driver.insert_sigaba_rotor('fast', SIGABA_ROTOR_7, 'm', INSERT_REVERSE)        
        csp889_state.driver.insert_sigaba_rotor('middle', SIGABA_ROTOR_8, 'o')
        csp889_state.driver.insert_sigaba_rotor('stator_r', SIGABA_ROTOR_9, 'o')        

        csp889_state.index.insert_sigaba_rotor('i_zero', SIGABA_INDEX_0, '0')
        csp889_state.index.insert_sigaba_rotor('i_one', SIGABA_INDEX_1, '0')        
        csp889_state.index.insert_sigaba_rotor('i_two', SIGABA_INDEX_2, '0', INSERT_REVERSE)        
        csp889_state.index.insert_sigaba_rotor('i_three', SIGABA_INDEX_3, '0')
        csp889_state.index.insert_sigaba_rotor('i_four', SIGABA_INDEX_4, '0')                                
        
        return csp889_state
    
    ## \brief This method returns the driver machine in use in this SIGABA state.
    #
    #  \returns An object of type SigabaSubMachineState.
    #                                                                
    @property
    def driver(self):
        return self._driver_machine

    ## \brief This method returns the cipher/crypt machine in use in this SIGABA state.
    #
    #  \returns An object of type SigabaSubMachineState.
    #                                                                
    @property
    def crypt(self):
        return self._crypt_machine

    ## \brief This method returns the index machine in use in this SIGABA state.
    #
    #  \returns An object of type SigabaSubMachineState.
    #                                                                
    @property
    def index(self):
        return self._index_machine

    ## \brief This method allows the caller to retrieve whether the managed state is for a CSP889 or a CSP2900
    #
    #  \returns A boolean. True means a CSP2900 state is currently managed.
    #                                                                    
    @property
    def csp_2900_flag(self):
        return self._is_csp_2900

    ## \brief This method allows the caller to configure whether the managed state is for a CSP889 or a CSP2900
    #
    #  \param [new_value] Is a boolean. True means a CSP2900 state is intended to be managed.
    #    
    #  \returns Nothing. 
    #                                                                        
    @csp_2900_flag.setter
    def csp_2900_flag(self, new_value):
        self._is_csp_2900 = new_value

    ## \brief This method transforms the state data stored in the driver, index and crypt/cipher machine into a
    #         byte array.
    #
    #  \returns A byte array.
    #                                                                
    def render_state(self):
        result = GLib.KeyFile() 
        result = self.driver.fill_ini(result)
        result = self.crypt.fill_ini(result)
        result = self.index.fill_ini(result)        
        result.set_boolean('stepper', 'is_csp_2900', self.csp_2900_flag)
        return result.to_data()[0].encode()
        
    ## \brief Returns a RotorMachine object that has been initialized with this machine state
    #
    #  \param [server_obj] An object of type tlvobject.TlvServer. This object is used to create the RotorMachine object.
    #
    #  \returns A RotorMachine object.
    #                                                                    
    def as_rotor_machine(self, server_obj):
        state = self.render_state()        
        return RotorMachine(state, server_obj.address)        

    ## \brief This method saves the machine state as returned by self.render_state() in a file which is named by parameter file_name.    
    #
    #  \param [file_name] Is a string. Has to contain the name of the file in which the state is stored.
    #    
    #  \returns A boolean. True means saving the state was successfull.
    #                                                                        
    def save(self, file_name):
        result = True
        try:
            data_to_save = self.render_state()
            f = open(file_name, 'wb')
            f.write(data_to_save)
            f.close()
        except:
            result = False
        
        return result


## \brief This class allows to create an INI file which represents the state of a Nema machine.
#            
class NemaState(GenericRotorMachineState):
    ## \brief Constructor.
    #
    #  \param [rotor_set] Is an object with the same interface as RotorSet. It has to specify the Nema rotor
    #         set which is to be used to determine rotor permutations and ring data.
    #
    def __init__(self, rotor_set):
        slot_names = ['etw', 'drive1', 'contact2', 'drive3', 'contact4', 'drive5', 'contact6', 'drive7', 'contact8', 'drive9', 'contact10']
        super().__init__('Nema', slot_names, rotor_set)
        self.is_war_machine = True
        self._default_alpha = 'ijklmnopqrstuvwxyzabcdefgh'
        self.insert_rotor('etw', NEMA_ETW, NEMA_ETW, 0, 0, INSERT_INVERSE)

    ## \brief This static method returns the default machine state for the Nema
    #
    #  \param [rotor_set] Is an object with the same interface as RotorSet. It has to specify the Nema rotor
    #         set which is to be used to determine rotor permutations and ring data. If rotor_set is None then
    #         the default Nema rotor set is used.
    #
    #  \returns A NemaState object. 
    #
    @staticmethod
    def get_default_state(rotor_set = None):    
        if rotor_set == None:
            rotor_set = RotorSet.get_std_set('nema')
    
        nema_state = NemaState(rotor_set)
        nema_state.insert_nema_rotor('drive1', NEMA_DRIVE_WHEEL_22, 'v')
        nema_state.insert_nema_rotor('contact2', NEMA_ROTOR_D, 'e')        
        nema_state.insert_nema_rotor('drive3', NEMA_DRIVE_WHEEL_15, 'o')        
        nema_state.insert_nema_rotor('contact4', NEMA_ROTOR_C, 's')
        nema_state.insert_nema_rotor('drive5', NEMA_DRIVE_WHEEL_14, 'q')        
        nema_state.insert_nema_rotor('contact6', NEMA_ROTOR_B, 'z')
        nema_state.insert_nema_rotor('drive7', NEMA_DRIVE_WHEEL_13, 'p')        
        nema_state.insert_nema_rotor('contact8', NEMA_ROTOR_A, 'q')        
        nema_state.insert_nema_rotor('drive9', NEMA_DRIVE_WHEEL_12, 'q')
        nema_state.insert_nema_rotor('contact10', NEMA_UKW, 'm')        
        
        return nema_state

    ## \brief This method allows the caller to retrieve whether the managed state is for a war or training Nema
    #
    #  \returns A boolean. True means a state for a war machine is currently managed.
    #                                                                        
    @property
    def is_war_machine(self):
        return self._is_war_machine

    ## \brief This method allows the caller to configure whether the managed state is for a war or training Nema
    #
    #  \param [new_val] Is a boolean. True means a state for a war machine is intended to be managed.
    #    
    #  \returns Nothing. 
    #                                                                            
    @is_war_machine.setter
    def is_war_machine(self, new_val):
        self._is_war_machine = new_val
                
        if new_val:
            red_wheel_data = self.make_red_wheel(NEMA_DRIVE_WHEEL_22, NEMA_DRIVE_WHEEL_1)
        else:
            red_wheel_data = self.make_red_wheel(NEMA_DRIVE_WHEEL_23, NEMA_DRIVE_WHEEL_2)
        
        self._config['redwheeldata'] = red_wheel_data

    ## \brief This method saves the ring data for the red wheel, which is special as it is a combination of two
    #         sets of ring data
    #
    #  \param [slot_name] Is a string. It specifies the slot name for which additional data is to be saved.
    #
    #  \param [ini_file] Is an object of type GLib.KeyFile(). It specifies the INI file object into which the
    #         additional data is to be saved.
    #
    #  \returns Nothing. 
    #                                                                            
    def _save_additional_rotor_data(self, slot_name, ini_file):
        if slot_name == 'drive1':
            ini_file.set_integer_list('rotor_drive1', 'ringdata', self._config['redwheeldata'])

    ## \brief This method is intended to create the ring data for the red wheel, which is special as it is a combination of two
    #         sets of ring data
    #
    #  \param [id_left] Is an integer. Specifies the id of the ring data which makes up the left side of the red wheel.
    #
    #  \param [id_right] Is an integer. Specifies the id of the ring data which makes up the right side of the red wheel.
    #
    #  \returns A sequence of integers. The sequence contains the created ring data.
    #                                                                                
    def make_red_wheel(self, id_left, id_right):
        left_notches = self._rotor_set.data[id_left]['ringdata']
        right_notches = self._rotor_set.data[id_right]['ringdata']
        
        result = []
        for i in range(len(left_notches)):
            result.append((left_notches[i] << 1) | right_notches[i])
            
        return result

    ## \brief This method adds a rotor to the data structure held by self._config.
    #
    #  \param [slot_name] Is a string. It specifies the name of the rotor slot into which a new rotor is to be
    #         inserted.
    #
    #  \param [rotor_id] Is an int. It specifies the id of the permutation which is to be used in the new rotor.
    #         The id has to be part of the data contained in self._rotor_set
    #
    #  \param [rotor_pos_as_char] Is a char, i.e. a string of length one. It specifies the rotor position of
    #         the new rotor.
    #
    #  \returns Nothing.
    #                                                                    
    def insert_nema_rotor(self, slot_name, rotor_id, rotor_pos_as_char):
        if slot_name != 'drive1':
            self.insert_rotor(slot_name, rotor_id, rotor_id, 2, self.get_pos_as_int(rotor_pos_as_char))
        else:
            red_id = NEMA_DRIVE_WHEEL_23            
            if self.is_war_machine:
                red_id = NEMA_DRIVE_WHEEL_22
            
            self.insert_rotor(slot_name, red_id, red_id, 2, self.get_pos_as_int(rotor_pos_as_char))

    ## \brief This method can be used to map an alphanumeric rotor position to numeric value.
    #
    #  \param [rotor_pos_as_char] Is a char, i.e. a one element string. Specifies the alphanumeric rotor positon which is
    #         to be mapped to a numeric value.
    #
    #  \returns An integer. The calculated numeric value.
    #                                                                                        
    def get_pos_as_int(self, rotor_pos_as_char):
        p = Permutation(self._default_alpha)
        result = p.from_val(rotor_pos_as_char)
        return result


## \brief This class allows to create an INI file which represents the state of a KL7 machine.
#
#  This class uses the permutation 'ab1cde2fg3hij4klm5no6pqr7st8uvw9xyz0' to map rotor positions and
#  ring offsets to numeric values. In other simulator software the positions named 0-9 in the above
#  permutations are designated b+ (1), e+ (2), ..., z+ (0).
#            
class KL7State(GenericRotorMachineState):
    ## \brief Constructor.
    #
    #  \param [rotor_set] Is an object with the same interface as RotorSet. It has to specify the KL7 rotor
    #         set which is to be used to determine rotor permutations and ring data.
    #
    def __init__(self, rotor_set):
        slot_names = ['kl7_rotor_1', 'kl7_rotor_2', 'kl7_rotor_3', 'kl7_rotor_4', 'kl7_rotor_5', 'kl7_rotor_6', 'kl7_rotor_7', 'kl7_rotor_8']
        super().__init__('KL7', slot_names, rotor_set)
        self._default_alpha = 'ab1cde2fg3hij4klm5no6pqr7st8uvw9xyz0'

    ## \brief This static method returns the default machine state for the KL7
    #
    #  \param [rotor_set] Is an object with the same interface as RotorSet. It has to specify the KL7 rotor
    #         set which is to be used to determine rotor permutations and ring data. If rotor_set is None then
    #         the default KL7 rotor set is used.
    #
    #  \returns A KL7State object. 
    #
    @staticmethod
    def get_default_state(rotor_set = None):
        if rotor_set == None:
            rotor_set = RotorSet.get_std_set('kl7')
    
        kl7_state = KL7State(rotor_set)
        kl7_state.insert_kl7_rotor('kl7_rotor_1', KL7_ROTOR_A, KL7_RING_1, 'k', 26, 'f')
        kl7_state.insert_kl7_rotor('kl7_rotor_2', KL7_ROTOR_B, KL7_RING_2, 'a', 0, 'a')        
        kl7_state.insert_kl7_rotor('kl7_rotor_3', KL7_ROTOR_C, KL7_RING_3, 'a', 0, 'a')                
        kl7_state.insert_stationary_rotor(KL7_ROTOR_L, 16)
        kl7_state.insert_kl7_rotor('kl7_rotor_5', KL7_ROTOR_D, KL7_RING_4, 'a', 0, 'a')
        kl7_state.insert_kl7_rotor('kl7_rotor_6', KL7_ROTOR_E, KL7_RING_5, 'a', 0, 'a')
        kl7_state.insert_kl7_rotor('kl7_rotor_7', KL7_ROTOR_F, KL7_RING_6, 'a', 0, 'a')
        kl7_state.insert_kl7_rotor('kl7_rotor_8', KL7_ROTOR_G, KL7_RING_7, 'a', 0, 'a')        
        
        return kl7_state

    ## \brief This method adds a movable KL7 rotor to the data structure held by self._config.
    #
    #  \param [slot_name] Is a string. It specifies the name of the rotor slot into which a new rotor is to be
    #         inserted.
    #
    #  \param [rotor_id] Is an int. It specifies the id of the permutation which is to be used in the new rotor.
    #         The id has to be part of the data contained in self._rotor_set
    #
    #  \param [ring_id] Is an int. It specifies the id of the ring data which is to be used in the new rotor.
    #         The id has to be part of the data contained in self._rotor_set
    #    
    #  \param [ring_offset_as_char] Is a char, i.e. a string of length one. It specifies the offset by which
    #         the ring is rotated with respect to the rotor. Allowed characters are 'ab1cde2fg3hij4klm5no6pqr7st8uvw9xyz0'
    #    
    #  \param [letter_ring_offset] Is an integer. It specifies the offset by which the letter ring is rotated
    #         with respect to the rotor. Allowed values are 0, ..., 35
    #    
    #  \param [rotor_pos_as_char] Is a char, i.e. a string of length one. It specifies the rotor position of
    #         the new rotor.
    #
    #  \returns Nothing.
    #    
    def insert_kl7_rotor(self, slot_name, rotor_id, ring_id, ring_offset_as_char, letter_ring_offset, rotor_pos_as_char):
        p = Permutation(self._default_alpha)
        self.insert_rotor(slot_name, rotor_id, ring_id, p.from_val(ring_offset_as_char), p.from_val(rotor_pos_as_char))
        self._config[slot_name]['letterring'] = letter_ring_offset

    ## \brief This method adds the stationary rotor ('kl7_rotor_4') to the data structure self._config
    #
    #  \param [rotor_id] Is an integer. It specifies the id of the permutation which is to be used in the new rotor.
    #         The id has to be part of the data contained in self._rotor_set
    #
    #  \param [letter_ring_offset] Is an integer. It specifies the offset by which the letter ring is rotated
    #         with respect to the rotor. Allowed values are 0, ..., 35. This determines in essence the rotor position
    #         of the stationary rotor.
    #    
    #  \returns Nothing.
    #        
    def insert_stationary_rotor(self, rotor_id, letter_ring_offset):
        self.insert_kl7_rotor('kl7_rotor_4', rotor_id, KL7_RING_WIDE, 'a', 0, self._default_alpha[letter_ring_offset])
        self._config['kl7_rotor_4']['letterring'] = 0

    ## \brief This method saves the letter ring offset data for each KL7 rotor in the reated INI file,
    #
    #  \param [slot_name] Is a string. It specifies the slot name for which additional data is to be saved.
    #
    #  \param [ini_file] Is an object of type GLib.KeyFile(). It specifies the INI file object into which the
    #         additional data is to be saved.
    #
    #  \returns Nothing. 
    #    
    def _save_additional_rotor_data(self, slot_name, ini_file):
        section_name = 'rotor_' + slot_name
        ini_file.set_integer(section_name, 'letterring', self._config[slot_name]['letterring'])
        
        if slot_name != 'kl7_rotor_4':
            ini_file.set_integer(section_name, 'rotordisplacement', (self._config[slot_name]['letterring'] + self._config[slot_name]['rotorpos']) % len(self._default_alpha))


## \brief This class allows to create an INI file which represents the state of a Schlüsselgerät 39 (SG39) machine.
#
#  The number rotor positions and number of pins of the drive wheels can be described as follows:
#
#  'rotor_1': a, ..., u and 21 pins
#  'rotor_2': a, ..., w and 23 pins
#  'rotor_3': a, ..., y and 25 pins
#
class SG39State(GenericRotorMachineState):
    ## \brief Constructor.
    #
    #  \param [rotor_set] Is an object with the same interface as RotorSet. It has to specify the SG39 rotor
    #         set which is to be used to determine rotor permutations and ring data.
    #
    def __init__(self, rotor_set):
        slot_names = ['rotor_1', 'rotor_2', 'rotor_3', 'rotor_4', 'umkehrwalze']
        super().__init__('SG39', slot_names, rotor_set)

    ## \brief This static method returns the default machine state for the SG39
    #
    #  \param [rotor_set] Is an object with the same interface as RotorSet. It has to specify the SG39 rotor
    #         set which is to be used to determine rotor permutations and ring data. If rotor_set is None then
    #         the default SG39 rotor set is used.
    #
    #  \returns A SG39State object. 
    #
    @staticmethod
    def get_default_state(rotor_set = None):        
        if rotor_set == None:
            rotor_set = RotorSet.get_std_set('sg39')
    
        rotor_set.change_reflector(ID_SG39_UKW, 'awbicvdketfmgnhzjulopqrysx')
        sg39_state = SG39State(rotor_set)
        sg39_state.set_plugboard('ldtrmihoncpwjkbyevsaxgfzuq')
        sg39_state.insert_sg39_rotor('rotor_1', SG39_ROTOR_5, 'd', [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0])
        sg39_state.insert_sg39_rotor('rotor_2', SG39_ROTOR_1, 'q', [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0])        
        sg39_state.insert_sg39_rotor('rotor_3', SG39_ROTOR_4, 'r', [1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0])  
        sg39_state.insert_sg39_rotor('rotor_4', SG39_ROTOR_3, 'f')  
        sg39_state.configure_sg39_drive_wheel('rotor_1', 'h', [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0])
        sg39_state.configure_sg39_drive_wheel('rotor_2', 'p', [1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1])        
        sg39_state.configure_sg39_drive_wheel('rotor_3', 'a', [0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0])                
        sg39_state.insert_sg39_rotor('umkehrwalze', ID_SG39_UKW, 'a')                     
        
        return sg39_state

    ## \brief This method adds a rotor to the data structure held by self._config.
    #
    #  \param [slot_name] Is a string. It specifies the name of the rotor slot into which a new rotor is to be
    #         inserted.
    #
    #  \param [rotor_id] Is an int. It specifies the id of the permutation which is to be used in the new rotor.
    #         The id has to be part of the data contained in self._rotor_set
    #
    #  \param [rotor_pos_as_char] Is a char, i.e. a string of length one. It specifies the rotor position of
    #         the new rotor.
    #
    #  \param [ring_data] Is a vector of 26 integers, where each element of the vector is a 0 or a 1. It specifies the
    #         ring data which is to be set on the corresponding rotors. For 'rotor_4' or 'umkehrwalze the vector can be
    #         None.
    #
    #  \returns Nothing.
    #    
    def insert_sg39_rotor(self, slot_name, rotor_id, rotor_pos_as_char, ring_data = None):
        p = Permutation(self._default_alpha)
        self.insert_rotor(slot_name, rotor_id, rotor_id, 0, p.from_val(rotor_pos_as_char))
        self._config[slot_name]['ringdata'] = ring_data

    ## \brief Adds information about the pins of a drive wheel to self._config.
    #
    #  \param [slot_name] Is a string. It specifies the name of the rotor slot which the drive wheel controls.
    #         Must be one of 'rotor_1', 'rotor_2', 'rotor_3'.
    #
    #  \param [wheelpos_as_char] Is a char, i.e. a string of length one. It specifies the rotor position of
    #         the wheel_pos.
    #
    #  \param [ring_data] Is a vector of integers, where each element of the vector is a 0 or a 1. It specifies the
    #         ring data which is to be set on the corresponding drive wheel. 
    #
    #  \returns Nothing.
    #        
    def configure_sg39_drive_wheel(self, slot_name, wheelpos_as_char, ring_data):
        p = Permutation(self._default_alpha)
        self._config[slot_name]['drivewheel'] = {'wheelpos':p.from_val(wheelpos_as_char), 'ringdata':ring_data}

    ## \brief Sets the permutation of the SG39's plugboard.
    #
    #  \param [plugboard_permutation] Is a string which deteremines to permutation to set.
    #
    #  \returns Nothing
    #        
    def set_plugboard(self, plugboard_permutation):
        p = Permutation()
        p.from_string(plugboard_permutation)
        self._config['plugboard'] = p.to_int_vector()

    ## \brief This method saves the variable ring data, wheel data and wheel position for the rotors to which
    #         this is applicable in the INI file.
    #
    #  \param [slot_name] Is a string. It specifies the slot name for which additional data is to be saved.
    #
    #  \param [ini_file] Is an object of type GLib.KeyFile(). It specifies the INI file object into which the
    #         additional data is to be saved.
    #
    #  \returns Nothing. 
    #    
    def _save_additional_rotor_data(self, slot_name, ini_file):
        section_name = 'rotor_' + slot_name
        
        if slot_name in ['rotor_1', 'rotor_2', 'rotor_3']:        
            ini_file.set_integer_list(section_name, 'ringdata', self._config[slot_name]['ringdata'])
            ini_file.set_integer_list(section_name, 'wheeldata', self._config[slot_name]['drivewheel']['ringdata'])
            ini_file.set_integer(section_name, 'wheelpos', self._config[slot_name]['drivewheel']['wheelpos'])

    ## \brief This method saves the plugboard permutation in the INI file.
    #
    #  \param [ini_file] Is an object of type GLib.KeyFile(). It specifies the INI file object into which the
    #         additional data is to be saved.
    #
    #  \returns Nothing. 
    #        
    def _save_additional_data(self, ini_file):
        ini_file.set_integer_list('plugboard', 'entry', self._config['plugboard'])


## \brief This class extends RotorSet by a method which can be used to calculate a permutation as realized
#         by the Enigma Umkehrwalze D.
#            
class EnigmaRotorSet(RotorSet):
    ## \brief Constructor.
    #
    def __init__(self):
        super().__init__()

    ## \brief Changes the permutation with the id es.UKW_D to a new value.
    #
    #  \param [new_perm] Is a string which deteremines to permutation to set. The string is a squence of
    #         12 letter pairs. The letter pair 'jy' is automatically added to these pairs as this pair is
    #         fixed in a UKW D.
    #
    #  \returns Nothing
    #
    def change_ukw_d(self, new_perm):
        help = Permutation('yzxwvutsrqponjmlkihgfedcba')
        help.involution_from_pairs(new_perm + 'jy')
        self.change_perm(es.UKW_D, help.to_int_vector())    


## \brief This class allows to create an INI file which represents the state of a Typex machine.
#            
class TypexState(GenericRotorMachineState):
    ## \brief Constructor.
    #
    #  \param [enigma_rotor_set] Is an object with the same interface as EnigmaRotorSet. It has to specify the Enigma rotor
    #         set which is to be used to determine rotor permutations and ring data.
    #
    def __init__(self, enigma_rotor_set):
        slots = ['eintrittswalze', 'stator1', 'stator2', 'fast', 'middle', 'slow', 'umkehrwalze']
        super().__init__('Typex', slots, enigma_rotor_set)
        self.insert_rotor('eintrittswalze', es.TYPEX_ETW, es.TYPEX_ETW, 0, 0, INSERT_INVERSE)
    
    ## \brief This static method returns the default machine state for the Typex
    #
    #  \param [rotor_set] Is an object with the same interface as EnigmaRotorSet. It has to specify the Typex rotor
    #         set which is to be used to determine rotor permutations and ring data. If rotor_set is None then
    #         the default Enigma rotor set is used.
    #
    #  \returns A TypexState object. 
    #
    @staticmethod
    def get_default_state(rotor_set = None):
        if rotor_set == None:
            rotor_set = RotorSet.get_std_set('enigma')
    
        rotor_set.change_reflector(es.TYPEX_SP_02390_UKW, 'arbycudheqfsglixjpknmotwvz')
        typex_state = TypexState(rotor_set)
        typex_state.insert_typex_rotor('stator1', es.TYPEX_SP_02390_E, 'a', 'a')
        typex_state.insert_typex_rotor('stator2', es.TYPEX_SP_02390_D, 'a', 'a')        
        typex_state.insert_typex_rotor('fast', es.TYPEX_SP_02390_C, 'a', 'a', INSERT_REVERSE)
        typex_state.insert_typex_rotor('middle', es.TYPEX_SP_02390_B, 'a', 'a')        
        typex_state.insert_typex_rotor('slow', es.TYPEX_SP_02390_A, 'a', 'a')  
        typex_state.insert_typex_rotor('umkehrwalze', es.TYPEX_SP_02390_UKW, 'a', 'a')                     
        
        return typex_state
    
    ## \brief This method adds a rotor to the data structure held by self._config.
    #
    #  \param [slot_name] Is a string. It specifies the name of the rotor slot into which a new rotor is to be
    #         inserted.
    #
    #  \param [rotor_id] Is an int. It specifies the id of the permutation which is to be used in the new rotor.
    #         The id has to be part of the data contained in self._rotor_set
    #
    #  \param [ring_offset_as_char] Is a char, i.e. a string of length one. It specifies the offset by which the
    #         ring is rotated with repsect to the wiring core.
    #
    #  \param [rotor_pos_as_char] Is a char, i.e. a string of length one. It specifies the rotor position of
    #         the new rotor.
    #
    #  \param [insert_type] Is an int. Currently the following three values are allowed: INSERT_NORMAL, 
    #         INSERT_INVERSE, INSERT_REVERSE. Inserting a rotor in inverse means that the inverse of the rotor
    #         permutation is used. Inserting a rotor in reverse means that the rotor is inserted backwards or 
    #         "the other way round".
    #
    #  \returns Nothing.
    #                                                                    
    def insert_typex_rotor(self, slot_name, rotor_id, ring_offset_as_char, rotor_pos_as_char, insert_type = INSERT_NORMAL):
        p = Permutation()
        self.insert_rotor(slot_name, rotor_id, rotor_id, p.from_val(ring_offset_as_char), p.from_val(rotor_pos_as_char), insert_type)


## \brief This class allows to create an INI file which represents the state of a machine belonging to the Enigma
#         family.
#            
class BasicEnigmaState(GenericRotorMachineState):
    ## \brief Constructor.
    #
    #  \param [machine_name] Is a string. It has to specify the name of the type of machine for which a state
    #         file is to be generated.
    #
    #  \param [machine_type] Is a string. It has to specify the specific Enigma variant for which a state file
    #         is to be created.
    #
    #  \param [slots] Is a sequence of strings. This squence has to specify the names of the rotor
    #         slots which are used by the machine type specified through parameter machine_type.
    #
    #  \param [rotor_set] Is an object with the same interface as EnigmaRotorSet. It has to specify the rotor
    #         set whch is to be used to determine rotor permutations and ring data.
    #
    def __init__(self, machine_name, machine_type, slots, rotor_set):
        super().__init__(machine_name, slots, rotor_set)
        self._machine_type = machine_type

    ## \brief This method adds a rotor to the data structure held by self._config.
    #
    #  \param [slot_name] Is a string. It specifies the name of the rotor slot into which a new rotor is to be
    #         inserted.
    #
    #  \param [rotor_id] Is an int. It specifies the id of the permutation which is to be used in the new rotor.
    #         The id has to be part of the data contained in self._rotor_set
    #
    #  \param [ring_offset_as_char] Is a char, i.e. a string of length one. It specifies the offset by which the
    #         ring is rotated with repsect to the wiring core.
    #
    #  \param [rotor_pos_as_char] Is a char, i.e. a string of length one. It specifies the rotor position of
    #         the new rotor.
    #
    #  \returns Nothing.
    #                                                                            
    def insert_enigma_rotor(self, slot_name, rotor_id, ring_offset_as_char, rotor_pos_as_char):
        p = Permutation()
        self.insert_rotor(slot_name, rotor_id, rotor_id, p.from_val(ring_offset_as_char), p.from_val(rotor_pos_as_char))

    ## \brief This method saves the UKW D wiring and the machine type in the INI file.
    #
    #  \param [ini_file] Is an object of type GLib.KeyFile(). It specifies the INI file object into which the
    #         additional data is to be saved.
    #
    #  \returns Nothing. 
    #
    def _save_additional_data(self, ini_file):  
        super()._save_additional_data(ini_file)      
        
        ini_file.set_integer_list('machine', 'ukwdwiring', self._rotor_set.data[es.UKW_D]['permutation'])
        ini_file.set_string('machine', 'machinetype', self._machine_type)


## \brief This class allows to create an INI file which represents the state of an Enigma machine without
#         a plugboard.
#
class UnsteckeredEnigmaState(BasicEnigmaState):
    ## \brief Constructor.
    #
    #  \param [machine_name] Is a string. It has to specify the name of the type of machine for which a state
    #         file is to be generated. Allowed values for machine_name are 'TirpitzEnigma', 'RailwayEnigma'
    #         'AbwehrEnigma' and 'KDEnigma'
    #
    #  \param [rotor_set] Is an object with the same interface as EnigmaRotorSet. It has to specify the rotor
    #         set whch is to be used to determine rotor permutations and ring data.
    #
    #  \param [etw_id] Is an integer which contains the id of the Eintrittswalze that is in use in the machine
    #         variant specified by machine type. Allowed values for etw_id are: es.WALZE_KD_ETW, es.WALZE_T_ETW,
    #         es.WALZE_ABW_ETW, es.WALZE_RB_ETW.
    #    
    def __init__(self, machine_name, rotor_set, etw_id):
        slots = ['eintrittswalze', 'fast', 'middle', 'slow', 'umkehrwalze']
        type_helper = {}
        type_helper['TirpitzEnigma'] = 'Tirpitz'
        type_helper['RailwayEnigma'] = 'Railway'
        type_helper['AbwehrEnigma'] = 'Abwehr'        
        type_helper['KDEnigma'] = 'KD'        
        super().__init__(machine_name, type_helper[machine_name], slots, rotor_set)
        
        self.insert_rotor('eintrittswalze', etw_id, etw_id, 0, 0, INSERT_INVERSE)
        
    ## \brief This static method returns the default machine state for various types of unsteckered Enigmas
    #
    #  \param [machine_name] Is a string. It has to specify the name of the type of machine for which a state
    #         file is to be generated. Allowed values for machine_name are 'TirpitzEnigma', 'RailwayEnigma'
    #         'AbwehrEnigma' and 'KDEnigma'
    #    
    #  \param [rotor_set] Is an object with the same interface as EnigmaRotorSet. It has to specify the rotor
    #         set which is to be used to determine rotor permutations and ring data. If rotor_set is None then
    #         the default Enigma rotor set is used.
    #
    #  \returns An UnsteckeredEnigmaState object. 
    #
    @staticmethod
    def get_default_state(machine_name, rotor_set = None):
        result = None
        
        if rotor_set == None:
            rotor_set = RotorSet.get_std_set('enigma')
        
        if machine_name == 'TirpitzEnigma':
            enigma_t_state = UnsteckeredEnigmaState(machine_name, rotor_set, es.WALZE_T_ETW)
            enigma_t_state.insert_enigma_rotor('fast', es.WALZE_T_V, 'b', 'm')
            enigma_t_state.insert_enigma_rotor('middle', es.WALZE_T_VIII, 'r', 'f')        
            enigma_t_state.insert_enigma_rotor('slow', es.WALZE_T_VII, 'q', 'c')  
            enigma_t_state.insert_enigma_rotor('umkehrwalze', es.UKW_T, 'k', 'a')                      
            result = enigma_t_state
        elif machine_name == 'KDEnigma':
            enigma_kd_state = UnsteckeredEnigmaState(machine_name, rotor_set, es.WALZE_KD_ETW)
            rotor_set.change_ukw_d('azbpcxdqetfogshvirknlmuw')
            enigma_kd_state.insert_enigma_rotor('fast', es.WALZE_KD_V, 'b', 'm')
            enigma_kd_state.insert_enigma_rotor('middle', es.WALZE_KD_VI, 'r', 'f')        
            enigma_kd_state.insert_enigma_rotor('slow', es.WALZE_KD_II, 'q', 'c')  
            enigma_kd_state.insert_enigma_rotor('umkehrwalze', es.UKW_D, 'a', 'a')                     
            result = enigma_kd_state
        elif machine_name == 'AbwehrEnigma':
            enigma_abw_state = UnsteckeredEnigmaState(machine_name, rotor_set, es.WALZE_ABW_ETW)
            enigma_abw_state.insert_enigma_rotor('slow', es.WALZE_ABW_III, 'a', 'a')
            enigma_abw_state.insert_enigma_rotor('middle', es.WALZE_ABW_II, 'a', 'a')        
            enigma_abw_state.insert_enigma_rotor('fast', es.WALZE_ABW_I, 'a', 'a')  
            enigma_abw_state.insert_enigma_rotor('umkehrwalze', es.UKW_ABW, 'a', 'a')                         
            result = enigma_abw_state
        else:
            enigma_rb_state = UnsteckeredEnigmaState('RailwayEnigma', rotor_set, es.WALZE_RB_ETW)
            enigma_rb_state.insert_enigma_rotor('fast', es.WALZE_RB_III, 'a', 'a')
            enigma_rb_state.insert_enigma_rotor('middle', es.WALZE_RB_II, 'a', 'a')        
            enigma_rb_state.insert_enigma_rotor('slow', es.WALZE_RB_I, 'a', 'a')  
            enigma_rb_state.insert_enigma_rotor('umkehrwalze', es.UKW_RB, 'a', 'a')                             
            result = enigma_rb_state
        
        return result    


## \brief This class allows to create an INI file which represents the state of an Enigma machine with
#         a plugboard.
#
class SteckeredEnigmaState(BasicEnigmaState):
    ## \brief Constructor.
    #
    #  \param [machine_name] Is a string. It has to specify the name of the type of machine for which a state
    #         file is to be generated.
    #
    #  \param [machine_type] Is a string. It has to specify the specific Enigma variant for which a state file
    #         is to be created.
    #
    #  \param [slots] Is a sequence of strings. This squence has to specify the names of the rotor
    #         slots which are used by the machine type specified through parameter machine_type.
    #
    #  \param [rotor_set] Is an object with the same interface as EnigmaRotorSet. It has to specify the rotor
    #         set whch is to be used to determine rotor permutations and ring data.
    #
    def __init__(self, machine_name, machine_type, slots, rotor_set):
        super().__init__(machine_name, machine_type, slots, rotor_set)

    ## \brief Sets the permutation of the Enigma's plugboard.
    #
    #  \param [cabling] Is a string. It has to specify the letter pairs that make ap the involution defined by the
    #         plugboard. In case use_uhr is True there have to be exactly 10 letter pairs.
    #
    #  \param [use_uhr] Is a boolean. It is True if the Enigma Uhr is in use in this machine.
    #
    #  \param [uhr_dial_pos] Is a integer. If uses_uhr is True this parameter has to be used to specify the
    #         dial pos of the Uhr. Valid dial positions are: 0, ..., 39.
    #
    #  \returns Nothing
    #    
    def set_stecker_brett(self, cabling, use_uhr = False, uhr_dial_pos = 0):
        self._config['plugboard'] = {}
        self._config['plugboard']['cabling'] = cabling
        self._config['plugboard']['uhr'] = use_uhr
        self._config['plugboard']['dialpos'] = uhr_dial_pos

    ## \brief This method saves plugboard permutation, uhr cabling and uhr dial pos in the INI file.
    #
    #  \param [ini_file] Is an object of type GLib.KeyFile(). It specifies the INI file object into which the
    #         additional data is to be saved.
    #
    #  \returns Nothing. 
    #    
    def _save_additional_data(self, ini_file):  
        super()._save_additional_data(ini_file)      
        entry_perm = Permutation()
        entry_perm.involution_from_pairs(self._config['plugboard']['cabling'])
        
        ini_file.set_boolean('plugboard', 'usesuhr', self._config['plugboard']['uhr'])
        ini_file.set_integer_list('plugboard', 'entry', entry_perm.to_int_vector())
        
        if self._config['plugboard']['uhr']:
            ini_file.set_string('plugboard', 'uhrcabling', self._config['plugboard']['cabling'])
            ini_file.set_integer('plugboard', 'uhrdialpos', self._config['plugboard']['dialpos'])            


## \brief This class allows to create an INI file which represents the state of an M3 or Services Enigma.
#
class ServicesEnigmaState(SteckeredEnigmaState):
    ## \brief Constructor.
    #
    #  \param [machine_type] Is a string. It has to specify the specific Enigma variant for which a state file
    #         is to be created. Allowed values are: 'M3' and 'Services'
    #
    #  \param [rotor_set] Is an object with the same interface as EnigmaRotorSet. It has to specify the rotor
    #         set whch is to be used to determine rotor permutations and ring data.
    #
    def __init__(self, machine_type, rotor_set):
        slots = ['fast', 'middle', 'slow', 'umkehrwalze']
        super().__init__('Enigma', machine_type, slots, rotor_set)

    ## \brief This static method returns the default machine state for an M4 Enigma
    #
    #  \param [rotor_set] Is an object with the same interface as EnigmaRotorSet. It has to specify the rotor
    #         set which is to be used to determine rotor permutations and ring data. If rotor_set is None then
    #         the default Enigma rotor set is used.
    #
    #  \returns A ServicesEnigmaState object. The returned machine state makes use of the Uhr.
    #
    @staticmethod
    def get_default_state(rotor_set = None):
        if rotor_set == None:
            rotor_set = RotorSet.get_std_set('enigma')
        
        enigma_I_state = ServicesEnigmaState('M3', rotor_set)
        enigma_I_state.insert_enigma_rotor('fast', es.WALZE_III, 'h', 'z')
        enigma_I_state.insert_enigma_rotor('middle', es.WALZE_IV, 'z', 't')        
        enigma_I_state.insert_enigma_rotor('slow', es.WALZE_I, 'p', 'r')  
        enigma_I_state.insert_enigma_rotor('umkehrwalze', es.UKW_B, 'a', 'a')
        enigma_I_state.set_stecker_brett('adcnetflgijvkzpuqywx', True, 27)                      

        return enigma_I_state


## \brief This class allows to create an INI file which represents the state of an M4 Enigma.
#
class M4EnigmaState(SteckeredEnigmaState):
    ## \brief Constructor.
    #
    #  \param [rotor_set] Is an object with the same interface as EnigmaRotorSet. It has to specify the rotor
    #         set whch is to be used to determine rotor permutations and ring data.
    #
    def __init__(self, rotor_set):
        slots = ['fast', 'middle', 'slow', 'griechenwalze', 'umkehrwalze']
        super().__init__('M4Enigma', 'M4', slots, rotor_set)
        
    ## \brief This static method returns the default machine state for an M4 Enigma
    #
    #  \param [rotor_set] Is an object with the same interface as EnigmaRotorSet. It has to specify the rotor
    #         set which is to be used to determine rotor permutations and ring data. If rotor_set is None then
    #         the default Enigma rotor set is used.
    #
    #  \returns A M4EnigmaState object. 
    #
    @staticmethod
    def get_default_state(rotor_set = None):
        if rotor_set == None:
            rotor_set = RotorSet.get_std_set('enigma')
    
        enigma_M4_state = M4EnigmaState(rotor_set)
        enigma_M4_state.insert_enigma_rotor('fast', es.WALZE_I, 'v', 'a')
        enigma_M4_state.insert_enigma_rotor('middle', es.WALZE_IV, 'a', 'n')        
        enigma_M4_state.insert_enigma_rotor('slow', es.WALZE_II, 'a', 'j')  
        enigma_M4_state.insert_enigma_rotor('griechenwalze', es.WALZE_BETA, 'a', 'v')          
        enigma_M4_state.insert_enigma_rotor('umkehrwalze', es.UKW_B_DN, 'a', 'a')
        enigma_M4_state.set_stecker_brett('atbldfgjhmnwopqyrzvx')                      
        
        return enigma_M4_state


## \brief This class allows access to objects provided by the rotor_machine_provider and thereby makes
#         rotor machine functionality available to the python3 side of the TLV infrastructure.
#
#  We say that the rotorsim.RotorMachine instance proxies the rotor machine object on the C++ side of the
#  TLV infrastructure.
#
class RotorMachine(tlvobject.TlvProxy):
    ## \brief Constructor. Creates a new rotor machine object on the TLV server.
    #
    #  \param [machine_state] Is a byte array. It has to specifiy a machine state as generated by the
    #         GenericRotorMachineState.render_state() method or by the save_ini or save method of the C++ 
    #         rotor_machine class and its children.
    #
    #  \param [server_address] Is a string. Has to contain the address of the TLV server that is to be used
    #         by this RotorMachine instance.
    #
    def __init__(self, machine_state, server_address):
        super().__init__(server_address)
        param = tlvobject.TlvEntry().to_byte_array(machine_state)
        res = self.do_method_call('new', 'rotorproxy', param)        
        self._handle = res[0]

    ## \brief Groups and formats an input text
    #
    #  \param [text_in] A string. The text to format.
    #
    #  \param [uppercase] A boolean. If True the output string is in upperase. Otherwise it is in lowercase.
    #
    #  \param [group_size] An integer. It has to contain the number of characters in a group.
    #
    #  \param [groups_per_line] An integer. It has to contain the number of groups per line
    #
    #  \returns A string formatted according to the parameters.
    #
    @staticmethod
    def group_text(text_in, uppercase = False, group_size = 5, groups_per_line = 10):
        result = ''
        current_groups = []
        current_group = ' '
        
        for i in text_in:
            if uppercase:
                i = i.upper()
            else:
                i = i.lower()
            
            current_group = current_group + i
            
            if len(current_group) == (group_size + 1):
                current_groups.append(current_group)
                current_group = ' '
                
                if len(current_groups) == groups_per_line:
                    result = result + '\n' + (''.join(current_groups)).strip()
                    current_groups = []
                
        last_line = (''.join(current_groups) + current_group).strip()
        
        if last_line != '':        
            result = result + '\n' + last_line
        
        result = result.strip()
            
        return result

    ## \brief Loads a machine state saved in a file and accordingly changes the state of the proxied rotor machine.
    #
    #  \param [file_name] A string. It contains name of a file which is used to store a machine state and is to be read.
    #
    #  \returns Nothing. In case of an error an exception is thrown.
    #
    def load_machine_state(self, file_name):
        f = open(file_name, 'rb')
        s = f.read()
        f.close()
        
        self.set_state(s)

    ## \brief Saves the state of the proxied rotor machine in a file.
    #
    #  \param [file_name] A string. It contains name of a file which is used to store a machine state.
    #
    #  \returns Nothing. In case of an error an exception is thrown.
    #
    def save_machine_state(self, file_name):
        s = self.get_state()
        f = open(file_name, 'wb')
        f.write(s)
        f.close()

    ## \brief Encrypts data using the TLV  rotor machine object proxied by this RotorMachine instance.
    #
    #  \param [data_to_encrypt] A string. It has to specify the plaintext that is to be encrypted.
    #
    #  \returns A string. The result specifies the ciphertext generated by the rotor machine.
    #    
    def encrypt(self, data_to_encrypt):
        param = tlvobject.TlvEntry().to_string(data_to_encrypt.lower())    
        res = self.do_method_call(self._handle, 'encrypt', param)
        return res[0]

    ## \brief Decrypts data using the TLV  rotor machine object proxied by this RotorMachine instance.
    #
    #  \param [data_to_decrypt] A string. It has to specify the ciphertext that is to be decrypted.
    #
    #  \returns A string. The result specifies the plaintext generated by the rotor machine.
    #    
    def decrypt(self, data_to_decrypt):
        param = tlvobject.TlvEntry().to_string(data_to_decrypt.lower())    
        res = self.do_method_call(self._handle, 'decrypt', param)
        return res[0]

    ## \brief Returns the current state of the TLV rotor machine object which is proxied by this 
    #         rotorsim.RotorMachine instance.
    #
    #  \returns A byte array. The result specifies the machine state.
    #            
    def get_state(self):
        param = tlvobject.TlvEntry().to_null()    
        res = self.do_method_call(self._handle, 'getstate', param)
        return res[0]

    ## \brief Changes the current state of the TLV rotor machine object which is proxied by this 
    #         rotorsim.RotorMachine instance to a new state.
    #
    #  \param [new_state] A byte array. It has to specify the new machine state.
    #
    #  \returns Nothing.
    #                
    def set_state(self, new_state):
        param = tlvobject.TlvEntry().to_byte_array(new_state)    
        res = self.do_method_call(self._handle, 'setstate', param)

    ## \brief Changes the current state of the TLV rotor machine object which is proxied by this 
    #         rotorsim.RotorMachine by randomizing it.
    #
    #  \param [randomize_param] A string. It is used to parametrize the ranomization process.
    #
    #  \returns Nothing. Exception is thwrown upon error.
    #                
    def randomize_state(self, randomize_param = ''):
        param = tlvobject.TlvEntry().to_string(randomize_param)    
        res = self.do_method_call(self._handle, 'randomizestate', param)


    ## \brief Steps the TLV rotor machine object proxied by this rotorsim.RotorMachine instance a given
    #         number of times.
    #
    #  \param [num_iterations] An integer. It has to specify how often the proxied rotor machine is to be
    #         stepped
    #
    #  \returns A sequence of strings. Each of the strings represents the visualized rotor positions reached
    #           after each stepping of the machine.
    #                
    def step(self, num_iterations =  1):
        param = tlvobject.TlvEntry().to_int(num_iterations)    
        res = self.do_method_call(self._handle, 'step', param)
        
        return res

    ## \brief Returns a description (i.e. the machine type) of the proxied rotor machine object.
    #
    #  \returns A string. It contains a description of the rotor machine which is proxied by this
    #           rotorsim.RotorMachine instance.
    #                
    def get_description(self):
        param = tlvobject.TlvEntry().to_null()    
        res = self.do_method_call(self._handle, 'getdescription', param)
        return res[0]

    ## \brief Returns a visualization of the rotor positions of the machine proxied by this rotorsim.RotorMachine
    #         instance.
    #
    #  \returns A string. It specifies the rotor positions the machine currently shows in its rotor windows.
    #
    def get_rotor_positions(self):
        param = tlvobject.TlvEntry().to_null()    
        res = self.do_method_call(self._handle, 'getpositions', param)
        return res[0]

    ## \brief Performs a number of setup steppings of the SIGABA machine which is proxied by this rotorsim.RotorMachine
    #         instance.
    #
    #  If the machine proxied by this rotorsim.RotorMachine instance is not a SIGABA variant an exception is thrown.
    #
    #  \param [rotor_num] An integer. It has to specify the number (1-5) of the driver rotor which is to be stepped.
    #
    #  \param [num_iterations] An integer. It has to specify how often the driver rotor designated by parameter
    #         rotor_num is to be stepped.
    #
    #  \returns A sequence of strings. Each of the strings represents the visualized rotor positions reached
    #           after each stepping of the machine.
    #                
    def sigaba_setup(self, rotor_num, num_iterations =  1):
        param = tlvobject.TlvEntry().to_sequence([tlvobject.TlvEntry().to_int(rotor_num), tlvobject.TlvEntry().to_int(num_iterations)])    
        res = self.do_method_call(self._handle, 'sigabasetup', param)
        
        return res

    ## \brief Steps the TLV rotor machine object proxied by this rotorsim.RotorMachine instance a given
    #         number of times and returns the permutations which are generated by the underlying machine at
    #         the corresponding position.
    #
    #  \param [num_iterations] An integer. It has to specify how often the proxied rotor machine is to be
    #         stepped. If num_iterations is 0 then no stepping is performed and the current permutation is
    #         returned.
    #
    #  \returns A sequence of integer sequences. Each integer sequence specifies a permutation.
    #                
    def get_permutations(self, num_iterations =  0):
        param = tlvobject.TlvEntry().to_int(num_iterations)    
        res = self.do_method_call(self._handle, 'getpermutations', param)
        res = list(map(lambda x: list(x), res))
        
        return res

