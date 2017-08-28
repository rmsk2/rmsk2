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

## @package rotorsimtest contains tests for the python3 to C++ interface.
#   
# \file rotorsimtest.py
# \brief This file contains classes that implement tests which can be used to verify that
#        the python3 to C++ interface based on the program tlv_rotorsim is functional and correct.

import simpletest
from pyrmsk2.rotorsim import *

## \brief This class serves as a base class for the verification of all rotor machines.
#
class RotorMachineFuncTest(simpletest.SimpleTest):
    ## \brief Constructor. 
    #
    #  \param [name] Is a string. It has to specifiy a textual description for the test.
    #
    #  \param [proc] Is an object that has the same interface as rotorsim.RotorMachine. It is used to conduct
    #         the decryption operations during the verification tests.
    #
    #  \param [state_helper] Is an object that has a make_state method. It expected that instances of
    #         rotorsim.StateHelper or cmdlinetest.CLIRotorState are used. 
    #
    def __init__(self, name, proc, state_helper = None):
        super().__init__(name)
        self._proc = proc
        self._state_proc = state_helper
    
    ## \brief Sets the object that is used to perform decrytpion operations.
    #
    #  \param [proc] Is an object that has the same interface as rotorsim.RotorMachine. It is used to conduct
    #         the decryption operations during the verification tests.
    #
    #  \returns Nothing.
    #        
    def set_processor(self, proc):
        self._proc = proc

    ## \brief Sets the object that is used to generate state data.
    #
    #  \param [state_helper] Is an object that has a make_state method. It expected that instances of
    #         rotorsim.StateHelper or cmdlinetest.CLIRotorState are used.
    #
    #  \returns Nothing.
    #        
    def set_state_proc(self, state_helper):
        self._state_proc = state_helper


## \brief This class serves as a base class for the verification of all the Enigma variants.
#
class EnigmaFuncTest(RotorMachineFuncTest):
    ## \brief Constructor. 
    #
    #  \param [name] Is a string. It has to specifiy a textual description for the test.
    #
    #  \param [proc] Is an object that has the same interface as rotorsim.RotorMachine. It is used to conduct
    #         the decryption operations during the verification tests.
    #
    def __init__(self, name, proc):
        super().__init__(name, proc)
        self._help = Permutation()


## \brief This class implements a verification test for the M4 Enigma.
#
class M4EnigmaTest(EnigmaFuncTest):
    ## \brief Constructor. 
    #
    #  \param [proc] Is an object that has the same interface as rotorsim.RotorMachine. It is used to conduct
    #         the decryption operations during the verification tests.
    #
    def __init__(self, proc = None):
        super().__init__("M4 Verification Test", proc)

    ## \brief Performs the verification test.
    #
    #  \returns A boolean. A return value of True means that the test was successfull.
    #        
    def test(self):
        result = super().test()
        enigma_M4_config = M4EnigmaState.get_default_config()        
        enigma_M4_state = self._state_proc.make_state('M4', enigma_M4_config.config, enigma_M4_config.rotor_pos);               
        self._proc.set_state(enigma_M4_state)
        
        decryption_result = self._proc.decrypt('nczwvusxpnyminhzxmqxsfwxwlkjahshnmcoccakuqpmkcsmhkseinjusblkiosxckubhmllxcsjusrrdvkohulxwccbgvliyxeoahxrhkkfvdrewez')
        self.append_note("Decryption result: " + decryption_result)
        result = (decryption_result.lower() == 'vonvonjlooksjhffttteinseinsdreizwoyyqnnsneuninhaltxxbeiangriffunterwassergedruecktywabosxletztergegnerstandnulachtd')
        
        return result


## \brief This class implements a verification test for the M3 or Services Enigma with Uhr.
#
class ServicesUhrTest(EnigmaFuncTest):
    ## \brief Constructor. 
    #
    #  \param [proc] Is an object that has the same interface as rotorsim.RotorMachine. It is used to conduct
    #         the decryption operations during the verification tests.
    #
    def __init__(self, proc = None):
        super().__init__("Services Uhr Test", proc)

    ## \brief Performs the verification test.
    #
    #  \returns A boolean. A return value of True means that the test was successfull.
    #        
    def test(self):
        result = super().test()
        enigma_I_config = ServicesEnigmaState.get_default_config('Services')
        enigma_I_config.config['usesuhr'] = 'true'
        enigma_I_config.config['plugs'] = '27:adcnetflgijvkzpuqywx'
        
        enigma_I_state = self._state_proc.make_state('Services', enigma_I_config.config, enigma_I_config.rotor_pos)

        self._proc.set_state(enigma_I_state)
        
        decryption_result = self._proc.decrypt('ukpfhallqcdnbffcghudlqukrbpyiyrdlwyalykcvossffxsyjbhbghdxawukjadkelptyklgfxqahxmmfpioqnjsgaufoxzggomjfryhqpccdivyicgvyx')
        self.append_note("Decryption result: " + decryption_result)
        result = (decryption_result.lower() == 'dasoberkommandoderwehrmaqtgibtbekanntxaachenxaachenxistgerettetxdurqgebuendelteneinsatzderhilfskraeftekonntediebedrohun')
        
        return result


