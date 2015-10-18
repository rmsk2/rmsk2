/***************************************************************************
 * Copyright 2015 Martin Grap
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ***************************************************************************/

#ifndef __tlv_stream_h__
#define __tlv_stream_h__

#include<string>
#include<vector>
#include<exception>

const unsigned int ERR_OK = 0;
const unsigned int ERR_SOCK_CREATE = 1;
const unsigned int SOCK_ERR_BIND = 2;
const unsigned int SOCK_ERR_ACCEPT = 3;
const unsigned int ERR_REMOVE_PATH = 4;
const unsigned int ERR_SOCK_READ = 5;
const unsigned int ERR_SOCK_WRITE = 6;
const unsigned int ERR_SOCK_PATH_LEN = 7;
const unsigned int ERR_DATA_LEN = 8;
const unsigned int ERR_ERROR = 42;

const unsigned int BUF_SIZE = 4096;

const unsigned char TAG_INT = 0;
const unsigned char TAG_STRING = 1;
const unsigned char TAG_BYTE_ARRAY = 2;
const unsigned char TAG_SEQUENCE = 3;
const unsigned char TAG_DOUBLE = 4;
const unsigned char TAG_NULL = 5;
const unsigned char TAG_RESULT_CODE = 6;

using namespace std;

class tlv_entry {
public:
    unsigned char tag;
    basic_string<unsigned char> value;
    void print();
    bool parse_all();
    vector<tlv_entry> children;
    
    void to_int(int val);
    void to_result_code(unsigned int val);    
    void to_null();
    void to_string(const string& str);
    void to_byte_array(const basic_string<unsigned char>& str);    
    void to_double(double val);
    void to_sequence(vector<tlv_entry>& components);

    bool tlv_convert(int& result);
    bool tlv_convert(unsigned int& result);    
    bool tlv_convert(string& result);
    bool tlv_convert(basic_string<unsigned char>&);
    bool tlv_convert(double& result);
    bool tlv_convert(vector<tlv_entry>& children);

protected:
    void print_rec(unsigned int indent);
};

class tlv_stream {
public:
    virtual unsigned int read_all(basic_string<unsigned char>& buffer, unsigned int bytes_to_read) = 0;
    virtual unsigned int write_all(basic_string<unsigned char>& buffer) = 0;
    virtual unsigned int read_available(basic_string<unsigned char>& data_read) = 0;
    virtual unsigned int read_tlv(tlv_entry& entry);
    virtual unsigned int write_tlv(tlv_entry& entry);
    virtual unsigned int write_success_tlv(tlv_entry& output_value);
    virtual unsigned int write_error_tlv(unsigned int error_code);
    static void make_header(basic_string<unsigned char>& header, unsigned char tag, unsigned short value_len);    
    static unsigned int parse_bytes(basic_string<unsigned char>& encoded_bytes, vector<tlv_entry>& entries);
    static void to_bytes(vector<tlv_entry>& entries, basic_string<unsigned char>& encoded_bytes);

    virtual ~tlv_stream() { ; }
};

class socket_tlv_stream : public tlv_stream {
public:
    socket_tlv_stream(int fd) { socket_fd = fd; }
    
    virtual unsigned int read_all(basic_string<unsigned char>& data_read, unsigned int bytes_to_read);
    virtual unsigned int write_all(basic_string<unsigned char>& data_to_write);
    virtual unsigned int read_available(basic_string<unsigned char>& data_read);

    virtual ~socket_tlv_stream();

protected:
    int socket_fd;
    unsigned char buffer[BUF_SIZE];
};

#endif /* __tlv_stream_h__ */ 
