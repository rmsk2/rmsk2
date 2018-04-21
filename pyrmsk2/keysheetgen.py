################################################################################
# Copyright 2018 Martin Grap
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

## @package keysheetgen A Python3 module which allows to generate key sheets for all machines provided by rmsk2 and rotorsim.
#           
# \file pyrmsk2/keysheetgen.py
# \brief This file imlements a keysheet generator for all rotor machines provided by rmsk2 and rotorsim.


import sys
import traceback
import os
import re
import argparse
import binascii
import pyrmsk2.rotorsim as rotorsim
import pyrmsk2.rotorrandom as rotorrandom


MACHINE_NAMES = ['M3', 'Services', 'M3D', 'ServicesD', 'ServicesUhr', 'M4', 'Railway', 'Abwehr', 'KD', \
                 'Tirpitz', 'Typex', 'TypexY269', 'TypexPlugsY2695', 'NemaWar', 'NemaTraining', 'CSP889', 'CSP2900', 'KL7', 'SG39']
                 
## \brief List that contains the allowed keywords for specifying the message procedure
PROC_TYPES = ['grundstellung', 'post1940', 'pre1940', 'sigaba']                 


## \brief An excpetion class that is used for constructing exception objects in this module. 
#
class KeysheetException(Exception):
    ## \brief An excpetion class that is used for constructing exception objects in this module. 
    #
    #  \param [error_message] Is a string. It has to contain an error message that is to be conveyed to 
    #         receiver of the corresponding exception.
    #
    def __init__(self, error_message):
        Exception.__init__(self, error_message)


## \brief A class that serves as the base class for all Column classes. All these classes know how to
#         calculate and format the contents or payload of an entry or cell in the keysheet. Column classes  
#         are not intended to know how to format the sheet itself.
#
class ColumnBase:
    ## \brief Constructor
    #
    #  \param [col_width] Is an integer. It specifies the width of the cell in characters. This information
    #         may not be used by all renderers. It is used by the TextKeysheetRenderer class.
    #
    def __init__(self, col_width):
        ## \brief Contains the column width.
        self._col_width = col_width

    ## \brief This property returns the column width as for instance specified in the constructor.
    #
    #  \returns An integer containing the column width.
    #    
    @property
    def col_width(self):
        return self._col_width

    ## \brief This property setter allows to set the column width.
    #
    #  \param [value] An integer containing the new column width.
    #            
    @col_width.setter
    def col_width(self, value):
        self._col_width = value

    ## \brief This method retrieves the information that is represented in the Column from a machine configuration
    #         an formats it accordingly.
    #
    #  \param [config] Is a dictionary that maps strings to strings. This dictionary is intended to contain the
    #         configuration of a rotor machine as returned by the rotorsim.RotorMachine.get_config() method. 
    #
    #  \param [randomizer] Is a rotorrandom.RotorRandom object. This object can be used to generate random strings for
    #         use in the column.
    #            
    #  \param [machine] Is a rotorsim.RotorMachine object. This object can be queried to retrieve additional keysheet
    #         information. It is expected that config contains the result of machine.get_config().
    #
    #  \returns A string. This string is to be included into the keysheet.
    #            
    def process(self, config, randomizer, machine):
        return ''    


## \brief A class that provides an interface for managing information about whether the column should be formatted
#         in upper or lowercase.
#
class CaseSensitiveColumn(ColumnBase):
    ## \brief Constructor
    #
    #  \param [col_width] Is an integer. It specifies the width of the cell in characters. This information
    #         may not be used by all renderers. It is used by the TextKeysheetRenderer class.
    #
    def __init__(self, col_width):
        super().__init__(col_width)
        ## \brief A boolean. If this value is true then the string returned by process() is in uppercase.
        self._uppercase = True

    ## \brief This property returns a flag that indiciates whether process() should return its result in uppercase or 
    #         not
    #
    #  \returns A boolean.
    #    
    @property
    def uppercase(self):
        return self._uppercase

    ## \brief This property setter allows to set the flag that determines whether process() returns a uppercase or
    #         lowercase string.
    #
    #  \param [value] A boolean. True means that an uppercase result is desired.
    #                    
    @uppercase.setter
    def uppercase(self, value):
        self._uppercase = value

   
## \brief A class that retrieves a string from the configuration of a rotor machine in order to put this value into
#         a keysheet cell.
#
class Column(CaseSensitiveColumn):
    ## \brief Constructor
    #
    #  \param [col_width] Is an integer. It specifies the width of the cell in characters. This information
    #         may not be used by all renderers. It is used by the TextKeysheetRenderer class.
    #
    #  \param [config_name] Is a string. This string represents they dictionary key which can be used to retrieve
    #         the desired information from a rotor machine configuration.
    #
    def __init__(self, col_width, config_name = None):
        super().__init__(col_width)
        ## \brief A string. Contains the key to use in order to get information from the configuration dictionary.
        self._config_name = config_name

    ## \brief This method retrieves the desired information from a rotor machine configuration and formats it in
    #         upper or lowercase.
    #
    #  \param [config] Is a dictionary that maps strings to strings. This dictionary is intended to contain the
    #         configuration of a rotor machine as returned by the rotorsim.RotorMachine.get_config() method. 
    #
    #  \param [randomizer] Is a rotorrandom.RotorRandom object. This object can be used to generate random strings for
    #         use in the column.
    #            
    #  \param [machine] Is a rotorsim.RotorMachine object. This object can be queried to retrieve additional keysheet
    #         information. It is expected that config contains the result of machine.get_config().
    #
    #  \returns A string. This string contains config[self._config_name] in upper or lowercase.
    #                    
    def process(self, config, randomizer, machine):
        result = ''
        
        if self._config_name != None:
            result = config[self._config_name]
            
            if self._uppercase:            
                result = result.upper()
                            
        return result

## \brief A class that knows how to represent the ring settings from an Enigma configuration in numeric form.
#
class RingColumn(Column):
    ## \brief This method retrieves the ring setting from an Enigma configaration an represents this as three or
    #         four numeric values.
    #
    #  \param [config] Is a dictionary that maps strings to strings. This dictionary is intended to contain the
    #         configuration of a rotor machine as returned by the rotorsim.RotorMachine.get_config() method. 
    #
    #  \param [randomizer] Is a rotorrandom.RotorRandom object. This object can be used to generate random strings for
    #         use in the column.
    #            
    #  \param [machine] Is a rotorsim.RotorMachine object. This object can be queried to retrieve additional keysheet
    #         information. It is expected that config contains the result of machine.get_config().
    #
    #  \returns A string. This string contains the ring settings.
    #                    
    def process(self, config, randomizer, machine):
        help = super().process(config, randomizer, machine)        
        result = ''
        
        for i in help:
            result += '{:02d} '.format(ord(i) - ord('A') + 1)
        
        return result.rstrip()


## \brief A class that knows how to represent the Uhr dial information from an Enigma configuration in numeric form.
#
class UhrDialColumn(Column):
    ## \brief Constructor
    #
    #  \param [col_width] Is an integer. It specifies the width of the cell in characters. This information
    #         may not be used by all renderers. It is used by the TextKeysheetRenderer class.
    #
    #  \param [config_name] Is a string. This string represents they dictionary key which can be used to retrieve
    #         the desired information from a rotor machine configuration.
    #
    def __init__(self, col_width, config_name = None):
        super().__init__(col_width, config_name)
        ## \brief Contains a regexp that allows to separate the dial information from the plugboard settings.
        self._check_for_uhr = re.compile('^(.+):(.+)$')

    ## \brief This method retrieves the Uhr dial from an Enigma configaration an represents this as a numeric value.
    #
    #  \param [config] Is a dictionary that maps strings to strings. This dictionary is intended to contain the
    #         configuration of a rotor machine as returned by the rotorsim.RotorMachine.get_config() method. 
    #
    #  \param [randomizer] Is a rotorrandom.RotorRandom object. This object can be used to generate random strings for
    #         use in the column.
    #            
    #  \param [machine] Is a rotorsim.RotorMachine object. This object can be queried to retrieve additional keysheet
    #         information. It is expected that config contains the result of machine.get_config().
    #
    #  \returns A string. This string contains the Uhr dial settings.
    #                    
    def process(self, config, randomizer, machine):
        result = super().process(config, randomizer, machine)        
        
        match = re.search(self._check_for_uhr, result)
        if match != None:
            result = match.group(1)
        
        return result