## \brief This class implements a verification test for the KD Enigma.
#
class KDTest(EnigmaFuncTest):
    ## \brief Constructor. 
    #
    #  \param [proc] Is an object that has the same interface as rotorsim.RotorMachine. It is used to conduct
    #         the decryption operations during the verification tests.
    #
    def __init__(self, proc = None):
        super().__init__("KD Test", proc)

    ## \brief Performs the verification test.
    #
    #  \returns A boolean. A return value of True means that the test was successfull.
    #        
    def test(self):
        result = super().test()
        enigma_kd_config = UnsteckeredEnigmaState.get_default_config('KDEnigma')
        enigma_kd_state = self._state_proc.make_state('KD', enigma_kd_config.config, enigma_kd_config.rotor_pos)  
        self._proc.set_state(enigma_kd_state)
        
        decryption_result = self._proc.decrypt('zvzdycwqbkqzsspmmojkguikuigwhgrqslrlckpzfrayhxrxgbfflxigesewydqufsulhojvuhaybaav')
        self.append_note("Decryption result: " + decryption_result)
        result = (decryption_result.lower() == 'obwohldierotorverdrahtungenderkdenigmanichtbekanntsindsimulierenwirdiesemaschine')
        
        return result


## \brief This class implements a verification test for the Typex.
#
class TypexTest(EnigmaFuncTest):
    ## \brief Constructor. 
    #
    #  \param [proc] Is an object that has the same interface as rotorsim.RotorMachine. It is used to conduct
    #         the decryption operations during the verification tests.
    #
    def __init__(self, proc = None):
        super().__init__("Typex Test", proc)

    ## \brief Performs the verification test.
    #
    #  \returns A boolean. A return value of True means that the test was successfull.
    #        
    def test(self):
        result = super().test()
        typex_config = TypexState.get_default_config()
        typex_state = self._state_proc.make_state('Typex', typex_config.config, typex_config.rotor_pos)
        self._proc.set_state(typex_state)
        
        decryption_result = self._proc.decrypt('ptwcichvmijbkvcazuschqyaykvlbswgqxrqujjnyqyqptrlaly')
        self.append_note("Decryption result: " + decryption_result)
        result = (decryption_result.lower() == "qwertyuiopasdfghjkl cbnm1234567890-/z%x£*() v',.a")
        
        return result


## \brief This class implements a verification test for the Typex using the alternative rotor set Y269.
#
class TypexTestY269(EnigmaFuncTest):
    ## \brief Constructor. 
    #
    #  \param [proc] Is an object that has the same interface as rotorsim.RotorMachine. It is used to conduct
    #         the decryption operations during the verification tests.
    #
    def __init__(self, proc = None):
        super().__init__("Typex Test Y269", proc)

    ## \brief Performs the verification test.
    #
    #  \returns A boolean. A return value of True means that the test was successfull.
    #        
    def test(self):
        result = super().test()
                
        typex_config = {}
        typex_config['rotorset'] = 'Y269'
        typex_config['plugs'] = ''
        typex_config['rings'] = 'aaaaa'
        typex_config['rotors'] = 'aNbNcRdNeN'
        typex_config['reflector'] = 'arbycudheqfsglixjpknmotwvz'
        typex_state = self._state_proc.make_state('Typex', typex_config, 'aaaaa')
        
        self._proc.set_state(typex_state)
        
        decryption_result = self._proc.decrypt('agdzdfthgeocgrmyjsbukuztd')
        self.append_note("Decryption result: " + decryption_result)
        result = (decryption_result.lower() == 'hallo dies ist ein test  ')
        
        return result


