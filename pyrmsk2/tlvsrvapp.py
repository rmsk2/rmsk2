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

## @package tlvsrvapp Contains classes that allow to implement command line appliations that use the TlvServer.
#           
# \file pyrmsk2/tlvsrvapp.py
# \brief This file provides some Python3 classes that allow to implement command line applications that make
#        use of the TlvServer. It hides the details of how to initialize, start and stop the TlvServer.

import sys
import traceback
import pyrmsk2.tlvobject as tlvobject
import pyrmsk2.rotorsim as rotorsim
import pyrmsk2.rotorrandom as rotorrandom

ERR_OK = 0
ERR_ERROR = 42

## \brief This class allows to implement command line applications that make use of the TlvServer. It hides the 
#         details of how to initialize, start and stop the TlvServer.
#
class TlvServerApp:
    ## \brief Constructor.
    #
    def __init__(self):
        self._server = None
        self._state_helper = None
        self._rotor_random = None
        self._rotor_machine = None

    ## \brief Returns the TlvServer instance that is used by this TlvServerApp instance.
    #
    #  \returns A tlvobject.TlvServer object.
    #        
    @property
    def server(self):
        return self._server

    ## \brief Returns a StateHelper object that allows to generate default machine configurations.
    #
    #  \returns A rotorsim.StateHelper object.
    #        
    @property
    def state_helper(self):
        return self._state_helper

    ## \brief Returns a rotorandom object that can be used to generate random strings and permutations.
    #
    #  \returns A rotorrandom.RotorRandom object.
    #        
    @property
    def random(self):
        return self._rotor_random

    ## \brief Returns a rotorsim.RotorMachine object that can be used by the main_func() method.
    #
    #  \returns A rotorsim.RotorMachine object.
    #        
    @property
    def machine(self):
        return self._rotor_machine

    ## \brief This method is the main method of the command line appilication it does initialize, start and
    #         stop the TlvServer as well as calling the main_func() method.
    #
    #  \param [argv] A vector of strings as specified on the command line
    #
    #  \returns Nothing.
    #    
    def main(self, argv):
        err_code = ERR_OK
        
        try:
        
            with tlvobject.TlvServer() as tlvsrv, rotorrandom.RotorRandom('abcdefghijklmnopqrstuvwxyz', tlvsrv.address) as rand:
                self._server = tlvsrv
                self._rotor_random = rand

                self._state_helper = rotorsim.StateHelper(self.server.address)
                default_state = self._state_helper.get_default_state('M4')

                with rotorsim.RotorMachine(default_state, self.server.address) as def_machine:
                    self._rotor_machine = def_machine
                    cli_args = self.parse_args(argv)
                    err_code = self.main_func(cli_args)
                        
        except Exception as e:
            err_code = ERR_ERROR
            self.exception_handler(e)
        finally:
            self._server = None
            self._rotor_random = None
            self._state_helper = None    
            self._rotor_machine = None      
        
        sys.exit(err_code)
                
    ## \brief This method is intended to contain the application main function.
    #
    #  \param [args] A dictionary that maps strings to objects. It is intended that the result from parse_args()
    #         is passed to main_func() in this parameter.
    #    
    #  \returns An integer. The error code that is resurned by the command line application.
    #        
    def main_func(self, args):
        result = ERR_OK
        
        return result

    ## \brief This method is intended to parse the command line arguments and return a dictionary resulting from that parsing.
    #
    #  \param [argv] A vector of strings as specified on the command line
    #    
    #  \returns A dictionary that maps strings to objects. This dictionary is then passed to the main_func() method.
    #        
    def parse_args(self, argv):
        result = {}
        
        return result

    ## \brief This method is called when an exception is caught by the main() method.
    #
    #  \returns Nothing.
    #            
    def exception_handler(self, e):        
        exc_type, exc_value, exc_traceback = sys.exc_info()
        #traceback.print_tb(exc_traceback, limit=10, file=sys.stdout)
        print(e)

