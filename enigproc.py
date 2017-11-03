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
## @package enigproc Implements a command line application that allows to en- and decrypt a message following one of several implemented message procedures.
#         
#           
# \file enigproc.py
# \brief Implements a command line application that allows to en- and decrypt a message following one of several implemented message procedures. 
#        

import sys
import pyrmsk2.tlvsrvapp as tlvsrvapp
import pyrmsk2.rotorsim as rotorsim
import pyrmsk2.rotorrandom as rotorrandom
import datetime
import argparse
import re
import functools
from pyrmsk2.keysheetgen import PROC_TYPES

## \brief Maximum number of real plaintext characters in a message part. 
COMMANDS = ['encrypt', 'decrypt']
## \brief Matches stuff like 1534 = 15tle = 15tl = 167 = RJF GNZ =
ENIGMA_HEADER_EXP = '^[0-9]{{4}} = [0-9]+(tl|tle) = [0-9]+tl = [0-9]+ = ([A-Z]{{{0}}}) ([A-Z]{{{0}}}) =$'
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
## \brief Dictionary key that names the number of ciphertext characters when using the default SIGABA message procedure 
MESSAGE_LENGTH = 'message_length'
## \brief Dummy value for the system indicator used when instantiating a MessageProcedure object for decryption.
DUMMY_SYS_INDICATOR = 'ert zui ops cfg'

GRUND_DEFAULT = ''

## \brief Constructing objects of the ShiftedIndicatorTransformer class or its children require to specify the characters that cause
#         shifting of the input alphabets. Use this constant to signify that there is no shifting character.
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
#  Children of this class are intended to be used to replace special characters in the input data that can not be
#  directly processed by the underlying rotor machine by characters or groups of characters that can be fed into the
#  machine and vice versa. The transformed plaintext is called the encoded plaintext.
#
class TransportEncoder:
    ## \brief Constructor
    #
    def __init__(self):
        ## \brief A vector of chars that is used to filter the input data when doing encryptions.
        self._allowed_plain_chars = 'abcdefghijklmnopqrstuvwxyz'
    
    ## \brief This method transforms a plaintext into an encoded form before that encoded form ist encrypted.
    #
    #  \param [plaintext] A string. Contains the plaintext to transform.
    #
    #  \returns A string. The encoded plaintext
    #
    def transform_plaintext_enc(self, plaintext):
        return ''.join(list(filter(lambda x: x in self._allowed_plain_chars, plaintext.lower())))

    ## \brief This method transforms a decryped (and encoded) plaintext into its original form.
    #
    #  \param [plaintext] A string. Contains the encoded plaintext to transform
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
    #  \param [full_plain] A string. Output text as created by the cipher machine.
    #
    #  \returns A string. It contains the transformed plaintext.
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


## \brief This class implements the transport encoder used by the SIGABA. When doing encryptions the SIGABA implementation of
#         rmsk2 does not allow the character Z as an input character because Z is used to represent the blank character. Among
#         other things this class transforms any Z charatcters in the plaintext into the character X as would be done by a
#         real SIGABA.
#
class SIGABAEncoder(TransportEncoder):
    ## \brief Constructor
    #
    def __init__(self):
        super().__init__()
        ## \brief Allowed input characters. Note the absence of Z and the presence of ' ' as the last character.
        self._allowed_plain_chars = 'abcdefghijklmnopqrstuvwxy '
        
    ## \brief This method filters out characters which are not allowed as input and transforms the plaintext according to
    #         the rules set out in the SIGABA message procedure.
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

    ## \brief This method transforms the raw decrypted plaintext coming out of the SIGABA according to
    #         the rules set out in the SIGABA message procedure back into a more human readable form. In a way this
    #         is the inverse of transform_plaintext_enc().
    #
    #  \param [full_plain] A string. Output text as created by the cipher machine.
    #
    #  \returns A string. It contains the transformed plaintext.
    #                    
    def transform_plaintext_dec(self, full_plain):
        full_plain = full_plain.lower()
        full_plain = full_plain.replace(' ques', '?')
        return full_plain        


