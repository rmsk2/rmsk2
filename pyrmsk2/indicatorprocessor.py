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

## @package indicatorprocessor An indicator processort knows how to generate the message groups that are used to communicate the message key to the recevier.
#         
#           
# \file indicatorprocessor.py
# \brief Implements the indicator processors known to rmsk2. 


import pyrmsk2.rotorrandom as rotorrandom
from pyrmsk2 import EnigmaException as EnigmaException

## \brief Dictionary key that names the rotor starting position during an en- or decryption
MESSAGE_KEY = 'message_key'
## \brief Dictionary key that names the rotor starting position when preparing a Post 1940 Enigma message
HEADER_GRP_1 = 'start_pos'
## \brief Dictionary key that names the encrypted message key when preparing a Post 1940 Enigma message
HEADER_GRP_2 = 'encrypted_message_key'
## \brief Dictionary key that names the indicator used for deriving the message key when using the default SIGABA message procedure 
INTERNAL_INDICATOR = 'internal_indicator'
## \brief Dictionary key that names the indicator used for identification of the crypto net when using the default SIGABA message procedure 
EXTERNAL_INDICATOR = 'external_indicator'

## \brief This class serves as a base class for a "thing" that knows how create indicator groups during encryption
#         and is able to reconstruct the message key from the indicator groups parsed from the ciphertext during
#         decryption. This class and its children are supposed to not know how the indicator groups are represented
#         in ciphertext. They deal with the groups as simple strings.
#
class IndicatorProcessor:
    ## \brief Constructor.
    #
    #  \param [server] An object that has the same interface as pyrmsk2.tlvobject.TlvServer.
    #
    #  \param [rand_gen] An object that has the same interface as pyrmsk2.rotorrandom.RotorRandom.
    #
    def __init__(self, server, rand_gen):
        ## \brief Holds the tlv server object which is to be used.
        self._server = server
        ## \brief Holds the rotor random object which is to be used.
        self._rand_gen = rand_gen        
    
    ## \brief Children have to oeverride this method. It is intended to create the indicator groups necessary
    #         to create a full message part during encryption.
    #
    #  \param [machine] A rotorsim.RotorMachine object. It is used to create encrypted indicator groups.
    #
    #  \param [this_part] An integer. It specifies the sequence number of the message part for which this method
    #         is called.
    #
    #  \param [num_parts] An integer. It has to specify the overall number of message parts of in the current encryption
    #         operation.
    #
    #  \returns A dictionary that maps strings to strings. It has to contain a key MESSAGE_KEY that specifies the
    #           starting positions of the machine's rotors at the beginning of the encryption of the body of this message
    #           part.
    #
    def create_indicators(self, machine, this_part, num_parts):
        result = {MESSAGE_KEY:''}
        
        return
            
    ## \brief Children have to override this method. It is intended to recreate the message key from the indicator groups
    #         as parsed from the ciphertext of a message part during decryption.
    #
    #  \param [machine] A rotorsim.RotorMachine object. It is used to create the encrypted indicator groups.
    #
    #  \param [already_parsed_indicators] A dictionary that maps strings to strings. When calling this method
    #         this dictionary has to contain the indicator groups as parsed from the current message part during decryption.
    #
    #  \returns A dictionary that maps strings to strings. It contains a key MESSAGE_KEY that specifies the starting positions 
    #           of the machines rotors at the beginning of the decryption of the body of this message part.
    #    
    def derive_message_key(self, machine, already_parsed_indicators):
        result = already_parsed_indicators
        result[MESSAGE_KEY] = ''
        
        return result

    ## \brief Resets the state of this object. Children that keep a state have to override this method. It is called at the
    #         beginning of encryption and decryption of messages.
    #
    #  \returns Nothing
    #
    def reset(self):
        pass    


