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

import simpletest
from rotorsim import *

class EnigmaFuncTest(simpletest.SimpleTest):
    def __init__(self, name, enig_rotor_set, proc):
        super().__init__(name)
        self._proc = proc
        self._rotor_set = enig_rotor_set
        self._help = Permutation()
        
    def set_processor(self, proc):
        self._proc = proc


class M4EnigmaTest(EnigmaFuncTest):
    def __init__(self, enig_rotor_set, proc = None):
        super().__init__("M4 Verification Test", enig_rotor_set, proc)
        
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


class M3UhrTest(EnigmaFuncTest):
    def __init__(self, enig_rotor_set, proc = None):
        super().__init__("M3 Uhr Test", enig_rotor_set, proc)
        
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


class KDTest(EnigmaFuncTest):
    def __init__(self, enig_rotor_set, proc = None):
        super().__init__("KD Test", enig_rotor_set, proc)
        
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


class TirpitzTest(EnigmaFuncTest):
    def __init__(self, enig_rotor_set, proc = None):
        super().__init__("Tirpitz Test", enig_rotor_set, proc)
        
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


class AbwehrTest(EnigmaFuncTest):
    def __init__(self, enig_rotor_set, proc = None):
        super().__init__("Abwehr Test", enig_rotor_set, proc)
        
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


class RailwayTest(EnigmaFuncTest):
    def __init__(self, enig_rotor_set, proc = None):
        super().__init__("Railway Test", enig_rotor_set, proc)
        
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


class AllEnigmaTests(simpletest.CompositeTest):
    def __init__(self, name):
        super().__init__(name)
        self._r_set = EnigmaRotorSet()
        self.add(M4EnigmaTest(self._r_set))
        self.add(M3UhrTest(self._r_set))
        self.add(KDTest(self._r_set))
        self.add(TirpitzTest(self._r_set))
        self.add(AbwehrTest(self._r_set))
        self.add(RailwayTest(self._r_set))
    
    def set_processor(self, proc):
        for i in self._test_cases:
            i.set_processor(proc)
    
    def test(self):
        result = True
        m4_state = RotorMachine.load_machine_state('reference/Enigma M4 Test 1.ini')
        with tlvobject.TlvServer(server_address='sock_fjsdhfjshdkfjh') as server, RotorMachine(m4_state, server.address) as machine:
            try:
                result = self._r_set.load('reference/enigma_rotor_set.ini')
                
                if not result:
                    self.append_note('Unable to load Enigma rotor set data')
                else:                    
                    self.set_processor(machine)
                    result = super().test()
            except:
                self.append_note("EXCEPTON!!!!")
                result = False
        
        return result                
        

class RotorMachineFuncTests(simpletest.SimpleTest):
    def __init__(self, name):
        super().__init__(name)
        
    def test(self):
        result = super().test()
        
        m4_state = RotorMachine.load_machine_state('reference/Enigma M4 Test 1.ini')
        csp2900_state = RotorMachine.load_machine_state('reference/CSP 2900 Test.ini')
        sg39_state = RotorMachine.load_machine_state('reference/SG39 Test.ini')        
            
        with tlvobject.TlvServer(server_address='sock_fjsdhfjshdkfjh') as server, RotorMachine(m4_state, server.address) as m4_obj:
            try:
                original_state = m4_obj.get_state()
                
                dec_result = m4_obj.decrypt('nczwvusx')
                last_result = (dec_result == 'vonvonjl')
                result = result and last_result
                if not last_result:
                    self.append_note("Unexpected M4 decryption result: " + dec_result)
                
                dec_result = m4_obj.decrypt('nczwvusx')
                last_result = (dec_result != 'vonvonjl')
                result = result and last_result
                if not last_result:
                    self.append_note("Unexpected M4 decryption result: " + dec_result)
                
                m4_obj.set_state(original_state)
                
                dec_result = m4_obj.decrypt('nczwvusx')                
                last_result = (dec_result == 'vonvonjl')
                result = result and last_result
                if not last_result:
                    self.append_note("Unexpected M4 decryption result: " + dec_result)
                                
                m4_obj.set_state(original_state)                        

                step_result = m4_obj.step(5)                
                last_result = ((len(step_result) == 5) and (step_result[4] == 'vjnf'))
                result = result and last_result
                if not last_result:
                    self.append_note("Unexpected rotor positon: " + str(step_result))
              
                description = m4_obj.get_description()
                last_result = (description == 'M4Enigma')
                result = result and last_result
                if not last_result:
                    self.append_note("Unexpected machine description: " + description)
                                
                m4_obj.set_state(csp2900_state)
                
                description = m4_obj.get_description()
                last_result = (description == 'CSP2900')
                result = result and last_result
                if not last_result:
                    self.append_note("Unexpected machine description: " + description)
                
                setup_step_result = m4_obj.sigaba_setup(1, 3)
                last_result = ((len(setup_step_result) == 3) and (setup_step_result[2] == '00000llplofvsvd'))
                result = result and last_result
                if not last_result:
                    self.append_note("Unexpected rotor position: " + str(setup_step_result))
               
                m4_obj.set_state(sg39_state)
                
                rotor_pos = m4_obj.get_rotor_positions()
                last_result = (rotor_pos == 'frqdaph')
                result = result and last_result
                if not last_result:
                    self.append_note("Unexpected rotor positions: " + rotor_pos)
                    
                perms = m4_obj.get_permutations(10)
                last_result = ((len(perms) == 11) and (len(perms[5]) == 26))
                result = result and last_result
                if not last_result:
                    self.append_note("Unexpected permutation result: " + str(perms))                
                
            except:
                self.append_note("EXCEPTON!!!!")
                result = False
        
        return result                

        
class RotorMachinePerfTest(simpletest.SimpleTest):
    def __init__(self, name, test_data, num_iterations = 22000):
        super().__init__(name)
        self._iterations = num_iterations
        self._test_data = test_data
            
    def test(self):
        result = super().test()

        m4_state = RotorMachine.load_machine_state('reference/Enigma M4 Test 1.ini')

        with tlvobject.TlvServer(server_address='sock_fjsdhfjshdkfjh') as server, RotorMachine(m4_state, server.address) as m4_obj:
            try:  
                dec_result = m4_obj.decrypt('nczwvusx')  
                result = result and (dec_result == 'vonvonjl')
                
                if not result:
                    self.append_note("M4 message not properly decrypted: {}".format())
                else:                    
                    jetzt = datetime.datetime.now()
                    
                    for i in range(self._iterations):
                        m4_obj.decrypt(self._test_data)
                    
                    spaeter = datetime.datetime.now()
                    self.append_note("Time needed for {} decryptions: {}".format(self._iterations, str(spaeter - jetzt)))
            except:
                self.append_note("EXCEPTON!!!!")
                result = False
        
        return result

    
def get_module_test(test_data = 'aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa', num_iterations = 2500):
    performance_test = RotorMachinePerfTest("rotorsim performance test", test_data, num_iterations)
    functional_test = RotorMachineFuncTests("rotorsim functional test")
    enigma_verification_test = AllEnigmaTests("Enigma verification test")
    all_tests = simpletest.CompositeTest('All rotorsim tests')    
    all_tests.add(functional_test)
    all_tests.add(performance_test)
    all_tests.add(enigma_verification_test)
    
    return all_tests

def execute_tests(test_data, num_iterations):
    tests = get_module_test(test_data, num_iterations)
    test_result = tests.test()
    tests.print_notes()