## \brief This class implements a transport encoder that knows an unshifted letter and a shifted figure alphabet. Any
#         character i contained only in the figure alphabet is replaced by >i<, i.e. during encryption the machine is put
#         into figures mode then the special character is processed and then the machine is immediately put back into
#         letter mode. This may not be efficient in some special cases.
#
class ShiftingEncoder(TransportEncoder):
    ## \brief Constructor.
    #
    #  \param [letter_alpha] A string. It has to contain the characters allowed in letter mode.
    #
    #  \param [figure_alpha] A string. It has to contain the characters allowed in figures mode.
    #
    #  \returns Nothing.
    #
    def __init__(self, letter_alpha, figure_alpha):
        ## \brief A string. Contains the characters allowed in letter mode.
        self._letter_alpha = letter_alpha
        ## \brief A string. Contains the characters allowed in figures mode.
        self._figure_alpha = figure_alpha

    ## \brief This method replaces any input character i that is only contained in the figures alphabet by >i<.
    #
    #  \param [plaintext] A string. Contains the unencoded plaintext.
    #
    #  \returns A string. The transformed plaintext.
    #    
    def transform_shifted_characters(self, plaintext):
        result = ''

        for i in plaintext:
            if i in self._letter_alpha:
                result += i
            elif i in self._figure_alpha:
                result += '>' + i + '<'
        
        return result                

    ## \brief This method transforms generic special characters (i.e. german umlauts) into characters that can be processed
    #         directly by all rotor machines.    
    #
    #  \param [plaintext] A string. Contains the plaintext to transform.
    #
    #  \returns A string. The encoded plaintext without umlauts.
    #    
    def transform_special_characters(self, plaintext):
        # Exclude the special generic shifting characters < and > from user supplied input text
        plaintext = ''.join(list(filter(lambda x: x not in '<>', plaintext.lower())))
        # Replace umlauts
        plaintext = plaintext.replace('ä', 'ae')
        plaintext = plaintext.replace('ö', 'oe')
        plaintext = plaintext.replace('ü', 'ue')                        
        plaintext = plaintext.replace('ß', 'ss')
        
        return plaintext
        
    
    ## \brief This method transforms a plaintext into an encoded form before that encoded form ist encrypted.
    #
    #  \param [plaintext] A string. Contains the plaintext to transform.
    #
    #  \returns A string. The encoded plaintext.
    #
    def transform_plaintext_enc(self, plaintext):
        # Transform umlauts and filter out generic shfiting characters
        plaintext = self.transform_special_characters(plaintext)        
        # Only allow characters that are in the letter or figures alphabet
        plaintext = ''.join(list(filter(lambda x: (x in self._letter_alpha) or (x in self._figure_alpha), plaintext)))
        
        result = self.transform_shifted_characters(plaintext)
                
        return result