## \brief A class that knows how to generate a random string which is to be included into a keysheet.
#
class RandStringColumn(CaseSensitiveColumn):
    ## \brief Constructor
    #
    #  \param [col_width] Is an integer. It specifies the width of the cell in characters. This information
    #         may not be used by all renderers. It is used by the TextKeysheetRenderer class.
    #
    #  \param [num_chars] Is an integer. This value specifies how many random characters are to be generated.
    #
    def __init__(self, col_width, num_chars):
        super().__init__(col_width)
        ## \brief Holds length of random string.
        self._num_chars = num_chars

    ## \brief This method determines a random string of the desired length.
    #
    #  \param [config] Is a dictionary that maps strings to strings. This dictionary is intended to contain the
    #         configuration of a rotor machine as returned by the rotorsim.RotorMachine.get_config() method. 
    #
    #  \param [randomizer] Is a rotorrandom.RotorRandom object. This object can be used to generate random strings for
    #         use in the column.
    #            
    #  \param [machine] Is a rotorsim.RotorMachine object. This object can be queried to retrieve additional keysheet
    #         information. It is expected that config contains the result of machine.get_config().
    #
    #  \returns A string. This string contains the random characters.
    #    
    def process(self, config, randomizer, machine):
        result = randomizer.get_rand_string(self._num_chars)
        
        if self._uppercase:
            result = result.upper()
        
        return result


## \brief A class that knows how to generate a check string which can be used to verify that the rotor machine 
#         has been properly setup according to the values in the keysheet. 
#
class CheckStringColumn(RandStringColumn):
    ## \brief Constructor
    #
    #  \param [col_width] Is an integer. It specifies the width of the cell in characters. This information
    #         may not be used by all renderers. It is used by the TextKeysheetRenderer class.
    #
    #  \param [num_chars] Is an integer. This value specifies how many check values are to be generated. It
    #         is used to determine how many characters are taken from the end of the encrypted check_string.
    #
    #  \param [resetter] Is a callable object that takes a string (the current rotor position) and returns
    #         the rotor positions to use in order to start the check value calculation.
    #
    #  \param [check_string] Is a string. This string is encrypted in order to generate the check value.
    #
    #  \param [step_first] Is a boolean. If this value is true the rotor machine is stepped before encrypting
    #         the check_string.
    #
    def __init__(self, col_width, num_chars, resetter, check_string, step_first = False):
        super().__init__(col_width, num_chars)
        ## \brief Holds the resetter that calculates the start position.
        self._resetter = resetter
        ## \brief Holds the check string.
        self._check_string = check_string
        ## \brief If True the machine is stepped before encrypting the check string.
        self._step_first = step_first

    ## \brief This method determines a the check value.
    #
    #  \param [config] Is a dictionary that maps strings to strings. This dictionary is intended to contain the
    #         configuration of a rotor machine as returned by the rotorsim.RotorMachine.get_config() method. 
    #
    #  \param [randomizer] Is a rotorrandom.RotorRandom object. This object can be used to generate random strings for
    #         use in the column.
    #            
    #  \param [machine] Is a rotorsim.RotorMachine object. This object can be queried to retrieve additional keysheet
    #         information. It is expected that config contains the result of machine.get_config().
    #
    #  \returns A string. Contains the calculated check value.
    #    
    def process(self, config, randomizer, machine):
        current_rotor_pos = machine.get_rotor_positions()
        # Reset rotor positions to start value for check value calcualation
        machine.set_rotor_positions(self._resetter(current_rotor_pos))
        
        if self._step_first:
            machine.step()
        
        check_string = self._check_string.lower()
        enc_res = machine.encrypt(check_string)
        # Take desired number of characters from the end of the encrypted check value
        result = enc_res[-self._num_chars:]
        # Group check value in five letter groups
        result = rotorsim.RotorMachine.group_text(result, uppercase = self._uppercase)        
        
        return result


## \brief A class that knows how to determine and format the current rotor position of a rotor machine in order
#         to include it into a keysheet.
#
class RotorPosColumn(CaseSensitiveColumn):
    ## \brief This method returns the rotor position of the machine spcified in parameter machine.
    #
    #  \param [config] Is a dictionary that maps strings to strings. This dictionary is intended to contain the
    #         configuration of a rotor machine as returned by the rotorsim.RotorMachine.get_config() method. 
    #
    #  \param [randomizer] Is a rotorrandom.RotorRandom object. This object can be used to generate random strings for
    #         use in the column.
    #            
    #  \param [machine] Is a rotorsim.RotorMachine object. This object can be queried to retrieve additional keysheet
    #         information. It is expected that config contains the result of machine.get_config().
    #
    #  \returns A string. Contains the retrieved rotor position.
    #    
    def process(self, config, randomizer, machine):
        result = machine.get_rotor_positions()
        
        if self._uppercase:
            result = result.upper()
        
        return result


## \brief A class that knows how to determine and format the current rotor position of a KL7 machine in order
#         to include it into a keysheet.
#
class KL7RotorPosColumn(RotorPosColumn):
    ## \brief This method returns the rotor position of the machine specified in parameter machine.
    #
    #  \param [config] Is a dictionary that maps strings to strings. This dictionary is intended to contain the
    #         configuration of a rotor machine as returned by the rotorsim.RotorMachine.get_config() method. 
    #
    #  \param [randomizer] Is a rotorrandom.RotorRandom object. This object can be used to generate random strings for
    #         use in the column.
    #            
    #  \param [machine] Is a rotorsim.RotorMachine object. This object can be queried to retrieve additional keysheet
    #         information. It is expected that config contains the result of machine.get_config().
    #
    #  \returns A string. Contains the retrieved rotor position without the fixed rotor number 4.
    #    
    def process(self, config, randomizer, machine):
        result = super().process(config, randomizer, machine)
        
        return result[0:3] + result[4:8]


## \brief A class that knows how to determine and format the current rotor position of the index rotors of a SIGABA machine
#         in order to include it into a keysheet.
#
class SIGABAIndexRotorPosColumn(RotorPosColumn):
    ## \brief This method returns the positions of the index rotors of the SIGABA machine specified in parameter machine.
    #
    #  \param [config] Is a dictionary that maps strings to strings. This dictionary is intended to contain the
    #         configuration of a rotor machine as returned by the rotorsim.RotorMachine.get_config() method. 
    #
    #  \param [randomizer] Is a rotorrandom.RotorRandom object. This object can be used to generate random strings for
    #         use in the column.
    #            
    #  \param [machine] Is a rotorsim.RotorMachine object. This object can be queried to retrieve additional keysheet
    #         information. It is expected that config contains the result of machine.get_config().
    #
    #  \returns A string. Contains the retrieved rotor positions of the index rotors.
    #    
    def process(self, config, randomizer, machine):
        result = super().process(config, randomizer, machine)
        
        return result[0:5]


