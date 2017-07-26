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

## @package enigrotorset C++ code generator written in python3 for creating Enigma rotor set data.
#           
#  Code generator that creates the files enigma_rotor_set.h and enigma_rotor_set.cpp from an
#  abstract definition of all permutations and ring data (notches) that are used in the
#  implementation of the enigma variants, including the Typex.
#  New rotors can be included by adding entries to the known_wheels data structure.
#
# \file pyrmsk2/enigrotorset.py
# \brief Code generator that creates the files enigma_rotor_set.h and enigma_rotor_set.cpp from an
#        abstract definition of all permutations and ring data (notches) that are used in the
#        implementation of the enigma variants, including the Typex.

import sys

STD_ALPHA = "abcdefghijklmnopqrstuvwxyz"

# Enigma_I (Services), M3 and M4 rotor ids
WALZE_I = 0
WALZE_II = 1
WALZE_III = 2
WALZE_IV = 3
WALZE_V = 4
WALZE_VI = 5
WALZE_VII = 6
WALZE_VIII = 7

# Abwehr Enigma rotor ids
WALZE_ABW_I = 8
WALZE_ABW_II = 9
WALZE_ABW_III = 10
WALZE_ABW_ETW = 11

# Railway Enigma rotor ids
WALZE_RB_I = 12
WALZE_RB_II = 13
WALZE_RB_III = 14
WALZE_RB_ETW = 15

# Tirpitz Enigma rotor ids
WALZE_T_I = 16
WALZE_T_II = 17
WALZE_T_III = 18
WALZE_T_IV = 19
WALZE_T_V = 20
WALZE_T_VI = 21
WALZE_T_VII = 22
WALZE_T_VIII = 23
WALZE_T_ETW = 24

# Typex rotor ids for rotor set SP_02390
TYPEX_SP_02390_A = 25
TYPEX_SP_02390_B = 26
TYPEX_SP_02390_C = 27
TYPEX_SP_02390_D = 28
TYPEX_SP_02390_E = 29
TYPEX_SP_02390_F = 30
TYPEX_SP_02390_G = 31
TYPEX_ETW = 32

# KD Enigma rotor ids
WALZE_KD_I = 33
WALZE_KD_II = 34
WALZE_KD_III = 35
WALZE_KD_IV = 36
WALZE_KD_V = 37
WALZE_KD_VI = 38
WALZE_KD_ETW = 39


# Typex rotor ids for rotor set Y_269
TYPEX_Y_269_A = 40
TYPEX_Y_269_B = 41
TYPEX_Y_269_C = 42
TYPEX_Y_269_D = 43
TYPEX_Y_269_E = 44
TYPEX_Y_269_F = 45
TYPEX_Y_269_G = 46
TYPEX_Y_269_H = 47
TYPEX_Y_269_I = 48
TYPEX_Y_269_J = 49
TYPEX_Y_269_K = 50
TYPEX_Y_269_L = 51
TYPEX_Y_269_M = 52
TYPEX_Y_269_N = 53


# Rotor ids for Umkehrwalze A, B, C and D of Services Enigma
UKW_A = 100
UKW_B = 101
UKW_C = 102
UKW_D = 111

# M4 rotor ids for M4 reflectors and greek wheels
UKW_B_DN = 103
UKW_C_DN = 104
WALZE_BETA = 105
WALZE_GAMMA = 106

# Rotor id for Abwehr Enigma reflector
UKW_ABW = 107

# Rotor id for Railway Enigma reflector
UKW_RB = 108

# Rotor id for Tirpitz Enigma reflector
UKW_T = 109

# Rotor id for Typex reflector in rotor set SP_02390
TYPEX_SP_02390_UKW = 110

# Rotor id for Typex reflector in rotor set Y_269
TYPEX_Y_269_UKW = 112


# Permutations for Services Enigma, M3 and M4 rotors
PERM_I   = "ekmflgdqvzntowyhxuspaibrcj"
PERM_II  = "ajdksiruxblhwtmcqgznpyfvoe"
PERM_III = "bdfhjlcprtxvznyeiwgakmusqo"
PERM_IV  = "esovpzjayquirhxlnftgkdcmwb"
PERM_V   = "vzbrgityupsdnhlxawmjqofeck"
PERM_VI  = "jpgvoumfyqbenhzrdkasxlictw"
PERM_VII = "nzjhgrcxmyswboufaivlpekqdt"
PERM_VIII= "fkqhtlxocbjspdzramewniuygv"