## \brief This class implements a verification test for the Tirpitz Enigma.
#
class TirpitzTest(EnigmaFuncTest):
    ## \brief Constructor. 
    #
    #  \param [proc] Is an object that has the same interface as rotorsim.RotorMachine. It is used to conduct
    #         the decryption operations during the verification tests.
    #
    def __init__(self, proc = None):
        super().__init__("Tirpitz Test", proc)

    ## \brief Performs the verification test.
    #
    #  \returns A boolean. A return value of True means that the test was successfull.
    #                
    def test(self):
        result = super().test()

        enigma_t_config = UnsteckeredEnigmaState.get_default_config('TirpitzEnigma')                                
        enigma_t_state = self._state_proc.make_state('Tirpitz', enigma_t_config.config, enigma_t_config.rotor_pos)
        self._proc.set_state(enigma_t_state)        
                                          
        decryption_result = self._proc.decrypt('rhmbwnbzgmmnkperufvnyjfkyqg')
        self.append_note("Decryption result: " + decryption_result)
        result = (decryption_result.lower() == 'anxrommelxspruchnummerxeins')
        
        return result
        

## \brief This class implements a verification test for the Abwehr Enigma.
#
class AbwehrTest(EnigmaFuncTest):
    ## \brief Constructor. 
    #
    #  \param [proc] Is an object that has the same interface as rotorsim.RotorMachine. It is used to conduct
    #         the decryption operations during the verification tests.
    #
    def __init__(self, proc = None):
        super().__init__("Abwehr Test", proc)

    ## \brief Performs the verification test.
    #
    #  \returns A boolean. A return value of True means that the test was successfull.
    #                        
    def test(self):
        result = super().test()
        
        enigma_abwehr_config = UnsteckeredEnigmaState.get_default_config('AbwehrEnigma')                                
        enigma_abwehr_state = self._state_proc.make_state('Abwehr', enigma_abwehr_config.config, enigma_abwehr_config.rotor_pos)
        self._proc.set_state(enigma_abwehr_state)        
        
        decryption_result = self._proc.decrypt('gjuiycmdguvttffqpzmxkvctzusobzldzumhqmjxwtzwmqnnuwidyeqpgvfzetolb')
        self.append_note("Decryption result: " + decryption_result)
        result = (decryption_result.lower() == 'aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa')
        
        return result


## \brief This class implements a verification test for the Railway Enigma.
#
class RailwayTest(EnigmaFuncTest):
    ## \brief Constructor. 
    #
    #  \param [proc] Is an object that has the same interface as rotorsim.RotorMachine. It is used to conduct
    #         the decryption operations during the verification tests.
    #
    def __init__(self, proc = None):
        super().__init__("Railway Test", proc)
    
    ## \brief Performs the verification test.
    #
    #  \returns A boolean. A return value of True means that the test was successfull.
    #        
    def test(self):
        result = super().test()
        
        enigma_railway_config = UnsteckeredEnigmaState.get_default_config('RailwayEnigma')                                
        enigma_railway_state = self._state_proc.make_state('Railway', enigma_railway_config.config, enigma_railway_config.rotor_pos)
        self._proc.set_state(enigma_railway_state)        

        
        decryption_result = self._proc.decrypt('zbijbjetellsdidqbyocxeohngdsxnwlifuuvdqlzsyrbtbwlwlxpgujbhurbikgtkdztgtexjxhulfkiuqnjbeqgccryitomeyirckuji')
        self.append_note("Decryption result: " + decryption_result)
        result = (decryption_result.lower() == 'aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa')
        
        return result


## \brief This class serves as a base class for verification tests of SIGABA machines.
#
class SigabaTest(RotorMachineFuncTest):
    ## \brief Constructor. 
    #
    #  \param [name] Is a string. It has to specifiy a textual description for the test.
    #    
    #  \param [proc] Is an object that has the same interface as rotorsim.RotorMachine. It is used to conduct
    #         the decryption operations during the verification tests.
    #
    def __init__(self, name, proc = None):
        super().__init__(name, proc)


## \brief This class implements a verification test for the CSP889 variant of the SIGABA.
#
class CSP889Test(SigabaTest):
    ## \brief Constructor. 
    #
    #  \param [proc] Is an object that has the same interface as rotorsim.RotorMachine. It is used to conduct
    #         the decryption operations during the verification tests.
    #
    def __init__(self, proc = None):
        super().__init__('CSP889 Test', proc)

    ## \brief Performs the verification test.
    #
    #  \returns A boolean. A return value of True means that the test was successfull.
    #        
    def test(self):
        result = super().test()
        
        csp889_config = SigabaMachineState.get_default_config()                
        csp889_state = self._state_proc.make_state('SIGABA', csp889_config.config, csp889_config.rotor_pos)                        
        self._proc.set_state(csp889_state)
        self._proc.sigaba_setup(1, 4)
        self._proc.sigaba_setup(3, 2)
        self._proc.sigaba_setup(5, 1)
                                          
        decryption_result = self._proc.decrypt('hiscbulieudekwremsdicbpwarhujhhixjhvxgbxrzypzpbybzycheafdgnffobzvwuasynfuczcsgbzrxxnamortkgugtcddmbnqhzrc')
        self.append_note("Decryption result: " + decryption_result)
        result = (decryption_result.lower() == 'dies ist ein toller test fuer die sigaba punkt die amis haben damals glatt mit leerxeichen verschluesselt')
        
        return result

    
