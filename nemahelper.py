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

## @package nemahelper Helper program written in python3 for generating Nema rotor set data.
#           
#  This program is used to transform the rotor set data (permutations and notches) from the format used 
#  in "Sullivan, Geoff and Weierud, Frode (1999) 'THE SWISS NEMA CIPHER MACHINE', Cryptologia, 23:4, 
#  310 — 328" into the format used in rotorvis.
#
# \file nemahelper.py
# \brief Helper program written in python3 for Nema rotor set data.
#
import sys

alpha =     'ijklmnopqrstuvwxyzabcdefgh'
std_alpha = 'abcdefghijklmnopqrstuvwxyz'

perm_A = [5, 14, 15, 19, 13, 2, 22, 10, 4, 18, 16, 26, 24, 9, 23, 25, 8, 20, 6, 11, 3, 1, 12, 21, 7, 17]
perm_B = [4, 7, 18, 9, 20, 15, 8, 11, 16, 1, 10, 24, 19, 25, 13, 22, 14, 21, 3, 2, 17, 6, 12, 5, 23, 26]
perm_C = [18, 17, 19, 26, 14, 10, 15, 7, 2, 11, 25, 20, 9, 5, 3, 21, 16, 4, 8, 6, 24, 13, 12, 23, 22, 1]
perm_D = [5, 22, 12, 19, 18, 3, 16, 8, 1, 9, 26, 11, 14, 4, 25, 7, 6, 10, 15, 2, 20, 23, 17, 24, 13, 21]
perm_E = [6, 19, 21, 24, 13, 2, 23, 11, 15, 4, 18, 1, 25, 22, 8, 7, 3, 20, 26, 10, 9, 12, 17, 14, 16, 5]
perm_F = [15, 4, 6, 11, 26, 8, 7, 19, 24, 17, 1, 16, 3, 20, 22, 10, 2, 21, 18, 14, 25, 23, 9, 5, 13, 12]
perm_U = [14, 12, 19, 18, 13, 22, 17, 21, 16, 11, 10, 2, 5, 1, 20, 9, 7, 4, 3, 15, 8, 6, 25, 26, 23, 24]

#           A  B  C  D  E  F  G  H  I  J  K  L  M  N  O  P  Q  R  S  T  U  V  W  X  Y  Z
notch_1  = [0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 1]
notch_2  = [0, 1, 0, 1, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0]
notch_12 = [0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1]
notch_13 = [1, 1, 0, 1, 1, 1, 1, 0, 0, 1, 1, 0, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 0]
notch_14 = [0, 0, 1, 0, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 0, 1, 0, 1, 0, 1]
notch_15 = [1, 0, 0, 1, 1, 0, 1, 0, 0, 0, 0, 0, 1, 0, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1, 1, 1]
notch_16 = [1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0]
notch_17 = [0, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 1, 0, 1, 1, 0]
notch_18 = [1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1]
notch_19 = [1, 1, 1, 0, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1]
notch_20 = [1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 1, 0, 1, 1, 1, 1]
notch_21 = [1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 0, 1, 1, 1, 1, 0, 1, 1, 1, 0, 1, 0, 0, 1, 0, 0]
notch_22 = [1, 1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0, 1, 1, 1, 1, 0, 0, 1, 1, 1, 0, 0]
notch_23 = [1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0]

def make_perm(data):
    result = ''
    
    for i in data:
        result += alpha[i - 1]
    
    return result

def make_notch_data(data, offset):
    result = ''
    
    for i in range(26):
        if data[i] == 1:
            result += std_alpha[(i + (offset)) % 26]
    
    return result;
            

help_A = make_perm(perm_A)
help_B = make_perm(perm_B)
help_C = make_perm(perm_C)
help_D = make_perm(perm_D)
help_E = make_perm(perm_E)
help_F = make_perm(perm_F)
help_U = make_perm(perm_U)

print('')
print("Permutationen")
print("--------------------------")
print('#define PERM_A   "' + help_A + '"')
print('#define PERM_B   "' + help_B + '"')
print('#define PERM_C   "' + help_C + '"')
print('#define PERM_D   "' + help_D + '"')
print('#define PERM_E   "' + help_E + '"')
print('#define PERM_F   "' + help_F + '"')
print('#define PERM_U   "' + help_U + '"')
# In Nema the qwertzu runs clockwise while rotor contact numbering is counter clockwise
# so the qwertzu appears when the below string is read backwards begining with position
# 0.
print('#define PERM_ETW "' + 'qmnbvcxylkjhgfdsapoiuztrew'  + '"')

print('') 
print("Nocken")
print("--------------------------")

print('#define NOTCH_1  "' + make_notch_data(notch_1, 0) + '"')
print('#define NOTCH_2  "' + make_notch_data(notch_2, 0) + '"')
print('#define NOTCH_12 "' + make_notch_data(notch_12, 0) + '"')
print('#define NOTCH_13 "' + make_notch_data(notch_13, 0) + '"')
print('#define NOTCH_14 "' + make_notch_data(notch_14, 0) + '"')
print('#define NOTCH_15 "' + make_notch_data(notch_15, 0) + '"')
print('#define NOTCH_16 "' + make_notch_data(notch_16, 0) + '"')
print('#define NOTCH_17 "' + make_notch_data(notch_17, 0) + '"')
print('#define NOTCH_18 "' + make_notch_data(notch_18, 0) + '"')
print('#define NOTCH_19 "' + make_notch_data(notch_19, 0) + '"')
print('#define NOTCH_20 "' + make_notch_data(notch_20, 0) + '"')
print('#define NOTCH_21 "' + make_notch_data(notch_21, 0) + '"')
print('#define NOTCH_22 "' + make_notch_data(notch_22, 0) + '"')
print('#define NOTCH_23 "' + make_notch_data(notch_23, 0) + '"')