# Notches for Services Enigma, M3 and M4 rotors
NOTCH_I    = "q"
NOTCH_II   = "e"
NOTCH_III  = "v"
NOTCH_IV   = "j"
NOTCH_V    = "z"
NOTCH_VI   = "zm"
NOTCH_VII  = "zm"
NOTCH_VIII = "zm"

# Permutations for M4 greek wheels
PERM_BETA = "leyjvcnixwpbqmdrtakzgfuhos"
PERM_GAMMA= "fsokanuerhmbtiycwlqpzxvgjd"

# Permutations for Services, M3 and M4 reflectors
PERM_UKW_A    = "ejmzalyxvbwfcrquontspikhgd"
PERM_UKW_B    = "yruhqsldpxngokmiebfzcwvjat"
PERM_UKW_C    = "fvpjiaoyedrzxwgctkuqsbnmhl"
PERM_UKW_D    = "fowulaqysrtezvbxgjikdncphm"
PERM_UKW_B_DN = "enkqauywjicopblmdxzvfthrgs"
PERM_UKW_C_DN = "rdobjntkvehmlfcwzaxgyipsuq"

# Permutations for Abwehr Enigma rotors
PERM_ABW_I     = "dmtwsilruyqnkfejcazbpgxohv"
PERM_ABW_II    = "hqzgpjtmoblncifdyawveusrkx"
PERM_ABW_III   = "uqntlszfmrehdpxkibvygjcwoa"
PERM_ABW_UKW   = "rulqmzjsygocetkwdahnbxpvif"
PERM_ABW_ETW   = "qwertzuioasdfghjkpyxcvbnml"

# Notches for Abwehr Enigma rotors
NOTCH_ABW_I    = "suvwzabcefgiklopq"
NOTCH_ABW_II   = "stvyzacdfghkmnq"
NOTCH_ABW_III  = "uwxaefhkmnr"

# Permutations for Railway Enigma rotors
PERM_RB_I  = "jgdqoxuscamifrvtpnewkblzyh"
PERM_RB_II = "ntzpsfbokmwrcjdivlaeyuxhgq"
PERM_RB_III= "jviubhtcdyakeqzposgxnrmwfl"
PERM_RB_UKW= "qyhognecvpuztfdjaxwmkisrbl"
PERM_RB_ETW= "qwertzuioasdfghjkpyxcvbnml"

# Notches for Railway Enigma rotors
NOTCH_RB_I     = "n"
NOTCH_RB_II    = "e"
NOTCH_RB_III   = "y"

# Permutations for Tirpitz Enigma rotors
PERM_T_I   = "kptyuelocvgrfqdanjmbswhzxi"
PERM_T_II  = "uphzlweqmtdjxcaksoigvbyfnr"
PERM_T_III = "qudlyrfekonvzaxwhmgpjbsict"
PERM_T_IV  = "ciwtbkxnrespflydagvhquojzm"
PERM_T_V   = "uaxgisnjbverdylfzwtpckohmq"
PERM_T_VI  = "xfuzgalvhcnysewqtdmrbkpioj"
PERM_T_VII = "bjvftxplnayozikwgdqeruchsm"
PERM_T_VIII= "ymtpnzhwkodajxeluqvgcbisfr"
PERM_T_ETW = "kzrouqhyaigblwvstdxfpnmcje"
PERM_T_UKW = "gekpbtaumocniljdxzyfhwvqsr"

# Notches for Tirpitz Enigma rotors
NOTCH_T_I    = "wzekq"
NOTCH_T_II   = "wzflr"
NOTCH_T_III  = "wzekq"
NOTCH_T_IV   = "wzflr"
NOTCH_T_V    = "ycfkr"
NOTCH_T_VI   = "xeimq"
NOTCH_T_VII  = "ycfkr"
NOTCH_T_VIII = "xeimq"

NOTCH_EMPTY  = ""

# Permutations for Typex rotors of set SP_02390
PERM_SP_02390_A = "rfnvbktihxqgcwaeolsmpydzuj"
PERM_SP_02390_B = "oltgenzujdibypsafwrqmcxkhv"
PERM_SP_02390_C = "twbhqdurmlnieaksvoycjgxfpz"
PERM_SP_02390_D = "lyumxsfbpzonkjceqiatgwrhdv"
PERM_SP_02390_E = "kgbtysoaivxcjprqznhlfwuemd"
PERM_SP_02390_F = "gmruybjzhfktdwqcosxaiepnvl"
PERM_SP_02390_G = "lusyeitrjapfkwcvmqhbgnxzod"
PERM_SP_02390_UKW = "yruhqsldpxngokmiebfzcwvjat"
PERM_TYPEX_ETW = "qwertzuioasdfghjkpyxcvbnml"

