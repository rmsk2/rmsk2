################################################################################
# Copyright 2015 Martin Grap
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
from rotorsim import *

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
        enigma_M4_state = M4EnigmaState(self._rotor_set)
        enigma_M4_state.insert_rotor('fast', es.WALZE_I, es.WALZE_I, self._help.from_val('v'), self._help.from_val('a'))
        enigma_M4_state.insert_rotor('middle', es.WALZE_IV, es.WALZE_IV, self._help.from_val('a'), self._help.from_val('n'))        
        enigma_M4_state.insert_rotor('slow', es.WALZE_II, es.WALZE_II, self._help.from_val('a'), self._help.from_val('j'))  
        enigma_M4_state.insert_rotor('griechenwalze', es.WALZE_BETA, es.WALZE_BETA, self._help.from_val('a'), self._help.from_val('v'))          
        enigma_M4_state.insert_rotor('umkehrwalze', es.UKW_B_DN, es.UKW_B_DN, 0, 0)
        enigma_M4_state.set_stecker_brett('atbldfgjhmnwopqyrzvx')                      
                
        self._proc.set_state(enigma_M4_state.render_state())
        decryption_result = self._proc.decrypt('nczwvusxpnyminhzxmqxsfwxwlkjahshnmcoccakuqpmkcsmhkseinjusblkiosxckubhmllxcsjusrrdvkohulxwccbgvliyxeoahxrhkkfvdrewez')
        self.append_note("Decryption result: " + decryption_result)
        result = (decryption_result.lower() == 'vonvonjlooksjhffttteinseinsdreizwoyyqnnsneuninhaltxxbeiangriffunterwassergedruecktywabosxletztergegnerstandnulachtd')
        
        return result

