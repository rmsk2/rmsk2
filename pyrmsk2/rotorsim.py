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

## @package rotorsim A Python3 interface to the C++ rotor machines implemented by rmsk2.
#           
# \file pyrmsk2/rotorsim.py
# \brief This file provides some Python3 classes that allow to en/decipher texts using the
#        C++ program tlv_object. On top of that these classes provide the functionality to
#        create rotor machine state files which then can be used with the rotorsim program.

from gi.repository import GLib
import datetime
import pyrmsk2.tlvobject as tlvobject

RESULT_OK = 0
RESULT_ERROR = 42
RESULT_ROTORSIM_FORMAT = 43


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

    ## \brief Returns the permutation in use in this instance in a string representation
    #
    #  \returns A string.
    #                                    
    def to_string(self):
        result = ''.join(list(map(lambda x: self.__alphabet[self.__val[x]], range(len(self.__alphabet)))))                
        
        return result

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


## \brief This class is intended to bind a rotor position, machine configuration and machine name together and therefore
#         can be used to store all relevant elements of a rotor machine state.
#
class StateSpec:
    ## \brief Constructor.
    #
    #  \param [rotor_pos] Is a string. It has to specify the rotor positions that are stored in this StateSpec
    #         instance.
    #
    #  \param [config] Is a string to string dictionary. It has to contain the the machine configuration that is
    #         stored in this StateSpec instance.      
    #
    #  \param [machine_name] Is a string which has to contain the name of the machine to which the config applies.      
    #
    def __init__(self, rotor_pos, config, machine_name = ''):
        self._rotor_pos = rotor_pos
        self._config = config
        self._machine_name = machine_name

    ## \brief Returns the rotor position stored in this StateSpec instance.
    #
    #  \returns A string, the rotor position.
    #                                            
    @property
    def rotor_pos(self):
        return self._rotor_pos
    
    ## \brief Returns machine configuration stored in this StateSpec instance.
    #
    #  \returns A string to string dictionary that contains the machine configuration.
    #                                            
    @property
    def config(self):
        return self._config

    ## \brief Returns the machine name stored in this StateSpec instance.
    #
    #  \returns A string.
    #                                            
    @property
    def name(self):
        return self._machine_name


## \brief This class allows to create a StateSpec object that specifies a default state for a SIGABA.
#            
class SigabaMachineState:        
    ## \brief This static method returns the default machine configuration for a SIGABA.
    #
    #  \returns A StateSpec object. This object represents the SIGABA default state.
    #    
    @staticmethod
    def get_default_config():
        config = {}
        config['cipher'] = '0N1N2R3N4N'
        config['control'] = '5N6N7R8N9N'
        config['index'] = '0N1N2R3N4N'
        config['csp2900'] = 'false'
        
        return StateSpec('00000oomoooomoo', config, 'SIGABA')        


## \brief This class allows to create a StateSpec object that specifies a default state for a Nema.
#             
class NemaState:        
    ## \brief This static method returns the default machine configuration for a Nema.
    #
    #  \returns A StateSpec object. This object represents the Nema default state.
    #    
    @staticmethod
    def get_default_config():
        config = {}
        config['rotors'] = 'abcd'
        config['ringselect'] = '12 13 14 15'
        config['warmachine'] = 'true'
        
        return StateSpec('mqqpzqsoev', config, 'Nema')                


## \brief This class allows to create a StateSpec object that specifies a default state for a KL7.
#                      
class KL7State:
    ## \brief This static method returns the default machine configuration for a KL7.
    #
    #  \returns A StateSpec object. This object represents the KL7 default state.
    #    
    @staticmethod
    def get_default_config():
        # {'notchrings': 'kaaaaaa', 'notchselect': '1 2 3 4 5 6 7', 'alpharings': '27 1 1 17 1 1 1 1', 'rotors': 'abcldefg'}
        config = {}
        config['rotors'] = 'abcldefg'
        config['alpharings'] = '27 1 1 17 1 1 1 1'
        config['notchselect'] = '1 2 3 4 5 6 7'
        config['notchrings'] =  'kaaaaaa'
        
        return StateSpec('faamaaaa', config, 'KL7')        
        

