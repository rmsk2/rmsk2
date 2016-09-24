################################################################################
# Copyright 2016 Martin Grap
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
    def __init__(self, name, proc):
        super().__init__(name)
        self._proc = proc
    
    ## \brief Sets the object that is used to perform decrytpion operations.
    #
    #  \param [proc] Is an object that has the same interface as rotorsim.RotorMachine. It is used to conduct
    #         the decryption operations during the verification tests.
    #
    #  \returns Nothing.
    #        
    def set_processor(self, proc):
        self._proc = proc


## \brief This class serves as a base class for the verification of all the Enigma variants.
#
class EnigmaFuncTest(RotorMachineFuncTest):
    ## \brief Constructor. 
    #
    #  \param [name] Is a string. It has to specifiy a textual description for the test.
    #
    #  \param [enig_rotor_set] Is an object of type rotorsim.EnigmaRotorSet. It specifies a rotor set
    #         which contains information about Enigma rotors and rings.
    #
    #  \param [proc] Is an object that has the same interface as rotorsim.RotorMachine. It is used to conduct
    #         the decryption operations during the verification tests.
    #
    def __init__(self, name, enig_rotor_set, proc):
        super().__init__(name, proc)
        self._rotor_set = enig_rotor_set
        self._help = Permutation()


## \brief This class implements a verification test for the M4 Enigma.
#
class M4EnigmaTest(EnigmaFuncTest):
    ## \brief Constructor. 
    #
    #  \param [enig_rotor_set] Is an object of type rotorsim.EnigmaRotorSet. It specifies a rotor set
    #         which contains information about Enigma rotors and rings.
    #
    #  \param [proc] Is an object that has the same interface as rotorsim.RotorMachine. It is used to conduct
    #         the decryption operations during the verification tests.
    #
    def __init__(self, enig_rotor_set, proc = None):
        super().__init__("M4 Verification Test", enig_rotor_set, proc)

    ## \brief Performs the verification test.
    #
    #  \returns A boolean. A return value of True means that the test was successfull.
    #        
    def test(self):
        result = super().test()
        enigma_M4_state = M4EnigmaState.get_default_state(self._rotor_set)                
        self._proc.set_state(enigma_M4_state.render_state())
        
        decryption_result = self._proc.decrypt('nczwvusxpnyminhzxmqxsfwxwlkjahshnmcoccakuqpmkcsmhkseinjusblkiosxckubhmllxcsjusrrdvkohulxwccbgvliyxeoahxrhkkfvdrewez')
        self.append_note("Decryption result: " + decryption_result)
        result = (decryption_result.lower() == 'vonvonjlooksjhffttteinseinsdreizwoyyqnnsneuninhaltxxbeiangriffunterwassergedruecktywabosxletztergegnerstandnulachtd')
        
        return result


## \brief This class implements a verification test for the M3 or Services Enigma with Uhr.
#
class ServicesUhrTest(EnigmaFuncTest):
    ## \brief Constructor. 
    #
    #  \param [enig_rotor_set] Is an object of type rotorsim.EnigmaRotorSet. It specifies a rotor set
    #         which contains information about Enigma rotors and rings.
    #
    #  \param [proc] Is an object that has the same interface as rotorsim.RotorMachine. It is used to conduct
    #         the decryption operations during the verification tests.
    #
    def __init__(self, enig_rotor_set, proc = None):
        super().__init__("Services Uhr Test", enig_rotor_set, proc)

    ## \brief Performs the verification test.
    #
    #  \returns A boolean. A return value of True means that the test was successfull.
    #        
    def test(self):
        result = super().test()
        enigma_I_state = ServicesEnigmaState.get_default_state('Services', self._rotor_set) 
        enigma_I_state.set_stecker_brett('adcnetflgijvkzpuqywx', True, 27)               
        self._proc.set_state(enigma_I_state.render_state())
        
        decryption_result = self._proc.decrypt('ukpfhallqcdnbffcghudlqukrbpyiyrdlwyalykcvossffxsyjbhbghdxawukjadkelptyklgfxqahxmmfpioqnjsgaufoxzggomjfryhqpccdivyicgvyx')
        self.append_note("Decryption result: " + decryption_result)
        result = (decryption_result.lower() == 'dasoberkommandoderwehrmaqtgibtbekanntxaachenxaachenxistgerettetxdurqgebuendelteneinsatzderhilfskraeftekonntediebedrohun')
        
        return result


