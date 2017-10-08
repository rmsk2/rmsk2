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

#ifndef __cmdline_base_h__
#define __cmdline_base_h__

/*! \file cmdline_base.h
 *  \brief Header file for the base class of all command line programs in this project.
 */ 

#include<iostream>
#include<vector>
#include<string>
#include<exception>
#include<boost/program_options.hpp>
#include<boost/shared_ptr.hpp>
#include<rotor_machine.h>

#define ROTORPOS_DEFAULT "xxnoposxx"

namespace po = boost::program_options;
using namespace std;

const int RETVAL_OK = 0;
const int ERR_WRONG_COMMAND_LINE = 1;
const int ERR_UNABLE_SET_ROTOR_POS = 2;
const int ERR_IO_FAILURE = 42;
const int ERR_ROTOR_MACHINE = 43;


/*! \brief A class which serves as the base class for several command line programs in this project.
 */
class cmdline_base {
public:
    /*! \brief Constructor.
     */
    cmdline_base(const char *description_text) : desc(description_text) { ; }

    /*! \brief This method parses the command line and processes the input data according to the purpose of this program.
     */
    virtual int parse(int argc, char **argv) { return RETVAL_OK; }

    /*! \brief This method executes the command requested by the user.
     */
    virtual int execute_command() { return RETVAL_OK; }

    /*! \brief Destructor.
     */
    virtual ~cmdline_base() { ; }
    
protected:

    /*! \brief This method prints a message that describes how to use rotorsim.
     */
    virtual void print_help_message(po::options_description *desc) { ; }

    /*! \brief This method creates the input stream used by this program. In case an error occurs NULL is returned. The parameter
     *         file_name has to specify the name of the input file or "" in case stdin is to be used. The second parameter is
     *         an ifstream instance which is returned as an istream if file_name is not the empty string.
     */
    istream *determine_input_stream(string& file_name, ifstream& file_in);

    /*! \brief This method creates the output stream used by this program. In case an error occurs NULL is returned. The parameter
     *         file_name has to specify the name of the output file or "" in case stdout is to be used. The second parameter is
     *         an ofstream instance which is returned as an ostream if file_name is not the empty string.
     */
    ostream *determine_output_stream(string& file_name, ofstream& file_out);

    /*! \brief This method saves the state of the rotor machine specified through parameter machine in the file named by the parameter
     *         file_name. Use "" as file_name in order to save the state to stdout. The parameter delimiter_first determines whether
     *         a separator character is written before (true) or after (false) the state data.
     *
     *  In case of success RETVAL_OK is returned.
     */
    int save_machine_state(string& file_name, rotor_machine *machine, bool delimiter_first = true);
    
    /*! \brief This method reads data from the stream specified in parameter in_s and stores that data in the string referenced
     *         by parameter data_read until either the value stored in delimiter is read or the end of the stream is reached.
     */
    int read_delimited_stream(istream *in_s, string& data_read, int delimiter);
    
    /*! \brief Holds a specification of the positional parameters that should be recognized. Here the only positional 
     *         parameter is the command (encrypt/decrypt) to execute.
     */
    po::positional_options_description p;    

    /*! \brief Holds a specification of all the command line parameters that are understood by rotorsim.
     */
    po::options_description desc;

    /*! \brief After parsing the command line the parsed data is stored in this object.
     */
    po::variables_map vm;        
};
 
#endif /* __cmdline_base_h__ */