## \brief This class allows to create a StateSpec object that specifies a default state for an SG39.
#            
class SG39State:
    ## \brief This static method returns the default machine configuration for a SG39.
    #
    #  \returns A StateSpec object. This object represents the SG39 default state.
    #    
    @staticmethod
    def get_default_config():
        config = {}
        config['rotorset'] = 'defaultset'
        config['rotors'] = '3415'
        config['rings'] = 'aaaa'
        config['reflector'] = 'awbicvdketfmgnhzjulopqrysx'
        config['plugs'] = 'ldtrmihoncpwjkbyevsaxgfzuq'
        config['pinswheel1'] = ''
        config['pinswheel2'] = 'abcdefghijklmnopqrstuvw'
        config['pinswheel3'] = 'cfilorux'
        config['pinsrotor1'] = ''
        config['pinsrotor2'] = ''        
        config['pinsrotor3'] = 'aeimquy'                
        
        return StateSpec('frqdaph', config, 'SG39')        


## \brief This class allows to create a StateSpec object that specifies a default state for a Typex.
#             
class TypexState:
    ## \brief This static method returns the default machine configuration for a Typex.
    #
    #  \returns A StateSpec object. This object represents the Typex default state.
    #    
    @staticmethod
    def get_default_config():
        config = {}
        config['rotorset'] = 'defaultset'
        config['plugs'] = ''
        config['rings'] = 'aaaaa'
        config['rotors'] = 'aNbNcRdNeN'
        config['reflector'] = 'arbycudheqfsglixjpknmotwvz'
        
        return StateSpec('aaaaa', config, 'Typex')        
    

## \brief This class allows to create a StateSpec object that specifies a default state for an unsteckered Enigma.
#            
class UnsteckeredEnigmaState:        
    ## \brief This static method returns the default machine configuration for various types of unsteckered Enigmas
    #
    #  \param [machine_name] Is a string. It has to specify the name of the type of machine for which a state
    #         file is to be generated. Allowed values for machine_name are 'TirpitzEnigma', 'RailwayEnigma'
    #         'AbwehrEnigma' and 'KDEnigma'
    #    
    #  \returns A StateSpec object. 
    #
    @staticmethod
    def get_default_config(machine_name):
        rotor_pos = ''
        name = ''
        config = {}
        
        if machine_name == 'TirpitzEnigma':
            rotor_pos = 'acfm'
            config['rotors'] = '785'
            config['rings'] = 'kqrb'  
            name = 'Tirpitz'                      
        elif machine_name == 'KDEnigma':
            rotor_pos = 'cfm'
            config['rotors'] = '265'
            config['rings'] = 'qrb'                        
            config['ukwdperm'] = 'avboctdmezfngxhqiskrlupw'
            name = 'KD'
        elif machine_name == 'AbwehrEnigma':
            rotor_pos = 'aaaa'
            config['rotors'] = '321'
            config['rings'] = 'aaaa'            
            name = 'Abwehr'
        else:
            rotor_pos = 'aaaa'
            config['rotors'] = '123'
            config['rings'] = 'aaaa'
            name = 'Railway'            
            
        return StateSpec(rotor_pos, config, name)


## \brief This class allows to create a StateSpec object that specifies a default state for an M3 or Services Enigma.
#            
class ServicesEnigmaState:        
    ## \brief This static method returns the default machine configuration for a Services or M3 Enigma.
    #
    #  \param [machine_name] Is a string. If the value of machine_name is 'M3' an M3 state is created. Else
    #         a Services Enigma state.
    #
    #  \returns A StateSpec object. This object represents the requested default state.
    #    
    @staticmethod
    def get_default_config(machine_name):
        if machine_name != 'M3':
            machine_name = 'Services'
    
        config = {}
        config['plugs'] = 'adcnetflgijvkzpuqywx'
        config['rings'] = 'pzh'
        config['rotors'] = '1143'
        config['ukwdperm'] = 'azbpcxdqetfogshvirknlmuw'
    
        # The usesuhr key has to be part of the config for a Services Enigma
        if machine_name != 'M3':
            config['usesuhr'] = 'false'
              
        return StateSpec('rtz', config, machine_name)        