## \brief This class serves as a base class for a "thing" that knows how create indicator groups during encryption
#         and is able to reconstruct the message key from the indicator groups parsed from the ciphertext during
#         decryption with an Enigma that uses the kenngruppen indicator system. It only adds methods that allow
#         to generate a new kenngruppe.
#
class EnigmaKenngruppenIndicatorProc(IndicatorProcessor):
    ## \brief Constructor.
    #
    #  \param [server] An object that has the same interface as pyrmsk2.tlvobject.TlvServer.
    #
    #  \param [rand_gen] An object that has the same interface as pyrmsk2.rotorrandom.RotorRandom.
    #
    #  \param [kenngruppen] A sequence of 3 letter strings. Specifies the kenngruppen to use.
    #
    #  \param [num_rotors] An integer. It has to specify the number of settable rotors in the Engima machine.
    #
    #  \returns Nothing.
    #
    def __init__(self,  server, rand_gen, kenngruppen, num_rotors):
        super().__init__(server, rand_gen)
        ## \brief Holds the kenngruppen to use.
        self._kenngruppen = kenngruppen
        ## \brief Holds a random permutation of 0 ... len(self._kenngruppen).        
        self._group_shuffle = []
        ## \brief Points to the current read position in self._group_shuffle.
        self._shuffle_pos = 0
        ## \brief Specifies the number of settable rotors in the machine.
        self._num_rotors = num_rotors
        ## \brief Verifies before encryption that an indicator candidate is valid.
        self._verifier = (lambda x: len(x) == self._num_rotors)                

    ## \brief This method changes the kenngruppen that are in used in this object.
    #
    #  \param [new_kenngruppen] A sequence of three letter strings. It has to contain the new kenngruppen.
    #
    #  \returns Nothing.
    #
    def set_kenngruppen(self, new_kenngruppen):
        self._kenngruppen = new_kenngruppen
        self.reset()

    ## \brief This method returns the kenngruppe which is to be used next.
    #
    #  \returns A string.
    #  
    def _get_next_kenngruppe(self):                
        if self._shuffle_pos >= len(self._group_shuffle):
            mix_alpha = 'abcdefghijklmnopqrstuvwxyz'[:len(self._kenngruppen)]
            
            # Determine a random permutation of 0 ... len(self._kenngruppen) - 1
            with rotorrandom.RotorRandom(mix_alpha, self._server.address) as mixer:
                self._group_shuffle = mixer.get_rand_permutation()
            
            self._shuffle_pos = 0                            
        
        current_index = self._group_shuffle[self._shuffle_pos]
        self._shuffle_pos += 1
        
        return self._kenngruppen[current_index]

    ## \brief This method resets the mechanism that is used to determine the next kenngruppe to use.
    #
    #  \returns Nothing.
    #
    def reset(self):
        self._group_shuffle = []
        self._shuffle_pos = 0 
        
    ## \brief This property returns the verifier that is used to check message key candidates before encrypting them
    #         using the grundstellung.
    #
    #  \returns A callable object that takes a string and returns a bool. It is used to verify a message key candidate.
    #
    @property
    def verifier(self):
        return self._verifier

    ## \brief This property setter allows to change the verifier.
    #
    #  \param [new_verifier] A callable object that takes a string and returns a bool.
    #
    #  \returns Nothing
    #
    @verifier.setter
    def verifier(self, new_verifier):
        self._verifier = new_verifier        


## \brief This class implements the indicator system that was used by the german army and air force from 1940
#         onward. The machine operator chooses a random starting position and a random message key. The indicator
#         groups consist of the random starting position and the message key encrypted using the starting position.
#
class Post1940EnigmaIndicatorProc(EnigmaKenngruppenIndicatorProc):
    ## \brief Constructor.
    #
    #  \param [server] An object that has the same interface as pyrmsk2.tlvobject.TlvServer.
    #
    #  \param [rand_gen] An object that has the same interface as pyrmsk2.rotorrandom.RotorRandom.
    #
    #  \param [kenngruppen] A sequence of strings. Specifies the kennruppen to use.
    #
    #  \param [num_rotors] An integer. It has to specify the number of settable rotors in the Engima machine.
    #
    #  \returns Nothing.
    #
    def __init__(self,  server, rand_gen, kenngruppen, num_rotors = 3):
        super().__init__(server, rand_gen, kenngruppen, num_rotors)

    ## \brief This method creates the indicator groups for the post 1940 messaging procedure.
    #
    #  \param [machine] A rotorsim.RotorMachine object. It is used to create the encrypted indicator groups.
    #
    #  \param [this_part] An integer. It specifies the sequence number of the message part for which this method
    #         is called.
    #
    #  \param [num_parts] An integer. It has to specify the overall number of message parts of in the current encryption
    #         operation.
    #
    #  \returns A dictionary that maps strings to strings. It contains the keys:
    #           MESSAGE_KEY: The starting position for the rotors when the body of the message part is encrypted.
    #           HEADER_GRP_1: The first group that is specified in the header of a message part (Start position to encrypt message key). 
    #           HEADER_GRP_2: The second group that is specified in the header of a message part (Encrypted message key).
    #           'kenngruppe': The kenngruppe, padded up to 5 characters, which is to be used for this message part.
    #
    def create_indicators(self, machine, this_part, num_parts):
        result = {}
        
        result[MESSAGE_KEY] = self._rand_gen.get_rand_string(self._num_rotors)
        
        while not self._verifier(result[MESSAGE_KEY]):
            result[MESSAGE_KEY] = self._rand_gen.get_rand_string(self._num_rotors)
        
        result[HEADER_GRP_1] = self._rand_gen.get_rand_string(self._num_rotors)
        machine.set_rotor_positions(result[HEADER_GRP_1])
        result[HEADER_GRP_2] = machine.encrypt(result[MESSAGE_KEY])
        result['kenngruppe'] = self._rand_gen.get_rand_string(2) + self._get_next_kenngruppe()
        
        return result    

    ## \brief This method recreates the message key from the indicator groups.
    #
    #  \param [machine] A rotorsim.RotorMachine object. It is used to decrypt the message key.
    #
    #  \param [already_parsed_indicators] A dictionary that maps strings to strings. When calling this method
    #         this dictionary has to contain the indicator groups as parsed from the current message part during decryption. I.e.
    #         it has to contain at least the the keys HEADER_GRP_1, HEADER_GRP_2.
    #
    #  \returns A dictionary that maps strings to strings. This method adds the key MESSAGE_KEY to already_parsed_indicators.
    #           It specifies the the starting position for the rotors when the body of a message part is decrypted.
    #        
    def derive_message_key(self, machine, already_parsed_indicators):
        result = already_parsed_indicators
        machine.set_rotor_positions(result[HEADER_GRP_1])
        result[MESSAGE_KEY] = machine.decrypt(result[HEADER_GRP_2])
        
        if not self._verifier(result[MESSAGE_KEY]):
            raise EnigmaException('Invalid indicator')
        
        return result


