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

import functools
import pyrmsk2.rotorrandom as rotorrandom
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


## \brief This class implements the Vigenere cipher on the basis of an arbitrary alphabet.
#
class Vigenere:
    ## \brief Constructor
    #
    #  \param [known_characters] A string. Contains the characters that make up the input and output alphabet.
    #
    #  \returns Nothing.
    #        
    def __init__(self, known_characters):
        self._alphabet = known_characters
        self._inv_alpha = {}
        
        count = 0
        
        for i in self._alphabet:
            self._inv_alpha[i] = count
            count += 1

    ## \brief This method implements the core of the en- and decryption operation.
    #
    #  \param [inputtest] A string. Contains either plaintext or ciphertext depending on the intended use of
    #         this method.
    #
    #  \param [password] A string. Contains the password made up from characters contained in self._alphabet
    #         that is used for en- and decryption. 
    #
    #  \param [proc] A callable object that takes two ints and returns an int. The caller has to supply
    #         a function that knows how to process a single character of plain-/ciphertext and password and
    #         uses this data to implement encryption or decryption. 
    #
    #  \returns A string. The result of en- or decryption.
    #            
    def _vigenere_process(self, inputtext, password, proc):
        count_pw = 0
        num_chars = len(self._alphabet)
        len_pw = len(password)
        raw_data = list(map(lambda x: self._inv_alpha[x], inputtext))
        raw_pw = list(map(lambda x: self._inv_alpha[x], password))        
        temp = []
        
        for i in raw_data:
            temp.append(proc(i, raw_pw[count_pw]))
            count_pw = (count_pw + 1) % len_pw
        
        result = ''.join(list(map(lambda x: self._alphabet[x], temp)))        
        
        return result    

    ## \brief Convenience method implementing encryption.
    #
    #  \param [plaintext] A string. Contains the plaintext.
    #
    #  \param [password] A string. Contains the password made up from characters contained in self._alphabet
    #         that is used for encryption. 
    #
    #  \returns A string. The result of encryption.
    #                    
    def encrypt(self, plaintext, password):
        len_alpha = len(self._alphabet)
        enc_func = lambda x,y: (x + y) % len_alpha
        
        return self._vigenere_process(plaintext, password, enc_func)

    ## \brief Convenience method implementing decryption.
    #
    #  \param [ciphertext] A string. Contains the ciphertext.
    #
    #  \param [password] A string. Contains the password made up from characters contained in self._alphabet
    #         that is used for decryption. 
    #
    #  \returns A string. The result of decryption.
    #                        
    def decrypt(self, ciphertext, password):
        len_alpha = len(self._alphabet)
        dec_func = lambda x,y: (x - y + len_alpha) % len_alpha
        
        return self._vigenere_process(ciphertext, password, dec_func)


## \brief This class is a simple helper which allows to read a string character by character.
#
class CharBuf:
    ## \brief Constructor
    #
    #  \param [all_chars] A string. Contains the string that is to be read
    #
    #  \returns Nothing.
    #        
    def __init__(self, all_chars):
        self._buffer = all_chars
        self._pos = 0

    ## \brief This method implements reading the characters of the string in the buffer.
    #
    #  \returns A string of length 1 or ''.
    #            
    def get_next_char(self):
        result = ''
        
        if self._pos < len(self._buffer):
            result = self._buffer[self._pos]
            self._pos += 1
        
        return result