## \brief A class that knows how to format a string with an even number of letters into a sequence of letter pairs. It
#         is primarily intended to be used to display configuration information for plugboards and plugable reflectors.
#
class PlugsColumn(Column):
    ## \brief Constructor
    #
    #  \param [col_width] Is an integer. It specifies the width of the cell in characters. This information
    #         may not be used by all renderers. It is used by the TextKeysheetRenderer class.
    #
    #  \param [config_name] Is a string. This string represents they dictionary key which can be used to retrieve
    #         the desired information from a rotor machine configuration.
    #
    def __init__(self, col_width, config_name = None):
        super().__init__(col_width, config_name)
        ## \brief Holds a regexp which is uesed to separate the Uhr dial information from the plugboard information.
        self._check_for_uhr = re.compile('^(.+):(.+)$')

    ## \brief This method returns the formatted letter pairs which represent the configuration of a plugboard or plugable
    #         reflector.
    #
    #  \param [config] Is a dictionary that maps strings to strings. This dictionary is intended to contain the
    #         configuration of a rotor machine as returned by the rotorsim.RotorMachine.get_config() method. 
    #
    #  \param [randomizer] Is a rotorrandom.RotorRandom object. This object can be used to generate random strings for
    #         use in the column.
    #            
    #  \param [machine] Is a rotorsim.RotorMachine object. This object can be queried to retrieve additional keysheet
    #         information. It is expected that config contains the result of machine.get_config().
    #
    #  \returns A string. Contains the desired configuration information.
    #
    def process(self, config, randomizer, machine):
        result = super().process(config, randomizer, machine)
        
        # Check for Enigma Uhr dial information
        match = re.search(self._check_for_uhr, result)
        if match != None:
            # And discard it if present
            result = match.group(2)
        
        # Format into letter pairs
        help = ''
        for i in range(len(result) // 2):
            help += '{} '.format(result[2 * i:2 * i + 2])
        
        return help.rstrip()


## \brief A class that knows how to generate and format three letter Kenngruppen information for inclusion into an
#         (Enigma) keysheet.
#
class KenngruppenColumn(ColumnBase):
    ## \brief Constructor
    #
    #  \param [col_width] Is an integer. It specifies the width of the cell in characters. This information
    #         may not be used by all renderers. It is used by the TextKeysheetRenderer class.
    #
    #  \param [num_groups] Is an integer. This value determines how many three letter Kenngruppen are to be generated.
    #
    def __init__(self, col_width, num_groups = 4):
        super().__init__(col_width)
        ## \brief Holds the number of Kenngruppen to generate.
        self._num_groups = num_groups

    ## \brief This method generates and returns a single three letter Kenngruppe. No character appears twice in the
    #         Kenngruppe generated by this method
    #
    #  \param [randomizer] Is a rotorrandom.RotorRandom object. This object is used to generate the Kenngruppe.
    #            
    #  \returns A string. Contains the Kenngruppe.
    #    
    def make_kenngruppe(self, randomizer):
        perm = rotorsim.Permutation()
        perm.from_int_vector(randomizer.get_rand_permutation())
        perm_alpha = perm.to_string()
        return perm_alpha[0:3]

    ## \brief This method returns the desired number of Kenngruppen and returns them to the caller. All generated
    #         Kenngruppen returned by one call of this method are unique.
    #
    #  \param [config] Is a dictionary that maps strings to strings. This dictionary is intended to contain the
    #         configuration of a rotor machine as returned by the rotorsim.RotorMachine.get_config() method. 
    #
    #  \param [randomizer] Is a rotorrandom.RotorRandom object. This object can be used to generate random strings for
    #         use in the column.
    #            
    #  \param [machine] Is a rotorsim.RotorMachine object. This object can be queried to retrieve additional keysheet
    #         information. It is expected that config contains the result of machine.get_config().
    #
    #  \returns A string. Contains the generated Kenngruppen.
    #    
    def process(self, config, randomizer, machine):
        result = ''
        groups_found = False
        all_groups = []
        
        # Loop while generated Kenngruppen are not unique
        while not groups_found:            
            all_groups.clear()
            for i in range(self._num_groups):
                all_groups.append(self.make_kenngruppe(randomizer))
            
            test_set = set(all_groups)
            # Check for repeated Kenngruppen
            groups_found = len(test_set) == len(all_groups)
        
        for i in all_groups:
            result += i + ' '
        
        return result.rstrip()


## \brief A class that knows how to retrieve and format the rotor settings information of an Enigma machine.
#
class RotorColumn(Column):
    ## \brief Constructor
    #
    #  \param [col_width] Is an integer. It specifies the width of the cell in characters. This information
    #         may not be used by all renderers. It is used by the TextKeysheetRenderer class.
    #
    #  \param [config_name] Is a string. This string represents they dictionary key which can be used to retrieve
    #         the rotor information from an Enigma configuration.
    #
    def __init__(self, col_width, config_name = None):
        super().__init__(col_width, config_name)
        self.ukw_names = ['B', 'C', 'D']
        self.greek_names = ['beta', 'gamma']        
        self.roman_numerals = ['I', 'II', 'III', 'IV', 'V', 'VI', 'VII', 'VIII']        

    ## \brief This method tranforms a three letter string of decimal characters (0-9) into a sequence of three roman numbers.
    #
    #  \param [rotor_spec] Is a string. It contains the rotor specification as retrieved from the Enigma configuration.
    #         I.e. as three characters from the range 0-9. 
    #
    #  \returns A string. Contains the roman numerals.
    #        
    def rotor_numerals(self, rotor_spec):
        result = self.roman_numerals[int(rotor_spec[0]) - 1]
        result += ' ' + self.roman_numerals[int(rotor_spec[1]) - 1]
        result += ' ' + self.roman_numerals[int(rotor_spec[2]) - 1]
        
        return result        

    ## \brief This method returns the formatted rotor settings. It includes information about the Umkehrwalze and
    #         possible greek wheels. It can be called for all Enigma variants supported by this software.
    #
    #  \param [config] Is a dictionary that maps strings to strings. This dictionary is intended to contain the
    #         configuration of a rotor machine as returned by the rotorsim.RotorMachine.get_config() method. 
    #
    #  \param [randomizer] Is a rotorrandom.RotorRandom object. This object can be used to generate random strings for
    #         use in the column.
    #            
    #  \param [machine] Is a rotorsim.RotorMachine object. This object can be queried to retrieve additional keysheet
    #         information. It is expected that config contains the result of machine.get_config().
    #
    #  \returns A string. Contains the generated rotor settings information.
    #            
    def process(self, config, randomizer, machine):
        result = ''
        if self._config_name != None:
            result = ''
            help = config[self._config_name]
            
            # Information about a UKW is present
            if len(help) > 3:
                result = self.ukw_names[int(help[0]) - 1] + ' '
            
            # Underlying machine is an M4. Include greek wheel.
            if len(help) == 5:
                result += self.greek_names[int(help[1]) - 1] + ' '
            
            # Transform remaining rotor information into roman numerals.
            result += self.rotor_numerals(help[-3:])
            
        return result


## \brief A class that abstracts the notion of a keysheet that is organized as a table with rows and columns.
#         Each row contains the machine settings for a specific day in a month. Each setting is contained in
#         an appropriately named column. 
#         
#         Each key sheet is valid for a specific month in a given year. It has a classification level and pertains
#         to a certain crypto net or key. Each column name maps to an object that knows how to calculate and format
#         the contents of an entry or cell in the corresponding column of the keysheet. Column classes are derived
#         from the class ColumnBase and are not intended to know how to format the sheet itself.
#
#         A key sheet can be filled with randomized machine settings that have been generated by applying the process
#         method of the column objects to a randomized machine setting. The settings of some machines do not fit
#         in a single table. They are therefore distributed over several appropriately named subsheets.
#
class Keysheet:
    ## \brief Constructor
    #
    #  \param [server] Is an rotorsim.tlvobject.TlvServer object. It is used to generate the rotorsim.RotorMachine and
    #         rotorrandom.RotorRandom needed by the fill() method.
    #
    #  \param [file_name_formatter] Is a callable object which allows to generate the file name of state files. Signature
    #         has to be file_name_formatter(dir_name, net_name, year, month, day). The file_name_formatter has to return
    #         a string.    
    #
    def __init__(self, server, file_name_formatter):
        ## \brief Contains the TLV server object specified in the constructor call.
        self._server = server
        ## \brief Is a string that contains the name of the machine type for which a key sheet is to be generated.
        self._machine_name = ''
        ## \brief Is an integer. Contains the year for which the key sheet is valid.
        self._year = 1900
        ## \brief Is an integer. Contains the month for which the key sheet is valid.        
        self._month = 1
        ## \brief Is a string. It contains the name of the crypto net for which the sheet is valid.
        self._net_name = ''
        ## \brief Is a string. It contains the classification level that is visible on the sheet.        
        self._classification = ''
        ## \brief Is a dictionary that maps strings (the column name) to ColumnBase objects that know        
        #         how to calculate values in this column.       
        self._column_mapping = {}
        ## \brief Is a vector of strings. It contains the column headers that appear on the key sheet. The strings
        #         in this vector have to be a subset of the keys of the self._column_mapping dictionary.
        self._columns = []
        ## \brief Is a vector of dictionaries. Each of the dictionaries maps strings (column name) to string values.
        #         There is such a dictionary for each of the 31 days of a month.
        self._settings = []
        ## \brief Is a vector of byte arrays. The bytes arrays represent states of rotorsim.RotorMachine objects as
        #         returned by the rotorsim.RotorMachine.get_state() method. From these objects the settings are 
        #         generated by the process methods of the column objects.
        self._machine_states = []
        ## \brief Is a string. Contains the name of the subsheet as it should appear on the sheet.
        self._subsheet = ''
        self._formatter = file_name_formatter

    ## \brief This property returns the year for which the sheet is valid.
    #
    #  \returns An integer (the year).
    #            
    @property
    def year(self):
        return self._year

    ## \brief This property setter allows to change the year for which the sheet is valid.
    #
    #  \param [new_val] An integer (the year).
    #                
    @year.setter
    def year(self, new_val):
        self._year = new_val

    ## \brief This property returns the name of the subsheet.
    #
    #  \returns A string.
    #            
    @property
    def subsheet(self):
        return self._subsheet

    ## \brief This property setter allows to change the subsheet name.
    #
    #  \param [new_val] A string.
    #                    
    @subsheet.setter
    def subsheet(self, new_val):
        self._subsheet = new_val

    ## \brief This property returns the month for which the sheet is valid.
    #
    #  \returns An integer.
    #            
    @property
    def month(self):
        return self._month

    ## \brief This property setter allows to change the month for which the sheet is valid.
    #
    #  \param [new_val] An integer (the month).
    #                    
    @month.setter
    def month(self, new_val):
        self._month = new_val

    ## \brief This property returns the name of the crypto net or key for which the sheet is valid.
    #
    #  \returns A string.
    #
    @property
    def net_name(self):
        return self._net_name

    ## \brief This property setter allows to change the name of the crypto net or key for which the sheet is valid.
    #
    #  \param [new_val] A string.
    #                
    @net_name.setter
    def net_name(self, new_val):
        self._net_name = new_val

    ## \brief This property returns the classification level of the sheet.
    #
    #  \returns A string.
    #                
    @property
    def classification(self):
        return self._classification

    ## \brief This property setter allows to change the classification level of the sheet.
    #
    #  \param [new_val] A string.
    #                    
    @classification.setter
    def classification(self, new_val):
        self._classification = new_val

    ## \brief This property returns the current mapping of column header names to ColumnBase objects.
    #
    #  \returns A dictionary that maps a string (the column name) to a ColumnBase object that knows how to
    #           claculate values in this column.
    #
    @property
    def column_mapping(self):
        return self._column_mapping

    ## \brief This property setter allows to change the mapping of column header names to ColumnBase objects.
    #
    #  \param [new_val] A dictionary that maps strings (the column name) to ColumnBase objects.
    #                    
    @column_mapping.setter
    def column_mapping(self, new_val):
        self._column_mapping = new_val

    ## \brief This property returns the list of column headers in the sequence as they should appear on the sheet.
    #
    #  \returns A vector of strings.
    #
    @property
    def columns(self):
        return self._columns

    ## \brief This property setter allows to change the column headers that should appear on the sheet.
    #
    #  \param [new_val] A vector of strings.
    #                        
    @columns.setter
    def columns(self, new_val):
        self._columns = new_val

    ## \brief This property returns the generated settings.
    #
    #  \returns A vector of dictionaries. For each day there is a dictionary that maps strings (the colum headers)
    #           to other strings (the values)
    #
    @property
    def settings(self):
        return self._settings

    ## \brief This property returns the machine states from which the key sheet contents has been derived.
    #
    #  \returns A vector of byte arrays. The byte arrays can be used to set the state of a rotorsim.RotorMachine object.
    #
    @property
    def machine_states(self):
        return self._machine_states

    ## \brief This method generates the settings for a single day from a rotorsim.RotorMachine object and appends these to
    #         self._settings. The machine state of the parameter machine also is appended to self._machine_states.
    #
    #  \param [random] Is a rotorrandom.RotorRandom object.
    #            
    #  \param [machine] Is a rotorsim.RotorMachine object. From this object the settings are derived,
    #
    #  \returns Nothing.
    #            
    def fill_helper(self, random, machine):
        current_config = machine.get_config()
        current_settings = {}
        current_state = machine.get_state()
        
        # Iterate over column names
        for j in self._columns:
            machine.set_state(current_state)
            # Determine value for column
            current_settings[j] = self._column_mapping[j].process(current_config, random, machine)
        
        self._settings.append(current_settings)
        self._machine_states.append(current_state)

    ## \brief This method saves all the machine states in self._machine_states in appropriately named files.
    #
    #  \param [file_name_prefix] Is a string. It sould specifiy a path to a directory including the '/' character.
    #         The machine states are saved into this directory. 
    #
    #  \returns A boolean. A return value of True means that an error occurred.
    #            
    def save_states(self, file_name_prefix):
        result = False
        count = len(self.settings)
        
        try:
            for i in self.machine_states:
                file_name = self._formatter(file_name_prefix, self.net_name, self.year, self.month, count)
                with open(file_name, 'wb') as file_out:
                    file_out.write(i)
                count -= 1
        except:
            result = True
        
        return result

    ## \brief This method generates the machine settings for a whole month consisting of 31 days. The machine states
    #         used for this are not randomly generated but taken from another Keysheet object.
    #  
    #  This is intended to be used when a machine requires several subsheets. other_sheet is the "main sheet" which
    #  has been filled with random machine states.
    #
    #  \param [other_sheet] Is a Keysheet object. The machine states stored in other_sheet.machine_states are recycled
    #         for generating the values for this sheet. 
    #
    #  \param [machine_state] Is a rotorsim.StateSpec() object.
    #
    #  \returns Nothing.
    #
    def fill_from_sheet(self, other_sheet, machine_state):        
        self._settings = []
        self._machine_states = []
        state_proc = rotorsim.StateHelper(self._server.address)
        
        with rotorsim.RotorMachine(state_proc.make_state(machine_state.name, machine_state.config, machine_state.rotor_pos), \
             self._server.address) as machine, rotorrandom.RotorRandom('abcdefghijklmnopqrstuvwxyz', self._server.address) as random: 
            self._machine_name = machine.get_description()
                                       
            for i in range(1, 32):
                machine.set_state(other_sheet.machine_states[i - 1])
                self.fill_helper(random, machine)
        
    ## \brief This method generates the machine settings for a whole month consisting of 31 days. The machine states
    #         used for this are randomly generated.
    #  
    #  \param [machine_state] Is an rotorsim.StateSpec() object.
    #
    #  \param [parameters] Is a string. This string is used as a randomizer parameter for rotorsim.RotorMachine.randomize_state().
    #
    #  \returns Nothing.
    #
    def fill(self, parameters, machine_state):        
        self._settings = []
        self._machine_states = []
        
        state_proc = rotorsim.StateHelper(self._server.address)
        
        with rotorsim.RotorMachine(state_proc.make_state(machine_state.name, machine_state.config, machine_state.rotor_pos), \
             self._server.address) as machine, rotorrandom.RotorRandom('abcdefghijklmnopqrstuvwxyz', self._server.address) as random: 
            self._machine_name = machine.get_description()
                                       
            for i in range(1, 32):
                machine.randomize_state(parameters)
                self.fill_helper(random, machine)


## \brief A class that abstracts a thing that knows how to transform a Keysheet object into a human
#         readable form. The rendered key sheet can either be in English or German.
#
class KeySheetRendererBase:
    ## \brief Constructor
    #
    def __init__(self):
        ## \brief Contains the months of the year in English.
        self._monate_deu = ['Januar', 'Februar', 'März', 'April', 'Mai', 'Juni', 'Juli', 'August', 'September', 'Oktober', 'November', 'Dezember']
        ## \brief Contains the months of the year in German.        
        self._monate_eng = ['January', 'February', 'March', 'April', 'May', 'June', 'July', 'August', 'September', 'October', 'November', 'December']
        ## \brief German version of 'for'.        
        self._for_deu = 'für'
        ## \brief English verison of 'for'        
        self._for_eng = 'for'
        ## \brief German version of 'Day'.
        self._day_deu = 'Tag'
        ## \brief English version of 'Day'.        
        self._day_eng = 'Day'
        ## \brief Language used in key sheet. Default is German.
        self.german = True
        ## \brief File extension used for saved sheets.        
        self._file_extension = ''

    ## \brief This property returns True if the key sheet language is German.
    #
    #  \returns A boolean.
    #                        
    @property
    def german(self):
        return self._german

    ## \brief This property returns the file extension that is to be used for saved rendered sheets.
    #
    #  \returns A string.
    #                        
    @property
    def file_extension(self):
        return self._file_extension

    ## \brief This property setter allows to change the file extension that is to be used for saved rendered sheets.
    #
    #  \param [new_value] A string. The new file extension without the '.' at the beginning.
    #                        
    @file_extension.setter
    def file_extension(self, new_value):
        self._file_extension = new_value

    ## \brief This property setter allows to change the language used in the sheet. This method also
    #         sets up the values self._for, self._monate and self._day which are actually used to render
    #         the sheet.
    #
    #  \param [new_value] A boolean.
    #                        
    @german.setter
    def german(self, new_value):
        self._german = new_value
        if self._german:
            self._for = self._for_deu
            self._monate = self._monate_deu
            self._day = self._day_deu
        else:
            self._for = self._for_eng
            self._monate = self._monate_eng
            self._day = self._day_eng

    ## \brief This method translates a number from the range 1..12 to the name of the corresponding
    #         month using the langauage defined in self.german.
    #
    #  \param [monat] Is an integer from the range 1..12.
    #
    #  \returns A string.
    #                        
    def get_month(self, monat):
        result = self._monate[0]
        
        if monat > 0:
            result = self._monate[(monat - 1) % 12]
        
        return result

    ## \brief This method renders a sheet or subsheet and writes the result to the file like object
    #         specified in parameter file_out.
    #
    #  \param [key_sheet] Is a Keysheet object. References the sheet that is to be rendered.
    #
    #  \param [file_out] Is a file like object. This object is used to store the generated output.
    #
    #  \returns Nothing.
    #                        
    def render_sheet(self, key_sheet, file_out):
        pass
    
    ## \brief This method begins to render a set of sheets by writing a document header to the file like object
    #         specified in parameter file_out.
    #
    #  \param [file_out] Is a file like object. This object is used to store the generated output.
    #
    #  \returns Nothing.
    #                        
    def render_start(self, file_out):
        pass

    ## \brief This method finishes to render a set of sheets by writing a document footer to the file like object
    #         specified in parameter file_out.
    #
    #  \param [file_out] Is a file like object. This object is used to store the generated output.
    #
    #  \returns Nothing.
    #                        
    def render_stop(self, file_out):
        pass


## \brief A class that abstracts a thing that knows how to transform a Keysheet object into a nicely formatted
#         text file.
#
class TextKeysheetRenderer(KeySheetRendererBase):
    ## \brief Constructor
    #
    def __init__(self):
        super().__init__()
        self.file_extension = 'txt'
        
    ## \brief This method can be used to append a number of blank characters to a given string in order to
    #         ensure that the returned string has a fixed length as specified by parameter length.
    #
    #  \param [value] Is a string. This is the string to be padded to the desired length.
    #
    #  \param [length] Is an integer. Specifies the length to which the string given as parameter value is
    #         to be padded.
    #
    #  \returns A string.
    #                            
    def format_column(self, value, length):
        result = value
        
        if len(value) < length:
            result += ' ' * (length - len(value))
        
        result += ' |'
        
        return result

    ## \brief This method can be used to append and prepend a number of blank characters to a given string in
    #         order to center that string inside a field of given length.
    #
    #  \param [width] Is an integer. Specifies the width in characters of the column.
    #
    #  \param [text] Is a string. This is the string to be centered in the column.
    #
    #  \returns A string.
    #                                    
    def center_text(self, width, text):
        result = ' ' * ((width // 2) - (len(text) // 2))
        return result + text
    
    ## \brief This method renders a sheet or subsheet as a nicely formatted text and writes the result to the file like object
    #         specified in parameter file_out.
    #
    #  \param [key_sheet] Is a Keysheet object. References the sheet that is to be rendered.
    #
    #  \param [file_out] Is a file like object. This object is used to store the generated output.
    #
    #  \returns Nothing.
    #                        
    def render_sheet(self, key_sheet, file_out):
        result = '\n'
        
        # Begin column header with day
        column_header = self.format_column(self._day, len(self._day))
        
        # Append remaining column headers
        for j in key_sheet.columns:
            col_width_temp = key_sheet.column_mapping[j].col_width
            column_header += self.format_column(self.center_text(col_width_temp, j), col_width_temp)
        
        column_header = '|' + column_header
        subsheet_name = ''
        
        # Determine subsheet name
        if key_sheet.subsheet != '':
            subsheet_name = '({})'.format(key_sheet.subsheet)
        
        # Add classification level, crypto net name and subsheet name to output data
        result += self.center_text(len(column_header), '{} {} {}'.format(key_sheet.classification, key_sheet.net_name, subsheet_name)) + '\n'
        # Add month and year to output data
        result += self.center_text(len(column_header), '{} {} {}'.format(self._for, self.get_month(key_sheet.month), key_sheet.year)) + '\n'       
        
        dashed_line = ('-' * len(column_header)) + '\n'
        
        # Add header line to output value
        result += dashed_line
        result += column_header + '\n'
        result += dashed_line
        
        # Add rows to out put data
        count = len(key_sheet.settings)
        for i in key_sheet.settings:        
            # Add day to row
            settings_string = '{:02d}'.format(count)
            settings_string = self.format_column(settings_string, len(self._day))
            
            # Add machine settings for the day            
            for j in key_sheet.columns:
                settings_string += self.format_column(i[j], key_sheet.column_mapping[j].col_width) 
            
            settings_string = '|' + settings_string
            result += settings_string + '\n'
            count -= 1
        
        result += dashed_line
        
        # Write output data to output file
        file_out.write(result)


## \brief A class that abstracts a thing that knows how to transform a Keysheet object into an HTML file.
#        
class HTMLKeysheetRenderer(KeySheetRendererBase):
    ## \brief Constructor
    #
    def __init__(self):
        super().__init__()
        self.file_extension = 'html'
        
    ## \brief This method renders a sheet or subsheet as an HTML document and writes the result to the file like object
    #         specified in parameter file_out.
    #
    #  \param [key_sheet] Is a Keysheet object. References the sheet that is to be rendered.
    #
    #  \param [file_out] Is a file like object. This object is used to store the generated output.
    #
    #  \returns Nothing.
    #                        
    def render_sheet(self, key_sheet, file_out):
        subsheet_name = ''
        
        if key_sheet.subsheet != '':
            subsheet_name = '({})'.format(key_sheet.subsheet)
        
        # Write table header
        file_out.write('<table>\n')

        # Output classification level, crypto net name and subsheet name    
        file_out.write('<caption><h4>{} {} {}</h4>\n'.format(key_sheet.classification, key_sheet.net_name, subsheet_name))
        # Output month and year
        file_out.write('<h4>{} {} {}</h4></caption>\n'.format(self._for, self.get_month(key_sheet.month), key_sheet.year))

        file_out.write('<tr>\n')
        file_out.write('<th>{}</th>\n'.format(self._day))    
            
        for i in key_sheet.columns:
            file_out.write('<th>{}</th>\n'.format(i))
        
        # End table header
        file_out.write('</tr>\n')
        
        count = len(key_sheet.settings)
        
        # Write a table row for each day
        for i in key_sheet.settings:
            file_out.write('<tr>\n')            
            row = '<td>{}</td>\n'.format(count)
            for j in key_sheet.columns:
                row += '<td>{}</td>'.format(i[j])
            
            count -= 1
            file_out.write('\n' + row)
            file_out.write('</tr>\n')    

        file_out.write('</table>\n')        
    
    ## \brief This method writes the HTML header to the file like object specified in parameter file_out.
    #
    #  \param [file_out] Is a file like object. This object is used to store the generated output.
    #
    #  \returns Nothing.
    #                        
    def render_start(self, file_out):
        file_out.write('<!doctype html>\n<html>\n<head>\n<meta charset="utf-8">\n')
        file_out.write('<style>\n')
        file_out.write('table, td, th { border: 1px solid black; }\n')
        file_out.write('table {border-collapse: collapse;}\n')
        file_out.write('th, td {padding: 3px;}\n')
        file_out.write('th, td {font-family: "Courier New", Courier, monospace;}\n')
        file_out.write('th, td {font-size: small;}\n')
        file_out.write('</style>\n')
        file_out.write('</head>\n<body>\n')

    ## \brief This method finishes the rendering by wiriting the closing body and html tags to the
    #         file like object specified in parameter file_out.
    #
    #  \param [file_out] Is a file like object. This object is used to store the generated output.
    #
    #  \returns Nothing.
    #                        
    def render_stop(self, file_out):
        file_out.write('</body>\n</html>\n')


## \brief A class that knows how to control a renderer in order to a create a key sheet for a specific year and month.
#        
class RenderController:
    ## \brief Constructor
    #
    #  \param [serv] Is an rotorsim.tlvobject.TlvServer object. It is used to generate the rotorsim.RotorMachine and
    #         rotorrandom.RotorRandom needed by the fill() method.
    #
    #  \param [machine_type] Is a string. It specifies the type of machine for which key sheets are to be generated.
    #
    #  \param [net_name] Is a string. It specifies the name of the crypto net or key for which the generated sheets
    #         are valid.
    #
    #  \param [classification] Is a string. It specifies the classification level of the generated key sheets.    
    #
    #  \param [msg_proc] Is a string. It specifies the message procedure in use with the sheet.    
    #
    #  \param [rotor_set_file] Is a string. It specifies the name of the rotor set file to load before generating
    #         the sheets. Has to be '' if no sheet ist to be loaded.
    #
    #  \param [formatter] Is a callable object which allows to generate the file name of state files. Signature
    #         has to be formatter(dir_name, net_name, year, month, day). The formatter has to return a string.
    #    
    def __init__(self, serv, machine_type, net_name, classification, msg_proc, rotor_set_file, formatter):
        self._server = serv
        self._machine_type = machine_type
        self._net_name = net_name
        self._classification = classification
        self._renderer = TextKeysheetRenderer()
        self._formatter = formatter
        self._msg_proc = msg_proc
        self._rotor_set_file_name = rotor_set_file

    ## \brief This property returns the renderer object which is used to generate the key sheet.
    #
    #  \returns An object of type KeySheetRendererBase.
    #    
    @property    
    def renderer(self):
        return self._renderer

    ## \brief This property setter allows to change the renderer object which is used to generate the key sheet.
    #
    #  \param [new_val] An object of type KeySheetRendererBase.
    #        
    @renderer.setter
    def renderer(self, new_val):
        self._renderer = new_val

    ## \brief This method generates a key sheet and writes it to the specified output object.
    #
    #  \param [year] Is an integer. Specifies the year which is to appear on the sheet.
    #
    #  \param [month] Is an integer. Specifies the month which is to appear on the sheet.
    #
    #  \param [out_file] Is a file like object. Output is written to this object.
    #
    #  \param [state_file_prefix] Is a string or None. Specifies the file name prefix which is used to save the
    #         machine states that have been generated for the key sheet. If this value is None then state files are
    #         not written.
    #
    #  \returns Nothing.
    #                                
    def generate_sheet(self, year, month, out_file, state_file_prefix = None):        
        state_and_param = self.configure_key_sheet(self._server, self._machine_type, year, month, self._net_name, self._classification, self._msg_proc)

        if state_and_param['state'] != None:
            try:
                # Load custom rotor set before generating sheets
                self.modify_rotor_set(state_and_param['state'], state_and_param['rotorsetname'], self._rotor_set_file_name)
                
                self._renderer.render_start(out_file)                
                
                # Fill main sheet
                main_sheet = state_and_param['sheets'][0]
                main_sheet.fill(state_and_param['randparm'], state_and_param['state'])
                self._renderer.german = state_and_param['isgerman']                                        
                            
                # Render main sheet
                self._renderer.render_sheet(main_sheet, out_file)
                
                # Optionally save machine states of main sheet
                if state_file_prefix != None:
                    if main_sheet.save_states(state_file_prefix):
                        raise KeysheetException('Unable to save state files')
                
                # Iterate over subsheets
                for i in state_and_param['sheets'][1:]:
                    # Fill subsheet from main sheet
                    i.fill_from_sheet(main_sheet, state_and_param['state'])
                    self._renderer.german = state_and_param['isgerman']
                    self._renderer.render_sheet(i, out_file)                                
            finally:        
                # End rendering
                self._renderer.render_stop(out_file)                            
        else:
            raise KeysheetException('Unknown machine name: {}'.format(self._machine_type))
    

    ## \brief This method loads a custom rotor set if this was requested by the user.
    #
    #  \param [machine_state] Is a byte array. It has to contain a machine state usable by RotorMachine.set_state()
    #
    #  \param [rotor_set_name] Is a string. It has to contain the name of the rotor set to modify. This value has to
    #         appear in the list of rotor set names returned by the RotorMachine.get_rotor_set_names() method for
    #         the machine reconstructed from the state given in parameter machine_state.
    #
    #  \param [rotor_set_file_name] Is a string. Has to contain the name of the file that contains the rotor set
    #         data to load. If this parameter equals '' this method does nothing.
    #
    #  \returns Nothing
    #                        
    def modify_rotor_set(self, machine_state, rotor_set_name, rotor_set_file_name):
        try:
            if rotor_set_file_name != '':
                with open(rotor_set_file_name, 'rb') as f:
                    rotor_set_data = f.read()
                    f.close()            
                
                with rotorsim.RotorMachine.from_machine_spec(machine_state, self._server.address) as machine:
                    machine.set_rotor_set_state(rotor_set_name, rotor_set_data)
        except:
            raise KeysheetException('Unable to load rotor set {}'.format(rotor_set_file_name))

    ## \brief This method uses the machine name and other parameters specified on the command line to generate
    #         an appropriately configured Keysheet object.
    #
    #  \param [tlv_server] Is a rotorsim.tlvobject.TlvServer object.
    #
    #  \param [machine_name] Is a string. This value determines the columns present in the key sheet.
    #
    #  \param [year] Is an integer. Specifies the year for which the sheet is valid.
    #
    #  \param [month] Is an integer. Specifies the month for which the sheet is valid.
    #
    #  \param [net_name] Is a string. Specifies the name of the crypto net or key for which the sheet is valid.
    #
    #  \param [classification] Is a string. Specifies the classification level of the sheet.
    #        
    #  \param [msg_proc_type] Is a string. Specifies the message procedure used with the sheet.
    #        
    #  \returns A dictionary containing the string keys:
    #           'state': Maps to a rotorsim.StateSpec() object that represents the default state for
    #                    the given machine type.
    #           'randparam': Maps to a string object that serves as a randomizer parameter in Keysheet.fill().
    #           'isgerman': Maps to a boolean that is True if the language on the sheet is German.
    #           'sheets': Maps to a vector of Keysheet objects. The first object is the "main sheet".
    #           'rotorsetname': Specifies the name of the rotor set that is to be used as a string. 
    #                        
    def configure_key_sheet(self, tlv_server, machine_name, year, month, net_name, classification, msg_proc_type = ''):
        result = {'state':None, 'randparm':'', 'isgerman':True, 'sheets':[], 'rotorsetname':'defaultset'}
        keysheet = Keysheet(tlv_server, self._formatter)
        
        keysheet.year = year
        keysheet.month = month
        keysheet.net_name = net_name
        keysheet.classification = classification        
        
        # Generic columns for an Enigma key sheet
        keysheet.column_mapping = {'Walzenlage':RotorColumn(20, 'rotors'), 'Ringstellung':RingColumn(12, 'rings'), \
                                   'Kenngruppen':KenngruppenColumn(15, 4), 'Umkehrwalze D':PlugsColumn(35, 'ukwdperm'), \
                                   'Steckerbrett':PlugsColumn(29, 'plugs'), 'Uhr':UhrDialColumn(3, 'plugs'), \
                                   'Grundst.':RotorPosColumn(8), 'System':RandStringColumn(6, 5)}
    
        if (machine_name == 'M3') or (machine_name == 'Services'): # M3 and Services
            # Columns to include
            if msg_proc_type == 'grundstellung':            
                keysheet.columns = ['Walzenlage', 'Ringstellung', 'Steckerbrett', 'Grundst.', 'System']
            else:
                keysheet.columns = ['Walzenlage', 'Ringstellung', 'Steckerbrett', 'Kenngruppen']                
                if msg_proc_type == 'pre1940':
                    keysheet.columns.append('Grundst.')
                
            
            if machine_name == 'M3':
                keysheet.column_mapping['Walzenlage'].col_width = 14
            else:
                keysheet.column_mapping['Walzenlage'].col_width = 11
            
            result['state'] = rotorsim.ServicesEnigmaState.get_default_config(machine_name)
            result['randparm'] = 'basic'
            
        elif (machine_name == 'M3D') or (machine_name == 'ServicesD'): # M3 and Services with Umkehrwalze D
            # Columns to include
            if msg_proc_type == 'grundstellung':            
                keysheet.columns = ['Walzenlage', 'Ringstellung', 'Steckerbrett', 'Umkehrwalze D', 'Grundst.', 'System']
            else:
                keysheet.columns = ['Walzenlage', 'Ringstellung', 'Steckerbrett', 'Umkehrwalze D', 'Kenngruppen']
                if msg_proc_type == 'pre1940':
                    keysheet.columns.append('Grundst.')                

            if machine_name == 'M3D':
                keysheet.column_mapping['Walzenlage'].col_width = 14
            else:
                keysheet.column_mapping['Walzenlage'].col_width = 11

            result['state'] = rotorsim.ServicesEnigmaState.get_default_config(machine_name[:-1])
            result['randparm'] = 'ukwdonly'
            
        elif machine_name == 'ServicesUhr': # Services with Uhr and Umkehrwalze D
            # Columns to include
            if msg_proc_type == 'grundstellung':            
                keysheet.columns = ['Walzenlage', 'Ringstellung', 'Steckerbrett', 'Uhr', 'Umkehrwalze D', 'Grundst.', 'System']
            else:
                keysheet.columns = ['Walzenlage', 'Ringstellung', 'Steckerbrett', 'Uhr', 'Umkehrwalze D', 'Kenngruppen']
                if msg_proc_type == 'pre1940':
                    keysheet.columns.append('Grundst.')                                   
        
            keysheet.column_mapping['Walzenlage'].col_width = 11
        
            result['state'] = rotorsim.ServicesEnigmaState.get_default_config('Services')
            result['randparm'] = 'fancy'
            
        elif machine_name == 'M4': # Enigma M4 without Kenngruppen
            # Columns to include
            if msg_proc_type == 'grundstellung':            
                keysheet.columns = ['Walzenlage', 'Ringstellung', 'Steckerbrett', 'Grundst.', 'System']
            else:
                keysheet.columns = ['Walzenlage', 'Ringstellung', 'Steckerbrett', 'Kenngruppen'] 
                if msg_proc_type == 'pre1940':
                    keysheet.columns.append('Grundst.')                                                                               
            
            result['state'] = rotorsim.M4EnigmaState.get_default_config()
            result['randparm'] = 'egal'    
            
        elif (machine_name == 'Railway') or (machine_name == 'Abwehr'): # Railway and Abwehr Enigma
            # Columns to include            
            if msg_proc_type == 'grundstellung':            
                keysheet.columns = ['Walzenlage', 'Ringstellung', 'Grundst.', 'System']
            else:
                keysheet.columns = ['Walzenlage', 'Ringstellung', 'Kenngruppen']
                if msg_proc_type == 'pre1940':
                    keysheet.columns.append('Grundst.')                                                         
            
            keysheet.column_mapping['Walzenlage'].col_width = 10            
            
            result['state'] = rotorsim.UnsteckeredEnigmaState.get_default_config(machine_name + 'Enigma')
            result['randparm'] = 'egal'
            
        elif machine_name == 'KD': # KD Enigma
            # Columns to include            
            if msg_proc_type == 'grundstellung':            
                keysheet.columns = ['Walzenlage', 'Ringstellung', 'Umkehrwalze D', 'Grundst.', 'System']
            else:
                keysheet.columns = ['Walzenlage', 'Ringstellung', 'Umkehrwalze D', 'Kenngruppen'] 
                if msg_proc_type == 'pre1940':
                    keysheet.columns.append('Grundst.')                                                                                
            
            keysheet.column_mapping['Walzenlage'].col_width = 10
            
            result['state'] = rotorsim.UnsteckeredEnigmaState.get_default_config(machine_name + 'Enigma')
            result['randparm'] = 'ukwdonly'
            
        elif machine_name == 'Tirpitz': # Tirpitz Enigma
            # Columns to include
            if msg_proc_type == 'grundstellung':            
                keysheet.columns = ['Walzenlage', 'Ringstellung', 'Grundst.', 'System']
            else:
                keysheet.columns = ['Walzenlage', 'Ringstellung', 'Kenngruppen']  
                if msg_proc_type == 'pre1940':
                    keysheet.columns.append('Grundst.')                                                                                                       
            
            keysheet.column_mapping['Walzenlage'].col_width = 12
            
            result['state'] = rotorsim.UnsteckeredEnigmaState.get_default_config(machine_name + 'Enigma')
            result['randparm'] = 'egal'
            
        elif (machine_name == 'Typex') or (machine_name == 'TypexY269')  or (machine_name == 'TypexPlugsY2695'): # Typex
            # Set up column mapping            
            keysheet.column_mapping = {'Wheel settings':PlugsColumn(14, 'rotors'), 'Rings':Column(5, 'rings'), \
                                       'Reflector':Column(26, 'reflector'), 'Plugboard':Column(26, 'plugs'), \
                                       'Basic':RandStringColumn(5, 5), 'System':RandStringColumn(6, 5), 'ID Groups':KenngruppenColumn(15, 4)}
                                       
            keysheet.column_mapping['Wheel settings'].uppercase = False
            keysheet.column_mapping['Rings'].uppercase = False
            keysheet.column_mapping['Reflector'].uppercase = False
            
            # Columns to include
            keysheet.columns = ['Wheel settings', 'Rings', 'Reflector']
            
            if machine_name == 'TypexPlugsY2695':
                keysheet.columns.append('Plugboard')
                keysheet.column_mapping['Plugboard'].uppercase = False
            
            if msg_proc_type == 'pre1940':
                keysheet.columns.append('Basic')            
                keysheet.columns.append('ID Groups')            
            elif msg_proc_type == 'post1940':
                keysheet.columns.append('ID Groups')
            else: 
                keysheet.columns.append('Basic')
                keysheet.columns.append('System')
            
            result['isgerman'] = False
            result['state'] = rotorsim.TypexState.get_default_config()
            
            if machine_name == 'Typex':            
                result['randparm'] = 'sp02390'
            else:                
                if machine_name == 'TypexPlugsY2695':
                    result['rotorsetname'] = 'Y2695'
                    result['randparm'] = 'plugsy2695'
                else:
                    result['rotorsetname'] = 'Y269'
                    result['randparm'] = 'y269'
                               
        elif (machine_name == 'NemaWar') or ((machine_name == 'NemaTraining')): # Nema war and training models
            # Set up column mapping            
            keysheet.column_mapping = {'Walzen':Column(6, 'rotors'), 'Nockenringe':Column(11, 'ringselect'), 'Codewort':RotorPosColumn(10), 'System':RandStringColumn(6, 5)}
            
            keysheet.column_mapping['Walzen'].uppercase = False
            
            # Columns to include
            keysheet.columns = ['Walzen', 'Nockenringe', 'Codewort', 'System']
            
            result['state'] = rotorsim.NemaState.get_default_config()
            result['randparm'] = machine_name[4:].lower()
                                    
        elif (machine_name == 'CSP889') or (machine_name == 'CSP2900'): # SIGABA CSP889 and CSP2900 models
            # Set up column mapping
            keysheet.column_mapping = {'Index Rotors':PlugsColumn(14, 'index'), 'Control Rotors':PlugsColumn(14, 'control'),\
                                       'Cipher Rotors':PlugsColumn(14, 'cipher'), 'Index Pos':SIGABAIndexRotorPosColumn(9),\
                                       '26-30 Check':CheckStringColumn(11, 5, lambda x:x[0:5]+'oooooooooo', 'a' * 30), \
                                       'Initial':RandStringColumn(7, 5), 'System':RandStringColumn(6, 5)} 
                                       
            keysheet.column_mapping['Index Rotors'].uppercase = False
            keysheet.column_mapping['Control Rotors'].uppercase = False
            keysheet.column_mapping['Cipher Rotors'].uppercase = False
            
            # Columns to include
            if msg_proc_type == 'grundstellung':            
                keysheet.columns = ['Index Rotors', 'Control Rotors', 'Cipher Rotors', 'Index Pos', 'Initial', 'System', '26-30 Check']
            else:
                keysheet.columns = ['Index Rotors', 'Control Rotors', 'Cipher Rotors', 'Index Pos', 'System', '26-30 Check']
            
            result['isgerman'] = False            
            result['state'] = rotorsim.SigabaMachineState.get_default_config()
            result['state'].csp_2900_flag = (machine_name == 'CSP2900')
            result['randparm'] = 'egal'
            
        elif machine_name == 'KL7': # KL7
            # Set up column mapping
            keysheet.column_mapping = {'Rotors':Column(8, 'rotors'), 'Alphabet Ring Pos':Column(23, 'alpharings'),\
                                       'Notch Rings':Column(15, 'notchselect'), 'Notch Ring Pos':Column(14, 'notchrings'),\
                                       'Basic Alignment':KL7RotorPosColumn(15), '36-45 Check':CheckStringColumn(11, 10, lambda x:'aaaaaaa', 'l' * 45, True), \
                                       'System':RandStringColumn(6, 5)}         
               
            keysheet.column_mapping['Notch Ring Pos'].uppercase = False
            keysheet.column_mapping['Rotors'].uppercase = False
            
            # Columns to include
            keysheet.columns = ['Rotors', 'Alphabet Ring Pos', 'Notch Rings', 'Notch Ring Pos', 'Basic Alignment', 'System', '36-45 Check']
            
            result['isgerman'] = False
            result['state'] = rotorsim.KL7State.get_default_config()
            result['randparm'] = 'egal'
            
        elif machine_name == 'SG39': # SG39
            # SG39 key sheets have two subsheets (Walzen and Stifte)
            # Walzen subsheet
            keysheet.subsheet = 'Walzen'
            
            # Set up column mapping for Walzen subheet
            keysheet.column_mapping = {'Walzen':Column(6, 'rotors'), 'Ringe':Column(5, 'rings'),\
                                       'Steckerbrett':Column(26, 'plugs'), 'Umkehrwalze':Column(26, 'reflector'),
                                       'System':RandStringColumn(6, 5), 'Grundst.':RotorPosColumn(8)}
                                       
            keysheet.column_mapping['Steckerbrett'].uppercase = False
            keysheet.column_mapping['Umkehrwalze'].uppercase = False
            keysheet.column_mapping['Ringe'].uppercase = False
            
            # Columns to include in Walzen subsheet
            keysheet.columns = ['Walzen', 'Ringe', 'Steckerbrett', 'Umkehrwalze', 'Grundst.', 'System']
        
            result['sheets'].append(keysheet)
            
            # Stifte subsheet            
            keysheet = Keysheet(tlv_server, self._formatter)
            
            keysheet.year = year
            keysheet.month = month
            keysheet.net_name = net_name
            keysheet.classification = classification
            keysheet.subsheet = 'Stifte'
            
            # Set up column mapping for Stifte subsheet
            keysheet.column_mapping = {'Rad Eins':Column(21, 'pinswheel1'), 'Rad Zwei':Column(23, 'pinswheel2'),'Rad Drei':Column(25, 'pinswheel3')}
            
            # Columns to include in Stifte subsheet
            keysheet.columns = ['Rad Eins', 'Rad Zwei', 'Rad Drei']
            
            for i in keysheet.columns:
                keysheet.column_mapping[i].uppercase = False
            
            result['state'] = rotorsim.SG39State.get_default_config()
            result['randparm'] = 'egal'
        
        result['sheets'].append(keysheet)
                
        return result


## \brief A class that serves as a base class for a thing that knows how to report certain events to the user. The base
#         class is very simple but this may get more complex when called from a GUI program.
# 
class ReporterBase:
    ## \brief This method reports an error to the user.
    #
    #  \param [message] Is a string. Holds the message which is intended to be displayed to the user.
    #
    #  \returns Nothing.
    #
    def report_error(self, message):
        print(message)

    ## \brief This method reports a progress to the user.
    #
    #  \param [message] Is a string. Holds the message which is intended to be displayed to the user.
    #
    #  \returns Nothing.
    #
    def report_progress(self, message):
        print(message)

    ## \brief This method can be used to signal that processing has been finished.
    #
    #  \returns Nothing.
    #    
    def all_done(self):
        pass


## \brief A class that implements the main program for the key sheet generator.
#        
class KeysheetGeneratorMain:    
        
    ## \brief This method checks whether a given string represents a nonegative integer and raises an appropriate
    #         exception if it is not.
    #  
    #  This is used in the command line parser.
    #
    #  \param [year] Is a string. Value to check.
    #
    #  \returns An integer. The returned value is the the parameter year as an integer.
    #
    @staticmethod
    def check_year(year):
        value = int(year)
        
        if value < 0:
            message = 'Year has to be >= 0'
            raise argparse.ArgumentTypeError(message)
        
        return value

    ## \brief This method checks whether the file like object is a "real" file and if yes closes it.
    #  
    #  \param [out_file] Is a file like object.
    #
    #  \returns Nothing.
    #
    @staticmethod
    def check_close(out_file):
        if (out_file != sys.stdout) and (out_file != None):
            out_file.close()                            

    ## \brief This method generates the file name which is used to save a key sheet.
    #  
    #  \param [dir_name] Is a string. It has to contain the path of the directory in which the sheet is to be stored.
    #
    #  \param [net_name] Is a string. It has to contain the name of the key or crypto net.
    #
    #  \param [year] Is an integer. It has to contain the year for which the sheet is to be generated.
    #
    #  \param [month] Is an integer. It has to contain the month for which the sheet is to be generated.
    #   
    #  \param [extension] Is a string. It has to contain the file extension which is to be used to save the sheet.
    #     
    #  \returns A string. The file name.
    #
    @staticmethod
    def format_sheet_name(dir_name, net_name, year, month, extension):
        if not dir_name.endswith('/'):
            dir_name = dir_name + '/'
            
        return '{}{}_{}.{}'.format(dir_name, year, month, extension)

    ## \brief This method generates the file name which is used to save a machine state.
    #  
    #  \param [dir_name] Is a string. It has to contain the path of the directory in which the state file is to be stored.
    #
    #  \param [net_name] Is a string. It has to contain the name of the key or crypto net.
    #
    #  \param [year] Is an integer. It has to contain the year for which the state file is to be saved.
    #
    #  \param [month] Is an integer. It has to contain the month for which the state file is to be saved.
    #   
    #  \param [day] Is an integer. It has to contain the day number in the month for which the state file is to be saved.
    #     
    #  \returns A string. The file name.
    #
    @staticmethod
    def format_state_name(dir_name, net_name, year, month, day):
        if not dir_name.endswith('/'):
            dir_name = dir_name + '/'
            
        return '{}{}_{}_{}.ini'.format(dir_name, year, month, day)

    ## \brief This method can be used to test whether a given machine can be used with the specified message procedure.
    #  
    #  \param [machine_name] Is a string. It specifies the name of the machine (See MACHINE_NAMES).
    #
    #  \param [msg_proc_type] Is a string. It specifies the name of the message procedure (See PROC_TYPES).
    #
    #  \returns Nothing. Throws exception if it fails.
    #
    @staticmethod
    def verify_proc_type(machine_name, msg_proc_type):
        if msg_proc_type == 'grundstellung':
            if machine_name not in MACHINE_NAMES:
                raise KeysheetException("Cannot use " + machine_name + " with " + msg_proc_type + " message procedure")
        elif msg_proc_type == 'post1940':
            if machine_name not in ['M3', 'Services', 'M3D', 'ServicesD', 'ServicesUhr', 'M4', 'Railway', 'Abwehr', 'KD', 'Tirpitz', 'Typex', 'TypexY269', 'TypexPlugsY2695']:
                raise KeysheetException("Cannot use " + machine_name + " with " + msg_proc_type + " message procedure")       
        elif msg_proc_type == 'pre1940':
            if machine_name not in ['M3', 'Services', 'M3D', 'ServicesD', 'ServicesUhr', 'M4', 'Railway', 'Abwehr', 'KD', 'Tirpitz', 'Typex', 'TypexY269', 'TypexPlugsY2695']:
                raise KeysheetException("Cannot use " + machine_name + " with " + msg_proc_type + " message procedure")       
        elif msg_proc_type == 'sigaba':
            if machine_name not in ['CSP889', 'CSP2900']:
                raise KeysheetException("Cannot use " + machine_name + " with " + msg_proc_type + " message procedure")       
        else:
            raise KeysheetException("Unknown message procedure " + msg_proc_type)

    ## \brief This method brings together all puzzle pieces in order to actually generate the key sheets.
    #  
    #  \param [args] Is an object having the following attributes:
    #         type           A string. The machine type.
    #         year           An integer. The year for which the sheets are valid.
    #         month          An integer or None. Specifies the month for which the sheet is to be valid. None means valid for a whole year.
    #         classification A string. Classification level which appears on sheet.
    #         net            A string. Key or crypto net name wjich appears on sheet.
    #         save_states    A boolean or None. Is True of state file are to be saved.
    #         out            A string or None. Output directory for all sheet or state files.
    #         html           A boolean. True if HTML output is to be generated.
    #         tlv_server     A string. Full path of the tlv_server binary.
    #         msg_proc_type  A string. Specifies the message procedure for which this sheet is intended. 
    #         load_set       A string. Specifies the name of the rotor set to load. Use '' to not load a rotor set.
    #
    #  \param [reporter] An object with the same interface as ReporterBase.
    #
    #  \returns Nothing.
    #
    @staticmethod
    def generate_sheets(args, reporter):
        try:            
            KeysheetGeneratorMain.verify_proc_type(args.type, args.msg_proc_type)
            
            out = sys.stdout
            renderer = None            
            
            if args.html:
                renderer = HTMLKeysheetRenderer()
            else:
                renderer = TextKeysheetRenderer()
                
            save_state_dir = None
            
            if args.save_states:
                if args.out != None:
                    save_state_dir = args.out
                else:
                    reporter.report_error("No output directory specified!")
                    reporter.all_done()
                    return            
                                                
            with rotorsim.tlvobject.TlvServer(binary = args.tlv_server) as serv:
                
                ctrl = RenderController(serv, args.type, args.net, args.classification, args.msg_proc_type, args.load_set, KeysheetGeneratorMain.format_state_name)
                ctrl.renderer = renderer

                try:                                
                    if args.month != None:
                        # Generate keyheet for a single month
                        
                        if args.out != None:
                            out = open(KeysheetGeneratorMain.format_sheet_name(args.out, args.net, args.year, args.month, renderer.file_extension), 'w')
                                        
                        ctrl.generate_sheet(args.year, args.month, out, save_state_dir)
                    else:
                        # Generate keyheets for a whole year

                        helper = KeySheetRendererBase()
                        helper.german = False

                        for i in range(12):                            
                            if args.out != None:
                                out = open(KeysheetGeneratorMain.format_sheet_name(args.out, args.net, args.year, i + 1, renderer.file_extension), 'w')
                                reporter.report_progress("Generating keysheet for: {}".format(helper.get_month(i + 1)))
                                                        
                            ctrl.generate_sheet(args.year, i + 1, out, save_state_dir)
                            KeysheetGeneratorMain.check_close(out)
                            
                        out = None # It makes no sense to close out a second time in finally clause
                finally:
                    KeysheetGeneratorMain.check_close(out)

        except KeysheetException as e:
            reporter.report_error('Unable to generate keysheet: {}'.format(e))        
            #reporter.report_error(str(e) + traceback.format_exc(10))            
        except rotorsim.tlvobject.TlvException as e:
            reporter.report_error('Problem talking to TLV server: {}'.format(e))
        except IOError as e:
            reporter.report_error('Problem opening or writing to output file: {}'.format(e))
        except OSError as e:
            reporter.report_error('Operating system error: {}'.format(e))
        except Exception as e:
            reporter.report_error('Unable to generate keysheet. {}'.format(e))                
            #reporter.report_error(str(e) + traceback.format_exc(10))
        
        reporter.all_done()
    

