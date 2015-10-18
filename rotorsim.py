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
from gi.repository import GLib
import enigrotorset as es
import os
import datetime
import tlvobject

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

    ## \brief Changes to permutation of the specified rotor.
    #
    #  \param [rotor_id] Is a integer holding the id of the rotor the permutation of which is to be changed.
    #
    #  \param [new_perm] Is an integer list specifying the new permutation.
    #
    #  \returns Nothing
    #    
    def change_perm(self, rotor_id, new_perm):
        self.data[rotor_id]['permutation'] = new_perm


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


class GenericRotorMachineState:
    def __init__(self, machine_name, slot_names, rotor_set):
        self._name = machine_name
        self._rotor_set = rotor_set
        self._slot_names = slot_names
        self._config = {}
        
        for slot in self._slot_names:
            self.insert_rotor(slot, rotor_set.ids[0], rotor_set.ids[0], 0, 0)

    def get_slot_names(self):
        return self._slot_names
        
    def _save_additional_rotor_data(self, slot_name, ini_file):
        pass
    
    def _save_additional_data(self, ini_file):
        pass

    def insert_rotor(self, slot_name, rotor_id, ring_id, ring_offset, rotor_pos, use_inverse = False):
        self._config[slot_name] = {'rid': rotor_id, 'ringid':ring_id, 'ringoffset':ring_offset, 'rotorpos':rotor_pos, 'inverse':use_inverse}
        
    def render_state(self):
        result = GLib.KeyFile()
        
        result.set_string('machine', 'name', self._name)
        result.set_string('machine', 'rotorsetname', 'defaultset')
        
        for i in self._slot_names:
            section_name = 'rotor_' + i
            conf = self._config[i]
            p = Permutation()
            p.from_int_vector(self._rotor_set.data[conf['rid']]['permutation'])
            
            if conf['inverse']:
                result.set_integer_list(section_name, 'permutation', p.to_inverse())
            else:
                result.set_integer_list(section_name, 'permutation', p.to_int_vector())
            
            result.set_integer_list(section_name, 'ringdata', self._rotor_set.data[conf['ringid']]['ringdata'])
            result.set_integer(section_name, 'rid', conf['rid'])
            result.set_integer(section_name, 'ringid', conf['ringid'])            
            result.set_boolean(section_name, 'insertinverse', False)
            result.set_integer(section_name, 'ringoffset', conf['ringoffset'])
            result.set_integer(section_name, 'rotordisplacement', (conf['rotorpos'] + p.neg(conf['ringoffset'])) % p.get_len()) 
            
            self._save_additional_rotor_data(i, result)
        
        self._save_additional_data(result)
        
        return result.to_data()[0].encode()
    
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


class EnigmaRotorSet(RotorSet):
    def __init__(self):
        super().__init__()
    
    def change_ukw_d(self, new_perm):
        help = Permutation('yzxwvutsrqponjmlkihgfedcba')
        help.involution_from_pairs(new_perm)
        self.change_perm(es.UKW_D, help.to_int_vector())


class BasicEnigmaState(GenericRotorMachineState):
    def __init__(self, machine_name, machine_type, slots, rotor_set):
        super().__init__(machine_name, slots, rotor_set)
        self._machine_type = machine_type

    def _save_additional_data(self, ini_file):  
        super()._save_additional_data(ini_file)      
        
        ini_file.set_integer_list('machine', 'ukwdwiring', self._rotor_set.data[es.UKW_D]['permutation'])
        ini_file.set_string('machine', 'machinetype', self._machine_type)


class UnsteckeredEnigmaState(BasicEnigmaState):
    def __init__(self, machine_name, rotor_set, etw_id):
        slots = ['eintrittswalze', 'fast', 'middle', 'slow', 'umkehrwalze']
        type_helper = {}
        type_helper['TirpitzEnigma'] = 'Tirpitz'
        type_helper['RailwayEnigma'] = 'Railway'
        type_helper['AbwehrEnigma'] = 'Abwehr'        
        type_helper['KDEnigma'] = 'KD'        
        super().__init__(machine_name, type_helper[machine_name], slots, rotor_set)
        
        self.insert_rotor('eintrittswalze', etw_id, etw_id, 0, 0, True)


