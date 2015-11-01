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

/*! \file tlv_stream.h
 *  \brief Header file for the tlv_entry, tlv_stream and socket_tlv_stream classes.
 */ 

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

/*! \brief Holds the maximum number of bytes to read in one go from the socket. */ 
const unsigned int BUF_SIZE = 4096;

/*! \brief Tag used to signify that a tlv_entry represents a 32 bit signed integer. */ 
const unsigned char TAG_INT = 0;
/*! \brief Tag used to signify that a tlv_entry represents a UTF-8 encoded string. */ 
const unsigned char TAG_STRING = 1;
/*! \brief Tag used to signify that a tlv_entry represents a byte array. */ 
const unsigned char TAG_BYTE_ARRAY = 2;
/*! \brief Tag used to signify that a tlv_entry represents a sequence of subordinate tlv_entry objects. */ 
const unsigned char TAG_SEQUENCE = 3;
/*! \brief Tag used to signify that a tlv_entry represents a floating point number. */ 
const unsigned char TAG_DOUBLE = 4;
/*! \brief Tag used to signify that a tlv_entry represents an empty or NULL value. */ 
const unsigned char TAG_NULL = 5;
/*! \brief Tag used to signify that a tlv_entry represents a 32 bit unsigned integer. */ 
const unsigned char TAG_RESULT_CODE = 6;

using namespace std;

/*! \brief A TLV class which is able to hold values of various types. The type is recorded in the form of a tag. The value or
 *         contents is stored in the form of a byte string.
 */ 
class tlv_entry {
public:
    /*! \brief Holds the type of the current value in the form of tag.
     */ 
    unsigned char tag;

    /*! \brief Holds the current value in the form of byte string.
     */ 
    basic_string<unsigned char> value;

    /*! \brief Prints the current value to stdout.
     */ 
    void print();

    /*! \brief Parses the contents bytes. Does not do much, if tlv_entry::tag is not TAG_SEQUENCE. In case it is
     *         TAG_SEQUNCE the contents bytes are parsed in order to fill the vector tlv_entry::children. 
     */ 
    bool parse_all();

    /*! \brief Holds the children of this tlv_entry instance after a successfull call of tlv_entry::parse_all().
     */ 
    vector<tlv_entry> children;

    /*! \brief Sets tag and value of this tlv_entry instance to represent the integer given in parameter val.
     */     
    void to_int(int val);

    /*! \brief Sets tag and value of this tlv_entry instance to represent the unsigned integer given in parameter val.
     */     
    void to_result_code(unsigned int val);    

    /*! \brief Sets tag and value of this tlv_entry instance to represent the NULL value.
     */     
    void to_null();
    
    /*! \brief Sets tag and value of this tlv_entry instance to represent the string value given in parameter str.
     */         
    void to_string(const string& str);

    /*! \brief Sets tag and value of this tlv_entry instance to represent the byte string value given in parameter str.
     */
    void to_byte_array(const basic_string<unsigned char>& str);    

    /*! \brief Sets tag and value of this tlv_entry instance to represent floating point number given in parameter val.
     */
    void to_double(double val);

    /*! \brief Sets tag and value of this tlv_entry instance to represent the sequence of tlv_entry objects specified in
    *          parameter components.
     */
    void to_sequence(vector<tlv_entry>& components);

    /*! \brief Converts the contents of this tlv_entry instance to an integer and stores the value in the integer referenced
     *         by parameter result. 
     *
     *  Returns true in case the conversion was successfull and false otherwise. Conversion will fail if the tag is not
     *  equal to TAG_INT.
     */ 
    bool tlv_convert(int& result);

    /*! \brief Converts the contents of this tlv_entry instance to an unsigned integer and stores the value in the 
     *         unsigned integer referenced by parameter result. 
     *
     *  Returns true in case the conversion was successfull and false otherwise. Conversion will fail if the tag is not
     *  equal to TAG_RESULT_CODE.
     */ 
    bool tlv_convert(unsigned int& result);    

    /*! \brief Converts the contents of this tlv_entry instance to a string and stores the value in the 
     *         string referenced by parameter result. 
     *
     *  Returns true in case the conversion was successfull and false otherwise. Conversion will fail if the tag is not
     *  equal to TAG_STRING.
     */ 
    bool tlv_convert(string& result);

    /*! \brief Converts the contents of this tlv_entry instance to a byte string and stores the value in the 
     *         byte string referenced by parameter result. 
     *
     *  Returns true in case the conversion was successfull and false otherwise. Conversion will fail if the tag is not
     *  equal to TAG_BYTE_ARRAY.
     */ 
    bool tlv_convert(basic_string<unsigned char>& result);

    /*! \brief Converts the contents of this tlv_entry instance to a double and stores the value in the 
     *         double referenced by parameter result. 
     *
     *  Returns true in case the conversion was successfull and false otherwise. Conversion will fail if the tag is not
     *  equal to TAG_DOUBLE.
     */ 
    bool tlv_convert(double& result);

