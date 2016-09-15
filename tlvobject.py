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

## @package tlvobject Contains classes that implement the python3 side of the TLV infrastructure.
#   
# \file rmsk2/tlvobject.py
# \brief This file contains classes that implement a generic client for the TLV server.
#

import socket
import subprocess
import os

## \brief Default address for the TLV server
SERVER_ADDRESS = './sock_tlvstream'
## \brief Default file name of the C++ TLV server
SERVER_BINARY = './tlv_rotorsim'
## \brief Maximum number of bytes to read from a socket in one go
BUF_SIZE = 4096
## \brief Maximum length of the contents of a TLV object
LEN_MAX = 65536

# Error codes
ERR_OK = 0
ERR_SOCK_CREATE = 1
SOCK_ERR_BIND = 2
SOCK_ERR_ACCEPT = 3
ERR_REMOVE_PATH = 4
ERR_SOCK_READ = 5
ERR_SOCK_WRITE = 6
ERR_SOCK_PATH_LEN = 7
ERR_DATA_LEN = 8
ERR_ERROR = 42

## \brief A TLV integer is a 32 bit signed integer
TAG_INT = 0
## \brief A TLV string is a UTF-8 encoded string
TAG_STRING = 1
## \brief A TLV byte array contains arbitrary binary data as an array of bytes
TAG_BYTE_ARRAY = 2
## \brief A TLV sequence is a container for other TLV objects
TAG_SEQUENCE = 3
## \brief A TLV double is a floating point number
TAG_DOUBLE = 4
## \brief A TLV NULL object is an empty object that has no contents bytes
TAG_NULL = 5
## \brief A TLV result code is a 32 bit unsigned integer
TAG_RESULT_CODE = 6

## \brief This function allows to retrieve the default path to the TLV server binary
def get_tlv_server_path():
    # Default is value in this module
    result = SERVER_BINARY
    
    # If there is an environment variable use that
    try:
        result = os.environ['TLVSERVER']
    except:
        pass
    
    # If there is a config file use that
    try:
        file_name = os.environ['HOME'] + '/.tlvserverpath'
        with open(file_name, 'r') as f:
            result = f.read()        
    except:
        pass
    
    result = result.strip()
    return result

## \brief An excpetion class that is used for constructing exception objects in this module. 
#
class TlvException(Exception):
    ## \brief An excpetion class that is used for constructing exception objects in this module. 
    #
    #  \param [error_message] Is a string. It has to contain an error message that is to be conveyed to 
    #         receiver of the corresponding exception.
    #
    def __init__(self, error_message):
        Exception.__init__(self, 'tlvobject: ' + error_message)