class SteckeredEnigmaState(BasicEnigmaState):
    def __init__(self, machine_name, machine_type, slots, rotor_set):
        super().__init__(machine_name, machine_type, slots, rotor_set)
    
    def set_stecker_brett(self, cabling, use_uhr = False, uhr_dial_pos = 0):
        self._config['plugboard'] = {}
        self._config['plugboard']['cabling'] = cabling
        self._config['plugboard']['uhr'] = use_uhr
        self._config['plugboard']['dialpos'] = uhr_dial_pos
    
    def _save_additional_data(self, ini_file):  
        super()._save_additional_data(ini_file)      
        entry_perm = Permutation()
        entry_perm.involution_from_pairs(self._config['plugboard']['cabling'])
        
        ini_file.set_boolean('plugboard', 'usesuhr', self._config['plugboard']['uhr'])
        ini_file.set_integer_list('plugboard', 'entry', entry_perm.to_int_vector())
        
        if self._config['plugboard']['uhr']:
            ini_file.set_string('plugboard', 'uhrcabling', self._config['plugboard']['cabling'])
            ini_file.set_integer('plugboard', 'uhrdialpos', self._config['plugboard']['dialpos'])            


class ServicesEnigmaState(SteckeredEnigmaState):
    def __init__(self, machine_type, rotor_set):
        slots = ['fast', 'middle', 'slow', 'umkehrwalze']
        super().__init__('Enigma', machine_type, slots, rotor_set)


class M4EnigmaState(SteckeredEnigmaState):
    def __init__(self, rotor_set):
        slots = ['fast', 'middle', 'slow', 'griechenwalze', 'umkehrwalze']
        super().__init__('M4Enigma', 'M4', slots, rotor_set)


class RotorMachine(tlvobject.TlvProxy):
    def __init__(self, machine_state, server_address):
        tlvobject.TlvProxy.__init__(self, server_address)
        param = tlvobject.TlvEntry().to_byte_array(machine_state)
        res = self.do_method_call('new', 'rotorproxy', param)        
        self._handle = res[0]
    
    def encrypt(self, data_to_encrypt):
        param = tlvobject.TlvEntry().to_string(data_to_encrypt)    
        res = self.do_method_call(self._handle, 'encrypt', param)
        return res[0]

    def decrypt(self, data_to_decrypt):
        param = tlvobject.TlvEntry().to_string(data_to_decrypt)    
        res = self.do_method_call(self._handle, 'decrypt', param)
        return res[0]
        
    def get_state(self):
        param = tlvobject.TlvEntry().to_null()    
        res = self.do_method_call(self._handle, 'getstate', param)
        return res[0]
    
    def set_state(self, new_state):
        param = tlvobject.TlvEntry().to_byte_array(new_state)    
        res = self.do_method_call(self._handle, 'setstate', param)

    def step(self, num_iterations =  1):
        param = tlvobject.TlvEntry().to_int(num_iterations)    
        res = self.do_method_call(self._handle, 'step', param)
        
        return res
    
    def get_description(self):
        param = tlvobject.TlvEntry().to_null()    
        res = self.do_method_call(self._handle, 'getdescription', param)
        return res[0]

    def get_rotor_positions(self):
        param = tlvobject.TlvEntry().to_null()    
        res = self.do_method_call(self._handle, 'getpositions', param)
        return res[0]

    def sigaba_setup(self, rotor_num, num_iterations =  1):
        param = tlvobject.TlvEntry().to_sequence([tlvobject.TlvEntry().to_int(rotor_num), tlvobject.TlvEntry().to_int(num_iterations)])    
        res = self.do_method_call(self._handle, 'sigabasetup', param)
        
        return res

    def get_permutations(self, num_iterations =  0):
        param = tlvobject.TlvEntry().to_int(num_iterations)    
        res = self.do_method_call(self._handle, 'getpermutations', param)
        res = list(map(lambda x: list(x), res))
        
        return res


def load_machine_state(file_name):
    f = open(file_name, 'rb')
    s = f.read()
    f.close()
    
    return s
    