## \brief This class implements a verification test for the CSP2900 variant of the SIGABA.
#
class CSP2900Test(SigabaTest):
    ## \brief Constructor. 
    #
    #  \param [proc] Is an object that has the same interface as rotorsim.RotorMachine. It is used to conduct
    #         the decryption operations during the verification tests.
    #
    def __init__(self, proc = None):
        super().__init__('CSP2900 Test', proc)
    
    ## \brief Performs the verification test.
    #
    #  \returns A boolean. A return value of True means that the test was successfull.
    #         
    def test(self):
        result = super().test()

        csp2900_config = SigabaMachineState.get_default_config()
        csp2900_config.config['csp2900'] = 'true'
        csp2900_state = self._state_proc.make_state('SIGABA', csp2900_config.config, csp2900_config.rotor_pos)                        
        self._proc.set_state(csp2900_state)

        self._proc.sigaba_setup(2, 3)
        self._proc.sigaba_setup(3, 3)
        self._proc.sigaba_setup(4, 3)
                                          
        decryption_result = self._proc.decrypt('bsfzeppcipicwhynfpnjxpnqmcleywutmhrhojypwwsflifobk')
        self.append_note("Decryption result: " + decryption_result)
        result = (decryption_result.lower() == 'aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa')
        
        return result


## \brief This class implements a verification test for the Nema.
#
class NemaTest(RotorMachineFuncTest):
    ## \brief Constructor. 
    #
    #  \param [proc] Is an object that has the same interface as rotorsim.RotorMachine. It is used to conduct
    #         the decryption operations during the verification tests.
    #
    def __init__(self, proc = None):
        super().__init__('Nema Test', proc)
    
    ## \brief Performs the verification test.
    #
    #  \returns A boolean. A return value of True means that the test was successfull.
    #         
    def test(self):
        result = super().test()
        
        nema_config = NemaState.get_default_config()        
        nema_state = self._state_proc.make_state('Nema', nema_config.config, nema_config.rotor_pos)
        self._proc.set_state(nema_state)      
                                          
        decryption_result = self._proc.decrypt('hrsbvootzucrwlmgrmgvwywovnf')
        self.append_note("Decryption result: " + decryption_result)
        result = (decryption_result.lower() == 'aaaaaaaaaaaaaaaaaaaaaaaaaaa')
        
        return result


## \brief This class implements a verification test for the KL7.
#
class KL7Test(RotorMachineFuncTest):
    ## \brief Constructor. 
    #
    #  \param [proc] Is an object that has the same interface as rotorsim.RotorMachine. It is used to conduct
    #         the decryption operations during the verification tests.
    #
    def __init__(self, proc = None):
        super().__init__('KL7 Test', proc)
    
    ## \brief Performs the verification test.
    #
    #  \returns A boolean. A return value of True means that the test was successfull.
    #         
    def test(self):
        result = super().test()
        kl7_config = KL7State.get_default_config()        
        kl7_state = self._state_proc.make_state('KL7', kl7_config.config, kl7_config.rotor_pos)
        self._proc.set_state(kl7_state)      
        self._proc.step()        
                                          
        decryption_result = self._proc.decrypt('lpzocrfybrjmwhzrtsiygtxhuodgyyiuogpamxkfcjpplqkhss')
        self.append_note("Decryption result: " + decryption_result)
        result = (decryption_result.lower() == 'hallo dies ist wieder ein test vvv 1234567890 aaa')
        
        return result