## \brief This class implements a verification test for the M3 or Services Enigma with Uhr.
#
class M3UhrTest(EnigmaFuncTest):
    ## \brief Constructor. 
    #
    #  \param [enig_rotor_set] Is an object of type rotorsim.EnigmaRotorSet. It specifies a rotor set
    #         which contains information about Enigma rotors and rings.
    #
    #  \param [proc] Is an object that has the same interface as rotorsim.RotorMachine. It is used to conduct
    #         the decryption operations during the verification tests.
    #
    def __init__(self, enig_rotor_set, proc = None):
        super().__init__("M3 Uhr Test", enig_rotor_set, proc)

    ## \brief Performs the verification test.
    #
    #  \returns A boolean. A return value of True means that the test was successfull.
    #        
    def test(self):
        result = super().test()
        enigma_I_state = ServicesEnigmaState('M3', self._rotor_set)
        enigma_I_state.insert_rotor('fast', es.WALZE_III, es.WALZE_III, self._help.from_val('h'), self._help.from_val('z'))
        enigma_I_state.insert_rotor('middle', es.WALZE_IV, es.WALZE_IV, self._help.from_val('z'), self._help.from_val('t'))        
        enigma_I_state.insert_rotor('slow', es.WALZE_I, es.WALZE_I, self._help.from_val('p'), self._help.from_val('r'))  
        enigma_I_state.insert_rotor('umkehrwalze', es.UKW_B, es.UKW_B, 0, 0)
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
        self._rotor_set.change_ukw_d('azbpcxdqetfogshvirjyknlmuw')
        enigma_kd_state = UnsteckeredEnigmaState('KDEnigma', self._rotor_set, es.WALZE_KD_ETW)
        enigma_kd_state.insert_rotor('fast', es.WALZE_KD_V, es.WALZE_KD_V, self._help.from_val('b'), self._help.from_val('m'))
        enigma_kd_state.insert_rotor('middle', es.WALZE_KD_VI, es.WALZE_KD_VI, self._help.from_val('r'), self._help.from_val('f'))        
        enigma_kd_state.insert_rotor('slow', es.WALZE_KD_II, es.WALZE_KD_II, self._help.from_val('q'), self._help.from_val('c'))  
        enigma_kd_state.insert_rotor('umkehrwalze', es.UKW_D, es.UKW_D, 0, 0)                     
                
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
        self._rotor_set.change_reflector(es.TYPEX_SP_02390_UKW, 'arbycudheqfsglixjpknmotwvz')
        typex_state = TypexState(self._rotor_set)
        typex_state.insert_rotor('stator1', es.TYPEX_SP_02390_E, es.TYPEX_SP_02390_E, 0, 0)
        typex_state.insert_rotor('stator2', es.TYPEX_SP_02390_D, es.TYPEX_SP_02390_D, 0, 0)        
        typex_state.insert_rotor('fast', es.TYPEX_SP_02390_C, es.TYPEX_SP_02390_C, 0, 0, INSERT_REVERSE)
        typex_state.insert_rotor('middle', es.TYPEX_SP_02390_B, es.TYPEX_SP_02390_B, 0, 0)        
        typex_state.insert_rotor('slow', es.TYPEX_SP_02390_A, es.TYPEX_SP_02390_A, 0, 0)  
        typex_state.insert_rotor('umkehrwalze', es.TYPEX_SP_02390_UKW, es.TYPEX_SP_02390_UKW, 0, 0)                     
                
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
        enigma_t_state = UnsteckeredEnigmaState('TirpitzEnigma', self._rotor_set, es.WALZE_T_ETW)
        enigma_t_state.insert_rotor('fast', es.WALZE_T_V, es.WALZE_T_V, self._help.from_val('b'), self._help.from_val('m'))
        enigma_t_state.insert_rotor('middle', es.WALZE_T_VIII, es.WALZE_T_VIII, self._help.from_val('r'), self._help.from_val('f'))        
        enigma_t_state.insert_rotor('slow', es.WALZE_T_VII, es.WALZE_T_VII, self._help.from_val('q'), self._help.from_val('c'))  
        enigma_t_state.insert_rotor('umkehrwalze', es.UKW_T, es.UKW_T, self._help.from_val('k'), self._help.from_val('a'))                      
                        
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
        enigma_abw_state = UnsteckeredEnigmaState('AbwehrEnigma', self._rotor_set, es.WALZE_ABW_ETW)
        enigma_abw_state.insert_rotor('slow', es.WALZE_ABW_III, es.WALZE_ABW_III, 0, 0)
        enigma_abw_state.insert_rotor('middle', es.WALZE_ABW_II, es.WALZE_ABW_II, 0, 0)        
        enigma_abw_state.insert_rotor('fast', es.WALZE_ABW_I, es.WALZE_ABW_I, 0, 0)  
        enigma_abw_state.insert_rotor('umkehrwalze', es.UKW_ABW, es.UKW_ABW, 0, 0)                         
                        
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
        enigma_rb_state = UnsteckeredEnigmaState('RailwayEnigma', self._rotor_set, es.WALZE_RB_ETW)
        enigma_rb_state.insert_rotor('fast', es.WALZE_RB_III, es.WALZE_RB_III, 0, 0)
        enigma_rb_state.insert_rotor('middle', es.WALZE_RB_II, es.WALZE_RB_II, 0, 0)        
        enigma_rb_state.insert_rotor('slow', es.WALZE_RB_I, es.WALZE_RB_I, 0, 0)  
        enigma_rb_state.insert_rotor('umkehrwalze', es.UKW_RB, es.UKW_RB, 0, 0)                             
                        
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
        csp889_state = SigabaMachineState(self._rotor_set, self._index_rotor_set)
        csp889_state.crypt.insert_sigaba_rotor('r_zero', SIGABA_ROTOR_0, 'o')
        csp889_state.crypt.insert_sigaba_rotor('r_one', SIGABA_ROTOR_1, 'o')        
        csp889_state.crypt.insert_sigaba_rotor('r_two', SIGABA_ROTOR_2, 'm', INSERT_REVERSE)        
        csp889_state.crypt.insert_sigaba_rotor('r_three', SIGABA_ROTOR_3, 'o')
        csp889_state.crypt.insert_sigaba_rotor('r_four', SIGABA_ROTOR_4, 'o')        

        csp889_state.driver.insert_sigaba_rotor('stator_l', SIGABA_ROTOR_5, 'o')
        csp889_state.driver.insert_sigaba_rotor('slow', SIGABA_ROTOR_6, 'o')        
        csp889_state.driver.insert_sigaba_rotor('fast', SIGABA_ROTOR_7, 'm', INSERT_REVERSE)        
        csp889_state.driver.insert_sigaba_rotor('middle', SIGABA_ROTOR_8, 'o')
        csp889_state.driver.insert_sigaba_rotor('stator_r', SIGABA_ROTOR_9, 'o')        

        csp889_state.index.insert_sigaba_rotor('i_zero', SIGABA_INDEX_0, '0')
        csp889_state.index.insert_sigaba_rotor('i_one', SIGABA_INDEX_1, '0')        
        csp889_state.index.insert_sigaba_rotor('i_two', SIGABA_INDEX_2, '0', INSERT_REVERSE)        
        csp889_state.index.insert_sigaba_rotor('i_three', SIGABA_INDEX_3, '0')
        csp889_state.index.insert_sigaba_rotor('i_four', SIGABA_INDEX_4, '0')        
                        
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
        csp2900_state = SigabaMachineState(self._rotor_set, self._index_rotor_set)
        csp2900_state.crypt.insert_sigaba_rotor('r_zero', SIGABA_ROTOR_0, 'o')
        csp2900_state.crypt.insert_sigaba_rotor('r_one', SIGABA_ROTOR_1, 'o')        
        csp2900_state.crypt.insert_sigaba_rotor('r_two', SIGABA_ROTOR_2, 'm', INSERT_REVERSE)        
        csp2900_state.crypt.insert_sigaba_rotor('r_three', SIGABA_ROTOR_3, 'o')
        csp2900_state.crypt.insert_sigaba_rotor('r_four', SIGABA_ROTOR_4, 'o')        

        csp2900_state.driver.insert_sigaba_rotor('stator_l', SIGABA_ROTOR_5, 'o')
        csp2900_state.driver.insert_sigaba_rotor('slow', SIGABA_ROTOR_6, 'o')        
        csp2900_state.driver.insert_sigaba_rotor('fast', SIGABA_ROTOR_7, 'm', INSERT_REVERSE)        
        csp2900_state.driver.insert_sigaba_rotor('middle', SIGABA_ROTOR_8, 'o')
        csp2900_state.driver.insert_sigaba_rotor('stator_r', SIGABA_ROTOR_9, 'o')        

        csp2900_state.index.insert_sigaba_rotor('i_zero', SIGABA_INDEX_0, '0')
        csp2900_state.index.insert_sigaba_rotor('i_one', SIGABA_INDEX_1, '0')        
        csp2900_state.index.insert_sigaba_rotor('i_two', SIGABA_INDEX_2, '0', INSERT_REVERSE)        
        csp2900_state.index.insert_sigaba_rotor('i_three', SIGABA_INDEX_3, '0')
        csp2900_state.index.insert_sigaba_rotor('i_four', SIGABA_INDEX_4, '0')
        
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
        nema_state = NemaState(self._rotor_set)
        nema_state.insert_nema_rotor('drive1', NEMA_DRIVE_WHEEL_22, 'v')
        nema_state.insert_nema_rotor('contact2', NEMA_ROTOR_D, 'e')        
        nema_state.insert_nema_rotor('drive3', NEMA_DRIVE_WHEEL_15, 'o')        
        nema_state.insert_nema_rotor('contact4', NEMA_ROTOR_C, 's')
        nema_state.insert_nema_rotor('drive5', NEMA_DRIVE_WHEEL_14, 'q')        
        nema_state.insert_nema_rotor('contact6', NEMA_ROTOR_B, 'z')
        nema_state.insert_nema_rotor('drive7', NEMA_DRIVE_WHEEL_13, 'p')        
        nema_state.insert_nema_rotor('contact8', NEMA_ROTOR_A, 'q')        
        nema_state.insert_nema_rotor('drive9', NEMA_DRIVE_WHEEL_12, 'q')
        nema_state.insert_nema_rotor('contact10', NEMA_UKW, 'm')        
        
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
        kl7_state = KL7State(self._rotor_set)
        kl7_state.insert_kl7_rotor('kl7_rotor_1', KL7_ROTOR_A, KL7_RING_1, 'k', 26, 'f')
        kl7_state.insert_kl7_rotor('kl7_rotor_2', KL7_ROTOR_B, KL7_RING_2, 'a', 0, 'a')        
        kl7_state.insert_kl7_rotor('kl7_rotor_3', KL7_ROTOR_C, KL7_RING_3, 'a', 0, 'a')                
        kl7_state.insert_stationary_rotor(KL7_ROTOR_L, 16)
        kl7_state.insert_kl7_rotor('kl7_rotor_5', KL7_ROTOR_D, KL7_RING_4, 'a', 0, 'a')
        kl7_state.insert_kl7_rotor('kl7_rotor_6', KL7_ROTOR_E, KL7_RING_5, 'a', 0, 'a')
        kl7_state.insert_kl7_rotor('kl7_rotor_7', KL7_ROTOR_F, KL7_RING_6, 'a', 0, 'a')
        kl7_state.insert_kl7_rotor('kl7_rotor_8', KL7_ROTOR_G, KL7_RING_7, 'a', 0, 'a')        
        
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
    #  \param [enig_rotor_set] Is an object of type rotorsim.RotorSet. It specifies a rotor set
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
        self._rotor_set.change_reflector(ID_SG39_UKW, 'awbicvdketfmgnhzjulopqrysx')
        sg39_state = SG39State(self._rotor_set)
        sg39_state.set_plugboard('ldtrmihoncpwjkbyevsaxgfzuq')
        sg39_state.insert_sg39_rotor('rotor_1', SG39_ROTOR_5, 'd', [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0])
        sg39_state.insert_sg39_rotor('rotor_2', SG39_ROTOR_1, 'q', [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0])        
        sg39_state.insert_sg39_rotor('rotor_3', SG39_ROTOR_4, 'r', [1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0])  
        sg39_state.insert_sg39_rotor('rotor_4', SG39_ROTOR_3, 'f', [])  
        sg39_state.configure_sg39_drive_wheel('rotor_1', 'h', [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0])
        sg39_state.configure_sg39_drive_wheel('rotor_2', 'p', [1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1])        
        sg39_state.configure_sg39_drive_wheel('rotor_3', 'a', [0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0])                
        sg39_state.insert_rotor('umkehrwalze', ID_SG39_UKW, ID_SG39_UKW, 0, 0)                     
                
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
    #  \param [rotor_set_file] Is a string. It specifies the name of the file which contains the relevant rotor set
    #         information.
    #
    #  \param [index_rotor_set_file] Is a string or None. It specifies the name of the file which contains the index rotor set
    #         information. This is only relevant for tests which make use of a SIGABA
    #
    #  \param [context] Is a callable object or function that takes a function f as an argument. That function f has to have the
    #         same signature as the method inner_test. The context object is responsible for creating the machine object with
    #         which f can be called and after that uses this machine object to call the function f that it was given as
    #         a parameter.
    #
    def __init__(self, name, rotor_set_file, index_rotor_set_file, context):
        super().__init__(name)
        self._rotor_set_file = rotor_set_file
        self._index_rotor_set_file = index_rotor_set_file
        self._r_set = RotorSet()
        self._index_r_set = RotorSet()
        self._context = context
    
    @property
    def rotor_set(self):
        return self._r_set

    @rotor_set.setter
    def rotor_set(self, new_val):
        self._r_set = new_val

    @property
    def index_rotor_set(self):
        return self._index_r_set

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
            result = self._r_set.load(self._rotor_set_file)
            
            if self._index_rotor_set_file != None:
                result = result and self._index_r_set.load(self._index_rotor_set_file)
            
            if not result:
                self.append_note('Unable to load rotor set data')
            else:                    
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
    m4_state = RotorMachine.load_machine_state('reference/Enigma M4 Test 1.ini')
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
        
        m4_state = RotorMachine.load_machine_state('reference/Enigma M4 Test 1.ini')
        csp2900_state = RotorMachine.load_machine_state('reference/CSP 2900 Test.ini')
        sg39_state = RotorMachine.load_machine_state('reference/SG39 Test.ini')        
            
        with tlvobject.TlvServer(server_address='sock_fjsdhfjshdkfjh') as server, RotorMachine(m4_state, server.address) as m4_obj:
            try:
                original_state = m4_obj.get_state()
                
                # Do a simple test decryptions
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
                                
                m4_obj.set_state(csp2900_state)
                                
                # Verify that returned description changed to new machine type
                description = m4_obj.get_description()
                last_result = (description == 'CSP2900')
                result = result and last_result
                if not last_result:
                    self.append_note("Unexpected machine description: " + description)
                
                # Test sigaba_setup() method                
                setup_step_result = m4_obj.sigaba_setup(1, 3)
                last_result = ((len(setup_step_result) == 3) and (setup_step_result[2] == '00000llplofvsvd'))
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


