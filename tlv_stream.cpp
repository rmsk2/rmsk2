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

/*! \file tlv_stream.cpp
 *  \brief Contains the implementation of the tlv_entry, tlv_stream and socket_tlv_stream classes.
 */ 

#include<sys/types.h>
#include<sys/socket.h>
#include<unistd.h>
#include<stdlib.h>
#include<sys/un.h>
#include<iostream>
#include<algorithm>
#include<boost/lexical_cast.hpp>
#include<tlv_stream.h>

const unsigned int LEN_MAX = 65536;

void tlv_entry::print()
{
    if (!parse_all())
    {
        cout << "ERROR: TLV encoding not well formed" << endl;
    }
    else
    {    
        print_rec(0);
    }
}

bool tlv_entry::parse_all()
{
    bool result = true;
    children.clear();
    
    if (tag == TAG_SEQUENCE)
    {
        result = tlv_convert(children);
        
        for (unsigned int count = 0; (count < children.size()) and result; count++)
        {
            result = children[count].parse_all();
        }
    }
    
    return result;
}

void tlv_entry::to_null()
{
    tag = TAG_NULL;
    value.clear();
    children.clear();
}

void tlv_entry::to_int(int val)
{
    unsigned int conv_help = (unsigned int)val;

    tag = TAG_INT;
    value.clear();
    children.clear();    
    
    value.push_back((unsigned char)((conv_help & (0xFF000000)) >> 24));
    value.push_back((unsigned char)((conv_help & (0x00FF0000)) >> 16));    
    value.push_back((unsigned char)((conv_help & (0x0000FF00)) >> 8));    
    value.push_back((unsigned char)(conv_help & 0x000000FF));        
}


void tlv_entry::to_result_code(unsigned int val)
{
    tag = TAG_RESULT_CODE;
    value.clear();
    children.clear();    
    
    value.push_back((unsigned char)((val & (0xFF000000)) >> 24));
    value.push_back((unsigned char)((val & (0x00FF0000)) >> 16));    
    value.push_back((unsigned char)((val & (0x0000FF00)) >> 8));    
    value.push_back((unsigned char)(val & 0x000000FF));        
}

void tlv_entry::to_string(const string& str)
{
    tag = TAG_STRING;
    children.clear();        
    value = basic_string<unsigned char>((unsigned char*)str.data(), str.length());
}

void tlv_entry::to_byte_array(const basic_string<unsigned char>& str)
{
    tag = TAG_BYTE_ARRAY;
    children.clear();        
    value = basic_string<unsigned char>(str.data(), str.length());
}

void tlv_entry::to_double(double val)
{
    tag = TAG_DOUBLE;
    string help;
    
    help = boost::lexical_cast<string>(val);
    children.clear();        
    value = basic_string<unsigned char>((unsigned char *)help.data(), help.length());
}

void tlv_entry::to_sequence(vector<tlv_entry>& components)
{
    tag = TAG_SEQUENCE;
    children.clear();
    tlv_stream::to_bytes(components, value);
}

void tlv_entry::print_rec(unsigned int indent)
{
    basic_string<unsigned char>::iterator iter;
    int res_int = 0;
    unsigned int res_result = 0;
    string res_string = "";
    double res_double = 0.0;
    string indent_string(indent, ' ');
    vector<tlv_entry>::iterator child_iter;
    
    switch(tag)
    {
        case TAG_INT:
            (void)tlv_convert(res_int);
            cout << indent_string << res_int << endl;
            break;
        case TAG_RESULT_CODE:
            (void)tlv_convert(res_result);
            cout << indent_string << "Result: " << res_result << endl;
            break;
        case TAG_STRING:
            (void)tlv_convert(res_string);
            cout << indent_string << res_string << endl;
            break;
        case TAG_DOUBLE:
            (void)tlv_convert(res_double);
            cout << indent_string << res_double << endl;
            break;
        case TAG_SEQUENCE:
            cout << indent_string << "SEQUENCE" << endl;

            for (child_iter = children.begin(); child_iter != children.end(); ++child_iter)
            {
                child_iter->print_rec(indent + 4);
            }
            
            break;
            
        case TAG_NULL:
            cout << indent_string << "NIL" << endl;
            break;
        default:
            cout << indent_string;
            // Print contents bytes as hex string
            
            ios_base::fmtflags stream_flags = cout.flags();
            // Save stream properties
            int s_width = cout.width();
            char fill_char = cout.fill();
            cout.fill('0');
            cout.setf(ios::hex, ios::dec|ios::hex|ios::oct);
            
            // Print individual bytes
            for (iter = value.begin(); iter != value.end(); ++iter)
            {
                cout.width(2);
                cout << (int)(*iter) << ' ';
            }
            
            cout << endl;
            
            // Restore stream properties
            cout.flags(stream_flags);
            cout.width(s_width);
            cout.fill(fill_char);
            break;
    }
}

