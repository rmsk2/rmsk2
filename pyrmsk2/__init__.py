################################################################################
# Copyright 2018 Martin Grap
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

import os

from pyrmsk2.tlvobject import get_tlv_server_path
from pyrmsk2.tlvobject import get_socket_name

## \brief Default for the documentation path
DEFAULT_DOC_PATH = './doc'

## \brief Major version
RMSK2_VERSION_MAJOR = 3
## \brief Minor version
RMSK2_VERSION_MINOR = 6
## \brief Revision
RMSK2_VERSION_REVISION = 2

## \brief This function returns the version string describing the version of rmsk2
#
#  \returns A string.
#
def get_version_string():
    result = ''
    
    if RMSK2_VERSION_REVISION == 0:
        result = '{}.{}'.format(RMSK2_VERSION_MAJOR, RMSK2_VERSION_MINOR)
    else:
        result = '{}.{}.{}'.format(RMSK2_VERSION_MAJOR, RMSK2_VERSION_MINOR, RMSK2_VERSION_REVISION)
    
    return result

## \brief This function determines the directory in which the mallard style documentation for
#         for the python3 programs is stored.
#
#  \param [script_file] Is a string or None. This function is expected to be called with the __file__
#         attribute of the calling module. In case None is specified as the value of the parameter
#         script_file then the constant DEFAULT_DOC_PATH is returned.
#
#  \returns A string. Containing the calculated path of the documentation directory excluding
#           the trailing / character.
#
def get_doc_path(script_file = __file__):
    result = DEFAULT_DOC_PATH
    
    if script_file != None:
        try:
            result = os.path.dirname(os.path.realpath(script_file)) + '/doc'
        except:
            result = DEFAULT_DOC_PATH
        
    return result
    
## \brief This class is used to signal exceptions to calling code.
#
class EnigmaException(Exception):
    def __init__(self, error_message):
        Exception.__init__(self, 'Message procedure: ' + error_message)

