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

#!/usr/bin/env python3
## @package enigproc Implements a command line application that allows to en- and decrypt a message following the message procedure defined by the german army and air force.
#         
#           
# \file enigproc.py
# \brief Implements a command line application that allows to en- and decrypt a message following the message procedure 
#        defined by the german army and air force for Enigma usage.

import sys
import pyrmsk2.tlvsrvapp as tlvsrvapp
import pyrmsk2.rotorsim as rotorsim
import pyrmsk2.rotorrandom as rotorrandom
import datetime
import argparse
import re
import functools

## \brief Maximum number of real plaintext characters in a message part. 
COMMANDS = ['encrypt', 'decrypt']
# 1534 = 15tle = 15tl = 167 = RJF GNZ =
ENIGMA_HEADER_EXP = '^[0-9]{{4}} = [0-9]+(tl|tle) = [0-9]+tl = [0-9]+ = ([A-Z]{{{0}}}) ([A-Z]{{{0}}}) =$'
MESSAGE_KEY = 'message_key'
HEADER_GRP_1 = 'start_pos'
HEADER_GRP_2 = 'encrypted_message_key'
INTERNAL_INDICATOR = 'internal_indicator'
EXTERNAL_INDICATOR = 'external_indicator'
MESSAGE_LENGTH = 'message_length'
PROC_TYPES = ['grundstellung', 'post1940', 'pre1940', 'sigaba']
DUMMY_SYS_INDICATOR = 'ert zui ops cfg'

NO_SHIFT_CHAR = ''

## \brief This class is used to signal exceptions to calling code.
#
class EnigmaException(Exception):
    def __init__(self, error_message):
        Exception.__init__(self, 'Message procedure: ' + error_message)

# ----------------------------------------------------------------------------------------------------


## \brief This class serves as a base class for a "thing" that knows how to "prepare" plaintexts before encryption
#         and reverse this preparation after decryption to reconstruct the original plaintext.
#
class TransportEncoder:
    ## \brief Constructor
    #
    def __init__(self):
        ## \brief A vector of chars that is used to filter the input data when doing encryptions.
        self._allowed_plain_chars = 'abcdefghijklmnopqrstuvwxyz'
    
    ## \brief This method transform a plaintext into an encoded form before that encoded form ist encrypted.
    #
    #  \param [plaintext] A string. Contains the plaintext to transform
    #
    #  \returns A string. The encoded plaintext
    #
    def transform_plaintext_enc(self, plaintext):
        return ''.join(list(filter(lambda x: x in self._allowed_plain_chars, plaintext.lower())))

    ## \brief This method transforms a decryped plaintext into its original form.
    #
    #  \param [plaintext] A string. Contains the plaintext to transform
    #
    #  \returns A string. The decoded plaintext
    #    
    def transform_plaintext_dec(self, plaintext):
        result = plaintext
        
        return result


## \brief This class implements the transport encoder used by the german army during WWII for use with the Engima.
#
class ArmyEncoder(TransportEncoder):
    ## \brief Constructor
    #
    def __init__(self):
        super().__init__()
        
    ## \brief This method filters out characters which are not allowed as input and transforms the plaintext according to
    #         the rules set out in the message procedure.
    #
    #  \param [full_plain] A string. Input text for an encryption as specified by the caller.
    #
    #  \returns A string. It contains the filtered and transformed plaintext.
    #                
    def transform_plaintext_enc(self, full_plain):
        full_plain = full_plain.lower()
        full_plain = full_plain.replace('.', 'x')
        full_plain = full_plain.replace(',', 'zz')
        full_plain = full_plain.replace('ch', 'q')
        full_plain = full_plain.replace('?', 'fragez')
        full_plain = full_plain.replace('ä', 'ae')
        full_plain = full_plain.replace('ö', 'oe')
        full_plain = full_plain.replace('ü', 'ue')                        
        full_plain = full_plain.replace('ß', 'ss')        
        return ''.join(list(filter(lambda x: x in self._allowed_plain_chars, full_plain)))

    ## \brief This method transforms the raw plaintext coming out of the machine according to
    #         the rules set out in the message procedure back into a more human readable form. In a way this
    #         is the inverse of transform_plaintext_enc().
    #
    #  \param [full_plain] A string. Input text as created by the cipher machine.
    #
    #  \returns A string. It contains transformed plaintext.
    #                    
    def transform_plaintext_dec(self, full_plain):
        full_plain = full_plain.lower()
        full_plain = full_plain.replace('zz', ', ')
        full_plain = full_plain.replace('qu', '#')
        full_plain = full_plain.replace('q', 'ch')
        full_plain = full_plain.replace('#', 'qu')
        full_plain = full_plain.replace('fragez', '?')
        full_plain = full_plain.replace('x', 'x ')
        return full_plain        


## \brief This class implements the transport encoder used by the german army during WWII for use with the Engima.
#
class SIGABAEncoder(TransportEncoder):
    ## \brief Constructor
    #
    def __init__(self):
        super().__init__()
        self._allowed_plain_chars = 'abcdefghijklmnopqrstuvwxy '
        
    ## \brief This method filters out characters which are not allowed as input and transforms the plaintext according to
    #         the rules set out in the message procedure.
    #
    #  \param [full_plain] A string. Input text for an encryption as specified by the caller.
    #
    #  \returns A string. It contains the filtered and transformed plaintext.
    #                
    def transform_plaintext_enc(self, full_plain):
        full_plain = full_plain.lower()
        full_plain = full_plain.replace('.', 'x')
        full_plain = full_plain.replace(',', 'x')
        full_plain = full_plain.replace('z', 'x')
        full_plain = full_plain.replace('?', ' ques')
        return ''.join(list(filter(lambda x: x in self._allowed_plain_chars, full_plain)))

    ## \brief This method transforms the raw plaintext coming out of the machine according to
    #         the rules set out in the message procedure back into a more human readable form. In a way this
    #         is the inverse of transform_plaintext_enc().
    #
    #  \param [full_plain] A string. Input text as created by the cipher machine.
    #
    #  \returns A string. It contains transformed plaintext.
    #                    
    def transform_plaintext_dec(self, full_plain):
        full_plain = full_plain.lower()
        full_plain = full_plain.replace(' ques', '?')
        return full_plain        


class ShiftingEncoder(TransportEncoder):
    ## \brief Constructor
    #
    def __init__(self, letter_alpha, figure_alpha):
        self._letter_alpha = letter_alpha
        self._figure_alpha = figure_alpha
    
    def transform_shifted_characters(self, plaintext):
        result = ''

        for i in plaintext:
            if i in self._letter_alpha:
                result += i
            else:            
                if (i != ' ') and (i in self._figure_alpha):
                    result += '>' + i + '<'
        
        return result
                
    
    ## \brief This method transform a plaintext into an encoded form before that encoded form ist encrypted.
    #
    #  \param [plaintext] A string. Contains the plaintext to transform
    #
    #  \returns A string. The encoded plaintext
    #
    def transform_plaintext_enc(self, plaintext):
        plaintext = ''.join(list(filter(lambda x: x not in '<>', plaintext.lower())))
        plaintext = plaintext.replace('ä', 'ae')
        plaintext = plaintext.replace('ö', 'oe')
        plaintext = plaintext.replace('ü', 'ue')                        
        plaintext = plaintext.replace('ß', 'ss')
        plaintext = ''.join(list(filter(lambda x: (x in self._letter_alpha) or (x in self._figure_alpha), plaintext)))
        
        result = self.transform_shifted_characters(plaintext)
                
        return result

class KL7Encoder(ShiftingEncoder):
    ## \brief Constructor
    #
    def __init__(self):
        super().__init__("abcdefghi>klmnopqrstuvwxy ", "abcd3fgh8>klmn9014s57<2x6 ")

    def transform_plaintext_enc(self, plaintext):
        plaintext = ''.join(list(filter(lambda x: x not in '<>', plaintext.lower())))
        plaintext = plaintext.replace('ä', 'ae')
        plaintext = plaintext.replace('ö', 'oe')
        plaintext = plaintext.replace('ü', 'ue')                        
        plaintext = plaintext.replace('ß', 'ss')
        plaintext = plaintext.replace('j', 'i')
        plaintext = plaintext.replace('z', 'x')        
        plaintext = ''.join(list(filter(lambda x: (x in self._letter_alpha) or (x in self._figure_alpha), plaintext)))
        
        result = self.transform_shifted_characters(plaintext)
                
        return result


## \brief This class implements a transport encoder that makes use of the features of the Typex that allow it to 
#         process quite a few special characters.
#
class TypexEncoder(ShiftingEncoder):
    ## \brief Constructor
    #
    def __init__(self):
        super().__init__("abcdefghijklmnopqrstu<w y>", "-'vz3%x£8*().,9014/57<2 6>")
            