bool tlv_entry::tlv_convert(vector<tlv_entry>& result)
{
    bool conv_success = true;

    if (tag == TAG_SEQUENCE)
    {
        if (tlv_stream::parse_bytes(value, result) != ERR_OK)
        {
            conv_success = false;
        }
    }
    else
    {
        conv_success = false;
    }
    
    return conv_success;
}

bool tlv_entry::tlv_convert(int& result)
{
    bool conv_success = true;
    unsigned int help = 0;
    
    if ((value.length() != 4) and (tag != TAG_INT))
    {
        conv_success = false;
    }
    else
    {        
        help = value[0];
        help <<= 8;
        help |= value[1];
        help <<= 8;
        help |= value[2];
        help <<= 8;
        help |= value[3];
        
        result = (int)help;
    }
    
    return conv_success;
}

bool tlv_entry::tlv_convert(unsigned int& result)
{
    bool conv_success = true;
        
    if ((value.length() != 4) and (tag != TAG_RESULT_CODE))
    {
        conv_success = false;
    }
    else
    {                
        result = value[0];
        result <<= 8;
        result |= value[1];
        result <<= 8;
        result |= value[2];
        result <<= 8;
        result |= value[3];
    }
    
    return conv_success;
}


bool tlv_entry::tlv_convert(string& result)
{
    bool conv_success = true;
    
    if (tag != TAG_STRING)
    {
        conv_success = false;
    }
    else
    {
        result = string((char *)value.data(), value.length());
    }

    return conv_success;
}

bool tlv_entry::tlv_convert(basic_string<unsigned char>& result)
{
    bool conv_success = true;
        
    if (tag != TAG_BYTE_ARRAY)
    {
        conv_success = false;
    }
    else
    {
        result = value;
    }

    return conv_success;
}

bool tlv_entry::tlv_convert(double& result)
{
    bool conv_success = true;
    string helper((char *)value.data(), value.length());

    if (tag != TAG_DOUBLE)
    {
        conv_success = false;
    }
    else
    {
        try
        {
            result = boost::lexical_cast<double>(helper);
        }
        catch(...)
        {
            conv_success = false;
        }
        
    }

    return conv_success;
}

/* ------------------------------------------------------------------------------------------- */

void tlv_stream::make_header(basic_string<unsigned char>& header, unsigned char tag, unsigned short value_len)
{
    header.clear();
    
    header += tag;
    header += (value_len & 0xFF00) >> 8;
    header += (value_len & 0xFF);    
}

unsigned int tlv_stream::parse_bytes(basic_string<unsigned char>& encoded_bytes, vector<tlv_entry>& entries)
{
    unsigned int result = ERR_OK;
    unsigned int read_position = 0, end_position = encoded_bytes.length();
    tlv_entry entry;
    unsigned int entry_length;
    
    entries.clear();
    
    while((read_position < end_position) and (result == ERR_OK))
    {
        if ((end_position - read_position) >= 3)
        {
            // Read header
            entry.tag = encoded_bytes[read_position];
            entry_length = (encoded_bytes[read_position + 1] << 8) | encoded_bytes[read_position + 2];
            read_position += 3;
            
            if ((end_position - read_position) >= entry_length)
            {
                // Read contents bytes
                entry.value = basic_string<unsigned char>(encoded_bytes.data() + read_position, entry_length);
                entries.push_back(entry);
                read_position += entry_length;
            }
            else
            {
                result = ERR_DATA_LEN;
            }
        }
        else
        {
            result = ERR_DATA_LEN;
        }
    }
    
    return result;
}