## \brief A class that implements the concept of a TLV (Tag Length Value) encoded data object.
#
#  The encoded form of a TLV object starts with a single byte (the tag) that specifies the type of the
#  data that it contains. The tag is followed by two length bytes which specify the length of the following
#  contents bytes. Objects of this type are used to talk to the C++ side of the TLV infrastructure.
#
class TlvEntry:
    ## \brief Constructor. 
    #
    #  The default tag is TAG_NULL and therefore there are no contens bytes.
    #
    def __init__(self):
        ## \brief Holds the tag of the value currently represented by this TlvEntry instance.
        self.tag = TAG_NULL
        ## \brief Holds the contents bytes of the value currently represented by this TlvEntry instance.        
        self.value = bytes()

    ## \brief This method transforms a signed integer into a array of four bytes that represent the 
    #         the integer in two's complement form. Big endian byte ordering is used.
    #
    #  \param [val] Is an integer. It contains the value that is to be transformed
    #
    #  \returns A byte array that represents the vlaue specified in parameter val.
    #
    @staticmethod        
    def int_to_bytes(val):
        if val < 0:
            val = -val
            val = val ^ 0xFFFFFFFF
            val = val + 1
        
        return val.to_bytes(4, byteorder='big')

    ## \brief This method sets this TlvEntry instance up to represent a signed integer.
    #
    #  \param [int_val] Is an integer. It contains the value that is to be represented.
    #
    #  \returns self.
    #    
    def to_int(self, int_val):
        self.tag = TAG_INT
        self.value = TlvEntry.int_to_bytes(int_val)
        return self

    ## \brief This method sets this TlvEntry instance up to represent an unsigned integer.
    #
    #  \param [result_val] Is an integer. It contains the value that is to be represented.
    #
    #  \returns self.
    #    
    def to_result(self, result_val):
        self.tag = TAG_RESULT_CODE
        result_val = result_val & 0xFFFFFFFF
        self.value = result_val.to_bytes(4, byteorder='big')
        return self

    ## \brief This method sets this TlvEntry instance up to represent a string.
    #
    #  \param [str_val] Is a string. It contains the value that is to be represented.
    #
    #  \returns self.
    #        
    def to_string(self, str_val):
        self.tag = TAG_STRING
        self.value = str_val.encode()
        return self

    ## \brief This method sets this TlvEntry instance up to represent a NULL object.
    #
    #  \returns self.
    #            
    def to_null(self):
        self.tag = TAG_NULL
        self.value = b''
        return self

    ## \brief This method sets this TlvEntry instance up to represent a sequence of TlvEntry objects.
    #
    #  \param [obj_sequence] Is a sequence of TlvEntry objects. It contains the values that are to be represented.
    #
    #  \returns self.
    #            
    def to_sequence(self, obj_sequence):
        self.tag = TAG_SEQUENCE
        self.value = TlvStream.to_bytes(obj_sequence)
        return self

    ## \brief This method sets this TlvEntry instance up to represent a floating point double value.
    #
    #  \param [double_val] Is a floating point number. It contains the value that is to be represented.
    #
    #  \returns self.
    #                
    def to_double(self, double_val):
        self.tag = TAG_DOUBLE
        self.value = str(double_val).encode()
        return self

    ## \brief This method sets this TlvEntry instance up to represent a byte array.
    #
    #  \param [byte_vector] Is a byte array. It contains the value that is to be represented.
    #
    #  \returns self.
    #                    
    def to_byte_array(self, byte_vector):
        self.tag = TAG_BYTE_ARRAY
        self.value = byte_vector
        return self

    ## \brief This method converts this TlvEntry instance into a regular python3 value.
    #
    #  \returns Either a signed integer, an unsigned integer, a string, a byte array, a floating point number, 
    #           the None value or a sequence holding values of the aforementioned types. The specific type returned
    #           depends on the value of self.tag. If an unknown tag value is encountered a hash with the keys "key" and
    #           "value" is returned.
    #                            
    def tlv_convert(self):
        result = None
        
        if self.tag == TAG_INT:
            if len(self.value) == 4:
                result = int.from_bytes(self.value, byteorder='big')
                if result & 0x80000000:
                    result = result ^ 0xFFFFFFFF
                    result = result + 1
                    result = -result
            else:
                raise TlvException('Format Error')
        elif self.tag == TAG_RESULT_CODE:
            if len(self.value) == 4:
                result = int.from_bytes(self.value, byteorder='big')
            else:
                raise TlvException('Format Error')
        elif self.tag == TAG_STRING:
            result = self.value.decode()
        elif self.tag == TAG_NULL:
            result = None
        elif self.tag == TAG_DOUBLE:
            result = eval(self.value.decode())
        elif self.tag == TAG_SEQUENCE:
            result = []
            res = TlvStream.parse_bytes(self.value)
            
            if res.err_code != ERR_OK:
                raise TlvException('Unable to parse')
            
            # Beware: Recursion happens here ;-)!
            for i in res.data:
                result.append(i.tlv_convert())
            
        elif self.tag == TAG_BYTE_ARRAY:
            result = self.value        
        else:             
            result = {'tag':self.tag, 'value':self.value}
        
        return result
    