# ----------------------------------------------------------------------------------------------------


## \brief This class serves as a base class for a "thing" that knows how create indicator groups during encryption
#         and is able to reconstruct the message key from the indicator groups parsed from the ciphertext during
#         decryption. This class and its children is supposed to not know how the indicator groups are represented
#         in ciphertext. They deal with the groups as simple strings.
#
class IndicatorProcessor:
    ## \brief Constructor.
    #
    #  \param [server] An object that has the same interface as pyrmsk2.tlvobject.TlvServer.
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
    #  \param [this_part] An integer. It specifies the sequence number of the message part for which this method
    #         is called.
    #  \param [num_parts] An integer. It has to specify the overall number of message parts of in the current encryption
    #         operation.
    #
    #  \returns A dictionary that maps strings to strings. It has to contain a key 'message_key' that specifies the
    #           starting positions of the machines rotors at the beginning of the encryption of the body of this message
    #           part.
    #
    def create_indicators(self, machine, this_part, num_parts):
        result = {MESSAGE_KEY:''}
        
        return
            
    ## \brief Children have to oeverride this method. It is intended to recreate the message key from the indicator groups
    #         as parsed from the ciphertext of a message part during decryption.
    #
    #  \param [machine] A rotorsim.RotorMachine object. It is used to create encrypted indicator groups.
    #  \param [already_parsed_indicators] A dictionary that maps strings to strings. When calling this method
    #         this dictionary has to contain the indicator groups as parsed from the current message part during decryption.
    #
    #  \returns A dictionary that maps strings to strings. It has to contain a key 'message_key' that specifies the
    #           starting positions of the machines rotors at the beginning of the decryption of the body of this message
    #           part.
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
#         decryption with an Enigma that uses the kenngruppen indicator system.
#
class EnigmaKenngruppenIndicatorProc(IndicatorProcessor):
    ## \brief Constructor.
    #
    #  \param [server] An object that has the same interface as pyrmsk2.tlvobject.TlvServer.
    #  \param [rand_gen] An object that has the same interface as pyrmsk2.rotorrandom.RotorRandom.
    #  \param [kenngruppen] A sequence of strings. Specifies the kennruppen to use.
    #
    def __init__(self,  server, rand_gen, kenngruppen, num_rotors):
        super().__init__(server, rand_gen)
        ## \brief Holds the kenngruppen to use.
        self._kenngruppen = kenngruppen
        ## \brief Holds a random permutation of 0 ... len(self._kenngruppen).        
        self._group_shuffle = []
        ## \brief Points to the current read position in self._group_shuffle.
        self._shuffle_pos = 0
        self._num_rotors = num_rotors        

    ## \brief This method changes the kenngruppen that are in used in this object.
    #
    #  \param [new_kenngruppen] A sequence of strings. It has to contain the new kenngruppen.
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
    #  \returns A string.
    #
    def reset(self):
        self._group_shuffle = []
        self._shuffle_pos = 0 


## \brief This class implements the indicator system that was used by the german army and air force from 1940
#         onward. The machine operator chooses a random starting position and a random message key. The indicator
#         groups consist of the random starting position and the message key encrypted with the starting position.
#
class Post1940EnigmaIndicatorProc(EnigmaKenngruppenIndicatorProc):
    ## \brief Constructor.
    #
    #  \param [server] An object that has the same interface as pyrmsk2.tlvobject.TlvServer.
    #  \param [rand_gen] An object that has the same interface as pyrmsk2.rotorrandom.RotorRandom.
    #  \param [kenngruppen] A sequence of strings. Specifies the kennruppen to use.
    #
    def __init__(self,  server, rand_gen, kenngruppen, num_rotors = 3):
        super().__init__(server, rand_gen, kenngruppen, num_rotors)

    ## \brief This method creates the indicator groups for the implemented post 1940 messaging procedure.
    #
    #  \param [machine] A rotorsim.RotorMachine object. It is used to create encrypted indicator groups.
    #  \param [this_part] An integer. It specifies the sequence number of the message part for which this method
    #         is called.
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
        result[HEADER_GRP_1] = self._rand_gen.get_rand_string(self._num_rotors)
        machine.set_rotor_positions(result[HEADER_GRP_1])
        result[HEADER_GRP_2] = machine.encrypt(result[MESSAGE_KEY])
        result['kenngruppe'] = self._rand_gen.get_rand_string(2) + self._get_next_kenngruppe()
        
        return result    

    ## \brief This method recreates the message key from the indicator groups.
    #
    #  \param [machine] A rotorsim.RotorMachine object. It is used to decrypt the message key.
    #  \param [already_parsed_indicators] A dictionary that maps strings to strings. When calling this method
    #         this dictionary has to contain the indicator groups as parsed from the current message part during decryption. I.e.
    #         it has to contain at least the the keys HEADER_GRP_1, HEADER_GRP_2.
    #
    #  \returns A dictionary that maps strings to strings. This method adds the key MESSAGE_KEY that specifies the the starting 
    #           position for the rotors when the body of a message part is decrypted.
    #        
    def derive_message_key(self, machine, already_parsed_indicators):
        result = already_parsed_indicators
        machine.set_rotor_positions(result[HEADER_GRP_1])
        result[MESSAGE_KEY] = machine.decrypt(result[HEADER_GRP_2])
        
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
    #  \param [rand_gen] An object that has the same interface as pyrmsk2.rotorrandom.RotorRandom.
    #  \param [kenngruppen] A sequence of strings. Specifies the kennruppen to use.
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
    #  \param [new_grundsellung] A string. The new grundstellung.
    #
    #  \returns Nothing
    #
    @grundstellung.setter
    def grundstellung(self, new_grundstellung):
        self._grundstellung = new_grundstellung

    ## \brief This method creates the indicator groups for the messaging procedure in force before 1940.
    #
    #  \param [machine] A rotorsim.RotorMachine object. It is used to create encrypted indicator groups.
    #  \param [this_part] An integer. It specifies the sequence number of the message part for which this method
    #         is called.
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
        machine.set_rotor_positions(self.grundstellung)
        result[HEADER_GRP_1] = machine.encrypt(result[MESSAGE_KEY])
        result[HEADER_GRP_2] = machine.encrypt(result[MESSAGE_KEY])
        result['kenngruppe'] = self._rand_gen.get_rand_string(2) + self._get_next_kenngruppe()
        
        return result    

    ## \brief This method recreates the message key from the indicator groups.
    #
    #  \param [machine] A rotorsim.RotorMachine object. It is used to decrypt the message key.
    #  \param [already_parsed_indicators] A dictionary that maps strings to strings. When calling this method
    #         this dictionary has to contain the indicator groups as parsed from the current message part during decryption. I.e.
    #         it has to contain at least the the keys HEADER_GRP_1, HEADER_GRP_2.
    #
    #  \returns A dictionary that maps strings to strings. This method adds the key MESSAGE_KEY that specifies the the starting 
    #           position for the rotors when the body of a message part is decrypted.
    #        
    def derive_message_key(self, machine, already_parsed_indicators):
        result = already_parsed_indicators
        machine.set_rotor_positions(self.grundstellung)
        result[MESSAGE_KEY] = machine.decrypt(result[HEADER_GRP_1])
        temp = machine.decrypt(result[HEADER_GRP_2])
        
        if result[MESSAGE_KEY] != temp:
            raise EnigmaException("Header groups do not create same message key")
        
        return result


## \brief This class is a simple struct that is to be returned by any message key tester.
#
class MsgKeyTestResult:
    def __init__(self, verified, transformed):
        self.verified = verified
        self.transformed = transformed