## \brief This class implements the indicator system that was used by the german army and air force before 1940.
#         Instead of the later system it utilizes a fixed rotor position, the grundstellung, which is used
#         to encrypt the message key twice. Early Polish and British decryption of Enigma traffic depended on
#         this known relation between the two indicator groups in the message header.
#
class Pre1940EnigmaIndicatorProc(EnigmaKenngruppenIndicatorProc):
    ## \brief Constructor.
    #
    #  \param [server] An object that has the same interface as pyrmsk2.tlvobject.TlvServer.
    #
    #  \param [rand_gen] An object that has the same interface as pyrmsk2.rotorrandom.RotorRandom.
    #
    #  \param [kenngruppen] A sequence of three letter strings. Specifies the kennruppen to use.
    #
    #  \param [grundstellung] A string. Specifies the grundstellung valid for the day.
    #
    #  \param [num_rotors] An integer. It has to specify the number of settable rotors in the Engima machine.
    #
    #  \returns Nothing.
    #    
    def __init__(self,  server, rand_gen, kenngruppen, grundstellung, num_rotors = 3):
        super().__init__(server, rand_gen, kenngruppen, num_rotors)
        ## \brief Holds the basic setting of the rotors which is used to encrypt the message key.
        self._grundstellung = grundstellung
    
    ## \brief This property returns the grundstellung.
    #
    #  \returns A string. The grundstellung
    #
    @property
    def grundstellung(self):
        return self._grundstellung

    ## \brief This property setter allows to change the grundstellung.
    #
    #  \param [new_grundstellung] A string of length self._num_rotors. The new grundstellung.
    #
    #  \returns Nothing
    #
    @grundstellung.setter
    def grundstellung(self, new_grundstellung):
        self._grundstellung = new_grundstellung

    ## \brief This method creates the indicator groups for the messaging procedure used before 1940.
    #
    #  \param [machine] A rotorsim.RotorMachine object. It is used to create encrypted indicator groups.
    #
    #  \param [this_part] An integer. It specifies the sequence number of the message part for which this method
    #         is called.
    #
    #  \param [num_parts] An integer. It has to specify the overall number of message parts of in the current encryption
    #         operation.
    #
    #  \returns A dictionary that maps strings to strings. It contains the keys:
    #           MESSAGE_KEY: The starting position for the rotors when the body of the message part is encrypted.
    #           HEADER_GRP_1: The first group that is specified in the header of a message part (Encrypted message key). 
    #           HEADER_GRP_2: The second group that is specified in the header of a message part (Encrypted message key again).
    #           'kenngruppe': The kenngruppe, padded up to 5 characters, which is to be used for this message part.
    #    
    def create_indicators(self, machine, this_part, num_parts):
        result = {}
        
        result[MESSAGE_KEY] = self._rand_gen.get_rand_string(self._num_rotors)
        
        while not self._verifier(result[MESSAGE_KEY]):
            result[MESSAGE_KEY] = self._rand_gen.get_rand_string(self._num_rotors)        
        
        machine.set_rotor_positions(self.grundstellung)
        result[HEADER_GRP_1] = machine.encrypt(result[MESSAGE_KEY])
        result[HEADER_GRP_2] = machine.encrypt(result[MESSAGE_KEY])
        result['kenngruppe'] = self._rand_gen.get_rand_string(2) + self._get_next_kenngruppe()
        
        return result    

    ## \brief This method recreates the message key from the indicator groups.
    #
    #  \param [machine] A rotorsim.RotorMachine object. It is used to decrypt the message key.
    #
    #  \param [already_parsed_indicators] A dictionary that maps strings to strings. When calling this method
    #         this dictionary has to contain the indicator groups as parsed from the current message part during decryption. I.e.
    #         it has to contain at least the the keys HEADER_GRP_1, HEADER_GRP_2.
    #
    #  \returns A dictionary that maps strings to strings. This method adds the key MESSAGE_KEY to already_parsed_indicators that
    #           specifies the position of the rotors when the body of a message part is decrypted.
    #        
    def derive_message_key(self, machine, already_parsed_indicators):
        result = already_parsed_indicators
        machine.set_rotor_positions(self.grundstellung)
        result[MESSAGE_KEY] = machine.decrypt(result[HEADER_GRP_1])
        temp = machine.decrypt(result[HEADER_GRP_2])
        
        if (result[MESSAGE_KEY] != temp) or (not self._verifier(result[MESSAGE_KEY])):
            raise EnigmaException("Header groups do not create same message key or message key invalid")
        
        return result


