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

import socket
import datetime
import subprocess
import time
import os
import tempfile
import simpletest

SERVER_ADDRESS = './sock_tlvstream'
SERVER_BINARY = './tlv_object'
BUF_SIZE = 4096
LEN_MAX = 65536

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

TAG_INT = 0
TAG_STRING = 1
TAG_BYTE_ARRAY = 2
TAG_SEQUENCE = 3
TAG_DOUBLE = 4
TAG_NULL = 5
TAG_RESULT_CODE = 6

class TlvException(Exception):
    def __init__(self, error_message):
        Exception.__init__(self, 'tlvobject: ' + error_message)


class TlvEntry:
    def __init__(self):
        self.tag = TAG_NULL
        self.value = bytes()

    @staticmethod        
    def int_to_bytes(val):
        if val < 0:
            val = -val
            val = val ^ 0xFFFFFFFF
            val = val + 1
        
        return val.to_bytes(4, byteorder='big')
    
    def to_int(self, int_val):
        self.tag = TAG_INT
        self.value = TlvEntry.int_to_bytes(int_val)
        return self

    def to_result(self, result_val):
        self.tag = TAG_RESULT_CODE
        result_val = result_val & 0xFFFFFFFF
        self.value = result_val.to_bytes(4, byteorder='big')
        return self
    
    def to_string(self, str_val):
        self.tag = TAG_STRING
        self.value = str_val.encode()
        return self
    
    def to_null(self):
        self.tag = TAG_NULL
        self.value = b''
        return self
    
    def to_sequence(self, obj_sequence):
        self.tag = TAG_SEQUENCE
        self.value = TlvStream.to_bytes(obj_sequence)
        return self
    
    def to_double(self, double_val):
        self.tag = TAG_DOUBLE
        self.value = str(double_val).encode()
        return self
    
    def to_byte_array(self, byte_vector):
        self.tag = TAG_BYTE_ARRAY
        self.value = byte_vector
        return self
        
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
            
            for i in res.data:
                result.append(i.tlv_convert())
            
        elif self.tag == TAG_BYTE_ARRAY:
            result = self.value        
        else:             
            result = {'tag':self.tag, 'value':self.value}
        
        return result
    
        
class TlvResult:
    def __init__(self, err_code, data):
        self.err_code = err_code
        self.data = data


class TlvStream:
    def __init__(self):
        pass
    
    @staticmethod
    def transact(sock, obj_name, method_name, tlv_params):
        object_name_tlv = TlvEntry()
        object_name_tlv.to_string(obj_name)
        method_name_tlv = TlvEntry()
        method_name_tlv.to_string(method_name)
        end_reached = False
        result_values = []
        
        if TlvStream.write_tlv(sock, object_name_tlv) != ERR_OK:
            raise TlvException("Sending object name failed")

        if TlvStream.write_tlv(sock, method_name_tlv) != ERR_OK:
            raise TlvException("Sending method name failed")
            
        if TlvStream.write_tlv(sock, tlv_params) != ERR_OK:
            raise TlvException("Sending parameters failed")
                    
        while (not end_reached):
            err_code = TlvStream.read_tlv(sock)
            if err_code.err_code != ERR_OK:
                raise TlvException("Error receiving result data " + str(err_code.err_code))
            
            if err_code.data.tag != TAG_RESULT_CODE:           
                result_values.append(err_code.data)
            else:
                end_reached = True
                    
        call_result_tlv = err_code.data                                
        rc = call_result_tlv.tlv_convert()
        
        if rc != ERR_OK:
            raise TlvException("Method call failed. Protocol error: " + str(rc))        
            
        return TlvStream.convert_all(result_values)
    
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
            result[0] = ERR_SOCK_READ
        
        return result

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

    @staticmethod    
    def make_header(tag, data_len):
        return tag.to_bytes(1, byteorder='big') + data_len.to_bytes(2, byteorder='big')

    @staticmethod    
    def parse_bytes(encoded_bytes):
        result = TlvResult(ERR_OK, [])
        end_position = len(encoded_bytes)
        read_position = 0
        
        while (result.err_code == ERR_OK) and (read_position < end_position):
            if (end_position - read_position) >= 3:
                entry = TlvEntry()
                entry.tag = int(encoded_bytes[read_position])
                entry_len = (int(encoded_bytes[read_position + 1]) << 8) + int(encoded_bytes[read_position + 2])
                read_position += 3
                
                if (end_position - read_position) >= entry_len:
                    entry.value = encoded_bytes[read_position:read_position + entry_len]
                    read_position += entry_len
                    result.data.append(entry)
                else:
                    result.err_code = ERR_DATA_LEN
            else:
                result.err_code = ERR_DATA_LEN        
        
        return result

    @staticmethod    
    def to_bytes(tlv_objects):
        result = bytes()
        
        for i in tlv_objects:
            result = result + TlvStream.make_header(i.tag, len(i.value))
            result = result + i.value
        
        return result

    @staticmethod
    def convert_all(tlv_obj_sequence):
        return list(map(lambda x: x.tlv_convert(), tlv_obj_sequence))

    @staticmethod    
    def write_tlv(sock, tlv_object):
        result = ERR_OK
        
        if len(tlv_object.value) <= LEN_MAX:        
            try:
                sock.sendall(TlvStream.make_header(tlv_object.tag, len(tlv_object.value)))
                sock.sendall(tlv_object.value)
            except:
                result = ERR_SOCK_WRITE
        else:
            result = ERR_DATA_LEN
        
        return result