# Permutations for Typex rotors of set Y_269
PERM_Y_269_A = "uwirlzpebjodkvafmtcshyxgnq"
PERM_Y_269_B = "ygbaowmtjrhpenfcxkuidqzlsv"
PERM_Y_269_C = "hwautkyjonlfiqzdcbrgpemxvs"
PERM_Y_269_D = "qybuhaojnctlizswfpmgevxdrk"
PERM_Y_269_E = "yiloktbwfqncghpxdjsvaumzre"
PERM_Y_269_F = "exaybhvusolcmqgwndiztpkfjr"
PERM_Y_269_G = "kboizmtxrhdcgpyausvlqjewnf"
PERM_Y_269_H = "hqxcnbirumoyaftgkzleswjdvp"
PERM_Y_269_I = "bumpzvykjxgtdrocslqiaehwfn"
PERM_Y_269_J = "kchgdvueobmlxrfwynqizpjats"
PERM_Y_269_K = "wjaveciqnkzgdubspxmfythlro"
PERM_Y_269_L = "igesoyldujamvhrcxfpztqnbwk"
PERM_Y_269_M = "hdrzvlbtioewcknsyfqxmupgja"
PERM_Y_269_N = "lcmsbeioqjxfawkthdrznvypgu"
PERM_Y_269_UKW = "yruhqsldpxngokmiebfzcwvjat"

# Notches for Typex rotors of set SP_02390
NOTCH_Y_269 = "fmsz"

# Notches for Typex rotors of set SP_02390
NOTCH_SP_02390 = "fmsz"

# Permutations for KD Enigma rotors
PERM_KD_I =   "veziojcxkyduntwaplqgbhsfmr"
PERM_KD_II =  "hgrbsjzetdlvpmqycxaokinfuw"
PERM_KD_III = "nwlhxgrbyojsazdvtpkfqmeuic"
PERM_KD_IV =  "dauiybtovemsgxlwjpnckfzrqh"
PERM_KD_V =   "dclkmsaehuiqryvbwfpzjngtox"
PERM_KD_VI =  "wayqcrtvkeuxszlgbjofndpmih"
PERM_KD_ETW   = "qwertzuioasdfghjkpyxcvbnml"

# Notches for KD Enigma rotors
NOTCH_KD_I    = "suyaehlnq"
NOTCH_KD_II   = "suyaehlnq"
NOTCH_KD_III  = "suyaehlnq"
NOTCH_KD_IV   = "suyaehlnq"
NOTCH_KD_V    = "suyaehlnq"
NOTCH_KD_VI   = "suyaehlnq"


known_ids = [WALZE_I, WALZE_II, WALZE_III, WALZE_IV, WALZE_V, WALZE_VI, WALZE_VII, WALZE_VIII, WALZE_ABW_I, WALZE_ABW_II, WALZE_ABW_III,
             WALZE_ABW_ETW, WALZE_RB_I, WALZE_RB_II, WALZE_RB_III, WALZE_RB_ETW, WALZE_T_I, WALZE_T_II, WALZE_T_III, WALZE_T_IV,
             WALZE_T_V, WALZE_T_VI, WALZE_T_VII, WALZE_T_VIII, WALZE_T_ETW, UKW_A, UKW_B, UKW_C, UKW_B_DN, UKW_C_DN, WALZE_BETA,
             WALZE_GAMMA, UKW_ABW, UKW_RB, UKW_T, TYPEX_SP_02390_A, TYPEX_SP_02390_B, TYPEX_SP_02390_C, TYPEX_SP_02390_D,
             TYPEX_SP_02390_E, TYPEX_SP_02390_F, TYPEX_SP_02390_G, TYPEX_ETW, TYPEX_Y_269_UKW, TYPEX_SP_02390_UKW, UKW_D, WALZE_KD_I,
             WALZE_KD_II, WALZE_KD_III, WALZE_KD_IV, WALZE_KD_V, WALZE_KD_VI, WALZE_KD_ETW, TYPEX_Y_269_A, TYPEX_Y_269_B, TYPEX_Y_269_C,
             TYPEX_Y_269_D, TYPEX_Y_269_E, TYPEX_Y_269_F, TYPEX_Y_269_G, TYPEX_Y_269_H, TYPEX_Y_269_I, TYPEX_Y_269_J, TYPEX_Y_269_K,
             TYPEX_Y_269_L, TYPEX_Y_269_M, TYPEX_Y_269_N]
             