## \brief This class implements a verification test for the SG39.
#
class SG39Test(EnigmaFuncTest):
    ## \brief Constructor. 
    #
    #  \param [proc] Is an object that has the same interface as rotorsim.RotorMachine. It is used to conduct
    #         the decryption operations during the verification tests.
    #
    def __init__(self, proc = None):
        super().__init__("SG39 Test", proc)

    ## \brief Performs the verification test.
    #
    #  \returns A boolean. A return value of True means that the test was successfull.
    #        
    def test(self):
        result = super().test()
        sg39_config = SG39State.get_default_config()
        sg39_state = self._state_proc.make_state('SG39', sg39_config.config, sg39_config.rotor_pos)                
        self._proc.set_state(sg39_state)
        
        decryption_result = self._proc.decrypt('obkdldrmiqlwiyggqohqlwchogmdskbeiqwrsrhensjkfuegeshbdnjhhkwixefpnmlvophbttzlfvtllmhgdsbrisfeetckcttjqbhzurxowizvtpdtuyqyfdkqjeryoopvqpijowmbvkizjipo')
        self.append_note("Decryption result: " + decryption_result)
        result = (decryption_result.lower() == 'aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa')
        
        return result


## \brief This class serves the purpose to bundle tests for indivudual rotor machines into a composite test.
#         Using the appropriate context object it can be used for verification tests based on the TLV interface 
#         and the command line program. All subordinate test cases have to have a set_processor() method as defined
#         in the class RotorMachineFuncTest.
#
class VerificationTests(simpletest.CompositeTest):
    ## \brief Constructor. 
    #
    #  \param [name] Is a string. It specifies an explanatory text which serves as the name of the test which is to
    #        be performed.      
    #
    #  \param [context] Is a callable object or function that takes a function f as an argument. That function f has to have the
    #         same signature as the method inner_test. The context object is responsible for creating the machine object with
    #         which f can be called and after that uses this machine object to call the function f that it was given as
    #         a parameter.
    #
    def __init__(self, name, context):
        super().__init__(name)
        self._context = context

    ## \brief Sets the object that is used to perform decrytpion operations in all subordinate test cases.
    #
    #  \param [proc] Is an object that has the same interface as rotorsim.RotorMachine. It is used to conduct
    #        the decryption operations during the verification tests.
    #
    #  \returns Nothing.
    #            
    def set_processor(self, proc):
        for i in self.test_cases:
            i.set_processor(proc)

    ## \brief Sets the object that is used to generate state data in all subordinate test cases.
    #
    #  \param [state_helper] Is an object that has a make_state method. It expected that instances of
    #         rotorsim.StateHelper or cmdlinetest.CLIRotorState are used. 
    #
    #  \returns Nothing.
    #            
    def set_state_proc(self, state_helper):
        for i in self.test_cases:
            i.set_state_proc(state_helper)

    ## \brief Performs the verification test.
    #
    #  \param [machine] Is an object with the same interface as rotorsim.RotorMachine. This object is used to
    #         do test en/decryptions.
    #
    #  \param [state_helper] Is an object that has a make_state method. It is expected that instances of
    #         rotorsim.StateHelper or cmdlinetest.CLIRotorState are used. 
    #    
    #  \returns A boolean. A return value of True means that the test was successfull.
    #    
    def inner_test(self, machine, state_helper = None):
        result = True
        try:            
            self.set_processor(machine)
            self.set_state_proc(state_helper)
            result = super().test()
        except:
            self.append_note("EXCEPTION!!!!")
            result = False
        
        return result                

    ## \brief Performs the verification test and uses the TLV interface to create the needed machine object.
    #
    #  \returns A boolean. A return value of True means that the test was successfull.
    #    
    def test(self):
        return self._context(self.inner_test)        


## \brief This function serves as the context "object" for verification tests using the TLV infrastructure.
#
def tlv_context(inner_test):
    result = True

    with tlvobject.TlvServer(server_address='sock_fjsdhfjshdkfjh') as server, RotorMachine.from_machine_name('M4', server.address) as machine:
        state_helper = StateHelper(server.address)
        result = inner_test(machine, state_helper)        
    
    return result       
    