## \brief A class that is intended to represent a generic return value.
#
class TlvResult:
    ## \brief Constructor. 
    #
    #  \param [err_code] Is an integer. It represents an error code. A value of ERR_OK signals successfull
    #         completion of the operation.
    #
    #  \param [data] The type of this parameter is generic. It contains the data returned by the
    #         operation.
    #
    def __init__(self, err_code, data):
        self.err_code = err_code
        self.data = data


## \brief A class that binds together a collection of static methods that deal with sending
#         and receiving TLV encoded objects via UNIX domain sockets.
#
class TlvStream:
    ## \brief Constructor. 
    #
    def __init__(self):
        pass

    ## \brief This method allows to conduct a transaction with a TLV server.
    #
    #  A transaction consists of sending two string objects (object and method name) and a third
    #  TLV object (the parameter, possibly a TLV sequence) to the TLV server. The server responds with
    #  a number of TLV objects that are read from the socket. As soon as a TLV object with tag TAG_RESULT_CODE
    #  is read it is assumed that the server has finished sending result values.
    #
    #  \param [sock] Is a socket object. This socket is used to communicate with the server.
    #
    #  \param [obj_name] Is a string. It designates the object which is to be accessed through the server.
    #    
    #  \param [method_name] Is a string. It designates the method which is to be called on the object named by
    #         the parameter obj_name.
    #    
    #  \param [tlv_params] Is a TlvEntry object. It specifies the parameters which are expected by the method named
    #         by the parameter method_name.
    #
    #  \returns The result of the method call in form of a sequence. The type of the sequence components is generic and
    #           depends on the values sent back by the server.
    #    
    @staticmethod
    def transact(sock, obj_name, method_name, tlv_params):
        object_name_tlv = TlvEntry()
        object_name_tlv.to_string(obj_name)
        method_name_tlv = TlvEntry()
        method_name_tlv.to_string(method_name)
        end_reached = False
        result_values = []
        
        # Send object name
        if TlvStream.write_tlv(sock, object_name_tlv) != ERR_OK:
            raise TlvException("Sending object name failed")

        # Send method name
        if TlvStream.write_tlv(sock, method_name_tlv) != ERR_OK:
            raise TlvException("Sending method name failed")

        # Send parameter
        if TlvStream.write_tlv(sock, tlv_params) != ERR_OK:
            raise TlvException("Sending parameters failed")

        # Retrieve server response
        while (not end_reached):
            err_code = TlvStream.read_tlv(sock)
            if err_code.err_code != ERR_OK:
                raise TlvException("Error receiving result data " + str(err_code.err_code))
            
            if err_code.data.tag != TAG_RESULT_CODE:
                # Server has sent a regular data element
                result_values.append(err_code.data)
            else:
                # Server has sent a result code object
                end_reached = True
                    
        call_result_tlv = err_code.data
        # Get returned result code
        rc = call_result_tlv.tlv_convert()
        
        if rc != ERR_OK:
            raise TlvException("Method call failed. Protocol error: " + str(rc))        
        
        # Create and return method call result
        return TlvStream.convert_all(result_values)

    ## \brief This method allows to read a specfic number of bytes from a socket. It does not return until
    #         the desired number of bytes has been read.
    #
    #  \param [sock] Is a socket object. This socket is used for reading data.
    #
    #  \param [bytes_to_read] Is an integer. It specifies the number of bytes to read from the socket.
    #  
    #  \returns A TlvResult object. In case of success the data member contains the bytes read in the form of a 
    #           byte array.
    #    
    @staticmethod
    def _read_defined(sock, bytes_to_read):
        result = TlvResult(ERR_OK, bytes())
        
        try:
            while (bytes_to_read > 0) and (result.err_code == ERR_OK):
                data = sock.recv(min(BUF_SIZE, bytes_to_read))

                if not data:
                    result.err_code = ERR_SOCK_READ
                    continue
                else:
                    bytes_to_read -= len(data)
                    result.data = result.data + data
        except:
            result.err_code = ERR_SOCK_READ
        
        return result

    ## \brief This method allows to read a single TlvEntry object from a socket. It does not return until a TLV
    #         object was read or an error was encountered.
    #
    #  \param [sock] Is a socket object. This socket is used for reading data.
    #
    #  \returns A TlvResult object. In case of success the data member contains the TlvEntry object read from 
    #           the socket.
    #    
    @staticmethod    
    def read_tlv(sock):
        result = TlvResult(ERR_OK, TlvEntry())

        header_res = TlvStream._read_defined(sock, 3)

        if header_res.err_code != ERR_OK:
            result.err_code = header_res.err_code
        else:
            result.data.tag = int(header_res.data[0])
            data_len = (int(header_res.data[1]) << 8) + int(header_res.data[2])
            data_res = TlvStream._read_defined(sock, data_len)
            
            if data_res.err_code == ERR_OK:
                result.data.value = data_res.data
            else:
                result.err_code = data_res.err_code
        
        return result                        

    ## \brief This method allows to construct the encoded header of a TLV object. The header consists of
    #         the single byte representing the tag followed by the two bytes which specify the length of the
    #         contents bytes. The length bytes use big endian byte order.
    #
    #  \param [tag] Is a integer. It designates the tag.
    #
    #  \param [data_len] Is a integer. It designates the number of contents bytes.
    #
    #  \returns A three element byte array that represents the encoded TLV header.
    #    
    @staticmethod    
    def make_header(tag, data_len):
        return tag.to_bytes(1, byteorder='big') + data_len.to_bytes(2, byteorder='big')

    ## \brief This method parses a byte array into a sequence of TlvEntry objects.
    #
    #  \param [encoded_bytes] Is a byte array. It contains a number of encoded TLV objects.
    #
    #  \returns A TlvResult object. In case of success the data member holds a sequence of TlvEntry objects
    #           that were parsed from the value given in parameter encoded_bytes.
    #    
    @staticmethod    
    def parse_bytes(encoded_bytes):
        result = TlvResult(ERR_OK, [])
        end_position = len(encoded_bytes)
        read_position = 0
        
        while (result.err_code == ERR_OK) and (read_position < end_position):
            if (end_position - read_position) >= 3:
                entry = TlvEntry()
                # Parse tag and length bytes
                entry.tag = int(encoded_bytes[read_position])
                entry_len = (int(encoded_bytes[read_position + 1]) << 8) + int(encoded_bytes[read_position + 2])
                read_position += 3
                
                # Parse contents bytes
                if (end_position - read_position) >= entry_len:
                    entry.value = encoded_bytes[read_position:read_position + entry_len]
                    read_position += entry_len
                    result.data.append(entry)
                else:
                    result.err_code = ERR_DATA_LEN
            else:
                result.err_code = ERR_DATA_LEN        
        
        return result

    ## \brief This method encodes a sequence of TlvEntry objects into a byte array.
    #
    #  \param [tlv_objects] Is a sequence of TlvEntry objects.
    #
    #  \returns A byte array that consists of the concatenated encodings of the TlvEntry objects specified by
    #           parameter tlv_objects.
    #    
    @staticmethod    
    def to_bytes(tlv_objects):
        result = bytes()
        
        for i in tlv_objects:
            result = result + TlvStream.make_header(i.tag, len(i.value))
            result = result + i.value
        
        return result

    ## \brief This method transforms a sequence of TlvEntry objects into a corresponding sequence of python3 values.
    #
    #  \param [tlv_obj_sequence] Is a sequence of TlvEntry objects.
    #
    #  \returns A sequence of generic python3 values. The types of the sequence components depend on the tags of the
    #           TlvEntry objects specified by parameter tlv_obj_sequence.
    #    
    @staticmethod
    def convert_all(tlv_obj_sequence):
        return list(map(lambda x: x.tlv_convert(), tlv_obj_sequence))

    ## \brief This method allows to write a single TlvEntry object to a socket. It does not return until the TLV
    #         object was sent or an error was encountered.
    #
    #  \param [sock] Is a socket object. This socket is used for sending data.
    #
    #  \param [tlv_object] Is a TlvEntry object. This object is encoded and sent through the socket.
    #
    #  \returns An integer. This integer represents an error code. A value of ERR_OK signifies successfull completion
    #           of the send operation.
    #    
    @staticmethod    
    def write_tlv(sock, tlv_object):
        result = ERR_OK
        
        if len(tlv_object.value) <= LEN_MAX:        
            try:
                data = TlvStream.make_header(tlv_object.tag, len(tlv_object.value))
                data = data + tlv_object.value
                sock.sendall(data)
            except:
                result = ERR_SOCK_WRITE
        else:
            result = ERR_DATA_LEN
        
        return result