# known_wheels maps the rotor id to the corresponding permutation, notches and a human
# readable name. The human readable nam is used in creating comments in the generated files.
known_wheels = {}
known_wheels[WALZE_I] = (PERM_I, NOTCH_I, "WALZE_I")
known_wheels[WALZE_II] = (PERM_II, NOTCH_II, "WALZE_II")
known_wheels[WALZE_III] = (PERM_III, NOTCH_III, "WALZE_III")
known_wheels[WALZE_IV] = (PERM_IV, NOTCH_IV, "WALZE_IV")
known_wheels[WALZE_V] = (PERM_V, NOTCH_V, "WALZE_V")
known_wheels[WALZE_VI] = (PERM_VI, NOTCH_VI, "WALZE_VI")
known_wheels[WALZE_VII] = (PERM_VII, NOTCH_VII, "WALZE_VII")
known_wheels[WALZE_VIII] = (PERM_VIII, NOTCH_VIII, "WALZE_VIII")
known_wheels[WALZE_ABW_I] = (PERM_ABW_I, NOTCH_ABW_I, "WALZE_ABW_I")
known_wheels[WALZE_ABW_II] = (PERM_ABW_II, NOTCH_ABW_II, "WALZE_ABW_II")
known_wheels[WALZE_ABW_III] = (PERM_ABW_III, NOTCH_ABW_III, "WALZE_ABW_III")
known_wheels[WALZE_ABW_ETW] = (PERM_ABW_ETW, NOTCH_EMPTY, "WALZE_ABW_ETW")
known_wheels[WALZE_RB_I] = (PERM_RB_I, NOTCH_RB_I, "WALZE_RB_I")
known_wheels[WALZE_RB_II] = (PERM_RB_II, NOTCH_RB_II, "WALZE_RB_II")
known_wheels[WALZE_RB_III] = (PERM_RB_III, NOTCH_RB_III, "WALZE_RB_III")
known_wheels[WALZE_RB_ETW] = (PERM_RB_ETW, NOTCH_EMPTY, "WALZE_RB_ETW")
known_wheels[WALZE_T_I] = (PERM_T_I, NOTCH_T_I, "WALZE_T_I")
known_wheels[WALZE_T_II] = (PERM_T_II, NOTCH_T_II, "WALZE_T_II")
known_wheels[WALZE_T_III] = (PERM_T_III, NOTCH_T_III, "WALZE_T_III")
known_wheels[WALZE_T_IV] = (PERM_T_IV, NOTCH_T_IV, "WALZE_T_IV")
known_wheels[WALZE_T_V] = (PERM_T_V, NOTCH_T_V, "WALZE_T_V")
known_wheels[WALZE_T_VI] = (PERM_T_VI, NOTCH_T_VI, "WALZE_T_VI")
known_wheels[WALZE_T_VII] = (PERM_T_VII, NOTCH_T_VII, "WALZE_T_VII")
known_wheels[WALZE_T_VIII] = (PERM_T_VIII, NOTCH_T_VIII, "WALZE_T_VIII")
known_wheels[WALZE_T_ETW] = (PERM_T_ETW, NOTCH_EMPTY, "WALZE_T_ETW")
known_wheels[UKW_A] = (PERM_UKW_A, NOTCH_EMPTY, "UKW_A")
known_wheels[UKW_B] = (PERM_UKW_B, NOTCH_EMPTY, "UKW_B")
known_wheels[UKW_C] = (PERM_UKW_C, NOTCH_EMPTY, "UKW_C")
known_wheels[UKW_D] = (PERM_UKW_D, NOTCH_EMPTY, "UKW_D")
known_wheels[UKW_B_DN] = (PERM_UKW_B_DN, NOTCH_EMPTY, "UKW_B_DN")
known_wheels[UKW_C_DN] = (PERM_UKW_C_DN, NOTCH_EMPTY, "UKW_C_DN")
known_wheels[UKW_ABW] = (PERM_ABW_UKW, NOTCH_EMPTY, "UKW_ABW")
known_wheels[UKW_RB] = (PERM_RB_UKW, NOTCH_EMPTY, "UKW_RB")
known_wheels[UKW_T] = (PERM_T_UKW, NOTCH_EMPTY, "UKW_T")
known_wheels[WALZE_BETA] = (PERM_BETA, NOTCH_EMPTY, "WALZE_BETA")
known_wheels[WALZE_GAMMA] = (PERM_GAMMA, NOTCH_EMPTY, "WALZE_GAMMA")
known_wheels[TYPEX_SP_02390_A] = (PERM_SP_02390_A, NOTCH_SP_02390, "TYPEX_SP_02390_A")
known_wheels[TYPEX_SP_02390_B] = (PERM_SP_02390_B, NOTCH_SP_02390, "TYPEX_SP_02390_B")
known_wheels[TYPEX_SP_02390_C] = (PERM_SP_02390_C, NOTCH_SP_02390, "TYPEX_SP_02390_C")
known_wheels[TYPEX_SP_02390_D] = (PERM_SP_02390_D, NOTCH_SP_02390, "TYPEX_SP_02390_D")
known_wheels[TYPEX_SP_02390_E] = (PERM_SP_02390_E, NOTCH_SP_02390, "TYPEX_SP_02390_E")
known_wheels[TYPEX_SP_02390_F] = (PERM_SP_02390_F, NOTCH_SP_02390, "TYPEX_SP_02390_F")
known_wheels[TYPEX_SP_02390_G] = (PERM_SP_02390_G, NOTCH_SP_02390, "TYPEX_SP_02390_G")
known_wheels[TYPEX_ETW] = (PERM_TYPEX_ETW, NOTCH_EMPTY, "TYPEX_ETW")
known_wheels[TYPEX_SP_02390_UKW] = (PERM_SP_02390_UKW, NOTCH_EMPTY, "TYPEX_SP_02390_UKW")
known_wheels[WALZE_KD_I] = (PERM_KD_I, NOTCH_KD_I, "WALZE_KD_I")
known_wheels[WALZE_KD_II] = (PERM_KD_II, NOTCH_KD_II, "WALZE_KD_II")
known_wheels[WALZE_KD_III] = (PERM_KD_III, NOTCH_KD_III, "WALZE_KD_III")
known_wheels[WALZE_KD_IV] = (PERM_KD_IV, NOTCH_KD_IV, "WALZE_KD_IV")
known_wheels[WALZE_KD_V] = (PERM_KD_V, NOTCH_KD_V, "WALZE_KD_V")
known_wheels[WALZE_KD_VI] = (PERM_KD_VI, NOTCH_KD_VI, "WALZE_KD_VI")
known_wheels[WALZE_KD_ETW] = (PERM_KD_ETW, NOTCH_EMPTY, "WALZE_KD_ETW")
known_wheels[TYPEX_Y_269_A] = (PERM_Y_269_A, NOTCH_Y_269, "TYPEX_Y_269_A")
known_wheels[TYPEX_Y_269_B] = (PERM_Y_269_B, NOTCH_Y_269, "TYPEX_Y_269_B")
known_wheels[TYPEX_Y_269_C] = (PERM_Y_269_C, NOTCH_Y_269, "TYPEX_Y_269_C")
known_wheels[TYPEX_Y_269_D] = (PERM_Y_269_D, NOTCH_Y_269, "TYPEX_Y_269_D")
known_wheels[TYPEX_Y_269_E] = (PERM_Y_269_E, NOTCH_Y_269, "TYPEX_Y_269_E")
known_wheels[TYPEX_Y_269_F] = (PERM_Y_269_F, NOTCH_Y_269, "TYPEX_Y_269_F")
known_wheels[TYPEX_Y_269_G] = (PERM_Y_269_G, NOTCH_Y_269, "TYPEX_Y_269_G")
known_wheels[TYPEX_Y_269_H] = (PERM_Y_269_H, NOTCH_Y_269, "TYPEX_Y_269_H")
known_wheels[TYPEX_Y_269_I] = (PERM_Y_269_I, NOTCH_Y_269, "TYPEX_Y_269_I")
known_wheels[TYPEX_Y_269_J] = (PERM_Y_269_J, NOTCH_Y_269, "TYPEX_Y_269_J")
known_wheels[TYPEX_Y_269_K] = (PERM_Y_269_K, NOTCH_Y_269, "TYPEX_Y_269_K")
known_wheels[TYPEX_Y_269_L] = (PERM_Y_269_L, NOTCH_Y_269, "TYPEX_Y_269_L")
known_wheels[TYPEX_Y_269_M] = (PERM_Y_269_M, NOTCH_Y_269, "TYPEX_Y_269_M")
known_wheels[TYPEX_Y_269_N] = (PERM_Y_269_N, NOTCH_Y_269, "TYPEX_Y_269_N")
known_wheels[TYPEX_Y_269_UKW] = (PERM_Y_269_UKW, NOTCH_EMPTY, "TYPEX_Y_269_UKW")