def test():
    s = load_machine_state("Enigma M4 Test 1.ini")
        
    help = Permutation()    
    r_set = EnigmaRotorSet()
    
    if r_set.load('enigma_rotor_set.ini'):        
        with tlvobject.TlvServer(server_address = 'sock_hsdkfjhskdjhfksjdhf') as server:
            with RotorMachine(s, server.address) as machine:
                jetzt = datetime.datetime.now()
                print(machine.decrypt('nczwvusx'))

                enigma_t_state = UnsteckeredEnigmaState('TirpitzEnigma', r_set, es.WALZE_T_ETW)
                enigma_t_state.insert_rotor('fast', es.WALZE_T_V, es.WALZE_T_V, help.from_val('b'), help.from_val('m'))
                enigma_t_state.insert_rotor('middle', es.WALZE_T_VIII, es.WALZE_T_VIII, help.from_val('r'), help.from_val('f'))        
                enigma_t_state.insert_rotor('slow', es.WALZE_T_VII, es.WALZE_T_VII, help.from_val('q'), help.from_val('c'))  
                enigma_t_state.insert_rotor('umkehrwalze', es.UKW_T, es.UKW_T, help.from_val('k'), help.from_val('a'))                      
                        
                machine.set_state(enigma_t_state.render_state())
                print(machine.decrypt('rhmbwnbzgmmnkperufvnyjfkyqg'))                
                
                enigma_I_state = ServicesEnigmaState('M3', r_set)
                enigma_I_state.insert_rotor('fast', es.WALZE_III, es.WALZE_III, help.from_val('h'), help.from_val('z'))
                enigma_I_state.insert_rotor('middle', es.WALZE_IV, es.WALZE_IV, help.from_val('z'), help.from_val('t'))        
                enigma_I_state.insert_rotor('slow', es.WALZE_I, es.WALZE_I, help.from_val('p'), help.from_val('r'))  
                enigma_I_state.insert_rotor('umkehrwalze', es.UKW_B, es.UKW_B, 0, 0)
                enigma_I_state.set_stecker_brett('adcnetflgijvkzpuqywx', True, 27)                      
                        
                machine.set_state(enigma_I_state.render_state())
                print(machine.decrypt('ukpfhallqcdnbffcghudlqukrbpyiyrdlwyalykcvossffxsyjbhbghdxawukjadkelptyklgfxqahxmmfpioqnjsgaufoxzggomjfryhqpccdivyicgvyx'))

                r_set.change_ukw_d('azbpcxdqetfogshvirjyknlmuw')
                enigma_kd_state = UnsteckeredEnigmaState('KDEnigma', r_set, es.WALZE_KD_ETW)
                enigma_kd_state.insert_rotor('fast', es.WALZE_KD_V, es.WALZE_KD_V, help.from_val('b'), help.from_val('m'))
                enigma_kd_state.insert_rotor('middle', es.WALZE_KD_VI, es.WALZE_KD_VI, help.from_val('r'), help.from_val('f'))        
                enigma_kd_state.insert_rotor('slow', es.WALZE_KD_II, es.WALZE_KD_II, help.from_val('q'), help.from_val('c'))  
                enigma_kd_state.insert_rotor('umkehrwalze', es.UKW_D, es.UKW_D, 0, 0)                      
                        
                machine.set_state(enigma_kd_state.render_state())
                print(machine.decrypt('xlmwoizeczzbfvmahnhrzerhnpwkjjorrxtebozcxncvdemaexvcfuxokbyntyjdongpgwwchftplrzr'))    
                
                enigma_M4_state = M4EnigmaState(r_set)
                enigma_M4_state.insert_rotor('fast', es.WALZE_I, es.WALZE_I, help.from_val('v'), help.from_val('a'))
                enigma_M4_state.insert_rotor('middle', es.WALZE_IV, es.WALZE_IV, help.from_val('a'), help.from_val('n'))        
                enigma_M4_state.insert_rotor('slow', es.WALZE_II, es.WALZE_II, help.from_val('a'), help.from_val('j'))  
                enigma_M4_state.insert_rotor('griechenwalze', es.WALZE_BETA, es.WALZE_BETA, help.from_val('a'), help.from_val('v'))          
                enigma_M4_state.insert_rotor('umkehrwalze', es.UKW_B_DN, es.UKW_B_DN, 0, 0)
                enigma_M4_state.set_stecker_brett('atbldfgjhmnwopqyrzvx')                      
                        
                machine.set_state(enigma_M4_state.render_state())
                print(machine.decrypt('nczwvusxpnyminhzxmqxsfwxwlkjahshnmcoccakuqpmkcsmhkseinjusblkiosxckubhmllxcsjusrrdvkohulxwccbgvliyxeoahxrhkkfvdrewez'))
                
                if not enigma_M4_state.save('egal.ini'):
                    print('Error saving M4 state')

                enigma_abw_state = UnsteckeredEnigmaState('AbwehrEnigma', r_set, es.WALZE_ABW_ETW)
                enigma_abw_state.insert_rotor('slow', es.WALZE_ABW_III, es.WALZE_ABW_III, 0, 0)
                enigma_abw_state.insert_rotor('middle', es.WALZE_ABW_II, es.WALZE_ABW_II, 0, 0)        
                enigma_abw_state.insert_rotor('fast', es.WALZE_ABW_I, es.WALZE_ABW_I, 0, 0)  
                enigma_abw_state.insert_rotor('umkehrwalze', es.UKW_ABW, es.UKW_ABW, 0, 0)                      

                machine.set_state(enigma_abw_state.render_state())
                print(machine.decrypt('gjuiycmdguvttffqpzmxkvctzusobzldzumhqmjxwtzwmqnnuwidyeqpgvfzetolb'))

                enigma_rb_state = UnsteckeredEnigmaState('RailwayEnigma', r_set, es.WALZE_RB_ETW)
                enigma_rb_state.insert_rotor('fast', es.WALZE_RB_III, es.WALZE_RB_III, 0, 0)
                enigma_rb_state.insert_rotor('middle', es.WALZE_RB_II, es.WALZE_RB_II, 0, 0)        
                enigma_rb_state.insert_rotor('slow', es.WALZE_RB_I, es.WALZE_RB_I, 0, 0)  
                enigma_rb_state.insert_rotor('umkehrwalze', es.UKW_RB, es.UKW_RB, 0, 0)                      

                machine.set_state(enigma_rb_state.render_state())
                print(machine.decrypt('zbijbjetellsdidqbyocxeohngdsxnwlifuuvdqlzsyrbtbwlwlxpgujbhurbikgtkdztgtexjxhulfkiuqnjbeqgccryitomeyirckuji'))  
                spaeter = datetime.datetime.now()
                print(spaeter - jetzt)                              
    else:
        print('Unable to load rotor set data')

