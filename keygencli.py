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

## @package keygencli A Python3 command line program which allows to generate key sheets for all machines provided by rmsk2 and rotorsim.
#           
# \file keygencli.py
# \brief This file imlements a command line keysheet generator for all rotor machines provided by rmsk2 and rotorsim.

import argparse
from pyrmsk2.keysheetgen import *

## \brief This is the main method.
#
#  \returns Nothing.
#    
def execute():
    # Set up command line parser        
    parser = argparse.ArgumentParser()
    parser.add_argument("type", choices=MACHINE_NAMES, help="Type of machine to generate a keysheet for")
    parser.add_argument("-y", "--year", type=KeysheetGeneratorMain.check_year, required=True, help="Year to appear on sheet")
    parser.add_argument("-m", "--month",  type=int, choices=[1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12], \
                                          help="Month to appear on sheet. Sheets for a whole year are generated when this option is not specified.")
    parser.add_argument("-n", "--net", required=True, help="Net name to appear on sheet")
    parser.add_argument("-c", "--classification", required=True, help="Classification level to appear on sheet")
    parser.add_argument("-s", "--save-states", action='store_true', help="Save machine states for each day of the month in output directory.")
    parser.add_argument("-o", "--out", metavar='DIRECTORY NAME', \
                        help="Store keysheet and optionally state files in directory as named by this option and not stdout.")
    parser.add_argument("--html", help="Generate HTML not text output", action='store_true')
    parser.add_argument("--tlv-server", help="Path to TLV server binary", default=rotorsim.tlvobject.get_tlv_server_path())
    
    # Calls sys.exit() when command line can not be parsed or when --help is requested
    args = parser.parse_args()        

    KeysheetGeneratorMain.generate_sheets(args, ReporterBase())

if __name__ == "__main__":
    execute()

