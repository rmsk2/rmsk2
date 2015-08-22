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

## @package kl7prep Helper program written in python3 for generating KL7 rotor set data.
#           
#  This program is used to transform the rotor set data (permutations and notches) from the format used 
#  in the help file "KL-7 Help.pdf" that comes with the KL7 simulator of Dirk Rijmenants into the format
#  used in rotorvis.
#
#  The help file is available at http://users.telenet.be/d.rijmenants/KL-7%20Help.pdf
#
# \file kl7prep.py
# \brief Helper program written in python3 for KL7 rotor set data.
#
import sys

cols = [
    '10111010101',
    '00100010111',
    '01011001100',
    '01000001000',
    '10000011101',
    '01001100000',
    '00111100100',
    '01100111110',
    '00000010000',
    '10110000011',
    '00001100011',
    '01000111000',
    '00000010100',
    '11011001010',
    '11110000011',
    '00000101110',
    '10000001101',
    '00011110011',
    '11101000000',
    '00010001100',
    '00000001000',
    '01001111111',
    '11000100000',
    '10101010000',
    '00010101101',
    '01111100011',
    '10010001000',
    '11101110100',
    '00001110110',
    '00110000010',
    '10010010101',
    '11110111100',
    '00000001111',
    '00011010010',
    '01101001010',
    '11000101000'
]       

rotors = [
    [29,27,14,8,35,4,28,11,5,13,20,3,25,33,18,15,7,12,34,16,17,1,9,30,24,23,2,32,10,19,6,26,36,22,31,21],
    [23,19,26,16,2,13,14,35,21,4,17,31,25,3,18,27,12,34,36,10,30,6,7,15,28,1,11,33,29,20,32,24,5,22,8,9],
    [19,26,28,36,6,25,31,18,27,10,5,1,32,9,11,33,23,17,29,12,13,2,16,15,35,8,24,22,30,3,34,14,7,20,21,4],
    [15,26,36,13,1,31,25,33,3,32,21,23,17,29,7,22,20,24,12,10,14,30,19,28,4,35,5,8,6,9,16,27,2,11,34,18],
    [13,4,2,16,17,30,21,5,33,7,29,8,9,36,35,23,34,25,20,22,28,15,1,19,24,27,10,11,6,12,32,26,14,3,18,31],
    [26,34,27,14,2,1,31,36,11,9,35,18,15,12,4,7,29,8,23,19,3,30,20,17,28,21,22,5,25,33,16,13,24,6,10,32],
    [20,19,9,32,36,2,6,33,12,28,4,10,3,24,29,16,22,18,30,17,7,34,15,23,31,25,27,1,21,26,8,5,13,35,11,14],
    [28,19,23,5,17,36,27,14,16,20,21,7,12,22,11,35,13,15,1,32,8,18,33,4,9,29,26,24,25,34,10,6,3,30,2,31],
    [25,6,35,12,21,22,19,32,20,23,30,18,1,16,31,11,24,13,33,7,36,9,34,2,10,8,26,29,15,17,4,28,14,3,27,5],
    [8,31,1,28,20,6,32,5,33,21,30,12,4,14,15,34,7,35,16,18,29,22,25,26,36,11,23,19,3,2,13,27,24,10,17,9],
    [15,13,36,23,6,21,32,18,31,20,1,24,10,35,19,28,7,8,26,12,29,22,25,30,5,9,2,27,16,4,17,3,34,14,11,33],
    [8,18,15,33,7,26,20,16,34,23,36,27,12,24,19,13,2,3,14,29,1,6,32,10,25,30,9,5,28,17,22,31,4,11,21,35],
    [36,6,29,28,24,26,21,22,20,35,15,19,23,30,1,8,12,7,13,27,31,32,9,33,10,16,14,18,34,2,3,17,25,11,4,5]
]

def print_rings(inp):
    for i in range(11):
        dat = ''
        for j in range(36):
            dat += inp[j][i]
        print('#define KL7RING_' + str(i + 1) + ' "' + dat + '"')

rotor_names = 'ABCDEFGHIJKLM'

def print_rotors(inp):
    for i in range(len(rotor_names)):
        line = 'static unsigned int kl7_rotor_' + rotor_names[i] + '[KL7_ROTOR_SIZE] = {'
        for j in rotors[i]:
            line += str(j - 1) + ','
        line = line[:-1]
        line += '};'
        print(line)

print_rings(cols)
print('')
print('')
print_rotors(rotors)