m4_state = load_machine_state('Enigma M4 Test 1.ini')
csp2900_state = load_machine_state('CSP 2900 Test.ini')
sg39_state = load_machine_state('SG39 Test.ini')

def tester():
    with tlvobject.TlvServer(server_address='sock_fjsdhfjshdkfjh') as server, RotorMachine(m4_state, server.address) as m4_obj:
        original_state = m4_obj.get_state()
        print(original_state.decode())
        print(m4_obj.decrypt('nczwvusx'))
        print(m4_obj.decrypt('nczwvusx'))
        m4_obj.set_state(original_state)
        print(m4_obj.decrypt('nczwvusx'))
        m4_obj.set_state(original_state)        
        print(m4_obj.step(5))
        print(m4_obj.get_description())
        m4_obj.set_state(csp2900_state)
        print(m4_obj.get_description())
        print(m4_obj.sigaba_setup(1, 3))
        m4_obj.set_state(sg39_state)
        print(m4_obj.get_rotor_positions())
        print(m4_obj.get_permutations(10))
        
def time_trial(num_iterations, test_data):    
    with tlvobject.TlvServer(server_address='sock_fjsdhfjshdkfjh') as server, RotorMachine(m4_state, server.address) as m4_obj:    
        print(m4_obj.decrypt('nczwvusx'))            
        jetzt = datetime.datetime.now()
        
        for i in range(num_iterations):
            m4_obj.decrypt(test_data)
        
        spaeter = datetime.datetime.now()
        print(spaeter - jetzt)        
    

