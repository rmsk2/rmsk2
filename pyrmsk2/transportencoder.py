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

## @package transportencoder A transport encoder knows how to transform the original plaintext into a string that only contains characters the rotor machine can understand.
#         
#           
# \file transportencoder.py
# \brief Implements the transport encoders known to rmsk2. 
#    

from pyrmsk2 import EnigmaException as EnigmaException

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


## \brief This class implements a transport encoder that accepts arbitrary unicode strings as input and transforms
#         the input in such a way that only a defined subset of characters appears in the output.
#
class ModernEncoder:
    ## \brief Constructor
    #
    #  \param [encoder_alphabet] A string. Contains the characters that may appear in encoded output.
    #
    #  \returns Nothing.
    #        
    def __init__(self, encoder_alphabet):
        ## \brief A string that stores the set of output characters.
        self._encoder_alpha = encoder_alphabet
        ## \brief An integer that stores the number of output characters.
        self._radix = len(self._encoder_alpha)
        ## \brief A dictionary that maps strings to ints. Maps any character to its position in self._encoder_alpha.
        self._inv_alpha = {}
        
        count = 0
        
        for i in self._encoder_alpha:
            self._inv_alpha[i] = count
            count += 1

    ## \brief This method transforms a plaintext into an encoded form before that encoded form ist encrypted.
    #
    #  \param [plaintext] A string. Contains the plaintext to transform.
    #
    #  \returns A string. The encoded plaintext
    #    
    def transform_plaintext_enc(self, data_to_encode):
        raw_bytes = data_to_encode.encode()
        
        return self.encode_bytes(raw_bytes)

    ## \brief This method transforms an array of bytes into a string which contains only characters from self._encoder_alpha.
    #
    #  \param [raw_bytes] A bytearray or bytes object. Contains the input bytes to transform.
    #
    #  \returns A string. The encoded input data.
    #        
    def encode_bytes(self, raw_bytes):
        result = ''
        
        for i in raw_bytes:
            result += self._encoder_alpha[i // self._radix]
            result += self._encoder_alpha[i % self._radix]
            
        return result

    ## \brief This method transforms a decryped (and encoded) plaintext into its original form.
    #
    #  \param [plaintext] A string. Contains the encoded plaintext to transform
    #
    #  \returns A string. The decoded plaintext
    #    
    def transform_plaintext_dec(self, data_to_decode):
        raw_bytes = self.decode_bytes(data_to_decode)
        
        return raw_bytes.decode()

    ## \brief This method transforms a string encoded with self.encode_bytes() back into a bytearray object.
    #
    #  \param [data_to_decode] A string that is to be decoded.
    #
    #  \returns A bytearray object.
    #            
    def decode_bytes(self, data_to_decode):
        raw_bytes = bytearray()
        raw_digits = []
        
        if (len(data_to_decode) % 2) != 0:
            raise EnigmaException('Input length not divisible by 2')
        
        for i in data_to_decode:
            if i not in self._encoder_alpha:
                raise EngimaException('Illegal character ' + i)
            
            raw_digits.append(self._inv_alpha[i])
        
        for i in range(len(raw_digits) // 2):
            temp = self._radix * raw_digits[2 * i] + raw_digits[2 * i + 1]
            
            if temp > 255:
                raise EnigmaException('Illegal byte value encountered at ' + str(2 * i))
        
            raw_bytes.append(temp)
        
        return raw_bytes


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