## \brief This class is a simple struct that is to be returned by any message key tester.
#
class MsgKeyTestResult:
    ## \brief Constructor.
    #
    #  \param [verified] A boolean. Is True if the message key candidate was successfully verified.
    #
    #  \param [transformed] A string. it contains the transformed message key candidate.
    #
    #  \returns Nothing.
    #    
    def __init__(self, verified, transformed):
        ## \brief A boolean.
        self.verified = verified
        ## \brief A string.
        self.transformed = transformed


## \brief This class implements an indicator system that uses a fixed rotor alignment (the Grundstellung) to derive.
#         the message key from a randomly selected indicator. In principle instances of this class can be used with any
#         rotor machine. It creates only one indicator group. The message key is derived from a random indicator by encrypting
#         this indicator using the Grundstellung. The random indicator group is transmitted in the clear to the receiver of the
#         message.
#
#  As some machines do not allow all characters in the range a-z during encryptions (KL7, Typex, SIGABA) or do not allow all output
#  characters in determining the message key (SG39) indicator candidates additionally have to be transformed and verified.
#
#  Before encryption it has to be verified that a random indicator is valid. The SIGABA for instance excludes certain values
#  like O and Z. Additionally some machines (SIGABA, KL7, Typex) do not allow all characters in the range a-z during encryptions.
#  This has to be compensated for by a transformation of the random indicator before encryption. Finally after encryption of the
#  verified and transformed indicator another check has to be performed as for instance the SG39 allows only certain values in
#  the last three characters of a rotor position.
#
class GrundstellungIndicatorProc(IndicatorProcessor):
    ## \brief Constructor.
    #
    #  \param [server] An object that has the same interface as pyrmsk2.tlvobject.TlvServer.
    #
    #  \param [rand_gen] An object that has the same interface as pyrmsk2.rotorrandom.RotorRandom.
    #
    #  \param [indicator_size] An integer. It has to specify the number of characters in an indicator group.
    #
    #  \param [step_before_proc] A boolean. If True the machine is stepped once before doing any encryptions. Needed for KL7.
    #
    #  \returns Nothing.
    #
    def __init__(self,  server, rand_gen, indicator_size, step_before_proc = False):
        super().__init__(server, rand_gen)
        ## \brief Holds the basic setting of the rotors which is used to encrypt the message key.
        self._grundstellung = ''
        ## \brief Holds the size in characters of the indicator group.
        self._indicator_size = indicator_size
        ## \brief Specifies the keyword that can be used by a formatter to create or parse the header lines.
        self._key_words = ['rand_indicator']
        ## \brief Verifies before encryption that an indicator candidate is valid.
        self._verifier = (lambda x: len(x) == self._indicator_size)
        ## \brief Transforms an indicator candidate before encryption if that is necessary.
        self._transformer = lambda x: x
        ## \brief Tests and if necessary transforms the generated message key after encryption.
        self._msg_key_tester = lambda x: MsgKeyTestResult(True, x)
        ## \brief Boolean that determiens if the underlying machine is stepped before encryptions.
        self._step_before_proc = step_before_proc

    ## \brief This property returns the keywords that can be used by an object with the same interface as Formatter.
    #
    #  \returns A sequence of strings.
    #    
    @property
    def key_words(self):
        return self._key_words
    
    ## \brief This property returns the grundstellung.
    #
    #  \returns A string. The grundstellung
    #
    @property
    def grundstellung(self):
        return self._grundstellung

    ## \brief This property setter allows to change the grundstellung.
    #
    #  \param [new_grundstellung] A string. The new grundstellung.
    #
    #  \returns Nothing
    #
    @grundstellung.setter
    def grundstellung(self, new_grundstellung):
        self._grundstellung = new_grundstellung
        
    ## \brief This property returns the verifier that is used to check message key candidates before encrypting them
    #         using the grundstellung.
    #
    #  \returns A callable object that takes a string and returns a bool. It is used to verify a message key candidate.
    #
    @property
    def verifier(self):
        return self._verifier

    ## \brief This property setter allows to change the verifier.
    #
    #  \param [new_verifier] A callable object that takes a string and returns a bool.
    #
    #  \returns Nothing
    #
    @verifier.setter
    def verifier(self, new_verifier):
        self._verifier = new_verifier        
    
    ## \brief This property returns the message key tester that is used to check message key candidates after encrypting them using
    #         the grundstellung.
    #
    #  \returns A callable object that takes a string and returns a MsgKeyTestResult object.
    #
    @property
    def msg_key_tester(self):
        return self._msg_key_tester

    ## \brief This property setter allows to change the message key tester.
    #
    #  \param [new_msg_key_tester] A callable object that takes a string and returns a MsgKeyTestResult object.
    #
    #  \returns Nothing
    #
    @msg_key_tester.setter
    def msg_key_tester(self, new_msg_key_tester):
        self._msg_key_tester = new_msg_key_tester        
    
    ## \brief This property returns the transformer that can be used to transform message key candidates before encryption.
    #
    #  \returns A callable object that takes a string and returns a string.
    #
    @property
    def transformer(self):
        return self._transformer

    ## \brief This property setter allows to change the transformer.
    #
    #  \param [new_transformer] A callable object that takes a string and returns a string.
    #
    #  \returns Nothing
    #
    @transformer.setter
    def transformer(self, new_transformer):
        self._transformer = new_transformer        

    ## \brief This method creates the indicator group for the Grundstellung messaging procedure.
    #
    #  \param [machine] A rotorsim.RotorMachine object. It is used to create the encrypted indicator group.
    #
    #  \param [this_part] An integer. It specifies the sequence number of the message part for which this method
    #         is called.
    #
    #  \param [num_parts] An integer. It has to specify the overall number of message parts of in the current encryption
    #         operation.
    #
    #  \returns A dictionary that maps strings to strings. It contains the keys:
    #           MESSAGE_KEY: The starting position for the rotors when the body of the message part is encrypted.
    #           'rand_indicator': The random indicator.
    #    
    def create_indicators(self, machine, this_part, num_parts):
        result = {}
        indicator_found = False
        indicator_candidate = ''
        
        while not indicator_found:
            indicator_candidate = self._rand_gen.get_rand_string(self._indicator_size)
            # Transform and verify indicator candidate before encryption
            candidate_found = self._verifier(self._transformer(indicator_candidate))
            
            if candidate_found:
                # Set machine to defined grundstellung
                machine.set_rotor_positions(self.grundstellung)
                result[self._key_words[0]] = indicator_candidate
                machine.go_to_letter_state()
                
                if self._step_before_proc:
                    machine.step()
                
                # Encrypt random indicator resulting in message key candidate
                msg_key_candidate = machine.encrypt(self._transformer(result[self._key_words[0]]))
                machine.go_to_letter_state()
                
                # Test message key candidate after encryption of random indicator
                test_res = self._msg_key_tester(msg_key_candidate)                
                indicator_found = test_res.verified
                
                if indicator_found:
                    result[MESSAGE_KEY] = test_res.transformed
        
        return result    

    ## \brief This method recreates the message key from the indicator group.
    #
    #  \param [machine] A rotorsim.RotorMachine object. It is used to recreate the message key.
    #
    #  \param [already_parsed_indicators] A dictionary that maps strings to strings. When calling this method this dictionary
    #         has to contain the indicator group as parsed from the current message part during decryption. I.e. it has to
    #         contain at least the the key contained in self._key_words.
    #
    #  \returns A dictionary that maps strings to strings. This method adds the key MESSAGE_KEY that specifies the the starting 
    #           position for the rotors when the body of a message part is decrypted.
    #        
    def derive_message_key(self, machine, already_parsed_indicators):
        result = already_parsed_indicators
        # Set machine to defined grundstellung
        machine.set_rotor_positions(self.grundstellung)
        # Compensate for blanks and shifting characters ...
        rand_indicator = self._transformer(result[self._key_words[0]])
        
        # Check that transformed indicator is valid
        if self._verifier(rand_indicator):    
            machine.go_to_letter_state()
            
            if self._step_before_proc:
                machine.step()            
            
            # Encrypt random indicator resulting in message key candidate    
            msg_key_candidate = machine.encrypt(rand_indicator)
            machine.go_to_letter_state()
            # Test message key candidate 
            test_res = self._msg_key_tester(msg_key_candidate)
            
            if test_res.verified:
                result[MESSAGE_KEY] = test_res.transformed
            else:
                raise EnigmaException('Indicator invalid')            
        else:
            raise EnigmaException('Indicator invalid')
        
        return result