void tlv_stream::to_bytes(vector<tlv_entry>& entries, basic_string<unsigned char>& encoded_bytes)
{
    encoded_bytes.clear();
    vector<tlv_entry>::iterator iter;
    basic_string<unsigned char> header;
    
    for (iter = entries.begin(); iter != entries.end(); ++iter)
    {
        make_header(header, iter->tag, iter->value.length());
        encoded_bytes += header;
        encoded_bytes += iter->value;
    }
}

unsigned int tlv_stream::read_tlv(tlv_entry& entry)
{
    unsigned int result = ERR_OK;
    basic_string<unsigned char> tag_length_help;
    unsigned int tlv_length;
    
    if ((result = read_all(tag_length_help, 3)) == ERR_OK)
    {
        entry.tag = tag_length_help[0];
        tlv_length = (tag_length_help[1] << 8) | tag_length_help[2];
        
        result = read_all(entry.value, tlv_length);
    }
    
    return result;
}

unsigned int tlv_stream::write_tlv(tlv_entry& entry)
{
    unsigned int result = ERR_OK;
    basic_string<unsigned char> header;
    
    if (entry.value.length() > LEN_MAX)
    {
        result = ERR_DATA_LEN;
    }
    else
    {
        make_header(header, entry.tag, (unsigned short)entry.value.length());
        
        if ((result = write_all(header)) == ERR_OK)
        {
            result = write_all(entry.value);
        }
    }
    
    return result;
}

unsigned int tlv_stream::write_success_tlv(tlv_entry& output_value)
{
    unsigned int result = ERR_OK;
    tlv_entry success_code;
    
    success_code.to_result_code(0);
    
    if ((result = write_tlv(output_value)) == ERR_OK)
    {
        result = write_tlv(success_code);
    }
    
    return result;
}

unsigned int tlv_stream::write_error_tlv(unsigned int error_code)
{
    unsigned int result = ERR_OK;
    tlv_entry err_code;
    
    err_code.to_result_code(error_code);
    result = write_tlv(err_code);
    
    return result;
}

/* ------------------------------------------------------------------------------------------- */

unsigned int socket_tlv_stream::read_available(basic_string<unsigned char>& data_read)
{
    unsigned int result = ERR_OK;
    int bytes_read;
    
    if ((bytes_read = read(socket_fd, buffer, BUF_SIZE)) > 0)
    {
        data_read = basic_string<unsigned char>(buffer, bytes_read);
    }
    else
    {
        result = ERR_SOCK_READ;
    }
    
    return result;
}

unsigned int socket_tlv_stream::read_all(basic_string<unsigned char>& data_read, unsigned int bytes_to_read)
{
    unsigned int result = ERR_OK; 
    int bytes_read_this_time;
    
    data_read.clear();
    
    while ((bytes_to_read > 0) and (result ==  ERR_OK))
    {
        if ((bytes_read_this_time = read(socket_fd, buffer, min(BUF_SIZE, bytes_to_read))) <= 0)
        {
            result = ERR_SOCK_READ;        
        }
        else
        {
            bytes_to_read -= bytes_read_this_time;
            data_read += basic_string<unsigned char>(buffer, bytes_read_this_time);
        }
    }
    
    return result;
}

unsigned int socket_tlv_stream::write_all(basic_string<unsigned char>& data_to_write)
{
    unsigned int result = ERR_OK;    
    int bytes_to_write = data_to_write.length(), bytes_already_written = 0, bytes_written_this_time;
    
    while((bytes_to_write > 0) and (result == ERR_OK))
    {
        if ((bytes_written_this_time = write(socket_fd, data_to_write.data() + bytes_already_written, bytes_to_write)) < 0)
        {
            result = ERR_SOCK_WRITE;            
        }
        else
        {
            bytes_to_write -= bytes_written_this_time;
            bytes_already_written += bytes_written_this_time;
        }       
    }
    
    return result;
}

socket_tlv_stream::~socket_tlv_stream() 
{ 
    shutdown(socket_fd, SHUT_RDWR);
    close(socket_fd); 
}