## \brief This class implements a verification test for the KD Enigma.
#
class KDTest(EnigmaFuncTest):
    ## \brief Constructor. 
    #
    #  \param [enig_rotor_set] Is an object of type rotorsim.EnigmaRotorSet. It specifies a rotor set
    #         which contains information about Enigma rotors and rings.
    #
    #  \param [proc] Is an object that has the same interface as rotorsim.RotorMachine. It is used to conduct
    #         the decryption operations during the verification tests.
    #
    def __init__(self, enig_rotor_set, proc = None):
        super().__init__("KD Test", enig_rotor_set, proc)

    ## \brief Performs the verification test.
    #
    #  \returns A boolean. A return value of True means that the test was successfull.
    #        
    def test(self):
        result = super().test()
        enigma_kd_state = UnsteckeredEnigmaState.get_default_state('KDEnigma', self._rotor_set)                
        self._proc.set_state(enigma_kd_state.render_state())
        
        decryption_result = self._proc.decrypt('xlmwoizeczzbfvmahnhrzerhnpwkjjorrxtebozcxncvdemaexvcfuxokbyntyjdongpgwwchftplrzr')
        self.append_note("Decryption result: " + decryption_result)
        result = (decryption_result.lower() == 'obwohldierotorverdrahtungenderkdenigmanichtbekanntsindsimulierenwirdiesemaschine')
        
        return result


## \brief This class implements a verification test for the Typex.
#
class TypexTest(EnigmaFuncTest):
    ## \brief Constructor. 
    #
    #  \param [enig_rotor_set] Is an object of type rotorsim.EnigmaRotorSet. It specifies a rotor set
    #         which contains information about Enigma rotors and rings.
    #
    #  \param [proc] Is an object that has the same interface as rotorsim.RotorMachine. It is used to conduct
    #         the decryption operations during the verification tests.
    #
    def __init__(self, enig_rotor_set, proc = None):
        super().__init__("Typex Test", enig_rotor_set, proc)

    ## \brief Performs the verification test.
    #
    #  \returns A boolean. A return value of True means that the test was successfull.
    #        
    def test(self):
        result = super().test()
        typex_state = TypexState.get_default_state(self._rotor_set)                
        self._proc.set_state(typex_state.render_state())
        
        decryption_result = self._proc.decrypt('ptwcichvmijbkvcazuschqyaykvlbswgqxrqujjnyqyqptrlaly')
        self.append_note("Decryption result: " + decryption_result)
        result = (decryption_result.lower() == "qwertyuiopasdfghjkl cbnm1234567890-/z%x£*() v',.a")
        
        return result


## \brief This class implements a verification test for the Tirpitz Enigma.
#
class TirpitzTest(EnigmaFuncTest):
    ## \brief Constructor. 
    #
    #  \param [enig_rotor_set] Is an object of type rotorsim.EnigmaRotorSet. It specifies a rotor set
    #         which contains information about Enigma rotors and rings.
    #
    #  \param [proc] Is an object that has the same interface as rotorsim.RotorMachine. It is used to conduct
    #         the decryption operations during the verification tests.
    #
    def __init__(self, enig_rotor_set, proc = None):
        super().__init__("Tirpitz Test", enig_rotor_set, proc)

    ## \brief Performs the verification test.
    #
    #  \returns A boolean. A return value of True means that the test was successfull.
    #                
    def test(self):
        result = super().test()
        enigma_t_state = UnsteckeredEnigmaState.get_default_state('TirpitzEnigma', self._rotor_set)                                
        self._proc.set_state(enigma_t_state.render_state())
                                          
        decryption_result = self._proc.decrypt('rhmbwnbzgmmnkperufvnyjfkyqg')
        self.append_note("Decryption result: " + decryption_result)
        result = (decryption_result.lower() == 'anxrommelxspruchnummerxeins')
        
        return result
        