## \brief This class serves as the base class for all indicator processors that can be used with the two SIGABA variants.
#         It adds helper methods that deal with simplifying getting and setting rotor postions and creating valid random
#         indicators.
#
class SIGABAIndicatorProcessorBase(IndicatorProcessor):
    ## \brief Constructor.
    #
    #  \param [server] An object that has the same interface as pyrmsk2.tlvobject.TlvServer.
    #
    #  \param [rand_gen] An object that has the same interface as pyrmsk2.rotorrandom.RotorRandom.
    #
    #  \returns Nothing.
    #
    def __init__(self, server, rand_gen):
        super().__init__(server, rand_gen)
        ## \brief Specifies the key word that can be used by a formatter to create or parse the header lines.
        self._key_words = [INTERNAL_INDICATOR]
        ## \brief Specifies how many characters are in an indicator.
        self._indicator_size = 5
        
    ## \brief This property returns the key words that can be used by an object with the same interface as Formatter.
    #
    #  \returns A sequence of strings.
    #    
    @property
    def key_words(self):
        return self._key_words

    ## \brief This method queries the underlying SIGABA for its rotor position and splits the resulting value into components
    #         which contains the positions of the index, control and cipher rotors.
    #
    #  \param [machine] A rotorsim.RotorMachine object. It is queried for its rotor position.
    #
    #  \returns A 3-tuple of strings. The first element contains the positions of the index rotors, the second that of the
    #           control rotors and the third the position of the cipher rotors.
    #
    def _get_parsed_rotor_pos(self, machine):
        positions = machine.get_rotor_positions()
        return (positions[:5], positions[5:10], positions[10:])

    ## \brief This method sets the rotor positions of the underlying SIGABA where this positions is specified by a 3.tuple of
    #         strings. 
    #
    #  \param [machine] A rotorsim.RotorMachine object. It is queried for its rotor position.
    #
    #  \param [pos] A 3-tuple of strings. The first element contains the positions of the index rotors, the second that of the
    #           control rotors and the third the position of the cipher rotors.
    #
    #  \returns Nothing.
    #
    def _set_parsed_rotor_pos(self, machine, pos):
        positions = machine.set_rotor_positions(pos[0] + pos[1] + pos[2])

    ## \brief This method generates a random indicator of size self._indicator_size which is a string of suitable length that
    #         does not contain 'o' or 'z'. 
    #
    #  \returns A string. The random indicator.
    #        
    def _make_indicator(self):
        result = ''
        internal_indicator_found = False
        
        while not internal_indicator_found:
            result = self._rand_gen.get_rand_string(self._indicator_size)
            internal_indicator_found = ('o' not in result) and ('z' not in result)
        
        return result    