## \brief This class implements a transport encoder that accepts arbitrary unicode strings as input and transforms
#         the input in such a way that only a defined subset of characters appears in the output. On top of that the
#         encoded text can be enciphered using the Vigenere cipher in order to mask the unfavourable frequency distribution 
#         generated by the encoder. Additionally this makes known plaintext attacks harder.
#
class ModernEncoder:
    ## \brief Constructor
    #
    #  \param [tlv_server] An object with the same interface as pyrmsk2.tlvobject.TlvServer.
    #
    #  \param [pw_length] An int. Contains the password length used for Vigenere encryption.
    #
    #  \param [use_vigenere] A boolean. If True the encoded value is also Vigenere encrypted.
    #
    #  \returns Nothing.
    #        
    def __init__(self, tlv_server, pw_length = 9, use_vigenere =  True):
        ## \brief Contains the password length used for Vigenere encryption
        self._pw_length = pw_length
        ## \brief Holds TLV server
        self._server = tlv_server
        ## \brief Contains all characters which are not translated
        self._direct_chars = 'etaoinsrhld'
        ## \brief Contains all characters which signify an encoded byte        
        self._escape_chars = 'bcfgkmpquwy'
        self._use_vigenere = use_vigenere
        self._all_characters = self._direct_chars + self._escape_chars
        
        self._inv_alpha = {}        
        count = 0        
        for i in self._all_characters:
            self._inv_alpha[i] = count
            count += 1
        
        self._inv_escape = {}
        count = 0        
        for i in self._escape_chars:
            self._inv_escape[i] = count
            count += 1        

    ## \brief This method transforms a plaintext into an encoded form before that encoded form ist encrypted.
    #
    #  \param [data_to_encode] A string. Contains the plaintext to transform.
    #
    #  \returns A string. The encoded plaintext
    #    
    def transform_plaintext_enc(self, data_to_encode):
        result = ''
        pw = ''
        
        result = self.encode_utf8(data_to_encode)
        
        if self._use_vigenere:        
            with rotorrandom.RotorRandom(self._all_characters, self._server.address) as rand:
                pw = rand.get_rand_string(self._pw_length)
                vig = Vigenere(self._all_characters)
                result = pw + vig.encrypt(result, pw)
        
        return result

    ## \brief This method transforms string into a string which contains only characters from self._all_characters.
    #
    #  \param [in_string] A string. Contains the input data to transform.
    #
    #  \returns A string. The encoded input data.
    #        
    def encode_utf8(self, in_string):
        result = ''
        temp = []
        
        for i in in_string:
            if i in self._direct_chars:
                temp += i
            else:
                raw_bytes = i.encode()                
                for j in raw_bytes:
                    temp_chars = self._escape_chars[j // 22] + self._all_characters[j % 22]                    
                    temp += temp_chars
        
        result = ''.join(temp)
        
        return result

    ## \brief This method transforms a decrypted (and encoded) plaintext into its original form.
    #
    #  \param [data_to_decode] A string. Contains the encoded plaintext to transform
    #
    #  \returns A string. The decoded plaintext
    #    
    def transform_plaintext_dec(self, data_to_decode):
    
        if not functools.reduce(lambda x,y: x and y, map(lambda x: x in self._all_characters, data_to_decode)):
            raise EnigmaException('Some input characters not in encoder alphabet')

        if self._use_vigenere:
            if (len(data_to_decode) - self._pw_length) < 0:
                raise EnigmaException('Input length too short')
        
            pw = data_to_decode[:self._pw_length]            
            
            ciphertext = data_to_decode[self._pw_length:]
            vig = Vigenere(self._all_characters)
            data_to_decode = vig.decrypt(ciphertext, pw)
        
        result = self.decode_utf8(data_to_decode)
        
        return result

    ## \brief This method transforms a string encoded with self.encode_utf8() back into its original form.
    #
    #  \param [in_string] A string that is to be decoded.
    #
    #  \returns A string.
    #            
    def decode_utf8(self, in_string):
        res = bytearray()
        in_buf = CharBuf(in_string)
        
        i = in_buf.get_next_char()
        
        while i != '':
            if i in self._direct_chars:
                res += i.encode()
            else:
                if i in self._escape_chars:
                    j = in_buf.get_next_char()
                    
                    if j != '':
                        if j in self._all_characters:
                            raw_byte = self._inv_escape[i] * 22 + self._inv_alpha[j]
                            
                            if raw_byte <= 255:
                                res.append(raw_byte)
                            else:
                                raise EnigmaException('Structure of encoded text invalid')                             
                        else:
                            raise EnigmaException('Structure of encoded text invalid')
                    else:
                        raise EnigmaException('Premature end of encoded text')                    
                else:
                    raise EnigmaException('Structure of encoded text invalid')
            
            i = in_buf.get_next_char()
        
        return res.decode()


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