## \brief A class that provides functionality for managing a TLV server instance.
#
class TlvServer:
    ## \brief Constructor. 
    #
    #  \param [binary] Is a string. Has to specify the file name of the binary of the TLV server.
    #
    #  \param [server_address] Is a string. Has to specify the address via which the TLV server is
    #         to be reached.
    #
    def __init__(self, binary = get_tlv_server_path(), server_address = SERVER_ADDRESS):
        ## \brief Holds the the server address
        self.address = server_address
        ## \brief Holds the file name of the server binary
        self.binary_name = binary
        ## \brief Holds the object which represents the subprocess as returned by subprocess.Popen.         
        self._server_process = None
        ## \brief A boolean which is true iff the server is currently running
        self.is_running = False

    ## \brief This method allows TlvServer instances to be used by the "with" operator. Starts the server
    #         process.
    #
    #  \returns self.
    #        
    def __enter__(self):
        self.start()
        return self

    ## \brief This method allows TlvServer instances to be used by the "with" operator. Stops the server
    #         process.
    #
    #  \returns Nothing.
    #                
    def __exit__(self, exception_type, exeception_value, traceback):
        self.stop()

    ## \brief This method allows to conduct a transaction with the TLV server. Which means it allows to call a method
    #         of an object which is managed by this server.
    #
    #  \param [object_name] Is a string. It specifies the name of the object which is to be used in the method call.
    #
    #  \param [method_name] Is a string. It names the method which is to be called on the object specified by the
    #         parameter object_name.
    #
    #  \param [params] Is a TlvEntry object. It represents the parameter which is to be used for the method call.
    #
    #  \returns A sequence. The type of the sequence components is generic and depends on the values sent back by the 
    #           server.
    #    
    def do_method_call(self, object_name, method_name, params):
        sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
        sock.connect(self.address)    
        res = TlvStream.transact(sock, object_name, method_name, params)    
        sock.shutdown(socket.SHUT_RDWR)
        sock.close()
        return res        

    ## \brief This method allows to retrieve the names of all objects known to this TLV server.
    #
    #  \returns A sequence of strings. Each component in the sequence is an object name.
    #        
    def list_objects(self):
        param = TlvEntry().to_null()
        return self.do_method_call("root", "listobjects", param)

    ## \brief This method allows to retrieve the names of all service providers known to this TLV server.
    #
    #  \returns A sequence of strings. Each component in the sequence is a name of a  service provider.
    #        
    def list_providers(self):
        param = TlvEntry().to_null()
        return self.do_method_call("root", "listproviders", param)

    ## \brief This method deletes all objects currently managed by this TLV server.
    #
    #  \returns An empty sequence,
    #        
    def delete_all(self):
        param = TlvEntry().to_null()
        return self.do_method_call("root", "clear", param)

    ## \brief This method deletes an object which is managed by this TLV server.
    #
    #  \param [object_name] Is a string. It specifies the name of the object which is to be deleted.
    #    
    #  \returns An empty sequence.
    #        
    def delete_object(self, object_name):
        param = TlvEntry().to_null()
        return self.do_method_call(object_name, "delete", param)

    ## \brief This method starts the TLV server process which this object represents.
    #
    #  \returns Nothing.
    #                
    def start(self):
        stop = False
        MAX_TRIES = 500
        if not self.is_running:
            
            if os.path.exists(self.address):
                raise TlvException("Socket " + self.address + " already exists")
            
            self._server_process = subprocess.Popen([self.binary_name, self.address])            
            
            # Wait for socket file to appear
            while not stop:
                stop = os.path.exists(self.address)
            
            # Wait until we actually can talk to the server
            exception_count = 0
            stop = False
            while not stop:
                try:
                    self.list_objects()
                    stop = True
                    self.is_running = True
                except:
                    exception_count += 1
                    
                    # Too many exceptions. Something is really wrong ...
                    if exception_count >= MAX_TRIES: 
                        stop = True
            
            if exception_count >= MAX_TRIES:
                raise TlvException("Unable to connect to server")
            

    ## \brief This method stops the TLV server process which this object represents.
    #
    #  \returns Nothing.
    #        
    def stop(self):
        if self.is_running:
            param = TlvEntry()

            sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
            sock.connect(self.address)    
            res = TlvStream.transact(sock, 'root', 'close', param)    
            sock.shutdown(socket.SHUT_RDWR)
            sock.close()
            
            self._server_process.wait()    
            res_code = self._server_process.returncode
            self.is_running = False