## \brief This class implements an indicator system that uses a fixed rotor alignment (the Grundstellung) to derive.
#         the message key from a randomly selected indicator. In principle instances of this class can be used with any
#         rotor machine. It creates only one indicator group the size of which has to be equal to the number of
#         settable rotors in the underlying rotor machine.
#
class GrundstellungIndicatorProc(IndicatorProcessor):
    ## \brief Constructor.
    #
    #  \param [server] An object that has the same interface as pyrmsk2.tlvobject.TlvServer.
    #  \param [rand_gen] An object that has the same interface as pyrmsk2.rotorrandom.RotorRandom.
    #  \param [indicator size] An integer. It has to specify the number of characters in an indicator group.
    #  \param [step_before_proc] A boolean. If True the machine is stepped once before doing any encryptions. Needed for KL7.
    #
    def __init__(self,  server, rand_gen, indicator_size, step_before_proc = False):
        super().__init__(server, rand_gen)
        ## \brief Holds the basic setting of the rotors which is used to encrypt the message key.
        self._grundstellung = ''
        ## \brief Holds the size in characters of the indicator group.
        self._indicator_size = indicator_size
        ## \brief Specifies the key word that can be used by a formatter to create or parse the header lines.
        self._key_words = ['rand_indicator']
        ## \brief Verifies that an indicator candidate is valid.
        self._verifier = (lambda x: len(x) == self._indicator_size)
        ## \brief Transforms an indicator candidate if that is necessary.
        self._transformer = lambda x: x
        ## \brief Tests and if necessary transforms the generated message key
        self._msg_key_tester = lambda x: MsgKeyTestResult(True, x)
        ## \brief Step underlying machine before encryptions
        self._step_before_proc = step_before_proc

    ## \brief This property returns the key words that can be used by an object with the same interface as Formatter.
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
    #  \param [new_grundsellung] A string. The new grundstellung.
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
    
    ## \brief This property returns the transformer that can be used to transform message key candidates.
    #
    #  \returns A callable object that takes a string and returns a string.
    #
    @property
    def transformer(self):
        return self._transformer

    ## \brief This property setter allows to change the transformer.
    #
    #  \param [new_ntransformer] A callable object that takes a string and returns a string.
    #
    #  \returns Nothing
    #
    @transformer.setter
    def transformer(self, new_transformer):
        self._transformer = new_transformer        

    ## \brief This method creates the indicator groups for the messaging procedure in force before 1940.
    #
    #  \param [machine] A rotorsim.RotorMachine object. It is used to create encrypted indicator groups.
    #  \param [this_part] An integer. It specifies the sequence number of the message part for which this method
    #         is called.
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
            candidate_found = self._verifier(self._transformer(indicator_candidate))
            
            if candidate_found:
                machine.set_rotor_positions(self.grundstellung)
                result['rand_indicator'] = indicator_candidate
                machine.go_to_letter_state()
                
                if self._step_before_proc:
                    machine.step()
                
                msg_key_candidate = machine.encrypt(self._transformer(result['rand_indicator']))
                machine.go_to_letter_state()
                
                test_res = self._msg_key_tester(msg_key_candidate)                
                indicator_found = test_res.verified
                
                if indicator_found:
                    result[MESSAGE_KEY] = test_res.transformed
        
        return result    

    ## \brief This method recreates the message key from the indicator groups.
    #
    #  \param [machine] A rotorsim.RotorMachine object. It is used to decrypt the message key.
    #  \param [already_parsed_indicators] A dictionary that maps strings to strings. When calling this method
    #         this dictionary has to contain the indicator groups as parsed from the current message part during decryption. I.e.
    #         it has to contain at least the the keys contained in self._key_words.
    #
    #  \returns A dictionary that maps strings to strings. This method adds the key MESSAGE_KEY that specifies the the starting 
    #           position for the rotors when the body of a message part is decrypted.
    #        
    def derive_message_key(self, machine, already_parsed_indicators):
        result = already_parsed_indicators
        machine.set_rotor_positions(self.grundstellung)
        rand_indicator = self._transformer(result['rand_indicator'])
        
        if self._verifier(rand_indicator):    
            machine.go_to_letter_state()
            
            if self._step_before_proc:
                machine.step()            
                
            msg_key_candidate = machine.encrypt(rand_indicator)
            machine.go_to_letter_state()            
            test_res = self._msg_key_tester(msg_key_candidate)
            
            if test_res.verified:
                result[MESSAGE_KEY] = test_res.transformed
            else:
                raise EnigmaException('Indicator invalid')            
        else:
            raise EnigmaException('Indicator invalid')
        
        return result


class SIGABAIndicatorProcessorBase(IndicatorProcessor):
    def __init__(self, server, rand_gen):
        super().__init__(server, rand_gen)
        ## \brief Specifies the key word that can be used by a formatter to create or parse the header lines.
        self._key_words = [INTERNAL_INDICATOR]
        self._indicator_size = 5
        
    ## \brief This property returns the key words that can be used by an object with the same interface as Formatter.
    #
    #  \returns A sequence of strings.
    #    
    @property
    def key_words(self):
        return self._key_words

    def _get_parsed_rotor_pos(self, machine):
        positions = machine.get_rotor_positions()
        return (positions[:5], positions[5:10], positions[10:])

    def _set_parsed_rotor_pos(self, machine, pos):
        positions = machine.set_rotor_positions(pos[0] + pos[1] + pos[2])
        
    def _make_indicator(self):
        result = ''
        internal_indicator_found = False
        
        while not internal_indicator_found:
            result = self._rand_gen.get_rand_string(self._indicator_size)
            internal_indicator_found = ('o' not in result) and ('z' not in result)
        
        return result    

class SIGABAGrundstellungIndicatorProcessor(SIGABAIndicatorProcessorBase):
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
    #  \param [new_grundsellung] A string. The new grundstellung.
    #
    #  \returns Nothing
    #
    @grundstellung.setter
    def grundstellung(self, new_grundstellung):
        self._grundstellung = new_grundstellung        

    def create_indicators(self, machine, this_part, num_parts):
        result = {MESSAGE_KEY:''}
        
        message_wheel_pos = self._make_indicator()
        index_pos, stepping_pos, cipher_pos = self._get_parsed_rotor_pos(machine)
        self._set_parsed_rotor_pos(machine, (index_pos, self._grundstellung, self._grundstellung))
        result[INTERNAL_INDICATOR] = machine.encrypt(message_wheel_pos)
        
        result[MESSAGE_KEY] = index_pos + message_wheel_pos + message_wheel_pos
        
        return result

    def derive_message_key(self, machine, already_parsed_indicators):
        result = already_parsed_indicators
        
        index_pos, stepping_pos, cipher_pos = self._get_parsed_rotor_pos(machine)
        self._set_parsed_rotor_pos(machine, (index_pos, self._grundstellung, self._grundstellung))
        decrypted_indicator = machine.decrypt(result[INTERNAL_INDICATOR])
        
        allowed_chars = set('abcdefghijklmnpqrstuvwxy')
        decrypted_indicator_set = set(decrypted_indicator)
        
        if len(allowed_chars.intersection(decrypted_indicator_set)) != len(decrypted_indicator_set):
            raise EnigmaException('Indicator invalid')
        else:                                               
            result[MESSAGE_KEY] = index_pos + decrypted_indicator + decrypted_indicator             
        
        return result


class SIGABABasicIndicatorProcessor(SIGABAIndicatorProcessorBase):
    def __init__(self, server, rand_gen):
        super().__init__(server, rand_gen)
    
    ## \brief Children have to oeverride this method. It is intended to create the indicator groups necessary
    #         to create a full message part during encryption.
    #
    #  \param [machine] A rotorsim.RotorMachine object. It is used to create encrypted indicator groups.
    #  \param [this_part] An integer. It specifies the sequence number of the message part for which this method
    #         is called.
    #  \param [num_parts] An integer. It has to specify the overall number of message parts of in the current encryption
    #         operation.
    #
    #  \returns A dictionary that maps strings to strings. It has to contain a key 'message_key' that specifies the
    #           starting positions of the machines rotors at the beginning of the encryption of the body of this message
    #           part.
    #
    def create_indicators(self, machine, this_part, num_parts):
        result = {MESSAGE_KEY:''}
        
        result[INTERNAL_INDICATOR] = self._make_indicator()
        result[MESSAGE_KEY] = self._setup_stepping(result[INTERNAL_INDICATOR], machine)
        
        return result
        
    def _setup_stepping(self, internal_indicator, machine):
        index_pos, stepping_pos, cipher_pos = self._get_parsed_rotor_pos(machine)
        self._set_parsed_rotor_pos(machine, (index_pos, 'ooooo', 'ooooo'))
        
        for i in range(5):
            index_pos, stepping_pos, cipher_pos = self._get_parsed_rotor_pos(machine)
            wheel_is_setup = stepping_pos[i] == internal_indicator[i]

            while not wheel_is_setup:
                machine.sigaba_setup(i + 1)
                index_pos, stepping_pos, cipher_pos = self._get_parsed_rotor_pos(machine)
                wheel_is_setup = stepping_pos[i] == internal_indicator[i]        
        
        return machine.get_rotor_positions()              
            
    ## \brief Children have to oeverride this method. It is intended to recreate the message key from the indicator groups
    #         as parsed from the ciphertext of a message part during decryption.
    #
    #  \param [machine] A rotorsim.RotorMachine object. It is used to create encrypted indicator groups.
    #  \param [already_parsed_indicators] A dictionary that maps strings to strings. When calling this method
    #         this dictionary has to contain the indicator groups as parsed from the current message part during decryption.
    #
    #  \returns A dictionary that maps strings to strings. It has to contain a key 'message_key' that specifies the
    #           starting positions of the machines rotors at the beginning of the decryption of the body of this message
    #           part.
    #    
    def derive_message_key(self, machine, already_parsed_indicators):
        result = already_parsed_indicators
        
        if ('o' in result[INTERNAL_INDICATOR]) or ('z' in result[INTERNAL_INDICATOR]):
            raise EnigmaException('Indicator invalid')
        else:        
            result[MESSAGE_KEY] = self._setup_stepping(result[INTERNAL_INDICATOR], machine)               
        
        return result
        