## \brief This class implements a verification test for the Abwehr Enigma.
#
class AbwehrTest(EnigmaFuncTest):
    ## \brief Constructor. 
    #
    #  \param [enig_rotor_set] Is an object of type rotorsim.EnigmaRotorSet. It specifies a rotor set
    #         which contains information about Enigma rotors and rings.
    #
    #  \param [proc] Is an object that has the same interface as rotorsim.RotorMachine. It is used to conduct
    #         the decryption operations during the verification tests.
    #
    def __init__(self, enig_rotor_set, proc = None):
        super().__init__("Abwehr Test", enig_rotor_set, proc)

    ## \brief Performs the verification test.
    #
    #  \returns A boolean. A return value of True means that the test was successfull.
    #                        
    def test(self):
        result = super().test()
        enigma_abw_state = UnsteckeredEnigmaState.get_default_state('AbwehrEnigma', self._rotor_set)                        
        self._proc.set_state(enigma_abw_state.render_state())
        
        decryption_result = self._proc.decrypt('gjuiycmdguvttffqpzmxkvctzusobzldzumhqmjxwtzwmqnnuwidyeqpgvfzetolb')
        self.append_note("Decryption result: " + decryption_result)
        result = (decryption_result.lower() == 'aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa')
        
        return result


## \brief This class implements a verification test for the Railway Enigma.
#
class RailwayTest(EnigmaFuncTest):
    ## \brief Constructor. 
    #
    #  \param [enig_rotor_set] Is an object of type rotorsim.EnigmaRotorSet. It specifies a rotor set
    #         which contains information about Enigma rotors and rings.
    #
    #  \param [proc] Is an object that has the same interface as rotorsim.RotorMachine. It is used to conduct
    #         the decryption operations during the verification tests.
    #
    def __init__(self, enig_rotor_set, proc = None):
        super().__init__("Railway Test", enig_rotor_set, proc)
    
    ## \brief Performs the verification test.
    #
    #  \returns A boolean. A return value of True means that the test was successfull.
    #        
    def test(self):
        result = super().test()
        enigma_rb_state = UnsteckeredEnigmaState.get_default_state('RailwayEnigma', self._rotor_set)                        
        self._proc.set_state(enigma_rb_state.render_state())
        
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
    #  \param [normal_rotor_set] Is an object of type rotorsim.RotorSet. It specifies a rotor set
    #         which contains information about the rotors used for the SIGABA's crypt and driver rotors.
    #
    #  \param [index_rotor_set] Is an object of type rotorsim.RotorSet. It specifies a rotor set
    #         which contains information about the rotors used for the SIGABA's index rotors.
    #
    #  \param [proc] Is an object that has the same interface as rotorsim.RotorMachine. It is used to conduct
    #         the decryption operations during the verification tests.
    #
    def __init__(self, name, normal_rotor_set, index_rotor_set, proc = None):
        super().__init__(name, proc)
        self._rotor_set = normal_rotor_set 
        self._index_rotor_set = index_rotor_set