## \brief This class performs a performance test of the TLV rotor machine interface.
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

        m4_state = RotorMachine.load_machine_state('reference/Enigma M4 Test 1.ini')

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


## \brief Returns a simpletest.SimpleTest object that allows to perform all the tests defined in this module.
#
#  \param [test_data] Is a string. It specifies the sample text that is used to measure the decryption speed.
#
#  \param [num_iterations] Is an integer. It specifies how many test decryptions are to be performed.
#         
#  \returns A simpletest.CompositeTest object.
#                
def get_module_test(test_data = 'aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa', num_iterations = 2500):
    performance_test = RotorMachinePerfTest("rotorsim performance test", test_data, num_iterations)
    functional_test = RotorMachineFuncTests("rotorsim functional test")

    enigma_verification_test = VerificationTests("Enigma verification test", 'reference/enigma_rotor_set.ini', None, tlv_context)
    enigma_verification_test.rotor_set = EnigmaRotorSet()
    enigma_verification_test.add(M4EnigmaTest(enigma_verification_test.rotor_set))
    enigma_verification_test.add(M3UhrTest(enigma_verification_test.rotor_set))    
    enigma_verification_test.add(KDTest(enigma_verification_test.rotor_set))
    enigma_verification_test.add(TirpitzTest(enigma_verification_test.rotor_set))    
    enigma_verification_test.add(AbwehrTest(enigma_verification_test.rotor_set))
    enigma_verification_test.add(RailwayTest(enigma_verification_test.rotor_set))    
    enigma_verification_test.add(TypexTest(enigma_verification_test.rotor_set))

    sigaba_verification_test = VerificationTests("SIGABA verification test", 'reference/sigaba_rotor_set.ini', 'reference/sigaba_rotor_set_index.ini', tlv_context)
    sigaba_verification_test.add(CSP889Test(sigaba_verification_test.rotor_set, sigaba_verification_test.index_rotor_set))
    sigaba_verification_test.add(CSP2900Test(sigaba_verification_test.rotor_set, sigaba_verification_test.index_rotor_set))    

    nema_verification_test = VerificationTests("Nema verification test", 'reference/nema_rotor_set.ini', None, tlv_context)
    nema_verification_test.add(NemaTest(nema_verification_test.rotor_set))

    kl7_verification_test = VerificationTests("KL7 verification test", 'reference/kl7_rotor_set.ini', None, tlv_context)
    kl7_verification_test.add(KL7Test(kl7_verification_test.rotor_set))

    sg39_verification_test = VerificationTests("SG39 verification test", 'reference/sg39_rotor_set.ini', None, tlv_context)
    sg39_verification_test.add(SG39Test(sg39_verification_test.rotor_set))
    
    all_tests = simpletest.CompositeTest('rotorsim')    
    all_tests.add(functional_test)
    all_tests.add(performance_test)
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