    /*! \brief Converts the contents of this tlv_entry instance to a sequence of tlv_entry objects and stores the result
     *         in the vector referenced by parameter result. 
     *
     *  Returns true in case the conversion was successfull and false otherwise. Conversion will fail if the tag is not
     *  equal to TAG_SEQUENCE.
     */
    bool tlv_convert(vector<tlv_entry>& children);

protected:
    /*! \brief Prints the current value of this tlv_entry instance to stdout.
     *
     *  This method walks through the structure of this tlv_entry recursively and uses the parameter indent to specify
     *  how many space characters are be used to indent the tlv_entry objects on the level currently reached by the recursive
     *  call.
     */ 
    void print_rec(unsigned int indent);
};

/*! \brief An abstract TLV class which defines an interface for reading/writing byte strings and tlv_entry objects from/to
 *          an unspecified I/O stream.
 */
class tlv_stream {
public:
    /*! \brief An abstract method which is expected to read the number of bytes specified in parameter bytes_to_read from
     *         the I/O stream and to store these in the byte string referenced by parameter buffer.
     *
     *  In case of success the value ERR_OK is to be returned.
     */
    virtual unsigned int read_all(basic_string<unsigned char>& buffer, unsigned int bytes_to_read) = 0;

    /*! \brief An abstract method which is expected to write all bytes of the parameter buffer to the I/O stream.
     *
     *  In case of success the value ERR_OK is to be returned.
     */
    virtual unsigned int write_all(basic_string<unsigned char>& buffer) = 0;

    /*! \brief An abstract method which is expected to read some of bytes from the I/O stream and to store these in
     *         the byte string referenced by parameter buffer.
     *
     *  In case of success the value ERR_OK is to be returned.
     */
    virtual unsigned int read_available(basic_string<unsigned char>& data_read) = 0;

    /*! \brief This method reads the next tlv_entry object from the I/O stream. The wire format is: One byte tag followed
     *         by two bytes specifying the length of the contents bytes followed by the contents bytes.
     *
     *  In case of success the value ERR_OK is returned.
     */
    virtual unsigned int read_tlv(tlv_entry& entry);

    /*! \brief This method writes the tlv_entry object specified in parameter entry to the I/O stream.
     *
     *  In case of success the value ERR_OK is returned.
     */
    virtual unsigned int write_tlv(tlv_entry& entry);

    /*! \brief This method writes the tlv_entry object specified by the parameter output_value to the I/O stream. Additionally
     *         a tlv_entry object of type TAG_RESULT_CODE with value ERR_OK is written to the I/O stream.
     *
     *  In case of success the value ERR_OK is returned.
     */
    virtual unsigned int write_success_tlv(tlv_entry& output_value);

    /*! \brief This method writes a tlv_entry object of type TAG_RESULT_CODE to the I/O stream. The value of that
     *         object is determined by the parameter error_code.
     *
     *  In case of success the value ERR_OK is returned.
     */
    virtual unsigned int write_error_tlv(unsigned int error_code);

    /*! \brief This method stores the header (tag + length bytes) for a tlv_entry with the tag specified by parameter
     *         tag and contents bytes of length value_len in the byte string referenced by parameter header.
     */
    static void make_header(basic_string<unsigned char>& header, unsigned char tag, unsigned short value_len);    

    /*! \brief This method parses the byte string referenced by parameter encoded_bytes into a vector of tlv_entry
     *         objects. The resulting objects are stored in the vector referenced by the parameter entries.
     */
    static unsigned int parse_bytes(basic_string<unsigned char>& encoded_bytes, vector<tlv_entry>& entries);

    /*! \brief This method encodes the tlv_entry objects stored in the vector referenced by parameter entries into
     *         a byte string. The resulting byte string is returned through the parameter encoded_bytes.
     */
    static void to_bytes(vector<tlv_entry>& entries, basic_string<unsigned char>& encoded_bytes);

    /* \brief Destructor.
     */
    virtual ~tlv_stream() { ; }
};

/*! \brief This TLV class implements the abstract interface of tlv_stream by using a socket to read/write bytes.
 */
class socket_tlv_stream : public tlv_stream {
public:
    /*! \brief Constructor. The parameter fd has to specify the file descriptor of the underlying socket.
     */
    socket_tlv_stream(int fd) { socket_fd = fd; }

    /*! \brief This method reads the number of bytes specified in parameter bytes_to_read from a socket and stores these
     *         in the byte string referenced by parameter data_read.
     *
     *  In case of success the value ERR_OK is to be returned.
     */    
    virtual unsigned int read_all(basic_string<unsigned char>& data_read, unsigned int bytes_to_read);

    /*! \brief This methods writes all bytes of the parameter data_to_write to the socket.
     *
     *  In case of success the value ERR_OK is to be returned.
     */
    virtual unsigned int write_all(basic_string<unsigned char>& data_to_write);

    /*! \brief This method reads some bytes from a socket and stores these in the byte string referenced by parameter
     *         data_read.
     *
     *  In case of success the value ERR_OK is to be returned.
     */    
    virtual unsigned int read_available(basic_string<unsigned char>& data_read);

    /* \brief Destructor.
     */
    virtual ~socket_tlv_stream();

protected:
    /*! \brief Holds the socket's file descriptor.
     */ 
    int socket_fd;

    /*! \brief Buffer which is used for read operations.
     */ 
    unsigned char buffer[BUF_SIZE];
};

#endif /* __tlv_stream_h__ */ 
