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

## @package make_glade_header C++ code generator written in python3 that encapsulates the glade
#                             file in a C++ header file
#           
#  Code generator that transforms the glade file needed by this software into a base64 encoded
#  string that can be included through a header file. This saves us from the trouble of locating the 
#  rotor_dialog_2.ui file during runtime. 
#  usage: make_glade_header.py glade_file_name output_file_name
#
# \file make_glade_header.py
# \brief Code generator that transforms the glade file needed by this software into a base64 encoded
#        string that can be included through a header file. This saves us from the trouble of locating the 
#        rotor_dialog_2.ui file during runtime.

import base64
import sys

LINE_LENGTH = 72

def make_glade_data(file_name, file_name_out):
    f = open(file_name, 'rb')
    raw = f.read()
    f.close()
    
    b64data = base64.standard_b64encode(raw).decode()
    
    f = open(file_name_out, 'w')
    f.write('#include<string>\r\n\r\n')
    f.write('//base64 encoded glade data. base64 encoding is less efficient but also less error prone\r\n')
    f.write('//than quoting all the special characters in the XML input \r\n')    
    f.write('static const string gladedata("\\\r\n')
    
    chunk_start = 0;
    
    while chunk_start < len(b64data):

        block_length = LINE_LENGTH
        end_char = '\\'        
        if (len(b64data) - chunk_start) < LINE_LENGTH:
            block_length = len(b64data) - chunk_start

        f.write(b64data[chunk_start:chunk_start + block_length] + end_char + '\r\n')
        chunk_start += block_length
    
    f.write('");\r\n')
    f.close()


if len(sys.argv) < 3:
    print("usage: make_glade_header.py <glade_file_name> <output_file_name>")
    sys.exit()

make_glade_data(sys.argv[1], sys.argv[2])