## \brief This class implements a verification test for the CSP889 variant of the SIGABA.
#
class CSP889Test(SigabaTest):
    ## \brief Constructor. 
    #
    #  \param [normal_rotor_set] Is an object of type rotorsim.RotorSet. It specifies a rotor set
    #         which contains information about the rotors used for the SIGABA's crypt and driver rotors.
    #
    #  \param [index_rotor_set] Is an object of type rotorsim.RotorSet. It specifies a rotor set
    #         which contains information about the rotors used for the SIGABA's index rotors.
    #
    #  \param [proc] Is an object that has the same interface as rotorsim.RotorMachine. It is used to conduct
    #         the decryption operations during the verification tests.
    #
    def __init__(self, normal_rotor_set, index_rotor_set, proc = None):
        super().__init__('CSP889 Test', normal_rotor_set, index_rotor_set, proc)

    ## \brief Performs the verification test.
    #
    #  \returns A boolean. A return value of True means that the test was successfull.
    #        
    def test(self):
        result = super().test()
        csp889_state = SigabaMachineState.get_default_state(self._rotor_set, self._index_rotor_set)                        
        self._proc.set_state(csp889_state.render_state())
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
    #  \param [normal_rotor_set] Is an object of type rotorsim.RotorSet. It specifies a rotor set
    #         which contains information about the rotors used for the SIGABA's crypt and driver rotors.
    #
    #  \param [index_rotor_set] Is an object of type rotorsim.RotorSet. It specifies a rotor set
    #         which contains information about the rotors used for the SIGABA's index rotors.
    #
    #  \param [proc] Is an object that has the same interface as rotorsim.RotorMachine. It is used to conduct
    #         the decryption operations during the verification tests.
    #
    def __init__(self, normal_rotor_set, index_rotor_set, proc = None):
        super().__init__('CSP2900 Test', normal_rotor_set, index_rotor_set, proc)
    
    ## \brief Performs the verification test.
    #
    #  \returns A boolean. A return value of True means that the test was successfull.
    #         
    def test(self):
        result = super().test()
        csp2900_state = SigabaMachineState.get_default_state(self._rotor_set, self._index_rotor_set)        
        csp2900_state.csp_2900_flag = True                        
        self._proc.set_state(csp2900_state.render_state())
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
    #  \param [normal_rotor_set] Is an object of type rotorsim.RotorSet. It specifies a rotor set
    #         which contains information about the rotors used by the Nema
    #
    #  \param [proc] Is an object that has the same interface as rotorsim.RotorMachine. It is used to conduct
    #         the decryption operations during the verification tests.
    #
    def __init__(self, normal_rotor_set, proc = None):
        super().__init__('Nema Test', proc)
        self._rotor_set = normal_rotor_set
    
    ## \brief Performs the verification test.
    #
    #  \returns A boolean. A return value of True means that the test was successfull.
    #         
    def test(self):
        result = super().test()
        nema_state = NemaState.get_default_state(self._rotor_set)        
        self._proc.set_state(nema_state.render_state())      
                                          
        decryption_result = self._proc.decrypt('hrsbvootzucrwlmgrmgvwywovnf')
        self.append_note("Decryption result: " + decryption_result)
        result = (decryption_result.lower() == 'aaaaaaaaaaaaaaaaaaaaaaaaaaa')
        
        return result


## \brief This class implements a verification test for the KL7.
#
class KL7Test(RotorMachineFuncTest):
    ## \brief Constructor. 
    #
    #  \param [normal_rotor_set] Is an object of type rotorsim.RotorSet. It specifies a rotor set
    #         which contains information about the rotors and rings used by the KL7
    #
    #  \param [proc] Is an object that has the same interface as rotorsim.RotorMachine. It is used to conduct
    #         the decryption operations during the verification tests.
    #
    def __init__(self, normal_rotor_set, proc = None):
        super().__init__('KL7 Test', proc)
        self._rotor_set = normal_rotor_set
    
    ## \brief Performs the verification test.
    #
    #  \returns A boolean. A return value of True means that the test was successfull.
    #         
    def test(self):
        result = super().test()
        kl7_state = KL7State.get_default_state(self._rotor_set)        
        self._proc.set_state(kl7_state.render_state())      
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
    #  \param [rotor_set] Is an object of type rotorsim.RotorSet. It specifies a rotor set
    #         which contains information about SG39 rotors.
    #
    #  \param [proc] Is an object that has the same interface as rotorsim.RotorMachine. It is used to conduct
    #         the decryption operations during the verification tests.
    #
    def __init__(self, rotor_set, proc = None):
        super().__init__("SG39 Test", rotor_set, proc)

    ## \brief Performs the verification test.
    #
    #  \returns A boolean. A return value of True means that the test was successfull.
    #        
    def test(self):
        result = super().test()
        sg39_state = SG39State.get_default_state(self._rotor_set)                
        self._proc.set_state(sg39_state.render_state())
        
        decryption_result = self._proc.decrypt('obkjdynovmmlwecxvyqstbepogmdskbengespfrpkrjkfivhgugknhclgzlgdqjrkwwvoprwszturkjfioyfbudxsytietcyppnyocoufqxvgozqpskhkmprdzyzcjgcszepfuppqmcitghyvpoo')
        self.append_note("Decryption result: " + decryption_result)
        result = (decryption_result.lower() == 'aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa')
        
        return result