## \brief This class implements a transport encoder that transforms plaintext according to the properties of and procedures
#         defined for the KL7 machine. The KL7 allows blanks in its input alphabet during encryptions. The characters J and
#         V are special in that they switch between letter and figures mode. Z is a replacement for the blank character. Any
#         Z typed during encryption input is encrypted and decrypted as an X. Any typed J is encrypted and decrypted as I.
#
class KL7Encoder(ShiftingEncoder):
    ## \brief Constructor
    #
    def __init__(self):
        super().__init__("abcdefghi>klmnopqrstuvwxy ", "abcd3fgh8>klmn9014s57<2x6 ")

    ## \brief This method transforms a plaintext into an encoded form before that encoded form ist encrypted.
    #
    #  \param [plaintext] A string. Contains the plaintext to transform.
    #
    #  \returns A string. The encoded plaintext
    #
    def transform_plaintext_enc(self, plaintext):
        # Transform umlauts and filter out generic shfiting characters
        plaintext = self.transform_special_characters(plaintext)
        # Transform additional special characters
        plaintext = plaintext.replace('j', 'i')
        plaintext = plaintext.replace('z', 'x')
        # Filter out stuff that is neither in the letter nor the figures alphabet
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
    #  \param [new_grundsellung] A string of length self._num_rotors. The new grundstellung.
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
        
        if result[MESSAGE_KEY] != temp:
            raise EnigmaException("Header groups do not create same message key")
        
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
    #  \param [indicator size] An integer. It has to specify the number of characters in an indicator group.
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
    #  \param [new_grundsellung] A string. The new grundstellung.
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
    #         together with the character and group count of the message part in form of a BodyStruct object.
    #
    #  \param [ciphertext] A string specifying the unformatted ciphertext.
    #
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
    #
    #  \param [indicators] A dictionary that maps strings to strings. It has to contain the indicator groups generated
    #         for this message part, which may be placed in the message header.
    #
    #  \param [this_part] An integer. It specifies the sequence number of the message part for which this method
    #         is called.
    #
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
    #
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
#         form "System indicator = Message number of this part/Number of all parts = Number of groups = header groups separated by space =".
#         The number and size of the header groups can be selected during object construction. 
#
#  Example header: 452TU = 4/5 = 50 = ESFTG JJUZG =
# 
#  Here we have two header groups of size 5.
#  
#  Full example:
#
#  AMESA = 1/1 = 19 = XTEI =
#
#  ZCBOA ZKQIP VGXPQ RMSPR RXBLX AKIBR PZKFZ YACER TURRM PGHVW
#  RUAUS PYMZL QUFPI VBFBE UKTKC CKGOZ SZREQ CNBAA EIBM
#
class GenericFormatter(Formatter):
    ## \brief This method formats the body of a rotor machine message.
    #
    #  \param [num_of_header_groups] An integer specifying how many groups are part of the header.
    #
    #  \param [header_group_size] An integer. It has to contain the size in characters of the groups that are part of the header.
    #
    #  \param [group_key_words] A sequence of strings. The first sequence element is used to reference the first header group, the
    #         second element the second header group and so on.
    #
    #  \returns Nothing.
    #
    def __init__(self, num_of_header_groups, header_group_size, group_key_words):
        super().__init__()
        self._num_header_groups = num_of_header_groups
        self._key_words = group_key_words
        self._header_group_size = header_group_size
        self._system_indicator = 'A0000'

    ## \brief This property returns the system indicator which identifies the key or crypto net to which the message belongs.
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
    #
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

    ## \brief This method parses the body of a rotor machine message. It simply converets the ciphertext to lowercase.
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
    #
    #  \param [indicators] A dictionary that maps strings to strings. It has to contain at least the indicator groups.
    #         referenced by self._key_words.
    #
    #  \param [this_part] An integer. It specifies the sequence number of the message part for which this method
    #         is called.
    #
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
    #
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
#         and four rotor Enigma machines using the rules in force during WWII in the german Army and Air Force.
#
#  Example:
#
#  1932 = 1tl = 1tl = 99 = OBQ HFQ =
#
#  IYDSK RVMGJ NGENJ CZROS MWEPQ JLDOM CFTAE QBLYX SKFHL TYOQE
#  CBTCL BYOQL OJQNG KCQRI WMPKT QVRWH XJIVQ IZGPS FHXCX LJTI
#
class EnigmaFormatter(Formatter):
    ## \brief Constructor.
    #
    #  \param [header_group_size] An integer. Specifies the number of characters in each of the two header groups.
    # 
    #  \returns Nothing.
    #
    def __init__(self, header_group_size = 3):
        super().__init__()
        self._header_group_size = header_group_size
        # Contains a regexp that matches the header
        self._header_exp = ENIGMA_HEADER_EXP.format(self._header_group_size)

    ## \brief This method formats the body of an Enigma message.
    #
    #  \param [ciphertext] A string specifying the unformatted ciphertext.
    #
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
    #
    #  \param [indicators] A dictionary that maps strings to strings. It has to contain at least the indicator groups 
    #         referenced by the keys HEADER_GRP_1 and HEADER_GRP_2.
    #
    #  \param [this_part] An integer. It specifies the sequence number of the message part for which this method
    #         is called.
    #
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
    #
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