# ----------------------------------------------------------------------------------------------------        

## \brief This class is a simple struct used by the methods of Formatter and its children.
#
class BodyStruct:
    def __init__(self):
        ## \brief A string containing the formatted ciphertext.
        self.text = ''
        ## \brief An integer. Holds the number of chars in the formatted ciphertext.
        self.num_chars = 0
        ## \brief An integer containing the number of groups in the formatted ciphertext.
        self.num_groups = 0

## \brief This class is a simple struct used by the methods of Formatter and its children.
#    
class ParsedBodyStruct:
    def __init__(self):
        ## \brief A string containing the formatted ciphertext without the indicator groups.
        self.text = ''
        ## \brief A dictionary mapping strings to strings containing the retrieved indicator groups.
        #         The keys used depend on the IndicatorProcessor in use.
        self.indicators = {}

## \brief This class serves as the base class for "a thing" that knows how to format and parse the bodies and headers of 
#         message parts during encryption an decryption. Children of this class know for instance where to put indicator
#         groups as well as how to group the message body.
#
class Formatter:
    ## \brief Constructor.
    #
    def __init__(self):
        ## \brief Number of characters in a group.
        self._group_size = 5
        ## \brief Groups per line.
        self._groups_per_line = 5

    ## \brief This property returns the limits, i.e. group size and number of groups per line.
    #
    #  \returns A tuple of two integers. The first component specifies the number of characters in a group.
    #           The second gives the number of groups in a line.
    #
    @property
    def limits(self):
        return (self._group_size, self._groups_per_line)

    ## \brief This property setter allows to change the limits, i.e. group size and number of groups per line.
    #
    #  \param [limits] A tuple of two integers. The first component specifies the number of characters in a group.
    #         The second gives the number of groups in a line.
    #
    #  \returns Nothing
    #        
    @limits.setter
    def limits(self, limits):
        self._group_size, self._groups_per_line = limits
    
    ## \brief Children have to override this method. It is intended to return a formatted ciphertext during encryptions 
    #         together with the character and group count of the message part in form of dictionary.
    #
    #  \param [ciphertext] A string specifying the unformatted ciphertext.
    #  \param [indicators] A dictionary that maps strings to strings. It has to contain the indicator groups generated
    #         for this message, which may be placed in the message body.
    #
    #  \returns A BodyStruct object.
    #
    def format_body(self, ciphertext, indicators):
        result = BodyStruct()
        
        return result

    ## \brief Children have to override this method. It is intended to parse a retrieved body of a message part during
    #         decryptions. Its task is to seperate indicator groups contained in the body from the ciphertext of the
    #         message part.
    #
    #  \param [body] A string specifying the formatted the ciphertext body of a message part.
    #
    #  \returns A ParsedBodyStruct object.
    #
    def parse_ciphertext_body(self, body):
        result = ParsedBodyStruct()
        
        return result
    
    ## \brief Children have to override this method. It is intended to generate a header for a message part. The
    #         header depends on the already formatted cpiphertext of the body and the indicator groups created by
    #         the IndicatorProcessor in use.
    #
    #  \param [formatted_body] A string specifying the formatted ciphertext body of a message part.
    #  \param [indicators] A dictionary that maps strings to strings. It has to contain the indicator groups generated
    #         for this message part, which may be placed in the message header.
    #  \param [this_part] An integer. It specifies the sequence number of the message part for which this method
    #         is called.
    #  \param [num_parts] An integer. It has to specify the overall number of message parts of in the current encryption
    #         operation.
    #
    #  \returns A string containing the created header.
    #
    def format_header(self, formatted_body, indicators, this_part, num_parts):
        result = ''
        
        return result

    ## \brief Children have to override this method. It is intended to parse a retrieved header of a message part during
    #         decryptions. Its task is to determine the indicator groups contained in the header of the message part.     
    #
    #  \param [indicators] A dictionary that maps strings to strings. It has to contain the indicator groups that have
    #         already been retreived from the message body. This method then adds the indicator groups found in the
    #         header.
    #  \param [header]: A string. It has to contain the header of the current message part.
    #
    #  \returns A dictionary that maps strings to strings. The returned dictionary is the dictionary given in parameter
    #           indicators to which all indicator groups found in the header are added.
    #    
    def parse_ciphertext_header(self, indicators, header):
        result = indicators
        
        return result

    ## \brief Resets the state of this object. Children that keep a state have to override this method. It is called at the
    #         beginning of encryption and decryption of messages.
    #
    #  \returns Nothing
    #    
    def reset(self):
        pass


## \brief This class implements a generic formatter that can be used with any rotor machine. It creates a header of the
#         form "Message number of this part/Number of all parts = Number of groups = header groups separated by space =".
#         The number and size of the header groups can be selected during object construction. 
#
#  Example: 345TT = 4/5 = 49 = ESF = 
#  
#  In this case there is one header group of size 3 
#
#  452TU = 4/5 = 50 = ESFTG JJUZG =
# 
#  And here we have two header groups of size 5.
#
class GenericFormatter(Formatter):
    ## \brief This method formats the body of a rotor machine message.
    #
    #  \param [num_of_header_groups] An integer specifying how many groups are part of the header.
    #  \param [header_group_size] An integer. It has to contain the size in characters of the groups that are part of the header.
    #  \param [group_key_words] A sequence of strings. The first sequence element is used to reference the first header group, the
    #         second element the second header group and so on.
    #
    def __init__(self, num_of_header_groups, header_group_size, group_key_words):
        super().__init__()
        self._num_header_groups = num_of_header_groups
        self._key_words = group_key_words
        self._header_group_size = header_group_size
        self._system_indicator = 'A0000'

    ## \brief This property returns the system indicator which identifies the key or crpyto net to which the message belongs.
    #
    #  \returns A string.
    #
    @property
    def system_indicator(self):
        return self._system_indicator

    ## \brief This property setter allows to change the system indicator.
    #
    #  \param [new_system_indicator] A string. The new system indicator to use.
    #
    #  \returns Nothing
    #        
    @system_indicator.setter
    def system_indicator(self, new_system_indicator):
        self._system_indicator = new_system_indicator
        
    ## \brief This method formats the body of a rotor machine message.
    #
    #  \param [ciphertext] A string specifying the unformatted ciphertext.
    #  \param [indicators] A dictionary that maps strings to strings. The indicators parameter is igonred by this implementation
    #         of format_body().
    #
    #  \returns A BodyStruct object.
    #
    def format_body(self, ciphertext, indicators):
        result = BodyStruct()
        result.num_chars = len(ciphertext)
        result.num_groups = len(ciphertext) // self._group_size
        
        if (len(ciphertext) % self._group_size) != 0:
            result.num_groups += 1
        
        result.text = rotorsim.RotorMachine.group_text(ciphertext, True, self._group_size, self._groups_per_line)        
        
        return result

    ## \brief This method parses the body of a rotor machine message. It simply converers the ciphertext to lowercase.
    #
    #  \param [body] A string specifying the formatted the ciphertext body of a message part.
    #
    #  \returns A ParsedBodyStruct object.
    #
    def parse_ciphertext_body(self, body):
        result = ParsedBodyStruct()
        
        result.text = body.lower()
        
        return result

    ## \brief This method creates a header for a rotor machine message.
    #
    #  \param [formatted_body] A BodyStruct object specifying the already formatted ciphertext body of a message part.
    #  \param [indicators] A dictionary that maps strings to strings. It has to contain at least the indicator groups 
    #         referenced by self._key_words.
    #  \param [this_part] An integer. It specifies the sequence number of the message part for which this method
    #         is called.
    #  \param [num_parts] An integer. It has to specify the overall number of message parts of in the current encryption
    #         operation.
    #
    #  \returns A string containing the created header.
    #    
    def format_header(self, formatted_body, indicators, this_part, num_parts):
        result = ''

        
        header = self._system_indicator + ' = '+ str(this_part) + '/' + str(num_parts) + ' = '
        header = header + str(formatted_body.num_groups) + ' = '
        
        indicator_groups = ''
        
        for i in self._key_words:        
            indicator_groups += indicators[i] + ' '
        
        indicator_groups = indicator_groups.strip()
        indicator_groups = indicator_groups.upper()
        
        result = header + indicator_groups + ' ='
        
        return result

    ## \brief This method retrieves the indicator groups specified in self._key_words from the header of a message
    #
    #  \param [indicators] A dictionary that maps strings to strings. It has to contain the indicator groups that have
    #         already been retreived from the message body. This method then adds the indicator groups found in the
    #         header.
    #  \param [header] A string. It has to contain the header of the current message part.
    #
    #  \returns A dictionary that maps strings to strings. The returned dictionary is the dictionary given in parameter
    #           indicators to which the indicator groups named in self._key_words are added.
    #                    
    def parse_ciphertext_header(self, indicators, header):
        result = indicators
        exp = '^[A-Z0-9]+ = ([0-9])+/([0-9])+ = ([0-9])+ = '
        
        for i in self._key_words:
            exp += '([A-Z]{{{0}}}) '.format(self._header_group_size)
        
        exp = exp.strip()
        exp += ' =$'
        
        header_exp = re.compile(exp)
        
        match = header_exp.search(header)
        if match != None:
            exp_group_index = 4
            for i in self._key_words:
                result[i] = match.group(exp_group_index).lower()
                exp_group_index += 1
        else:        
            raise EnigmaException('Header has wrong format')
            
        return result
        

