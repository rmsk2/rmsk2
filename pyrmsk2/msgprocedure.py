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

## @package msgprocedure Implements the MessageProcedure class that uses indicator processors, formatters and transport encoders to process messages.
#         
#           
# \file msgprocedure.py
# \brief Implements message procedures. 

import re
import functools

import pyrmsk2.rotorsim as rotorsim
import pyrmsk2.rotorrandom as rotorrandom
from pyrmsk2.transportencoder import *
from pyrmsk2.indicatorprocessor import *
from pyrmsk2.formatter import *


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
        
        if self._step_before_proc:
            self._machine.step()            
        
        # Use message length to strip padding off at the end of the message body
        if MESSAGE_LENGTH in indicators.keys():
            ciphertext = help.text[:indicators[MESSAGE_LENGTH]]

        return self._machine.decrypt(ciphertext) # decrypt


# ----------------------------------------------------------------------------------------------------


## \brief This class helps to derive indicators intended for the message procedure implemented by GrundstellungIndicatorProc when
#         using machines that accept only a subset of a-z in their unshifted input alphabet. Or put in another way: It helps
#         with deriving indicators for machines which use some of the characters in a-z as stand ins for special characters like
#         space, letter shift or figure shift.
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
    #         letter kenngruppen. Raises an exception if this is not possible.
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
        result.msg_size = 750
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