## \brief This class knows how to format and parse message bodies and headers during en- and decryptions done with any
#         of the SIGABA variants. The first group in the message gives the system indicator, the second the indicator from
#         which the message key is derived. These groups are repeated in inverted order at the end of the message.
#
#  Example:
#
#  Date/Time                   Number of characters in plaintext
#  
#  311825Z OCT 2017 - 1 OF 1 - 109
#
#  AMESA DWDAU RRUDG EHNHM TIMPO QHCHY SOFXF FNXLW XIMAO SBWMC
#  TUYKM JBILH FLCQQ XLXHF HSEQJ UBKDR DZUXV ZLJDX ZRXBM EBRJK
#  NHWER QETJT LZXFE OJOYX DWDAU AMESA
#
#  The date time group has the following format DDHHMMZ MMM YYYY
#
class SIGABAFormatter(Formatter):
    ## \brief Constructor.
    #
    #  \returns Nothing.
    #
    def __init__(self):
        super().__init__()
        # System indicator
        self._external_indicator = 'AAAAA'
        # Used for date time group
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
    #
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
    #
    #  \param [indicators] A dictionary that maps strings to strings. It has to contain at least the indicator group 
    #         referenced by the key INTERNAL_INDICATOR.
    #
    #  \param [this_part] An integer. It specifies the sequence number of the message part for which this method
    #         is called.
    #
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
    #
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
# In short the transport encoder knows how to transform the original plaintext, maybe containing characters which
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
    #
    #  \param [rand_gen] An object that has the same interface as pyrmsk2.rotorrandom.RotorRandom.
    #
    #  \param [server] An object that has the same interface as pyrmsk2.tlvobject.TlvServer.
    #
    #  \param [step_before_proc] A boolean. Is true when the underlying machine has to be stepped once before
    #         en- or decryption of any message part. Needed for KL7.
    #
    #  \returns Nothing.
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
        ## \brief If True then the underlying machine is stepped once before an en- or decryption.
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
    #
    #  \param [this_part] An integer. It specifies the sequence number of the message part for which this method
    #         is called.
    #
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
        # Is true when the "parser" is looking for the (next) header line
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
        
        self.indicator_proc.reset() # Reset state of indicator processor.
        self.formatter.reset() # Reset state of formatter.
        
        parts = self.parse_message_part(ciphertext) # Parse message into parts
        
        # Input text is now parsed into a sequence of MsgPartStruct objects
        # each of which represents a message part.
        
        self._machine.go_to_letter_state()
        
        # Process individual parts
        for i in parts:
            result += self.decrypt_part(i) # decrypt           
        
        result = self.encoder.transform_plaintext_dec(result)
        
        return result

    ## \brief This method decrypts a message part the ciphertext of which is specified in parameter cipher_text_part.
    #
    #  \param [cipher_text_part] A MsgPartStruct object containing the header and body of the message part to decrypt.
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
        
        # Use message length to strip padding off at the end of the message body
        if MESSAGE_LENGTH in indicators.keys():
            ciphertext = help.text[:indicators[MESSAGE_LENGTH]]

        return self._machine.decrypt(ciphertext) # decrypt


## \brief This class helps to derive indicators intended for the message procedure implemented by GrundstellungIndicatorProc when
#         using machines that accept only a subset of a-z in their unshifted input alphabet. Or put in another way: It helps
#         with deriving indicators for machines which use some of the characters in a-z as stand ins for special characters like
#         space, letter shigt or figure shift.
# 
class SpecialCharIndicatorHelper:
    ## \brief Constructor
    #
    #  \param [chars_to_avoid] A string that contains the characters which may not appear in indicators for the message procedure
    #         imlemented by GrundstellungIndicatorProc.
    #
    #  \returns Nothing.
    #    
    def __init__(self, chars_to_avoid):
        ## \brief Contains invalid characters
        self._chars_to_avoid = set(chars_to_avoid)

    ## \brief This method verifies an indicator candidate.
    #
    #  \param [indicator_candidate] A string. Holds the indicator candidate to be verified.
    #
    #  \returns A boolean. Return value is True in case the indicator candidate is acceptable.
    #        
    def verify_indicator(self, indicator_candidate):
        return len(set(indicator_candidate).intersection(self._chars_to_avoid)) == 0