## \brief This class performs verification tests for the proper implementation of the rotorsim.RotorMachine class
#         which uses the TLV backend to provide rotor machine functionality.
#
class RotorMachineFuncTests(simpletest.SimpleTest):
    ## \brief Constructor. 
    #
    #  \param [name] Is a string. It specifies an explanatory text which serves as the name of the test which is to
    #        be performed.      
    #
    def __init__(self, name):
        super().__init__(name)

    ## \brief Performs the test.
    #
    #  \returns A boolean. A return value of True means that the test was successfull.
    #            
    def test(self):
        result = super().test()
                    
        with tlvobject.TlvServer(server_address='sock_fjsdhfjshdkfjh') as server:
            state_proc = StateHelper(server.address)
            
            m4_enigma_config = M4EnigmaState.get_default_config()
            m4_state = state_proc.make_state('M4', m4_enigma_config.config, m4_enigma_config.rotor_pos)
            
            csp889_config = SigabaMachineState.get_default_config()
            csp889_state = state_proc.make_state('SIGABA', csp889_config.config, csp889_config.rotor_pos)
            
            sg39_config = SG39State.get_default_config()
            sg39_state = state_proc.make_state('SG39', sg39_config.config, sg39_config.rotor_pos)
            
            with RotorMachine(m4_state, server.address) as m4_obj:
                try:
                    original_state = m4_obj.get_state()
                    
                    # Do a simple test decryption
                    dec_result = m4_obj.decrypt('nczwvusx')
                    last_result = (dec_result == 'vonvonjl')
                    result = result and last_result
                    if not last_result:
                        self.append_note("Unexpected M4 decryption result: " + dec_result)
                    
                    # Verfiy that the machine's state has changed by the preceeding decryptions
                    dec_result = m4_obj.decrypt('nczwvusx')
                    last_result = (dec_result != 'vonvonjl')
                    result = result and last_result
                    if not last_result:
                        self.append_note("Unexpected M4 decryption result: " + dec_result)
                    
                    # Restore original state
                    m4_obj.set_state(original_state)
                    
                    # Verify that the original state has been restored by performing the same test decryption again
                    dec_result = m4_obj.decrypt('nczwvusx')                
                    last_result = (dec_result == 'vonvonjl')
                    result = result and last_result
                    if not last_result:
                        self.append_note("Unexpected M4 decryption result: " + dec_result)
                                    
                    m4_obj.set_state(original_state)                        
                    
                    # Verify step() method
                    step_result = m4_obj.step(5)                
                    last_result = ((len(step_result) == 5) and (step_result[4] == 'vjnf'))
                    result = result and last_result
                    if not last_result:
                        self.append_note("Unexpected rotor positon: " + str(step_result))
                    
                    # Test get_description() method
                    description = m4_obj.get_description()
                    last_result = (description == 'M4Enigma')
                    result = result and last_result
                    if not last_result:
                        self.append_note("Unexpected machine description: " + description)
                                    
                    m4_obj.set_state(csp889_state)
                                    
                    # Verify that returned description changed to new machine type
                    description = m4_obj.get_description()
                    last_result = (description == 'CSP889')
                    result = result and last_result
                    if not last_result:
                        self.append_note("Unexpected machine description: " + description)
                    
                    # Test sigaba_setup() method                
                    setup_step_result = m4_obj.sigaba_setup(1, 3)
                    last_result = ((len(setup_step_result) == 3) and (setup_step_result[2] == '00000lomooolonm'))
                    result = result and last_result
                    if not last_result:
                        self.append_note("Unexpected rotor position: " + str(setup_step_result))
                   
                    m4_obj.set_state(sg39_state)
                    
                    # Test get_rotor_positions() method                                
                    rotor_pos = m4_obj.get_rotor_positions()
                    last_result = (rotor_pos == 'frqdaph')
                    result = result and last_result
                    if not last_result:
                        self.append_note("Unexpected rotor positions: " + rotor_pos)
                        
                    # Test get_permutations() method 
                    m4_obj.set_state(original_state)                               
                    perms = m4_obj.get_permutations(10)
                    last_result = ((len(perms) == 11) and (len(perms[5]) == 26) and (perms[0] == [12, 4, 3, 2, 1, 7, 21, 5, 11, 22, 16, 8, 0, 14, 13, 18, 10, 23, 15, 24, 25, 6, 9, 17, 19, 20]))
                    result = result and last_result
                    if not last_result:
                        self.append_note("Unexpected permutation result: " + str(perms))                
                    
                except:
                    self.append_note("EXCEPTON!!!!")
                    result = False
        
        return result                