## \brief This class serves the purpose to bundle tests for indivudual rotor machines into a composite test.
#         Using the appropriate context object it can be used for verification tests based on the TLV interface 
#         and the command line program.
#
class VerificationTests(simpletest.CompositeTest):
    ## \brief Constructor. 
    #
    #  \param [name] Is a string. It specifies an explanatory text which serves as the name of the test which is to
    #        be performed.      
    #
    #  \param [rotor_set] Is a RotorSet object. It represents the relevant rotor set.
    #         
    #  \param [index_rotor_set] Is a RotorSet object. It represents the index rotor set. This is only relevant for
    #         tests which make use of a SIGABA.
    #
    #  \param [context] Is a callable object or function that takes a function f as an argument. That function f has to have the
    #         same signature as the method inner_test. The context object is responsible for creating the machine object with
    #         which f can be called and after that uses this machine object to call the function f that it was given as
    #         a parameter.
    #
    def __init__(self, name, rotor_set, index_rotor_set, context):
        super().__init__(name)
        self._r_set = rotor_set
        self._index_r_set = index_rotor_set
        self._context = context

    ## \brief Returns the rotor set that is used for the cipher and driver submachines.
    #
    #  \returns An object with the same interface as rotorsim.RotorSet.
    #            
    @property
    def rotor_set(self):
        return self._r_set

    ## \brief Sets the rotor set that is used for the cipher and driver submachines.
    #
    #  \param [new_val] An object with the same interface as rotorsim.RotorSet that contains the new rotor set.
    #
    #  \returns Nothing.
    #            
    @rotor_set.setter
    def rotor_set(self, new_val):
        self._r_set = new_val

    ## \brief Returns the rotor set that is used for the index submachine.
    #
    #  \returns An object with the same interface as rotorsim.RotorSet.
    #            
    @property
    def index_rotor_set(self):
        return self._index_r_set

    ## \brief Sets the rotor set that is used for the index submachine.
    #
    #  \param [new_val] An object with the same interface as rotorsim.RotorSet that contains the new rotor set.
    #
    #  \returns Nothing.
    #            
    @index_rotor_set.setter
    def index_rotor_set(self, new_val):
        self._index_r_set = new_val

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

    ## \brief Performs the verification test.
    #
    #  \param [machine] Is an object with the same interface as rotorsim.RotorMachine. This object is used to
    #         do test en/decryptions.
    #
    #  \returns A boolean. A return value of True means that the test was successfull.
    #    
    def inner_test(self, machine):
        result = True
        try:            
            self.set_processor(machine)
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
    m4_state = M4EnigmaState.get_default_state().render_state()
    with tlvobject.TlvServer(server_address='sock_fjsdhfjshdkfjh') as server, RotorMachine(m4_state, server.address) as machine:
        result = inner_test(machine)        
    
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
        
        m4_state = M4EnigmaState.get_default_state().render_state()
        csp889_state = SigabaMachineState.get_default_state().render_state()
        sg39_state = SG39State.get_default_state().render_state()        
            
        with tlvobject.TlvServer(server_address='sock_fjsdhfjshdkfjh') as server, RotorMachine(m4_state, server.address) as m4_obj:
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
                perms = m4_obj.get_permutations(10)
                last_result = ((len(perms) == 11) and (len(perms[5]) == 26))
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
        
        m4_state = M4EnigmaState.get_default_state().render_state()

        with tlvobject.TlvServer(server_address='sock_fjsdhfjshdkfjh') as server, RotorMachine(m4_state, server.address) as m4_obj:
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
        
        machine_state = M4EnigmaState.get_default_state().render_state()

        with tlvobject.TlvServer(server_address='sock_fjsdhfjshdkfjh') as server, RotorMachine(machine_state, server.address) as machine:
            try:  
                randomizer_params = machine.get_randomizer_params()
                
                result = result and (randomizer_params == [])
                if not result:
                    self.append_note("Incorrect list of randomizer parameters returned for M4: {}".format(str(randomizer_params)))
                else:
                    self.append_note("Randomizer parameters returned for M4: {}".format(str(randomizer_params)))
                    
                machine_state = SG39State.get_default_state().render_state()
                machine.set_state(machine_state)
                
                randomizer_params = machine.get_randomizer_params()
                this_result = (randomizer_params == ['one', 'two', 'three'])
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
    # \param [list_of_states] Is a list of objects which have the same interface as GenericRotorMachineState.
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
        
        machine_state = M4EnigmaState.get_default_state().render_state()

        with tlvobject.TlvServer(server_address='sock_fjsdhfjshdkfjh') as server, RotorMachine(machine_state, server.address) as machine:
            try:  
                for i in self._all_states:
                    # Set machine to test state
                    machine.set_state(i.render_state())
                    self.append_note("Testing {} state".format(machine.get_description()))                    
                    # Randomize machine state
                    machine.randomize_state()
                    # Save randomized state
                    state1 = machine.get_state()
                    test_message = machine.encrypt('diesisteintest')
                    # Destroy randomized state
                    machine.randomize_state()
                    
                    # Attempt to reload randomized state
                    if not i.load_from_data(state1):
                        self.append_note("Loding failed!!")
                        result = False
                        break
                    
                    # Loading was successfull. Reset machine to randomized state
                    machine.set_state(i.render_state())
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
    enigma_verification_test = VerificationTests("Enigma verification test", RotorSet.get_std_set('enigma'), None, context)
    enigma_verification_test.add(M4EnigmaTest(enigma_verification_test.rotor_set))
    enigma_verification_test.add(ServicesUhrTest(enigma_verification_test.rotor_set))    
    enigma_verification_test.add(KDTest(enigma_verification_test.rotor_set))
    enigma_verification_test.add(TirpitzTest(enigma_verification_test.rotor_set))    
    enigma_verification_test.add(AbwehrTest(enigma_verification_test.rotor_set))
    enigma_verification_test.add(RailwayTest(enigma_verification_test.rotor_set))    
    enigma_verification_test.add(TypexTest(enigma_verification_test.rotor_set))

    sigaba_verification_test = VerificationTests("SIGABA verification test", RotorSet.get_std_set('sigaba'), RotorSet.get_std_set('sigaba_index'), context)
    sigaba_verification_test.add(CSP889Test(sigaba_verification_test.rotor_set, sigaba_verification_test.index_rotor_set))
    sigaba_verification_test.add(CSP2900Test(sigaba_verification_test.rotor_set, sigaba_verification_test.index_rotor_set))    

    nema_verification_test = VerificationTests("Nema verification test", RotorSet.get_std_set('nema'), None, context)
    nema_verification_test.add(NemaTest(nema_verification_test.rotor_set))

    kl7_verification_test = VerificationTests("KL7 verification test", RotorSet.get_std_set('kl7'), None, context)
    kl7_verification_test.add(KL7Test(kl7_verification_test.rotor_set))

    sg39_verification_test = VerificationTests("SG39 verification test", RotorSet.get_std_set('sg39'), None, context)
    sg39_verification_test.add(SG39Test(sg39_verification_test.rotor_set))
    
    all_tests = simpletest.CompositeTest('rotorsim')    
    
    if not verification_only:
        performance_test = RotorMachinePerfTest("rotorsim performance test", test_data, num_iterations)
        functional_test = RotorMachineFuncTests("rotorsim functional test")
        test_states = []
        test_states.append(M4EnigmaState.get_default_state())
        test_states.append(ServicesEnigmaState.get_default_state('Services'))        
        test_states.append(UnsteckeredEnigmaState.get_default_state('AbwehrEnigma'))
        test_states.append(UnsteckeredEnigmaState.get_default_state('KDEnigma'))
        test_states.append(UnsteckeredEnigmaState.get_default_state('TirpitzEnigma'))
        test_states.append(UnsteckeredEnigmaState.get_default_state('RailwayEnigma'))
        test_states.append(SigabaMachineState.get_default_state())
        test_states.append(KL7State.get_default_state())
        test_states.append(SG39State.get_default_state())
        test_states.append(TypexState.get_default_state())
        test_states.append(NemaState.get_default_state())        
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