class TlvServer:
    def __init__(self, binary = SERVER_BINARY, server_address = SERVER_ADDRESS):
        self.address = server_address
        self.binary_name = binary
        self._server_process = None
        self.is_running = False
    
    def __enter__(self):
        self.start()
        return self
        
    def __exit__(self, exception_type, exeception_value, traceback):
        self.stop()
    
    def do_method_call(self, object_name, method_name, params):
        sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
        sock.connect(self.address)    
        res = TlvStream.transact(sock, object_name, method_name, params)    
        sock.shutdown(socket.SHUT_RDWR)
        return res        
    
    def list_objects(self):
        param = TlvEntry().to_null()
        return self.do_method_call("root", "listobjects", param)

    def list_providers(self):
        param = TlvEntry().to_null()
        return self.do_method_call("root", "listproviders", param)

    def delete_all(self):
        param = TlvEntry().to_null()
        return self.do_method_call("root", "clear", param)

    def delete_object(self, object_name):
        param = TlvEntry().to_null()
        return self.do_method_call(object_name, "delete", param)
        
    def start(self):
        stop = False
        if not self.is_running:
            
            if os.path.exists(self.address):
                raise TlvException("Socket " + self.address + " already exists")
            
            self._server_process = subprocess.Popen([self.binary_name, self.address])
            self.is_running = True
            
            while not stop:
                stop = os.path.exists(self.address)
        
    def stop(self):
        if self.is_running:
            param = TlvEntry()

            sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
            sock.connect(self.address)    
            res = TlvStream.transact(sock, 'root', 'close', param)    
            sock.shutdown(socket.SHUT_RDWR)
            
            self._server_process.wait()    
            res_code = self._server_process.returncode
            self.is_running = False

    def timeout_test():
        param = TlvEntry().to_string('egal')

        sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
        sock.connect(self.address)    
        res = TlvStream.write_tlv(sock, param)
        res = TlvStream.read_tlv(sock)    
        sock.shutdown(socket.SHUT_RDWR)


class TlvProxy:
    def __init__(self, server_name):
        self._handle = ""
        self._server_name = server_name
    
    def __enter__(self):
        return self
        
    def __exit__(self, exception_type, exeception_value, traceback):
        self.delete()
    
    def do_method_call(self, object_name, method_name, params):
        sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
        sock.connect(self._server_name)    
        res = TlvStream.transact(sock, object_name, method_name, params)    
        sock.shutdown(socket.SHUT_RDWR)
        return res        

    def delete(self):
        param = TlvEntry().to_null()
        res = self.do_method_call(self._handle, 'delete', param)
        self._handle = ''
        
    def get_handle(self):
        return self._handle


class TestEcho(TlvProxy):
    def __init__(self, server_name):
        TlvProxy.__init__(self, server_name)
        param = TlvEntry().to_null()
        res = self.do_method_call('new', 'echo', param)        
        self._handle = res[0]            

    def echo(self, param):
        res = self.do_method_call(self._handle, 'echo', param)
    
        return res
    