## \brief This class helps to derive indicators for the SG39.
#  
#  The grundstellung messageing procedure as implemented by the GrundstellungIndicatorProc generates a message key by encrypting a
#  random string. The resulting value is then used as a rotor position for en- or decryption. This poses a problem when using a SG39
#  because the last three characters in the rotor position of that machine must not contain certain characters. Therefore only such
#  random indicators can be used that encrypt to a string that does not contain the 'forbidden' characters in the relevant positions.
#
#  This class can be used to determine whether a given 10 character string contains 7 characters that can be used as a valid rotor
#  setting for an SG39. The first 4 characters can have any value from a-z. The remaining 6 characters are first searched for any
#  character from the range a-y, then for a character between a and w and finally a character beetween a and u.
#
class SG39IndicatorHelper:
    ## \brief Constructor
    #
    #  \returns Nothing.
    #
    def __init__(self):
        pass
    
    ## \brief This method can be used to test whether a 10 character string given in the parameter indicator_candidate contains a
    #         valid SG39 rotor position.
    #
    #  \param [indicator_candidate] A string. It contains the 10 character string that resulted form the encryption of the indicator.
    #
    #  \returns A boolean which is True if the indicator candidate contains a valid SG39 rotor position and False if this is not the
    #           case.
    #    
    def verify(self, indicator_candidate):
        return self.test(indicator_candidate).verified

    ## \brief This method attempts to extract a valid 7 character SG39 rotor position from the 10 character string given in the parameter
    #         indicator_candidate.
    #
    #  \param [indicator_candidate] A string. It contains the 10 character string that resulted form the encryption of the indicator.
    #
    #  \returns A string. In case of success it has a length of 7 characters. In case of a failure the length is less than 7.
    #
    def transform(self, indicator_candidate):
        return self.test(indicator_candidate).transformed

    ## \brief This method attempts to extract a valid 7 character SG39 rotor position from the 10 character string given in the parameter
    #         indicator_candidate.
    #
    #  \param [indicator_candidate] A string. It contains the 10 character string that resulted form the encryption of the indicator.
    #
    #  \returns A MsgKeyTestResult object. Its 'verified' field contains True if extraction was successfull. If it was the field
    #           'transformed' holds a usable SG39 rotor position.
    #            
    def test(self, indicator_candidate):
        result = MsgKeyTestResult(False, indicator_candidate[:4])
        wheel_sizes = ['y', 'w', 'u'] # Maximum allowed characters in positions 5, 6 and 7       
        values_found = []
        
        # Strip off the first 4 characters of the indicator candidate which are not special               
        wheel_part = indicator_candidate[4:]
        read_pos = 0
        
        # Iterate over the maximum allowed characters for the last three positions
        for i in wheel_sizes:
            current_value_found = False
            
            # Search for a character that is <= i
            while (not current_value_found) and (read_pos < 6):               
                if wheel_part[read_pos] <= i:
                    # OK we found one!
                    current_value_found = True
                    result.transformed += wheel_part[read_pos]
                
                read_pos += 1

            values_found.append(current_value_found)                
        
        # Aggreagate individual test results by 'anding' them together
        result.verified = functools.reduce(lambda x,y: x and y, values_found)        
        
        return result