## \brief This class allows to create a StateSpec object that specifies a default state for an M4 Enigma.
#            
class M4EnigmaState:
    ## \brief This static method returns the default machine configuration for an M4 Enigma.
    #
    #  \returns A StateSpec object. This object represents the M4 default state.
    #    
    @staticmethod
    def get_default_config():
        config = {}
        config['plugs'] = 'atbldfgjhmnwopqyrzvx'
        config['rings'] = 'aaav'
        config['rotors'] = '11241'
        
        return StateSpec('vjna', config, 'M4')


## \brief This class allows to convert between the different notations for UKW D plugs.
#
class UKWDHelper:
    ## \brief Constructor.
    #
    def __init__(self, server_address):
        self._server_address = server_address
    
    ## \brief Transforms a UKW D wiring specification in Bletchley Park notation into the notation used by the German Air force in WWII.
    #
    #  \param [plug_spec] A string. It contains the specification of a UKW D wiring in Bletchley Park notation (contact bo fixed) in lower
    #         case letters.
    #
    #  \returns A string. This string contains the UKW D wiring specification in German Air Force notation.
    #
    def bp_to_gaf_wiring(self, plug_spec):
        param = tlvobject.TlvEntry().to_string(plug_spec)
        res = tlvobject.TlvServer.method_call(self._server_address, "rmsk2", "bptogafwiring", param)
        return res[0]

    ## \brief Transforms a UKW D wiring specification in German Air Force notation into the notation used by Bletchley Park in WWII.
    #
    #  \param [plug_spec] A string. It contains the specification of a UKW D wiring in German Air Force notation (contact jy fixed) in lower
    #         case letters.
    #
    #  \returns A string. This string contains the UKW D wiring specification in Bletchley Park notation.
    #
    def gaf_to_bp_wiring(self, plug_spec):
        param = tlvobject.TlvEntry().to_string(plug_spec)
        res = tlvobject.TlvServer.method_call(self._server_address, "rmsk2", "gaftobpwiring", param)
        return res[0]