## \brief This class implements an indicator processor for the two SIGABA variants that uses a random rotor position as the message key.
#         Encrypting this random rotor position with a basic setting or grundstellung results in the indicator group that is sent along in
#         the header of the message.
#
#  It has to be stressed that in contrast to the generic grundstellung message procedure implemented by GrundstellungIndicatorProc this
#  procedure uses the randomly chosen rotor position directly as the message key and transmits it to the receicer by including an encrypted
#  version of the random message key in the message header. 
#  
#  The message procedure implemented by this class is described here: https://maritime.org/doc/crypto/ecm/sec03.htm#pg31
#
class SIGABAGrundstellungIndicatorProcessor(SIGABAIndicatorProcessorBase):
    ## \brief Constructor.
    #
    #  \param [server] An object that has the same interface as pyrmsk2.tlvobject.TlvServer.
    #
    #  \param [rand_gen] An object that has the same interface as pyrmsk2.rotorrandom.RotorRandom.
    #
    #  \returns Nothing.
    #
    def __init__(self, server, rand_gen):
        super().__init__(server, rand_gen)
        ## \brief Holds the basic setting of the rotors which is used to encrypt the message key.
        self._grundstellung = ''
        
    ## \brief This property returns the grundstellung.
    #
    #  \returns A string. The grundstellung
    #
    @property
    def grundstellung(self):
        return self._grundstellung

    ## \brief This property setter allows to change the grundstellung.
    #
    #  \param [new_grundstellung] A string. The new grundstellung.
    #
    #  \returns Nothing
    #
    @grundstellung.setter
    def grundstellung(self, new_grundstellung):
        self._grundstellung = new_grundstellung        

    ## \brief This method creates the indicator group for the Grundstellung messaging procedure. The grundstellung has a length
    #         of five characters and is used to determine the position of the cipher and the control rotors when creating the internal 
    #         indicator.
    #
    #  \param [machine] A rotorsim.RotorMachine object. It is used to create the encrypted indicator group.
    #
    #  \param [this_part] An integer. It specifies the sequence number of the message part for which this method
    #         is called.
    #
    #  \param [num_parts] An integer. It has to specify the overall number of message parts of in the current encryption
    #         operation.
    #
    #  \returns A dictionary that maps strings to strings. It contains the keys:
    #           MESSAGE_KEY: The starting position for the rotors when the body of the message part is encrypted.
    #           INTERNAL_INDICATOR: The encrypted random indicator.
    #
    def create_indicators(self, machine, this_part, num_parts):
        result = {MESSAGE_KEY:''}
        
        message_wheel_pos = self._make_indicator()
        index_pos, stepping_pos, cipher_pos = self._get_parsed_rotor_pos(machine)
        # Use the grundstellung for the control and the cipher rotors
        self._set_parsed_rotor_pos(machine, (index_pos, self._grundstellung, self._grundstellung))
        # Indicator group is the encrypted version of the message key
        result[INTERNAL_INDICATOR] = machine.encrypt(message_wheel_pos)
        
        # Use the five character message_wheel_pos to set the position of the cipher and the control rotors
        result[MESSAGE_KEY] = index_pos + message_wheel_pos + message_wheel_pos
        
        return result

    ## \brief This method recreates the message key from the indicator group by decrypting that group using the grundstellung.
    #
    #  \param [machine] A rotorsim.RotorMachine object. It is used to recreate the message key.
    #
    #  \param [already_parsed_indicators] A dictionary that maps strings to strings. When calling this method this dictionary
    #         has to contain the indicator group as parsed from the current message part during decryption. I.e. it has to
    #         contain at least the the key contained in self._key_words.
    #
    #  \returns A dictionary that maps strings to strings. This method adds the key MESSAGE_KEY that specifies the the starting 
    #           position for the rotors when the body of a message part is decrypted.
    #
    def derive_message_key(self, machine, already_parsed_indicators):
        result = already_parsed_indicators
        
        index_pos, stepping_pos, cipher_pos = self._get_parsed_rotor_pos(machine)
        # Set underlying machine to grundstellung
        self._set_parsed_rotor_pos(machine, (index_pos, self._grundstellung, self._grundstellung))
        # Decrypt indcator
        decrypted_indicator = machine.decrypt(result[INTERNAL_INDICATOR])
        
        # Make sure decrypted indicaotr does not cntain 'o' or 'z'
        allowed_chars = set('abcdefghijklmnpqrstuvwxy')
        decrypted_indicator_set = set(decrypted_indicator)
        
        if len(allowed_chars.intersection(decrypted_indicator_set)) != len(decrypted_indicator_set):
            raise EnigmaException('Indicator invalid')
        else:
            # Use decrypted data to set the positions of the cipher and the control rotors                                               
            result[MESSAGE_KEY] = index_pos + decrypted_indicator + decrypted_indicator             
        
        return result