## \brief This class knows how to format and parse message bodies and headers during en- and decryptions done with three
#         rotor Enigma machines using the rules in force during WWII in the german Army and Air Force.
#
class EnigmaFormatter(Formatter):
    ## \brief Constructor.
    #
    def __init__(self, header_group_size = 3):
        super().__init__()
        self._header_group_size = header_group_size
        self._header_exp = ENIGMA_HEADER_EXP.format(self._header_group_size)

    ## \brief This method formats the body of an Enigma message.
    #
    #  \param [ciphertext] A string specifying the unformatted ciphertext.
    #  \param [indicators] A dictionary that maps strings to strings. It has to contain the indicator groups generated
    #         for this message part. In particular there has to be a key 'kenngruppe' which maps to the kenngruppe that is
    #         placed as the first group of the formatted ciphertext.
    #
    #  \returns A BodyStruct object.
    #
    def format_body(self, ciphertext, indicators):
        result = BodyStruct()
        ciphertext = indicators['kenngruppe'] + ciphertext
        result.num_chars = len(ciphertext)
        result.num_groups = len(ciphertext) // self._group_size
        
        if (len(ciphertext) % self._group_size) != 0:
            result.num_groups += 1
        
        result.text = rotorsim.RotorMachine.group_text(ciphertext, True, self._group_size, self._groups_per_line)        
        
        return result

    ## \brief This method parses the body of an Enigma message. I.e. it retrieves the kenngruppe from the formatted
    #         ciphertext.
    #
    #  \param [body] A string specifying the formatted the ciphertext body of a message part.
    #
    #  \returns A ParsedBodyStruct object.
    #
    def parse_ciphertext_body(self, body):
        result = ParsedBodyStruct()
        
        if len(body) < 5:
            raise EnigmaException('Ciphertext has to contain at least one group')
        
        result.indicators['kenngruppe'] = body[:5].lower()
        result.text = body[5:].lower()
        
        return result

    ## \brief This method creates a header for an Enigma based message.
    #
    #  \param [formatted_body] A BodyStruct object specifying the already formatted ciphertext body of a message part.
    #  \param [indicators] A dictionary that maps strings to strings. It has to contain at least the indicator groups 
    #         referenced by the keys HEADER_GRP_1 and HEADER_GRP_2.
    #  \param [this_part] An integer. It specifies the sequence number of the message part for which this method
    #         is called.
    #  \param [num_parts] An integer. It has to specify the overall number of message parts of in the current encryption
    #         operation.
    #
    #  \returns A string containing the created header.
    #    
    def format_header(self, formatted_body, indicators, this_part, num_parts):
        result = ''
        now = datetime.datetime.now()
        
        teile_text = 'tle'
        if num_parts <= 1:
            teile_text = 'tl' 
        
        header = now.strftime('%H%M') + ' = ' + str(num_parts) + teile_text + ' = ' + str(this_part) + 'tl' + ' = '
        header = header + str(formatted_body.num_chars) + ' = '
        result = header + (indicators[HEADER_GRP_1] + ' ' + indicators[HEADER_GRP_2] + ' =').upper()
        
        return result

    ## \brief This method retrieves the indicator groups HEADER_GRP_1 and HEADER_GRP_2 from the header of an Enigma
    #         message.    
    #
    #  \param [indicators] A dictionary that maps strings to strings. It has to contain the indicator groups that have
    #         already been retreived from the message body. This method then adds the indicator groups found in the
    #         header.
    #  \param [header] A string. It has to contain the header of the current message part.
    #
    #  \returns A dictionary that maps strings to strings. The returned dictionary is the dictionary given in parameter
    #           indicators to which the indicator groups HEADER_GRP_1 and HEADER_GRP_2 are added.
    #                    
    def parse_ciphertext_header(self, indicators, header):
        result = indicators
        header_exp = re.compile(self._header_exp)
        
        match = header_exp.search(header)
        if match != None:
            result[HEADER_GRP_1] = match.group(2).lower()
            result[HEADER_GRP_2] = match.group(3).lower()
        else:        
            raise EnigmaException('Header has wrong format')
            
        return result


class SIGABAFormatter(Formatter):
    ## \brief Constructor.
    #
    def __init__(self):
        super().__init__()
        self._external_indicator = 'AAAAA'
        self._months = ['JAN', 'FEB', 'MAR', 'APR', 'MAY', 'JUN', 'JUL', 'AUG', 'SEP', 'OCT', 'NOV', 'DEC']

    ## \brief This property returns the external indicator which identifies the key or crpyto net to which the message belongs.
    #
    #  \returns A string.
    #
    @property
    def external_indicator(self):
        return self._external_indicator

    ## \brief This property setter allows to change the external indicator.
    #
    #  \param [new_external_indicator] A string. The new external indicator to use.
    #
    #  \returns Nothing
    #        
    @external_indicator.setter
    def external_indicator(self, new_external_indicator):
        self._external_indicator = new_external_indicator        

    ## \brief This method formats the body of an SIGABA message.
    #
    #  \param [ciphertext] A string specifying the unformatted ciphertext.
    #  \param [indicators] A dictionary that maps strings to strings. It has to contain the indicator groups generated
    #         for this message part. In particular there has to be a key INTERNAL_INDICATOR which is used to derive the
    #         message key.
    #
    #  \returns A BodyStruct object.
    #
    def format_body(self, ciphertext, indicators):
        result = BodyStruct()        
        
        result.num_chars = len(ciphertext)
        
        if (len(ciphertext) % self._group_size) != 0:
            ciphertext = ciphertext + ('x' * self._group_size)[:self._group_size - (len(ciphertext) % self._group_size)]
        
        ciphertext = self.external_indicator + indicators[INTERNAL_INDICATOR] + ciphertext + indicators[INTERNAL_INDICATOR] + self.external_indicator                
        result.num_groups = len(ciphertext) // self._group_size
                
        result.text = rotorsim.RotorMachine.group_text(ciphertext, True, self._group_size, self._groups_per_line)        
        
        return result

    ## \brief This method parses the body of a SIGABA message. I.e. it retrieves the internal and external indicator from
    #         the ciphertext.
    #
    #  \param [body] A string specifying the formatted the ciphertext body of a message part.
    #
    #  \returns A ParsedBodyStruct object.
    #
    def parse_ciphertext_body(self, body):
        result = ParsedBodyStruct()
        
        body = body.replace(' ', '')
        body = body.replace('\n', '')
        
        if len(body) < 20:
            raise EnigmaException('Ciphertext has to contain at least four groups')
        
        ext_front = body[:5].lower()
        int_front = body[5:10].lower()  
        
        ext_back = body[-5:].lower()
        int_back = body[-10:-5].lower()        
        
        if (ext_front != ext_back) or (int_front != int_back):
            raise EnigmaException('Indicator groups inconsistent')
        
        result.indicators[INTERNAL_INDICATOR] = int_front
        result.indicators[EXTERNAL_INDICATOR] = ext_front        
        
        result.text = body[10:-10].lower()
        
        return result

    ## \brief This method creates a header for an Enigma based message.
    #
    #  \param [formatted_body] A BodyStruct object specifying the already formatted ciphertext body of a message part.
    #  \param [indicators] A dictionary that maps strings to strings. It has to contain at least the indicator group 
    #         referenced by the key INTERNAL_INDICATOR.
    #  \param [this_part] An integer. It specifies the sequence number of the message part for which this method
    #         is called.
    #  \param [num_parts] An integer. It has to specify the overall number of message parts of in the current encryption
    #         operation.
    #
    #  \returns A string containing the created header.
    #    
    def format_header(self, formatted_body, indicators, this_part, num_parts):
        result = ''
        now = datetime.datetime.utcnow()

        # Generated header: 211809Z MAY 2017 - 2 OF 5 - 280        
                
        header = now.strftime('%d%H%M') + 'Z ' + self._months[now.month - 1] + now.strftime(' %Y') + ' - ' + str(this_part) + ' OF ' + str(num_parts) + ' - '
        header = header + str(formatted_body.num_chars) 
        result = header.upper()
        
        return result

    ## \brief This method parses the message header
    #
    #  \param [indicators] A dictionary that maps strings to strings. It has to contain the indicator groups that have
    #         already been retreived from the message body.
    #  \param [header] A string. It has to contain the header of the current message part.
    #
    #  \returns A dictionary that maps strings to strings. The returned dictionary is the dictionary given in parameter
    #           indicators to which the key MESSAGE_LENGTH has been added.
    #                    
    def parse_ciphertext_header(self, indicators, header):
        result = indicators
        exp = '^[0-9]{6}Z [A-Z]{3} [0-9]{4} - [0-9]+ OF [0-9]+ - ([0-9]+)'
        header_exp = re.compile(exp)
        
        match = header_exp.search(header)
        if match == None:
            raise EnigmaException('Header has wrong format')
        else:
            result[MESSAGE_LENGTH] = int(match.group(1))
            
        return result
    