## \brief A class which provides boiler plate functionality for accessing and managing objects provided by
#         a TLV server.
#
class TlvProxy:
    ## \brief Constructor. 
    #
    #  \param [server_name] Is a string. Has to specify the UNIX domain socket via which the TLV server which manages 
    #         the desired TLV object can be reached.
    #
    def __init__(self, server_name):
        ## \brief Holds the name of the TLV object which is represented by this TlvProxy instance.
        self._handle = ""
        ## \brief Holds the server address.
        self._server_name = server_name

    ## \brief This method allows TlvProxy instances to be used by the "with" operator.
    #
    #  \returns self.
    #            
    def __enter__(self):
        return self

    ## \brief This method allows TlvProxy instances to be used by the "with" operator. Deletes the TLV object represented
    #         by this TlvProxy instance.
    #
    #  \returns self.
    #                    
    def __exit__(self, exception_type, exeception_value, traceback):
        self.delete()

    ## \brief This method allows to conduct a transaction with the TLV server which means it allows to call a method 
    #         of an object which is managed by the TLV server.
    #
    #  \param [object_name] Is a string. It specifies the name of the object which is to be used in the method call.
    #
    #  \param [method_name] Is a string. It names the method which is to be called on the object specified by the
    #         parameter object_name.
    #
    #  \param [params] Is a TlvEntry object. It represents the parameter which is to be used for the method call.
    #
    #  \returns A sequence. The type of the sequence components is generic and depends on the values sent back by the 
    #           server.
    #    
    def do_method_call(self, object_name, method_name, params):
        sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
        sock.connect(self._server_name)    
        res = TlvStream.transact(sock, object_name, method_name, params)    
        sock.shutdown(socket.SHUT_RDWR)
        sock.close()
        return res        

    ## \brief This method deletes the TLV object which is represented by this TlvProxy instance.
    #
    #  \returns Nothing.
    #        
    def delete(self):
        param = TlvEntry().to_null()
        res = self.do_method_call(self._handle, 'delete', param)
        self._handle = ''

    ## \brief Returns the name of the TLV object which is managed by this TlvProxy instance.
    #
    #  \returns A string containing the object name.
    #                
    def get_handle(self):
        return self._handle