alpha_mapping = {}
for i in range(26):
    alpha_mapping[STD_ALPHA[i]] = i

## \brief This function converts the permutation specified as a string in parameter perm into
#         a vector of ints where alpha_mapping is used to map each character of perm into an int.
#
def perm_to_int_vector(perm):
    result = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
    
    for i in range(26):
        result[i] = alpha_mapping[perm[i]]
        
    return result

## \brief This function returns ring data in the form of an int vector of length 26 where 
#         each position in the returned vector that corresponds to a value in the string 
#         parameter ring_data is set to 1.
#
def ring_data_to_int_vector(ring_data):
    result = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]

    for i in ring_data:
        result[alpha_mapping[i]] = 1
        
    return result

## \brief This function converts the vector of ints given in parameter vec into a string where
#         each int component of the output is followed by a ;.
#
def int_vector_to_string(vec):
    result = ''

    for i in vec:
        result += str(i) + ";"
    
    return result

## \brief This function generates a section in an ini file which corresponds to the given rotor_id.
#         The format of the created ini entries equals the ones that the rotor_set class uses to 
#         save rotor_set data.
#
def write_rotor_entry(rotor_id, fd):
    result = '# Data for ' + known_wheels[rotor_id][2] + '\\n\\\n'
    result += '[rotorid_'
    result += str(rotor_id) + ']\\n\\\n'
    
    result += 'permutation='
    perm_as_ints = perm_to_int_vector(known_wheels[rotor_id][0])
    
    result += int_vector_to_string(perm_as_ints)
    
    result += '\\n\\\nringdata='
    ring_data = ring_data_to_int_vector(known_wheels[rotor_id][1])
        
    result += int_vector_to_string(ring_data)
    
    result += '\\n\\\n'
    
    fd.write(bytes(result + "\\n\\\n", 'ascii'))