# ----------------------------------------------------------------------------------------------------    

## \brief This class is a simple struct used by the methods of MessageProcedure.
#
class MsgPartStruct:
    def __init__(self):
        ## \brief A string containing the body of a message part.
        self.body = ''
        ## \brief A string containing the header of the message part.
        self.header = ''

## \brief This class controls doing en-/decryptions with a rotor machine. Longer messages are divided into parts. The
#         maximum number of characters per part can be set using the msg_size property. In order to implement its
#         functionality objects of this class use a TransportEncoder, a Formatter and an IndicatorProcessor object.
#
# In short the encoder knows how to transform the original plaintext, maybe containing characters which
# are not in the input alphabet of the rotor machine, into a string that only contains characters the rotor machine
# can understand. It also does the reverse, i.e. transforming the decrypted ciphertext into its original form with
# special characters. 
#
# The formatter knows how to construct and parse a message part from the already encrypted ciphertext and the indicator
# groups as created by the indicator_proc. It also retrieves the indicator groups from the message body and header when
# doing decryptions. While the formatter knows where to find/to put the indicator groups in the ciphertext it does
# not know what they mean or how they are used.
#
# This is only known to the indicator_proc. It does not know where these groups come from but it knows how to create
# and process them in order to create/retrieve the message key.
#
class MessageProcedure:
    ## \brief Constructor
    #
    #  \param [machine] An object that has the same interface as pymsk2.rotorsim.RotorMachine.
    #  \param [rand_gen] An object that has the same interface as pyrmsk2.rotorrandom.RotorRandom.
    #  \param [server] An object that has the same interface as pyrmsk2.tlvobject.TlvServer.
    #    
    def __init__(self, machine, rand_gen, server, step_before_proc = False):
        ## \brief Maximum number of plaintext characters in a message part.
        self._max_msg_size = 245
        ## \brief An object with the same interface as TransportEncoder.
        self._encoder = None
        ## \brief An object with the same interface as Formatter.
        self._formatter = None
        ## \brief An object with the same interface as IndicatorProcessor.
        self._indicator_proc = None
        ## \brief Holds the rotor machine object which is to be used.
        self._machine = machine
        ## \brief Holds the rotor random object which is to be used.
        self._rand_gen = rand_gen
        ## \brief Holds the tlv server object which is to be used.
        self._server = server
        ## \brief If True then the underlying machine is stepped once before an en- or decryption
        self._step_before_proc = step_before_proc

    ## \brief This property returns the maximum number of plaintext characters allowed in a message part.
    #
    #  \returns An integer.
    #
    @property
    def msg_size(self):
        return self._max_msg_size    

    ## \brief This property setter allows to change the maximum number of plaintext characters per message part.
    #
    #  \param [new_msg_size] An integer that specifies the new value for the property.
    #
    #  \returns Nothing
    #            
    @msg_size.setter
    def msg_size(self, new_msg_size):
        self._max_msg_size = new_msg_size

    ## \brief This property returns the TransportEncoder object in use in this MessageProcedure instance.
    #
    #  \returns An object with the same interface as TransportEncoder.
    #    
    @property
    def encoder(self):
        return self._encoder

    ## \brief This property setter allows to change the TransportEncoder in use with this MessageProcedure instance.
    #
    #  \param [new_encoder] An object with the same interface as TransportEncoder.
    #
    #  \returns Nothing
    #    
    @encoder.setter
    def encoder(self, new_encoder):
        self._encoder = new_encoder

    ## \brief This property returns the Formatter object in use in this MessageProcedure instance.
    #
    #  \returns An object with the same interface as Formatter.
    #    
    @property
    def formatter(self):
        return self._formatter

    ## \brief This property setter allows to change the Formatter in use with this MessageProcedure instance.
    #
    #  \param [new_formatter] An object with the same interface as Formatter.
    #
    #  \returns Nothing
    #        
    @formatter.setter
    def formatter(self, new_formatter):
        self._formatter = new_formatter

    ## \brief This property returns the IndicatorProcessor object in use in this MessageProcedure instance.
    #
    #  \returns An object with the same interface as IndicatorProcessor.
    #    
    @property
    def indicator_proc(self):
        return self._indicator_proc

    ## \brief This property setter allows to change the IndicatorProcessor in use with this MessageProcedure instance.
    #
    #  \param [new_indicator_proc] An object with the same interface as IndicatorProcessor.
    #
    #  \returns Nothing
    #            
    @indicator_proc.setter
    def indicator_proc(self, new_indicator_proc):
        self._indicator_proc = new_indicator_proc

    ## \brief This method encrypts a plaintext given in the parameter plaintext. If necessary the message is split
    #         into several parts which are encrypted seperateley.
    #
    #  \param [plaintext] A string. Has to contain the message to encrypt.
    #
    #  \returns A sequence of strings. Each sequence element is an encrypted message part.
    #                            
    def encrypt(self, plaintext):
        result = []
        self.indicator_proc.reset()
        self.formatter.reset()
        
        raw_plaintext = self.encoder.transform_plaintext_enc(plaintext)

        # Calculate number of parts
        num_parts = len(raw_plaintext) // self.msg_size
        if (len(raw_plaintext) % self.msg_size) != 0:
            num_parts += 1
        
        raw_text = raw_plaintext
        self._machine.go_to_letter_state()        
        
        # Encrypt the individual parts
        for i in range(num_parts):
            this_part = raw_text[:self.msg_size]
            raw_text = raw_text[self.msg_size:]            
            result.append(self.encrypt_part(this_part, i + 1, num_parts))        
        
        return result

    ## \brief This method encrypts a message part and formats it in the way determined by the formatter.
    #
    #  \param [part_plain_text] A string specifying the plaintext of the message part.
    #  \param [this_part] An integer. It specifies the sequence number of the message part for which this method
    #         is called.
    #  \param [num_parts] An integer. It has to specify the overall number of message parts of in the current encryption
    #         operation.
    #
    #  \returns A string containing the encrypted and formatted ciphertext.
    #        
    def encrypt_part(self, part_plain_text, this_part, num_parts):                
        # Encrypt message
        indicator_inputs = self.indicator_proc.create_indicators(self._machine, this_part, num_parts)
        self._machine.set_rotor_positions(indicator_inputs[MESSAGE_KEY])
        
        if self._step_before_proc:
            self._machine.step()
        
        part_ciphertext = self._machine.encrypt(part_plain_text)
        
        body = self.formatter.format_body(part_ciphertext, indicator_inputs)
        header = self.formatter.format_header(body, indicator_inputs, this_part, num_parts)        
        
        # Create fully formatted ciphertext
        result = header + '\n\n' + body.text
        
        return result

    ## \brief This method splits a ciphertext consisting of several message parts into the message parts.
    #
    #  \param [ciphertext] A string. It has to contain the combined formatted ciphertext message parts.
    #
    #  \returns A sequence of MsgPartStruct objects.
    #                                
    def parse_message_part(self, ciphertext):
        parts = []
        look_for_header = True
        last_line_empty = True
        current_part = MsgPartStruct() 
        lines = ciphertext.split('\n')
        
        # Parse input text into message parts
        for i in lines:
            if len(i.strip()) != 0:
                last_line_empty = False
                
                if look_for_header:
                    current_part.header += i.strip()
                else:
                    current_part.body += i.strip()
            else:
                if not last_line_empty:                
                    if not look_for_header:
                        # part is finished
                        parts.append(current_part)
                        current_part = MsgPartStruct()
                                            
                    look_for_header = not look_for_header
                    
                last_line_empty = True        
        
        # Add last part, if we were looking for lines in body when input was exhausted
        if not look_for_header:
            parts.append(current_part)
        
        return parts
              

    ## \brief This method decrypts a ciphertext specified in parameter ciphertext. If the encrypted message
    #         has several parts then these parts have to be appended to each other before calling this method.
    #
    #  It is expected by this method that the string ciphertext has the following structure: Header and body
    #  of each message part are seperated by at least one empty line. All message parts (apart fom the last)
    #  are also to be followed by at least one empty line.
    #
    #  \param [ciphertext] A string. Has to contain the combined formatted ciphertext message parts.
    #
    #  \returns A string. Holds the plaintext of the message.
    #                                
    def decrypt(self, ciphertext):
        result = ""        
        
        self.indicator_proc.reset()
        self.formatter.reset()
        
        parts = self.parse_message_part(ciphertext)
        
        # Input text is now parsed into a sequence of dictionaries with the keys 'body' and 'header'
        # each of which represents a message part.
        
        self._machine.go_to_letter_state()
        
        # Process individual parts
        for i in parts:
            result += self.decrypt_part(i) # decrypt           
        
        result = self.encoder.transform_plaintext_dec(result)
        
        return result

    ## \brief This method decrypts a message part the ciphertext of which is specified in parameter cipher_text_part.
    #
    #  \param [cipher_text_part] A MsgPartStruct object containing the header and body of the message part to decrypt,
    #
    #  \returns A string. Holds the plaintext of this message part.
    #        
    def decrypt_part(self, cipher_text_part):
        help = self.formatter.parse_ciphertext_body(cipher_text_part.body) # Determine ciphertext and potentially indicator information contained in the body
        ciphertext = help.text
        indicators = help.indicators
        indicators = self.formatter.parse_ciphertext_header(indicators, cipher_text_part.header) # Determine rest of indicators from header           
        indicators = self.indicator_proc.derive_message_key(self._machine, indicators) # Derive message key from indicators   
        self._machine.set_rotor_positions(indicators[MESSAGE_KEY]) # Set message key
        
        #print(self._machine.get_rotor_positions())
        
        if self._step_before_proc:
            self._machine.step()
            
        #print(self._machine.get_rotor_positions())            
        
        if MESSAGE_LENGTH in indicators.keys():
            ciphertext = help.text[:indicators[MESSAGE_LENGTH]]

        return self._machine.decrypt(ciphertext) # decrypt