class TestArithmetic(TlvProxy):
    def __init__(self, server_name):
        TlvProxy.__init__(self, server_name)
        param = TlvEntry().to_null()
        res = self.do_method_call('new', 'arithmetic', param)        
        self._handle = res[0]            
            
    def add(self, left_arg, right_arg):
        param = TlvEntry()
        param.to_sequence([TlvEntry().to_int(left_arg), TlvEntry().to_int(right_arg)])
        res = self.do_method_call(self._handle, 'add', param)
    
        return res[0]


class TlvFuncTest(simpletest.SimpleTest):
    def __init__(self, name, left_argument, right_argument):
        super().__init__(name)
        self._left_summand = left_argument
        self._right_summand = right_argument

    @staticmethod
    def get_test_sequence():
        double_test = TlvEntry().to_double(1254.6553)
        null_test = TlvEntry().to_null()
        string_test = TlvEntry().to_string('hollaraedulioe')
        int_test = TlvEntry().to_int(-123456)
        non_empty_byte_array = TlvEntry().to_byte_array(b'\x00\x01\x02\x03\x04\x05\x06')
        uint = TlvEntry().to_result(0xFFFFFFFF)
        string_1_in_seq = TlvEntry().to_string('komp1')
        string_2_in_seq = TlvEntry().to_string('komp2')
        test_seq = TlvEntry().to_sequence([string_1_in_seq, string_2_in_seq])
        return [double_test, null_test, string_test, int_test, non_empty_byte_array, uint, test_seq]                

    def test(self):
        result = super().test()
        with TlvServer('./tlv_object', 'sock_tmpjffdfkdfgj') as s:
            with TestArithmetic(s.address) as a, TestEcho(s.address) as e:
                try:
                    add_result = a.add(self._left_summand, self._right_summand)
                    self.append_note("Arithmetic add test {} + {} = {}".format(self._left_summand, self._right_summand, add_result))
                    last_result = (add_result == (self._left_summand + self._right_summand))
                    result = result and last_result
                    
                    if not last_result:
                        self.append_note("Add test FAILED")
                                        
                    echo_result = e.echo(TlvEntry().to_sequence(self.get_test_sequence()))
                    echo_result = echo_result[0]
                    
                    last_result = (echo_result[0] == 1254.6553) and (echo_result[2] == 'hollaraedulioe') and (echo_result[3] == -123456) and (echo_result[6][0] == 'komp1')                    
                    result = result and last_result
                    self.append_note("Echo result: " + str(echo_result))
                    
                    if not last_result:
                        self.append_note("Echo test FAILED")                                        
                    
                    obj_list = s.list_objects()                    
                    self.append_note("Objects returned by server: " + str(obj_list))
                    last_result = (len(obj_list) == 2)
                    result = result and last_result
                    
                    if not last_result:
                        self.append_note("Unexpected number of objects returned")

                    self.append_note("Providers known to server: " + str(s.list_providers()))
                except:
                    self.append_note("EXCEPTON!!!!")
                    result = False
        
        return result


class TlvPerfTest(simpletest.SimpleTest):
    def __init__(self, name, num_iterations = 22000):
        super().__init__(name)
        self._iterations = num_iterations
            
    def test(self):
        result = super().test()

        with TlvServer('./tlv_object', 'sock_tmpjffdfkdfgj') as s, TestArithmetic(s.address) as a:
            try:
                jetzt = datetime.datetime.now()
                
                for i in range(self._iterations):
                    res = a.add(1, 1)

                spaeter = datetime.datetime.now()
                self.append_note("Time needed for {} iterations: {}".format(self._iterations, str(spaeter - jetzt)))
            except:
                self.append_note("EXCEPTON!!!!")
                result = False        
        
        return result

def get_module_test(num_iterations = 22000):
    functional_test = TlvFuncTest('TLV functional test', 17, 4)
    performance_test = TlvPerfTest('TLV performance test', num_iterations)
    all_tests = simpletest.CompositeTest('All TLV tests')
    all_tests.add(functional_test)
    all_tests.add(performance_test)
    
    return all_tests

def execute_tests(num_iterations = 22000):
    tests = get_module_test(num_iterations)
    test_result = tests.test()
    tests.print_notes()