## \brief This class can be used to construct MessageProcedure objects for various machine types and messageing
#         procedures.
#  
class MessageProcedureFactory:
    ## \brief Constructor.
    #
    #  \param [machine] An object that has the same interface as pyrmsk2.rotorsim.RotorMachine.
    #
    #  \param [rand_gen] An object that has the same interface as pyrmsk2.rotorrandom.RotorRandom.
    #
    #  \param [server] An object that has the same interface as pyrmsk2.tlvobject.TlvServer.   
    #
    def __init__(self, machine, rand_gen, server):
        ## \brief Holds the rotor machine object which is to be used.
        self._machine = machine
        ## \brief Holds the rotor random object which is to be used.
        self._rand_gen = rand_gen
        ## \brief Holds the tlv server object which is to be used.
        self._server = server        

    ## \brief This method partially constructs a MessageProcedure object for use with Enigma machines. The returned
    #         MessageProcedure procedure has no indicator processor.
    #
    #  \param [num_rotors] An integer. It has to specifiy how many settable rotors the Enigma has (i.e. 3 or 4).
    #
    #  \returns A MessageProcedure object.
    #
    def _get_incomplete_enigma(self, num_rotors = 3):
        result = MessageProcedure(self._machine, self._rand_gen, self._server)
        result.formatter = EnigmaFormatter(num_rotors)
        result.formatter.limits = (5, 10)
        result.msg_size = 245
        result.encoder = ArmyEncoder()
        
        return result

    ## \brief This method parses a string containing several kenngruppen seperated by blanks into a vector of three
    #         letter kenngruppen. Raises an excption if this is not possible.
    #
    #  \param [system_indicator] A string. Contains the kenngruppen as specified on the command line. 
    #
    #  \returns A vector of (3 letter) strings.
    #    
    @staticmethod
    def get_and_test_kenngruppen(system_indicator):
        kenngruppen_raw = system_indicator.split()
        kenngruppen = list(filter(lambda x: len(x) == 3, kenngruppen_raw))

        if len(kenngruppen) == 0:
            raise EnigmaException('No usable Kenngruppen specified!')
        
        return kenngruppen                        

    ## \brief This method constructs a MessageProcedure object for an Enigma using the Post 1940 procedure.
    #
    #  \param [system_indicator] A string. Has to contain the kenngruppen as one string separated by space characters.
    #
    #  \param [grundstellung] A string. Value is ignored.
    #
    #  \param [num_rotors] An integer. It has to specifiy how many settable rotors the Enigma has (i.e. 3 or 4).
    #
    #  \returns A MessageProcedure object.
    #    
    def get_post1940_enigma(self, system_indicator, grundstellung, num_rotors = 3):    
        result = self._get_incomplete_enigma(num_rotors)
        result.indicator_proc = Post1940EnigmaIndicatorProc(self._server, self._rand_gen, self.get_and_test_kenngruppen(system_indicator), num_rotors)
        
        return result

    ## \brief This method constructs a MessageProcedure object for an Enigma using the Pre 1940 procedure.
    #
    #  \param [system_indicator] A string. Has to contain the kenngruppen as one string separated by space characters.
    #
    #  \param [grundstellung] A string. The grundstellung which is used to generate the message keys.
    #
    #  \param [num_rotors] An integer. It has to specifiy how many settable rotors the Enigma has (i.e. 3 or 4).
    #
    #  \returns A MessageProcedure object.
    #    
    def get_pre1940_enigma(self, system_indicator, grundstellung, num_rotors = 3):    
        result = self._get_incomplete_enigma(num_rotors)
        result.indicator_proc = Pre1940EnigmaIndicatorProc(self._server, self._rand_gen, self.get_and_test_kenngruppen(system_indicator), grundstellung, num_rotors)
        
        return result

    ## \brief This method constructs a MessageProcedure object for a 4 wheel Enigma using the Post 1940 procedure.
    #
    #  \param [system_indicator] A string. Has to contain the kenngruppen as one string separated by space characters.
    #
    #  \param [grundstellung] A string. Value is ignored.
    #
    #  \returns A MessageProcedure object.
    #
    def get_post1940_4wheel_enigma(self, system_indicator, grundstellung):
        result = self.get_post1940_enigma(system_indicator, grundstellung, 4)
        
        return result           

    ## \brief This method constructs a MessageProcedure object for a 4 wheel Enigma using the Pre 1940 procedure.
    #
    #  \param [system_indicator] A string. Has to contain the kenngruppen as one string separated by space characters.
    #
    #  \param [grundstellung] A string. The grundstellung which is used to generate the message keys.
    #
    #  \returns A MessageProcedure object.
    #    
    def get_pre1940_4wheel_enigma(self, system_indicator, grundstellung):
        result = self.get_pre1940_enigma(system_indicator, grundstellung, 4)
        
        return result           

    ## \brief This method constructs a MessageProcedure object for an arbitrary machine that uses the grundstellung procedure
    #         as implemented by GrundstellungIndicatorProc, the basic ArmyEncoder transport encoder and a GenericFormatter.
    #
    #  \param [system_indicator] A string. Has to contain a string that identifies the key or crypto net in which messages are
    #         to be sent.
    #
    #  \param [grundstellung] A string. The grundstellung which is used to generate the message keys.
    #
    #  \param [indicator_group_size] An integer. It has to specifiy how many characters the unencrypted indicator sent along with
    #         the message does contain. Normally corresponds to the number of settable rotors of the machine (not when using the SG39).
    #
    #  \param [step_before_use] A boolean. Specifies if the underlying machine is to be stepped before en- or decryptions. 
    #         Calling this method with a parameter value of True only makes sense when a KL7 is used.
    #
    #  \returns A MessageProcedure object.
    #    
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

    ## \brief This method constructs a MessageProcedure object for a 3 wheel Enigma that uses the grundstellung procedure
    #         as implemented by GrundstellungIndicatorProc, the basic ArmyEncoder transport encoder and a GenericFormatter.
    #
    #  \param [system_indicator] A string. Has to contain a string that identifies the key or crypto net in which messages are
    #         to be sent.
    #
    #  \param [grundstellung] A string. The three letter grundstellung which is used to generate the message keys.
    #
    #  \returns A MessageProcedure object.
    #            
    def get_generic_enigma(self, system_indicator, grundstellung):
        result = self.get_generic_machine(system_indicator, grundstellung, 3)
        result.msg_size = 250
        
        return result        

    ## \brief This method constructs a MessageProcedure object for an M4 Enigma that uses the grundstellung procedure
    #         as implemented by GrundstellungIndicatorProc, the basic ArmyEncoder transport encoder and a GenericFormatter.
    #         Differs from get_generic_4wheel_enigma() only by using 4 letter groups.
    #
    #  \param [system_indicator] A string. Has to contain a string that identifies the key or crypto net in which messages are
    #         to be sent.
    #
    #  \param [grundstellung] A string. The four letter grundstellung which is used to generate the message keys.
    #
    #  \returns A MessageProcedure object.
    #            
    def get_generic_m4(self, system_indicator, grundstellung):
        result = self.get_generic_machine(system_indicator, grundstellung, 4)
        result.msg_size = 248
        result.formatter.limits = (4, 8)
        
        return result

    ## \brief This method constructs a MessageProcedure object for a 4 wheel Enigma that uses the grundstellung procedure
    #         as implemented by GrundstellungIndicatorProc, the basic ArmyEncoder transport encoder and a GenericFormatter.
    #         Ciphertext uses 5 letter groups.
    #
    #  \param [system_indicator] A string. Has to contain a string that identifies the key or crypto net in which messages are
    #         to be sent.
    #
    #  \param [grundstellung] A string. The four letter grundstellung which is used to generate the message keys.
    #
    #  \returns A MessageProcedure object.
    #            
    def get_generic_4wheel_enigma(self, system_indicator, grundstellung):
        result = self.get_generic_machine(system_indicator, grundstellung, 4)
        result.msg_size = 250
        
        return result

    ## \brief This method constructs a MessageProcedure object for Nema that uses the grundstellung procedure
    #         as implemented by GrundstellungIndicatorProc, the basic ArmyEncoder transport encoder and a GenericFormatter.
    #
    #  \param [system_indicator] A string. Has to contain a string that identifies the key or crypto net in which messages are
    #         to be sent.
    #
    #  \param [grundstellung] A string. The ten letter grundstellung which is used to generate the message keys.
    #
    #  \returns A MessageProcedure object.
    #
    def get_generic_nema(self, system_indicator, grundstellung):
        result = self.get_generic_machine(system_indicator, grundstellung, 10)
        result.msg_size = 350
        result.formatter.limits = (5, 10)
        
        return result

    ## \brief This method constructs a MessageProcedure object for a Typex that uses the grundstellung procedure
    #         as implemented by GrundstellungIndicatorProc, the TypexEncoder transport encoder and a GenericFormatter.
    #         Ciphertext uses 5 letter groups.
    #
    #  \param [system_indicator] A string. Has to contain a string that identifies the key or crypto net in which messages are
    #         to be sent.
    #
    #  \param [grundstellung] A string. The five letter grundstellung which is used to generate the message keys.
    #
    #  \returns A MessageProcedure object.
    #
    def get_generic_typex(self, system_indicator, grundstellung):
        result = self.get_generic_machine(system_indicator, grundstellung, 5)
        typex_verifier = SpecialCharIndicatorHelper('xzv')
        result.indicator_proc.verifier = typex_verifier.verify_indicator
        result.encoder = TypexEncoder()
        
        return result

    ## \brief This method constructs a MessageProcedure object for an SG39 that uses the grundstellung procedure
    #         as implemented by GrundstellungIndicatorProc, the basic ArmyEncoder transport encoder and a GenericFormatter.
    #         Ciphertext uses 5 letter groups.
    #
    #  \param [system_indicator] A string. Has to contain a string that identifies the key or crypto net in which messages are
    #         to be sent.
    #
    #  \param [grundstellung] A string. The seven letter grundstellung which is used to generate the message keys.
    #
    #  \returns A MessageProcedure object.
    #
    def get_generic_sg39(self, system_indicator, grundstellung):
        result = self.get_generic_machine(system_indicator, grundstellung, 10)
        result.msg_size = 250
        sg39_ind_helper = SG39IndicatorHelper()
        result.indicator_proc.msg_key_tester = sg39_ind_helper.test
        
        return result

    ## \brief This method constructs a MessageProcedure object for a KL7 that uses the grundstellung procedure
    #         as implemented by GrundstellungIndicatorProc, the KL7Encoder transport encoder and a GenericFormatter.
    #         Ciphertext uses 5 letter groups.
    #
    #  \param [system_indicator] A string. Has to contain a string that identifies the key or crypto net in which messages are
    #         to be sent.
    #
    #  \param [grundstellung] A string. The seven letter grundstellung which is used to generate the message keys.
    #
    #  \returns A MessageProcedure object.
    #
    def get_generic_kl7(self, system_indicator, grundstellung):
        result = self.get_generic_machine(system_indicator, grundstellung, 7, True)
        result.msg_size = 500
        result.formatter.limits = (5, 10)
        kl7_verifier = SpecialCharIndicatorHelper('zj')
        result.indicator_proc.verifier = kl7_verifier.verify_indicator        
        result.encoder = KL7Encoder()
        
        return result

    ## \brief This method constructs a MessageProcedure object for a SIGABA that uses the built in procedure for message key
    #         generation as implemented by SIGABABasicIndicatorProcessor, the SIGABAEncoder transport encoder and a SIGABAFormatter.
    #         Ciphertext uses 5 letter groups.
    #
    #  \param [system_indicator] A string. Has to contain a string that identifies the key or crypto net in which messages are
    #         to be sent.
    #
    #  \param [grundstellung] A string. The five letter internal indicator which is used to generate the message keys.
    #
    #  \returns A MessageProcedure object.
    #    
    def get_sigaba_basic(self, system_indicator, grundstellung):
        result = MessageProcedure(self._machine, self._rand_gen, self._server)
        result.indicator_proc = SIGABABasicIndicatorProcessor(self._server, self._rand_gen)
        result.formatter = SIGABAFormatter()
        result.formatter.external_indicator = system_indicator
        result.formatter.limits = (5, 10)
        result.msg_size = 1730
        result.encoder = SIGABAEncoder()
        
        return result    

    ## \brief This method constructs a MessageProcedure object for a SIGABA that uses the grundstellung procedure as implemented 
    #         by SIGABAGrundstellungIndicatorProcessor, the SIGABAEncoder transport encoder and a SIGABAFormatter. Ciphertext
    #         uses 5 letter groups.
    #
    #  \param [system_indicator] A string. Has to contain a string that identifies the key or crypto net in which messages are
    #         to be sent.
    #
    #  \param [grundstellung] A string. The five letter grundstellung which is used to generate the message keys.
    #
    #  \returns A MessageProcedure object.
    #
    def get_sigaba_grundstellung(self, system_indicator, grundstellung):
        result = self.get_sigaba_basic(system_indicator, grundstellung)
        result.indicator_proc = SIGABAGrundstellungIndicatorProcessor(self._server, self._rand_gen)
        result.indicator_proc.grundstellung = grundstellung
        
        return result