## \brief A class that allows to en/decode dictionary objects to/from TLV format
#
class TlvDict:
    ## \brief This method takes a dictionary specified in parameter dict_data and returns a correspondig
    #         TlvEntry object.
    #
    #  \param [dict_data] A dictionary. Keys and values are transformed into strings.
    #
    #  \returns A TlvEntry object
    #    
    @staticmethod
    def dict_to_tlv(dict_data):
        entry = TlvEntry()
        entry_data = []
        
        for i in dict_data.keys():
            key = TlvEntry().to_string(str(i))
            value = TlvEntry().to_string(str(dict_data[i]))
            seq = TlvEntry().to_sequence([key, value])
            entry_data.append(seq)
        
        entry.to_sequence(entry_data)
        
        return entry

    ## \brief This method takes a parsed dictionary as returned by TlvStream.transact and turns it into a dictionary 
    #
    #  \param [parsed_dict] A sequence of two element sequences. Both elements of that sequence are turned into strings.
    #
    #  \returns A dictionary object.
    #
    @staticmethod        
    def parsed_tlv_to_dict(parsed_dict):
        result = {}
        
        for i in parsed_dict:
                result[str(i[0])] = str(i[1])
        
        return result

## \brief A class which allows to manage and access echo objects managed by a TLV server.
#
class TestEcho(TlvProxy):
    ## \brief Constructor. Creates a new echo object on the server. Its name/handle is stored in self._handle.
    #
    #  \param [server_name] Is a string. Has to specify the UNIX domain socket via which the TLV server which manages 
    #         the newly created TLV test aritmetic object can be reached.
    #
    def __init__(self, server_name):
        TlvProxy.__init__(self, server_name)
        param = TlvEntry().to_null()
        res = self.do_method_call('new', 'echo', param)        
        self._handle = res[0]            

    ## \brief This method sends a TlvEntry object to an echo object on a TLV server. There it is parsed, copied and sent
    #         back to the client.
    #
    #  \param [param] Is a TlvEntry object.
    #
    #  \returns A sequence. The type of the sequence components is generic and depends on the values sent back by the 
    #           server.    
    #    
    def echo(self, param):
        res = self.do_method_call(self._handle, 'echo', param)
    
        return res

    ## \brief This method sends a dictionary object to an echo object on a TLV server. There it is parsed, modified and sent
    #         back to the client.
    #
    #  \param [param] Is a dictionary object.
    #
    #  \returns A sequence. The type of the sequence components is generic and depends on the values sent back by the 
    #           server.    
    #    
    def echo_dict(self, param):
        res = self.do_method_call(self._handle, 'echodict', TlvDict.dict_to_tlv(param))
    
        return TlvDict.parsed_tlv_to_dict(res[0])

    
## \brief A class which allows to manage and access test arithmetic objects managed by a TLV server.
#
class TestArithmetic(TlvProxy):
    ## \brief Constructor. Creates a new test arithmetic object on the server. Its name/handle is stored in self._handle.
    #
    #  \param [server_name] Is a string. Has to specify the UNIX domain socket via which the TLV server which manages 
    #         the newly created TLV test aritmetic object can be reached.
    #
    def __init__(self, server_name):
        TlvProxy.__init__(self, server_name)
        param = TlvEntry().to_null()
        res = self.do_method_call('new', 'arithmetic', param)        
        self._handle = res[0]            

    ## \brief This method allows to add two integer numbers through the test arithmetic TLV object managed by this object.
    #
    #  \param [left_arg] Is a 32 bit signed integer. Specifies the left summand,
    #    
    #  \param [right_arg] Is a 32 bit signed integer. Specifies the right summand,
    #    
    #  \returns An integer which is the result of the addition of left_arg and right_arg.
    #                    
    def add(self, left_arg, right_arg):
        param = TlvEntry()
        param.to_sequence([TlvEntry().to_int(left_arg), TlvEntry().to_int(right_arg)])
        res = self.do_method_call(self._handle, 'add', param)
    
        return res[0]