## \brief This class implements an indicator processor for the two SIGABA variants that uses the built in features of the SIGABA to derive
#         a message key.
#
#  The message procedure implemented by this class is described here: https://maritime.org/doc/crypto/ecm/sec03.htm
#
class SIGABABasicIndicatorProcessor(SIGABAIndicatorProcessorBase):
    ## \brief Constructor.
    #
    #  \param [server] An object that has the same interface as pyrmsk2.tlvobject.TlvServer.
    #
    #  \param [rand_gen] An object that has the same interface as pyrmsk2.rotorrandom.RotorRandom.
    #
    #  \returns Nothing.
    #
    def __init__(self, server, rand_gen):
        super().__init__(server, rand_gen)
    
    ## \brief This method creates the indicator groups as prescribed by the messaging procedure described in the document
    #         mentioned above. In short: The SIGABA's cipher and control rotors are set to 'o' and then each control rotor
    #         is stepped manually by a number of steps derived from a randomly chosen indicator. The control rotors then
    #         have reached the positions given in the random indicator and the cipher rotors are at a pseudorandom position
    #         which is then used to encrypt the message body.
    #
    #  \param [machine] A rotorsim.RotorMachine object. It is used to create encrypted indicator groups.
    #
    #  \param [this_part] An integer. It specifies the sequence number of the message part for which this method
    #         is called.
    #
    #  \param [num_parts] An integer. It has to specify the overall number of message parts of in the current encryption
    #         operation.
    #
    #  \returns A dictionary that maps strings to strings. It contains the keys:
    #           MESSAGE_KEY: The starting position for the rotors when the body of the message part is encrypted.
    #           INTERNAL_INDICATOR: The random indicator.
    #
    def create_indicators(self, machine, this_part, num_parts):
        result = {MESSAGE_KEY:''}
        
        result[INTERNAL_INDICATOR] = self._make_indicator()
        result[MESSAGE_KEY] = self._setup_stepping(result[INTERNAL_INDICATOR], machine)
        
        return result

    ## \brief This method does the "manual" setup stepping of the control rotors and returns the rotor position which
    #         is used to encrypt or decrypt a message part.
    #
    #  \param [internal_indicator] A five character string. It specifies the positions to which the control are to be
    #         stepped.
    #
    #  \param [machine] A rotorsim.RotorMachine object. It is used to create encrypted indicator groups.
    #
    #  \returns A string. It contains the rotor position of all the rotors in the machine at the end of the setup 
    #           stepping.
    #        
    def _setup_stepping(self, internal_indicator, machine):
        index_pos, stepping_pos, cipher_pos = self._get_parsed_rotor_pos(machine)
        # Set cipher and control rotors to 'ooooo'
        self._set_parsed_rotor_pos(machine, (index_pos, 'ooooo', 'ooooo'))
        
        # Iterate over control rotors
        for i in range(5):
            index_pos, stepping_pos, cipher_pos = self._get_parsed_rotor_pos(machine)
            wheel_is_setup = stepping_pos[i] == internal_indicator[i]
            
            # Continue setup stepping as long as the current control rotor has not reached its intended position
            while not wheel_is_setup:
                machine.sigaba_setup(i + 1)
                index_pos, stepping_pos, cipher_pos = self._get_parsed_rotor_pos(machine)
                wheel_is_setup = stepping_pos[i] == internal_indicator[i]        
        
        return machine.get_rotor_positions()              
            
    ## \brief This method recreates the message key from the indicator group specified in the header of a message part.
    #
    #  \param [machine] A rotorsim.RotorMachine object. It is used to create encrypted indicator groups.
    #
    #  \param [already_parsed_indicators] A dictionary that maps strings to strings. When calling this method
    #         this dictionary has to contain the indicator groups as parsed from the current message part during decryption.
    #
    #  \returns A dictionary that maps strings to strings. This method adds the MESSAGE_KEY key the value of which contains
    #           the starting positions of all SIGABA rotors for message en- or decryption.
    #    
    def derive_message_key(self, machine, already_parsed_indicators):
        result = already_parsed_indicators
        
        # Indicators containing 'o' or 'z' are invalid
        if ('o' in result[INTERNAL_INDICATOR]) or ('z' in result[INTERNAL_INDICATOR]):
            raise EnigmaException('Indicator invalid')
        else:        
            result[MESSAGE_KEY] = self._setup_stepping(result[INTERNAL_INDICATOR], machine)               
        
        return result