class ShiftedIndicatorTransformer:
    def __init__(self, letter_alpha, figure_alpha, space_char, shift_char, unshift_char, shift_figure_char, unshift_figure_char):
        self._letter_alpha = letter_alpha
        self._figure_alpha = figure_alpha
        self._space_char = space_char
        self._shift_char = shift_char
        self._unshift_char = unshift_char
        self._shift_figure_char = shift_figure_char
        self._unshift_figure_char = unshift_figure_char        
        self._inverse_std = {}
        
        count = 0
        for i in 'abcdefghijklmnopqrstuvwxyz':
            self._inverse_std[i] = count
            count += 1   
            
    def transform(self, indicator_candidate):
        result = ''
        current_alpha = self._letter_alpha
        
        for i in indicator_candidate:
            if i == self._space_char:
                result += ' '
            else:
                if current_alpha == self._letter_alpha:            
                    if i == self._shift_char:
                        current_alpha = self._figure_alpha
                        result += '>'
                    elif i == self._unshift_char:
                        current_alpha = self._letter_alpha
                        result += '<'
                    else:
                        result += current_alpha[self._inverse_std[i]]
                else:            
                    if i == self._shift_figure_char:
                        current_alpha = self._figure_alpha
                        result += '>'
                    elif i == self._unshift_figure_char:
                        current_alpha = self._letter_alpha
                        result += '<'
                    else:
                        result += current_alpha[self._inverse_std[i]]
        
        return result


class TypexIndicatorTransformer(ShiftedIndicatorTransformer):
    def __init__(self):
        super().__init__("abcdefghijklmnopqrstu<w y>", "-'vz3%x£8*().,9014/57<2 6>", 'x', 'z', 'v', 'z', 'v')


class KL7IndicatorTransformer(ShiftedIndicatorTransformer):
    def __init__(self):
        super().__init__("abcdefghi>klmnopqrstuvwxy ", "abcd3fgh8>klmn9014s57<2x6 ", 'z', 'j', NO_SHIFT_CHAR, 'j', 'v')


class SG39IndicatorHelper:
    def __init__(self):
        pass
    
    def verify(self, indicator_candidate):
        return self.test(indicator_candidate).verified

    def transform(self, indicator_candidate):
        return self.test(indicator_candidate).transformed
            
    def test(self, indicator_candidate):
        result = MsgKeyTestResult(False, indicator_candidate[:4])
        wheel_sizes = ['y', 'w', 'u']        
        values_found = []
                       
        wheel_part = indicator_candidate[4:]
        read_pos = 0
        
        for i in wheel_sizes:
            current_value_found = False

            while (not current_value_found) and (read_pos < 6):               
                if wheel_part[read_pos] <= i:
                    current_value_found = True
                    result.transformed += wheel_part[read_pos]
                
                read_pos += 1

            values_found.append(current_value_found)                
        
        result.verified = functools.reduce(lambda x,y: x and y, values_found)        
        
        return result


class MessageProcedureFactory:
    def __init__(self, machine, rand_gen, server):
        ## \brief Holds the rotor machine object which is to be used.
        self._machine = machine
        ## \brief Holds the rotor random object which is to be used.
        self._rand_gen = rand_gen
        ## \brief Holds the tlv server object which is to be used.
        self._server = server        

    def _get_incomplete_enigma(self, num_rotors = 3):
        result = MessageProcedure(self._machine, self._rand_gen, self._server)
        result.formatter = EnigmaFormatter(num_rotors)
        result.formatter.limits = (5, 10)
        result.msg_size = 245
        result.encoder = ArmyEncoder()
        
        return result
    
    @staticmethod
    def get_and_test_kenngruppen(system_indicator):
        kenngruppen_raw = system_indicator.split()
        kenngruppen = list(filter(lambda x: len(x) == 3, kenngruppen_raw))

        if len(kenngruppen) == 0:
            raise EnigmaException('No usable Kenngruppen specified!')
        
        return kenngruppen                        
    
    def get_post1940_enigma(self, system_indicator, grundstellung, num_rotors = 3):    
        result = self._get_incomplete_enigma(num_rotors)
        result.indicator_proc = Post1940EnigmaIndicatorProc(self._server, self._rand_gen, self.get_and_test_kenngruppen(system_indicator), num_rotors)
        
        return result

    def get_pre1940_enigma(self, system_indicator, grundstellung, num_rotors = 3):    
        result = self._get_incomplete_enigma(num_rotors)
        result.indicator_proc = Pre1940EnigmaIndicatorProc(self._server, self._rand_gen, self.get_and_test_kenngruppen(system_indicator), grundstellung, num_rotors)
        
        return result

    def get_post1940_4wheel_enigma(self, system_indicator, grundstellung):
        result = self.get_post1940_enigma(system_indicator, grundstellung, 4)
        
        return result           

    def get_pre1940_4wheel_enigma(self, system_indicator, grundstellung):
        result = self.get_pre1940_enigma(system_indicator, grundstellung, 4)
        
        return result           

    def get_generic_machine(self, system_indicator, grundstellung, indicator_group_size, step_before_use = False):
        result = MessageProcedure(self._machine, self._rand_gen, self._server, step_before_use)
        result.indicator_proc = GrundstellungIndicatorProc(self._server, self._rand_gen, indicator_group_size, step_before_use)
        result.indicator_proc.grundstellung = grundstellung
        result.formatter = GenericFormatter(1, indicator_group_size, result.indicator_proc.key_words)
        result.formatter.system_indicator = system_indicator
        result.formatter.limits = (5, 10)
        result.msg_size = 500
        result.encoder = ArmyEncoder()
        
        return result
        
    def get_generic_enigma(self, system_indicator, grundstellung):
        result = self.get_generic_machine(system_indicator, grundstellung, 3)
        result.msg_size = 250
        
        return result        

    def get_generic_m4(self, system_indicator, grundstellung):
        result = self.get_generic_machine(system_indicator, grundstellung, 4)
        result.msg_size = 248
        result.formatter.limits = (4, 8)
        
        return result

    def get_generic_4wheel_enigma(self, system_indicator, grundstellung):
        result = self.get_generic_machine(system_indicator, grundstellung, 4)
        result.msg_size = 250
        
        return result

    def get_generic_nema(self, system_indicator, grundstellung):
        result = self.get_generic_machine(system_indicator, grundstellung, 10)
        result.msg_size = 350
        result.formatter.limits = (5, 10)
        
        return result

    def get_generic_typex(self, system_indicator, grundstellung):
        result = self.get_generic_machine(system_indicator, grundstellung, 5)
        typex_trans = TypexIndicatorTransformer()
        result.indicator_proc.transformer = typex_trans.transform
        result.encoder = TypexEncoder()
        
        return result

    def get_generic_sg39(self, system_indicator, grundstellung):
        result = self.get_generic_machine(system_indicator, grundstellung, 10)
        result.msg_size = 250
        sg39_ind_helper = SG39IndicatorHelper()
        result.indicator_proc.msg_key_tester = sg39_ind_helper.test
        
        return result

    def get_generic_kl7(self, system_indicator, grundstellung):
        result = self.get_generic_machine(system_indicator, grundstellung, 7, True)
        result.msg_size = 500
        result.formatter.limits = (5, 10)
        kl7_trans = KL7IndicatorTransformer()
        result.indicator_proc.transformer = kl7_trans.transform
        
        result.encoder = KL7Encoder()
        
        return result
    
    def get_sigaba_basic(self, system_indicator, grundstellung):
        result = MessageProcedure(self._machine, self._rand_gen, self._server)
        result.indicator_proc = SIGABABasicIndicatorProcessor(self._server, self._rand_gen)
        result.formatter = SIGABAFormatter()
        result.formatter.external_indicator = system_indicator
        result.formatter.limits = (5, 10)
        result.msg_size = 1730
        result.encoder = SIGABAEncoder()
        
        return result    

    def get_sigaba_grundstellung(self, system_indicator, grundstellung):
        result = self.get_sigaba_basic(system_indicator, grundstellung)
        result.indicator_proc = SIGABAGrundstellungIndicatorProcessor(self._server, self._rand_gen)
        result.indicator_proc.grundstellung = grundstellung
        
        return result