## \brief This class provides a performance test of the TLV rotor machine interface.
#        
class RotorMachinePerfTest(simpletest.SimpleTest):
    ## \brief Constructor. 
    #
    # \param [name] Is a string. It specifies an explanatory text which serves as the name of the test which is to
    #        be performed.   
    #   
    # \param [test_data] Is a string. It specifies the sample text that is used to measure the decryption speed.
    #
    # \param [num_iterations] Is an integer. It specifies how many test decryptions are to be performed.         
    #
    def __init__(self, name, test_data, num_iterations = 22000):
        super().__init__(name)
        self._iterations = num_iterations
        self._test_data = test_data

    ## \brief Performs the test.
    #
    #  \returns A boolean. A return value of True means that the test was successfull.
    #            
    def test(self):
        result = super().test()       

        with tlvobject.TlvServer(server_address='sock_fjsdhfjshdkfjh') as server: 
            state_proc = StateHelper(server.address)
            
            m4_enigma_config = M4EnigmaState.get_default_config()
            m4_state = state_proc.make_state('M4', m4_enigma_config.config, m4_enigma_config.rotor_pos)        
        
            with RotorMachine(m4_state, server.address) as m4_obj:
                try:  
                    dec_result = m4_obj.decrypt('nczwvusx')  
                    result = result and (dec_result == 'vonvonjl')
                    
                    if not result:
                        self.append_note("M4 message not properly decrypted: {}".format(dec_result))
                    else:                    
                        jetzt = datetime.datetime.now()
                        
                        for i in range(self._iterations):
                            m4_obj.decrypt(self._test_data)
                        
                        spaeter = datetime.datetime.now()
                        self.append_note("Time needed for {} decryptions: {}".format(self._iterations, str(spaeter - jetzt)))
                except:
                    self.append_note("EXCEPTION!!!!")
                    result = False
        
        return result


## \brief This class tests whether retrieving randomizer parameters works as expected.
#        
class RandParmTest(simpletest.SimpleTest):
    ## \brief Constructor. 
    #
    # \param [name] Is a string. It specifies an explanatory text which serves as the name of the test which is to
    #        be performed.   
    #
    def __init__(self, name):
        super().__init__(name)

    ## \brief Performs the test.
    #
    #  \returns A boolean. A return value of True means that the test was successfull.
    #    
    def test(self):
        result = super().test()       

        with tlvobject.TlvServer(server_address='sock_fjsdhfjshdkfjh') as server: 
        
            state_proc = StateHelper(server.address)            
            machine_state = state_proc.get_default_state('M4')        
            
            with RotorMachine(machine_state, server.address) as machine:            
                try:  
                    randomizer_params = machine.get_randomizer_params()
                    
                    result = result and (randomizer_params == [])
                    if not result:
                        self.append_note("Incorrect list of randomizer parameters returned for M4: {}".format(str(randomizer_params)))
                    else:
                        self.append_note("Randomizer parameters returned for M4: {}".format(str(randomizer_params)))
                        
                    machine_state = state_proc.get_default_state('SG39')
                    machine.set_state(machine_state)
                    
                    randomizer_params = machine.get_randomizer_params()
                    this_result = (randomizer_params == ['one', 'two', 'three', 'special', 'enigmam4'])
                    result = result and this_result
                    if not this_result:
                        self.append_note("Incorrect list of randomizer parameters returned for SG39: {}".format(str(randomizer_params)))
                    else:
                        self.append_note("Randomizer parameters returned for SG39: {}".format(str(randomizer_params)))
                    
                    if result:        
                        self.append_note('OK')
                except:
                    self.append_note("EXCEPTION!!!!")
                    result = False
        
        return result


## \brief This class tests whether state randomization is possible and ensures that loading and parsing state information
#         is done correctly.
#        
class RandomizeTest(simpletest.SimpleTest):
    ## \brief Constructor. 
    #
    # \param [name] Is a string. It specifies an explanatory text which serves as the name of the test which is to
    #        be performed.   
    #   
    # \param [list_of_states] Is a list of tuples. The first element a string and gives the machine name. The second
    #        is a is StateSpec() object and specifies the state.
    #
    def __init__(self, name, list_of_states):
        super().__init__(name)
        self._all_states = list_of_states

    ## \brief Performs the test.
    #
    #  \returns A boolean. A return value of True means that the test was successfull.
    #    
    def test(self):
        result = super().test()

        with tlvobject.TlvServer(server_address='sock_fjsdhfjshdkfjh') as server: 
            state_proc = StateHelper(server.address) 
            machine_state = state_proc.get_default_state('M4')
            
            with RotorMachine(machine_state, server.address) as machine:
                try:                      
                    for i in self._all_states:
                        # Set machine to test state
                        state_help = state_proc.make_state(i[0], i[1].config, i[1].rotor_pos)
                        machine.set_state(state_help)
                        self.append_note("Testing {} state".format(machine.get_description()))                    
                        # Randomize machine state
                        machine.randomize_state()
                        # Save original randomized state
                        state1 = machine.get_state()
                        test_message = machine.encrypt('diesisteintest')
                        # Destroy original randomized state
                        machine.randomize_state()
                                                
                        # Reset machine to original randomized state
                        machine.set_state(state1)
                        dec_result = machine.decrypt(test_message)
                        
                        # Check whether test decryption worked
                        if dec_result != 'diesisteintest':
                            self.append_note("Decryption failed!! Result: {}".format(dec_result))
                            result = False
                            break
                        
                        self.append_note('OK')
                except:
                    self.append_note("EXCEPTION!!!!")
                    result = False
        
        return result


