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

#!/usr/bin/env python3
## @package enigproc Implements a command line application that allows to en- and decrypt a message following one of several implemented message procedures.
#         
#           
# \file enigproc.py
# \brief Implements a command line application that allows to en- and decrypt a message following one of several implemented message procedures. 
#        

import sys
import pyrmsk2.tlvsrvapp as tlvsrvapp
import pyrmsk2
import argparse
import re
from pyrmsk2.keysheetgen import PROC_TYPES
from pyrmsk2 import EnigmaException as EnigmaException
import pyrmsk2.msgprocedure as msgprocedure

## \brief Commands that are understood by this program. 
COMMANDS = ['encrypt', 'decrypt']
## \brief Dummy value for the system indicator used when instantiating a MessageProcedure object for decryption.
DUMMY_SYS_INDICATOR = 'ert zui ops cfg'

GRUND_DEFAULT = ''


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
    #  \returns A dictionary containing the keys 'in_file', 'out_file', 'config_file', 'sys_indicator', 'doencrypt', 'grundstellung',
    #           'msg_proc_type' and 'use_modern_encoder'.
    #        
    def parse_args(self, argv):
        # Set up command line parser        
        indicator_help = "System indicator to use. In case the system indicator is a Kenngruppe it has to contain several (four) three letter strings seperated by blanks."        
        parser = argparse.ArgumentParser(description='enigproc.py ' + pyrmsk2.get_version_string() +
                                         '. A program that allows to en- and decrypt messages using rotor machines and one of serveral message procedures.',
                                         epilog='Example: enigproc.py encrypt -f state.ini -i input.txt -s "dff gtr lki vfd" -t post1940')
        parser.add_argument("command", choices=COMMANDS, help="Action to take. Encrypt or decrypt.")
        parser.add_argument("-i", "--in-file", required=False, default='', help="Input file containing plaintext or ciphertext. If missing data is read from stdin.")
        parser.add_argument("-o", "--out-file", default='-', help="Store output in file named by this parameter. Print to stdout if not specified.")
        parser.add_argument("-f", "--config-file", required=True, help="Machine state (as created for instance by rotorstate) to use.")
        parser.add_argument("-s", "--sys-indicator", default='', help=indicator_help)
        parser.add_argument("-g", "--grundstellung", default=GRUND_DEFAULT, help="A basic setting or grundstellung if required by the messaging procedure")
        parser.add_argument("-t", "--msg-proc-type", required=True, choices=PROC_TYPES, help="Type of messaging procedure")
        parser.add_argument("-m", "--modern-encoder", required=False, action="store_true", default=False, help="Use modern encoder.")        
        
        # Calls sys.exit() when command line can not be parsed or when --help is requested
        args = parser.parse_args()
        result =  {'in_file': args.in_file, 'out_file': args.out_file, 'config_file': args.config_file, 'sys_indicator':args.sys_indicator, 'doencrypt':args.command != COMMANDS[1]}
        result['grundstellung'] = args.grundstellung.lower()
        result['msg_proc_type'] = args.msg_proc_type
        result['use_modern_encoder'] = args.modern_encoder
                        
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
        # Output all but the last part
        for i in formatted_parts[:-1]:
            out_file.write(i)
            out_file.write('\n\n') # Separate parts by two empty lines
        
        # Output last part        
        last_entry = formatted_parts[-1:]        
        if len(last_entry) != 0:
            out_file.write(last_entry[0])
            out_file.write('\n') # Use only one LF in last line      

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
        factory = msgprocedure.MessageProcedureFactory(self.machine, self.random, self.server)
        
        if proc_type == 'post1940':
            if machine_name in ['Enigma', 'M3', 'KDEnigma']:
                return factory.get_post1940_enigma(sys_indicator, grundstellung)
            elif machine_name in ['AbwehrEnigma', 'TirpitzEnigma', 'M4Enigma', 'RailwayEnigma']:
                return factory.get_post1940_4wheel_enigma(sys_indicator, grundstellung)
            elif machine_name == 'Typex':
                return factory.get_post1940_typex(sys_indicator, grundstellung)                
            else:
                raise EnigmaException('Unsupported message procedure for machine type')
        elif proc_type == 'pre1940':
            if grundstellung == GRUND_DEFAULT:
                raise EnigmaException('Grundstellung missing. Add -g/--grundstellung option.')
        
            if machine_name in ['Enigma', 'M3', 'KDEnigma']:
                return factory.get_pre1940_enigma(sys_indicator, grundstellung)
            elif machine_name in ['AbwehrEnigma', 'TirpitzEnigma', 'M4Enigma', 'RailwayEnigma']:
                return factory.get_pre1940_4wheel_enigma(sys_indicator, grundstellung)
            elif machine_name == 'Typex':
                return factory.get_pre1940_typex(sys_indicator, grundstellung)                
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
        if args['in_file'] != '':
            with open(args['in_file'], 'r') as f_in:
                text = f_in.read()
        else:
            text = sys.stdin.read()                
        
        if do_encrypt:
            # Perform encryption
            if args['sys_indicator'] == '':
                raise EnigmaException('A system indicator has to be provided via the -s/--sys-indicator option')
                
            enigma_proc = self._generate_msg_proc_obj(self.machine.get_description(), args['sys_indicator'], args['grundstellung'], args['msg_proc_type'])
            
            if args['use_modern_encoder']:
                enigma_proc.encoder = msgprocedure.transportencoder.ModernEncoder(self.server)
                                            
            out_text_parts = enigma_proc.encrypt(text)
        else:
            # Perform decryption
            enigma_proc = self._generate_msg_proc_obj(self.machine.get_description(), DUMMY_SYS_INDICATOR, args['grundstellung'], args['msg_proc_type'])
            
            if args['use_modern_encoder']:
                enigma_proc.encoder = msgprocedure.transportencoder.ModernEncoder(self.server)
            
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