# ----------------------------------------------------------------------------------------------------


## \brief This class implements a command line application that allows to en- and decrypt a message following one of several
#         messageing procedures includung the procedure used by the german army and air force from 1940 on.
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
    #  \returns A dictionary containing the keys 'in_file', 'out_file', 'config_file', 'sys-indicator' and 'doencrypt', 'grundstellung' and
    #           'type'.
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
        parser.add_argument("-g", "--grundstellung", default=GRUND_DEFAULT, help="A basic setting or grundstellung if required by the messaging procedure")
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
    #
    #  \param [out_file] A file like object having a write() method.
    #
    #  \returns Nothing.
    #            
    def _output_formatted_message(self, formatted_parts, out_file):
        for i in formatted_parts:
            out_file.write(i)
            out_file.write('\n\n')        

    ## \brief This method constructs a MessageProcedure object for a given machine and messageing procedure type. Raises an
    #         exception if the combination of requested messageing procedure and machine type is impossible or not yet implemented.
    #
    #  \param [machine_name] A String. Specifies the machine type. Allowed values are: 'Enigma', 'M3', 'KDEnigma',
    #         'AbwehrEnigma', 'TirpitzEnigma', 'M4Enigma', 'RailwayEnigma', 'CSP889', 'CSP2900', 'Typex', 'KL7', 'SG39',
    #         'Nema'.
    #
    #  \param [system_indicator] A string. Has to contain a string that identifies the key or crypto net in which messages are
    #         to be sent. Value of -s/--sys-indicator command line parameter.
    #
    #  \param [grundstellung] A string. The grundstellung which is used to generate the message keys. Value of -g/grundstellung
    #         command line parameter.
    #
    #  \param [proc_type] A string. Specifies the type of the messageing procedure. Value of -t/--type command line parameter.
    #
    #  \returns A MessageProcedure object.
    #
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
            if grundstellung == GRUND_DEFAULT:
                raise EnigmaException('Grundstellung missing. Add -g/--grundstellung option.')
        
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