## \brief This function creates an ini file that represents all the rotors used in the implementation
#         of the Enigma and its variants. The format of the created file is such that it can be loaded
#         by the rotor_set::load_ini(Glib::KeyFile& ini_file) method.
#
def write_rotor_set(out_file_name):             
    
    fd = open(out_file_name, "wb")

    fd.write(bytes('//This is autogenerated code, do not change it. The desired changes have to be made in enigrotorset.py\n', 'ascii'))
    
    id_str = ''
    for i in known_ids:
        id_str = id_str + str(i) + ';'

    fd.write(bytes('string enigma_rotor_set = "\\n\\\n', 'ascii'))
    fd.write(bytes('[general]\\n\\\n', 'ascii'))
    fd.write(bytes('ids=' + id_str + "\\n\\\n", 'ascii'))
    fd.write(bytes("\\n\\\n", 'ascii'));

    for i in known_ids:
        write_rotor_entry(i, fd)

    fd.write(bytes('";\n\n', 'ascii'))
    fd.close()

## \brief This function produces the symbolic constants that are uses to represent the rotor ids
#         in the C++ code of the Enigma simulator. 
#
def write_constants(out_file_name):
    fd = open(out_file_name, "wb")
    
    fd.write(bytes('//This is autogenerated code, do not change it. The desired changes have to be made in enigrotorset.py\n', 'ascii'))
    fd.write(bytes('#ifndef __enigma_rotor_set_h__\n', 'ascii'))
    fd.write(bytes('#define __enigma_rotor_set_h__\n', 'ascii'))
    
    for i in known_ids:
        fd.write(bytes("const unsigned int " + known_wheels[i][2] + " = " + str(i) + ";\n", 'ascii'))
    
    fd.write(bytes('#endif /*__enigma_rotor_set_h__*/\n', 'ascii'))
    fd.write(bytes('\n\n', 'ascii'))    
    fd.close()
    
if __name__ == "__main__":
    # execute only if run as a script
    write_rotor_set("enigma_rotor_set.cpp")
    write_constants("enigma_rotor_set.h")