## \brief This class allows to create machine states which can then be used to construct RotorMachine objects.
#
class StateHelper:
    ## \brief Constructor.
    #
    def __init__(self, server_address):
        self._server_address = server_address
    
    ## \brief Returns a default state for the machine named in the parameter machine_name.
    #
    #  \param [machine_name] A string. It contains the name of the machine for which a default state is to be returned. Allowed
    #         values are: Services, M3, M4, Railway, Tirpitz, Abwehr, KD, Typex, KL7, Nema, SG39, SIGABA.
    #
    #  \returns A byte Array. This byte array contains the requested default state.
    #
    def get_default_state(self, machine_name):
        param = tlvobject.TlvEntry().to_string(machine_name)
        res = tlvobject.TlvServer.method_call(self._server_address, "rmsk2", "getdefaultstate", param)
        return res[0]

    ## \brief Returns a state as defined in parameters machine_name, machine_state and rotor_pos
    #
    #  \param [machine_name] A string. It contains the name of the machine for which a default state is to be returned. Allowed
    #         values are: Services, M3, M4, Railway, Tirpitz, Abwehr, KD, Typex, KL7, Nema, SG39, SIGABA.
    #
    #  \param [machine_config] A string to string dictionary. It has to specify the desired machine configuration
    #
    #  \param [rotor_pos] A string. Specifies the position of the rotors that is to be set on the returned state.
    #    
    #  \returns A byte Array. This byte array contains the requested state.
    #
    def make_state(self, machine_name, machine_config, rotor_pos = ''):
        params = [tlvobject.TlvEntry().to_string(machine_name), tlvobject.TlvDict.dict_to_tlv(machine_config), tlvobject.TlvEntry().to_string(rotor_pos)]
        param = tlvobject.TlvEntry().to_sequence(params)
        res = tlvobject.TlvServer.method_call(self._server_address, "rmsk2", "makestate", param)
        return res[0]    

    ## \brief Returns a state as defined by the parameter state_spec
    #
    #  \param [state_spec] A StateSpec object that specifies the desired machine state.
    #    
    #  \returns A byte Array. This byte array contains the requested state.
    #
    def make_from_state_spec(self, state_spec):
        return self.make_state(state_spec.name, state_spec.config, state_spec.rotor_pos)


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
    #         methods of the StateHelper class, retrieved by the get_state() or load_machine_state() methods
    #         of this class or by the save_ini or save method of the C++ rotor_machine   class and its children.
    #
    #  \param [server_address] Is a string. Has to contain the address of the TLV server that is to be used
    #         by this RotorMachine instance.
    #
    def __init__(self, machine_state, server_address):
        super().__init__(server_address)
        param = tlvobject.TlvEntry().to_byte_array(machine_state)
        res = self.do_method_call('new', 'rotorproxy', param)        
        self._handle = res[0]
    
    ## \brief Value to use for config flags that are to be set to true    
    CONF_TRUE = 'true'
    ## \brief Value to use for config flags that are to be set to false        
    CONF_FALSE = 'false'

    ## \brief Creates a new rotor machine object on the TLV server.
    #
    #  \param [machine_spec] Is a StateSpec object.
    #
    #  \param [server_address] Is a string. Has to contain the address of the TLV server that is to be used
    #         by the new RotorMachine instance.
    #    
    @classmethod
    def from_machine_spec(cls, machine_spec, server_address):
        h = StateHelper(server_address)
        state = h.make_from_state_spec(machine_spec)
        return cls(state, server_address)

    ## \brief Creates a new rotor machine object on the TLV server.
    #
    #  \param [machine_name] Is a string. Allowed values are: Services, M3, M4, Railway, Tirpitz, Abwehr, KD, Typex, KL7, 
    #         Nema, SG39, SIGABA.
    #
    #  \param [server_address] Is a string. Has to contain the address of the TLV server that is to be used
    #         by the new RotorMachine instance.
    #    
    @classmethod
    def from_machine_name(cls, machine_name, server_address):
        h = StateHelper(server_address)
        state = h.get_default_state(machine_name)
        return cls(state, server_address)

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

    ## \brief Returns the current state of the TLV rotor machine object which is proxied by this 
    #         rotorsim.RotorMachine instance in form of a dictionary of key value pairs.
    #
    #  \returns A dictionary of key value pairs where keys and values are strings. The result specifies the machine state.
    #            
    def get_config(self):
        param = tlvobject.TlvEntry().to_null()    
        res = self.do_method_call(self._handle, 'getconfig', param)
        return tlvobject.TlvDict.parsed_tlv_to_dict(res[0])

    ## \brief Changes the current state of the TLV rotor machine object which is proxied by this 
    #         rotorsim.RotorMachine instance to a new state as specified by the key value pairs in
    #         the parameter new_config.
    #
    #  \param [new_config] A dictionary with strings as keys and values. It has to specify the new machine 
    #         configuration.
    #
    #  \returns Nothing. Throws exception when ot fails.
    #                
    def set_config(self, new_config):
        param = tlvobject.TlvDict.dict_to_tlv(new_config)    
        res = self.do_method_call(self._handle, 'setconfig', param)

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
    #         stepped. If num_iterations is negative then a single stepping is performed.
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

    ## \brief Returns a list of all randomizer parameters supported by the underlying machine.
    #
    #  \returns A vector of strings containing the supported randomizer parameters. Can be empty if the underlying
    #           machine does not support any randomizer parameters.
    #                
    def get_randomizer_params(self):
        param = tlvobject.TlvEntry().to_null()    
        res = self.do_method_call(self._handle, 'getrandparm', param)
        
        if (len(res) == 1) and (res[0] == ''):
            res = []

        return res

    ## \brief Returns a visualization of the rotor positions of the machine proxied by this rotorsim.RotorMachine
    #         instance.
    #
    #  \returns A string. It specifies the rotor positions the machine currently shows in its rotor windows.
    #
    def get_rotor_positions(self):
        param = tlvobject.TlvEntry().to_null()    
        res = self.do_method_call(self._handle, 'getpositions', param)
        return res[0]

    ## \brief Set the displacement of the rotors in such a way that the positions specified in parameter new_positions
    #         appears in the rotor windows.
    #
    #  \param [new_positions] A string. Has to specify the desired rotor positions in the correct format for the underlying
    #         rotor machine.               
    #
    #  \returns Nothing. Throws exception when it fails.
    #
    def set_rotor_positions(self, new_positions):
        param = tlvobject.TlvEntry().to_string(new_positions)    
        res = self.do_method_call(self._handle, 'setpositions', param)

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

