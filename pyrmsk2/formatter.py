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

## @package formatter A formatter knows how to construct and parse a message part from the already encrypted ciphertext and the indicator groups as created by the indicator processor.
#         
#           
# \file formatter.py
# \brief Implements the formatters known to rmsk2. 


import datetime
import re

from pyrmsk2 import EnigmaException as EnigmaException
from pyrmsk2.rotorsim import RotorMachine as RotorMachine
from pyrmsk2.indicatorprocessor import HEADER_GRP_1 as HEADER_GRP_1
from pyrmsk2.indicatorprocessor import HEADER_GRP_2 as HEADER_GRP_2
from pyrmsk2.indicatorprocessor import INTERNAL_INDICATOR as INTERNAL_INDICATOR
from pyrmsk2.indicatorprocessor import EXTERNAL_INDICATOR as EXTERNAL_INDICATOR


## \brief Matches stuff like 1534 = 15tle = 15tl = 167 = RJF GNZ =
ENIGMA_HEADER_EXP = '^[0-9]{{4}} = [0-9]+(tl|tle) = [0-9]+tl = [0-9]+ = ([A-Z]{{{0}}}) ([A-Z]{{{0}}}) =$'
## \brief Dictionary key that names the number of ciphertext characters when using the default SIGABA message procedure 
MESSAGE_LENGTH = 'message_length'


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
    #  \param [indicators] A dictionary that maps strings to strings. The indicators parameter is ignored by this implementation
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
        
        result.text = RotorMachine.group_text(ciphertext, True, self._group_size, self._groups_per_line)        
        
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
        
        result.text = RotorMachine.group_text(ciphertext, True, self._group_size, self._groups_per_line)        
        
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
                
        result.text = RotorMachine.group_text(ciphertext, True, self._group_size, self._groups_per_line)        
        
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
    