# ----------------------------------------------------------------------------------------------------


## \brief This class implemets a command line application that allows to en- and decrypt a message following
#         the message procedure defined by the german army and air force from 1940 on.
#
class EngimaProc(tlvsrvapp.TlvServerApp):
    ## \brief Constructor.
    #
    def __init__(self):
        super().__init__()

    ## \brief This method decrypts a single part of a whole ciphertext message.
    #
    #  \param [argv] A vector of strings representing the command line parameters, i.e. sys.argv.
    #
    #  \returns A dictionary containing the keys 'in_file', 'out_file', 'config_file', 'sys-indicator' and 'doencrypt'.
    #        
    def parse_args(self, argv):
        # Set up command line parser        
        indicator_help = "System indicator to use. In case the system indicator is a Kenngruppe it has to contain several (four) three letter strings seperated by blanks."        
        parser = argparse.ArgumentParser(description='A program that allows to en- and decrypt messages according to the WWII Enigma message procedure.',
                                         epilog='Example: enigproc.py encrypt -f state.ini -i input.txt -s "dff gtr lki vfd"')
        parser.add_argument("command", choices=COMMANDS, help="Action to take. Encrypt or decrypt.")
        parser.add_argument("-i", "--in-file", required=True, help="Input file containing plaintext.")
        parser.add_argument("-o", "--out-file", default='-', help="Store output in file named by this parameter. Print to stdout if not specified.")
        parser.add_argument("-f", "--config-file", required=True, help="Machine state (as created for instance by rotorstate) to use.")
        parser.add_argument("-s", "--sys-indicator", default='', help=indicator_help)
        parser.add_argument("-g", "--grundstellung", default='', help="A basic setting or grundstellung if required by the messaging procedure")
        parser.add_argument("-t", "--type", required=True, choices=PROC_TYPES, help="Type of messaging procedure")        
        
        # Calls sys.exit() when command line can not be parsed or when --help is requested
        args = parser.parse_args()
        result =  {'in_file': args.in_file, 'out_file': args.out_file, 'config_file': args.config_file, 'sys-indicator':args.sys_indicator, 'doencrypt':args.command != COMMANDS[1]}
        result['grundstellung'] = args.grundstellung.lower()
        result['type'] = args.type
                        
        return result

    ## \brief This method writes the message parts given to a file like object.
    #
    #  \param [formatted_parts] A vector of strings. Each element represents an en- or decrypted message part.
    #  \param [out_file] A file like object having a write() method.
    #
    #  \returns Nothing.
    #            
    def _output_formatted_message(self, formatted_parts, out_file):
        for i in formatted_parts:
            out_file.write(i)
            out_file.write('\n\n')        

    def _generate_msg_proc_obj(self, machine_name, sys_indicator, grundstellung, proc_type):
        factory = MessageProcedureFactory(self.machine, self.random, self.server)
        
        if proc_type == 'post1940':
            if machine_name in ['Enigma', 'M3', 'KDEnigma']:
                return factory.get_post1940_enigma(sys_indicator, grundstellung)
            elif machine_name in ['AbwehrEnigma', 'TirpitzEnigma', 'M4Enigma', 'RailwayEnigma']:
                return factory.get_post1940_4wheel_enigma(sys_indicator, grundstellung)
            else:
                raise EnigmaException('Unsupported message procedure for machine type')
        elif proc_type == 'pre1940':
            if machine_name in ['Enigma', 'M3', 'KDEnigma']:
                return factory.get_pre1940_enigma(sys_indicator, grundstellung)
            elif machine_name in ['AbwehrEnigma', 'TirpitzEnigma', 'M4Enigma', 'RailwayEnigma']:
                return factory.get_pre1940_4wheel_enigma(sys_indicator, grundstellung)
            else:
                raise EnigmaException('Unsupported message procedure for machine type')
        elif proc_type == 'sigaba':
            if machine_name in ['CSP889', 'CSP2900']:
                return factory.get_sigaba_basic(sys_indicator, grundstellung)
            else:
                raise EnigmaException('Unsupported message procedure for machine type')
        elif proc_type == 'grundstellung':
            if machine_name in ['Enigma', 'M3', 'KDEnigma']:
                return factory.get_generic_enigma(sys_indicator, grundstellung)
            elif machine_name in ['AbwehrEnigma', 'TirpitzEnigma', 'RailwayEnigma']:
                return factory.get_generic_4wheel_enigma(sys_indicator, grundstellung)
            elif machine_name == 'M4Enigma':
                return factory.get_generic_m4(sys_indicator, grundstellung)
            elif machine_name == 'Typex':
                return factory.get_generic_typex(sys_indicator, grundstellung)
            elif machine_name in ['CSP889', 'CSP2900']:
                return factory.get_sigaba_grundstellung(sys_indicator, grundstellung)
            elif machine_name == 'KL7':
                return factory.get_generic_kl7(sys_indicator, grundstellung)
            elif machine_name == 'Nema':
                return factory.get_generic_nema(sys_indicator, grundstellung)
            elif machine_name == 'SG39':
                return factory.get_generic_sg39(sys_indicator, grundstellung)
            else:
                raise EnigmaException('Unsupported message procedure for machine type')
        else:        
            raise EnigmaException('Type of message procedure unknown')

    ## \brief This method verifies the parameters as specified on the command line and controls en-/decryption.
    #
    #  \param [args] A dictionary as returned by parse_args(). 
    #
    #  \returns An integer. It is used as the result code of this program.
    #                
    def main_func(self, args):
        result = tlvsrvapp.ERR_OK        
        text = ''
        out_text = ''
        do_encrypt = args['doencrypt']        

        # Load machine state
        self.machine.load_machine_state(args['config_file'])
        
        #print(self.machine.get_description())
        
        # Load input text
        with open(args['in_file'], 'r') as f_in:
            text = f_in.read()                
        
        if do_encrypt:
            # Perform encryption
            if args['sys-indicator'] == '':
                raise EnigmaException('A system indicator has to be provided via the -s/--sys-indicator option')
                
            enigma_proc = self._generate_msg_proc_obj(self.machine.get_description(), args['sys-indicator'], args['grundstellung'], args['type'])                                
            out_text_parts = enigma_proc.encrypt(text)
        else:
            # Perform decryption
            enigma_proc = self._generate_msg_proc_obj(self.machine.get_description(), DUMMY_SYS_INDICATOR, args['grundstellung'], args['type'])
            out_text_parts = [enigma_proc.decrypt(text)]        
        
        # Save output data
        if args['out_file'] == '-':
            # Write to stdout
            self._output_formatted_message(out_text_parts, sys.stdout)
        else:
            # Write to file
            with open(args['out_file'], 'w') as out_file:
                self._output_formatted_message(out_text_parts, out_file)        
                
        return result        
        
        
if __name__ == "__main__":
    app = EngimaProc()    
    app.main(sys.argv)