## \brief Returns a simpletest.SimpleTest object that allows to perform all the tests defined in this module.
#
#  \param [test_data] Is a string. It specifies the sample text that is used to measure the decryption speed.
#
#  \param [num_iterations] Is an integer. It specifies how many test decryptions are to be performed.
#         
#  \param [context] Is a callable object or function that takes a function f as an argument. That function f has to have the
#         same signature as the method VerificationTests.innner_tests. The context object is responsible for creating the
#         machine object with which f can be called and after that uses this machine object to call the function f that it
#         was given as a parameter.
# 
#  \param [verification_only] is a boolean which has to be true if only the machine verification tests but not the performance
#         and basic functional tests for the TLV infrastructure are to be returned.
#         
#  \returns A simpletest.CompositeTest object.
#                
def get_module_test(test_data = 'aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa', num_iterations = 2500, context = tlv_context, verification_only = False):
    enigma_verification_test = VerificationTests("Enigma verification test", context)
    enigma_verification_test.add(M4EnigmaTest())
    enigma_verification_test.add(ServicesUhrTest())    
    enigma_verification_test.add(KDTest())
    enigma_verification_test.add(TirpitzTest())    
    enigma_verification_test.add(AbwehrTest())
    enigma_verification_test.add(RailwayTest())    
    enigma_verification_test.add(TypexTest())
    enigma_verification_test.add(TypexTestY269())    

    sigaba_verification_test = VerificationTests("SIGABA verification test", context)
    sigaba_verification_test.add(CSP889Test())
    sigaba_verification_test.add(CSP2900Test())    

    nema_verification_test = VerificationTests("Nema verification test", context)
    nema_verification_test.add(NemaTest())

    kl7_verification_test = VerificationTests("KL7 verification test", context)
    kl7_verification_test.add(KL7Test())

    sg39_verification_test = VerificationTests("SG39 verification test", context)
    sg39_verification_test.add(SG39Test())
    
    all_tests = simpletest.CompositeTest('rotorsim')    
    
    if not verification_only:
        performance_test = RotorMachinePerfTest("rotorsim performance test", test_data, num_iterations)
        functional_test = RotorMachineFuncTests("rotorsim functional test")
        test_states = []
        test_states.append(('M4', M4EnigmaState.get_default_config()))
        test_states.append(('Services', ServicesEnigmaState.get_default_config('Services')))        
        test_states.append(('Abwehr', UnsteckeredEnigmaState.get_default_config('AbwehrEnigma')))
        test_states.append(('KD', UnsteckeredEnigmaState.get_default_config('KDEnigma')))
        test_states.append(('Tirpitz', UnsteckeredEnigmaState.get_default_config('TirpitzEnigma')))
        test_states.append(('Railway', UnsteckeredEnigmaState.get_default_config('RailwayEnigma')))
        test_states.append(('SIGABA', SigabaMachineState.get_default_config()))
        test_states.append(('KL7', KL7State.get_default_config()))
        test_states.append(('SG39', SG39State.get_default_config()))
        test_states.append(('Typex', TypexState.get_default_config()))
        test_states.append(('Nema', NemaState.get_default_config()))        
        rand_test = RandomizeTest('State randomization test', test_states)
        rand_parm_test = RandParmTest('Randomizer parameter test')    
        all_tests.add(functional_test)
        all_tests.add(performance_test)
        all_tests.add(rand_test)
        all_tests.add(rand_parm_test)
    
    all_tests.add(enigma_verification_test)
    all_tests.add(sigaba_verification_test)
    all_tests.add(nema_verification_test)
    all_tests.add(kl7_verification_test)    
    all_tests.add(sg39_verification_test)
    
    return all_tests

## \brief Performs all the tests defined in this module.
#
#  \param [test_data] Is a string. It specifies the sample text that is used to measure the decryption speed.
#
#  \param [num_iterations] Is an integer. It specifies how many test decryptions are to be performed.
#         
#  \returns Nothing.
#                
def execute_tests(test_data, num_iterations):
    tests = get_module_test(test_data, num_iterations)
    test_result = tests.test()
    tests.print_notes()
