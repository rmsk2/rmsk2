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

/*! \file rotor_sim.cpp
 *
 *  \brief Implements a generic command line simulator for all machines simulated by enigma and rotorvis.
 */     

#include<iostream>
#include<vector>
#include<string>
#include<exception>
#include<boost/program_options.hpp>
#include<boost/scoped_ptr.hpp>
#include<rotor_machine.h>

namespace po = boost::program_options;
using namespace std;

const int RETVAL_OK = 0;
const int ERR_WRONG_COMMAND_LINE = 1;
const int ERR_IO_FAILURE = 42;


/*! \brief A class which implements the generic command line simulator called rotorsim.
 */
class rotorsim {
public:
    /*! \brief Constructor.
     */
    rotorsim();

    /*! \brief This method parses the command line and processes the input data using the appropriate rotor machine.
     */
    int parse(int argc, char **argv);
    
protected:

    /*! \brief This method prints a message that describes how to use rotorsim.
     */
    void print_help_message(po::options_description *desc);

    /*! \brief This method prints a message that describes how to use rotorsim.
     */
    int execute_command();

    /*! \brief This method performs the actual en/decyption of the input data using the rotor machine specified
     *         on the command line.
     */
    int process_stream(istream *in, ostream *out, int output_grouping, sigc::slot<Glib::ustring, gunichar> proc_func, sigc::slot<bool, gunichar> symbol_is_ok);

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

    /*! \brief Holds the grouping width as specified on the command line by the -g option.
     */
    int grouping_width;

    /*! \brief Holds the name of the input file as specified on the command line by the -i option or "" if
     *         no name has been given.
     */
    string input_file;

    /*! \brief Holds the name of the output file as specified on the command line by the -o option or "" if
     *         no name has been given.
     */
    string output_file;

    /*! \brief Holds the name of the configuration file as specified on the command line by the -f option. The
     *         data in this file is used to restore the state of the rotor machine.
     */
    string config_file;

    /*! \brief Holds the command that is to be executed as specified on the command line by the -c option or as
     *         a positional parameter.
     */
    string command;

};

rotorsim::rotorsim()
    :   desc("Allowed options")
{
    p.add("command", 1);    

    desc.add_options()
        ("help,h", "Produce help message")
        ("config-file,f", po::value<string>(), "Configuration file to read")
        ("input-file,i", po::value<string>(), "Input file to read. Optional. stdin used if missing.")        
        ("output-file,o", po::value<string>(), "Output file to produce. Optional. stdout used if missing.")                    
        ("command,c", po::value<string>(), "Command to execute. Can be used without -c or --command")
        ("grouping,g", po::value<int>(&grouping_width)->default_value(0), "Grouping to use for output. Optional. No grouping if missing.")            
    ;
}

int rotorsim::execute_command()
{
    int result = RETVAL_OK;
    boost::scoped_ptr<rotor_machine> the_machine(rmsk::restore_from_file(config_file));
    istream *in = &cin;
    ostream *out = &cout;
    ifstream file_in;
    ofstream file_out;
    // functor that allows to encrypt or decrypt a character
    sigc::slot<Glib::ustring, gunichar> processor;
    // functor that allows to check whether a given symbol is a valid input character for
    // the underlying rotor machine at the time when it is called
    sigc::slot<bool, gunichar> verifier;
    
    do
    {
        // Check wehther the state of the rotor machine was successfully restored
        if (the_machine.get() == NULL)
        {
            result = ERR_IO_FAILURE;
            cout << "Unable to load machine configuration " << config_file << endl;
            break;
        }
        
        // Open input file
        if (input_file != "")
        {
            file_in.open(input_file, ifstream::in | ifstream::binary);
            
            if (!file_in.good())
            {
                result = ERR_IO_FAILURE;
                cout << "Unable to open input file " << input_file << endl;
                break;
            }
            else
            {
                in = &file_in;
            }
        }
        
        // Open output file
        if (output_file != "")
        {
            file_out.open(output_file, ofstream::out | ofstream::binary);
            
            if (!file_out.good())
            {
                result = ERR_IO_FAILURE;
                cout << "Unable to open output file " << output_file << endl;
                break;
            }
            else
            {
                out = &file_out;
            }
        }        
        
        // Prepare functors for processing and validity checking
        if (command == "encrypt")
        {
            processor = sigc::mem_fun(*(the_machine->get_keyboard().get()), &rotor_keyboard::symbol_typed_encrypt);
            verifier = sigc::mem_fun(*(the_machine->get_keyboard().get()), &rotor_keyboard::is_valid_input_encrypt);
        }
        else
        {
            processor = sigc::mem_fun(*(the_machine->get_keyboard().get()), &rotor_keyboard::symbol_typed_decrypt);
            verifier = sigc::mem_fun(*(the_machine->get_keyboard().get()), &rotor_keyboard::is_valid_input_decrypt);            
        }
        
        // Do processing
        result = process_stream(in, out, grouping_width, processor, verifier);
    
    } while(0);
    
    file_in.close();
    file_out.close();
    
    return result;
}

int rotorsim::process_stream(istream *in, ostream *out, int output_grouping, sigc::slot<Glib::ustring, gunichar> proc_func, sigc::slot<bool, gunichar> symbol_is_ok)
{
    int result = RETVAL_OK;
    int char_read;
    unsigned int group_count = 0;
    unsigned int chars_in_group = 0;
    string conv_help;
    const unsigned int GROUPS_PER_LINE = 10;
    
    try
    {
        while (in->good() and out->good())
        {
            // Read input character
            char_read = in->get();
            
            if ((char_read != EOF) and symbol_is_ok(::tolower((char)char_read)))
            {
                // Change input character to lowercase, process it and change the result
                // into uppercase
                conv_help = proc_func(::tolower((char)char_read)).uppercase();
                
                if (conv_help.length() > 0)
                {                    
                    (*out) <<  conv_help;
                    chars_in_group++;
                    
                    // Do we have to do grouping?
                    if (output_grouping != 0)
                    {
                        // Yes
                                                
                        if ((chars_in_group % output_grouping) == 0)
                        {
                            // A group was finished
                            (*out) << " ";
                            chars_in_group = 0;
                            group_count++;
                            
                            if (group_count == GROUPS_PER_LINE)
                            {
                                // A line of groups was finished
                                (*out) << endl;
                                group_count = 0;
                            }
                        }
                    }
                }
            }
        }
        
        // Check if input and output streams are still ok. If the failbit is set on either 
        // of the streams then flag an error. If the failbit is set but the corresponding
        // stream has also reached EOF, then everything is still OK.
        if (((in->fail() and !in->eof()) or (out->fail() and !out->eof())))
        {
            result = ERR_IO_FAILURE;
            cout << "IO error!" << endl;        
        }
        else
        {
            (*out) << endl;
        }        
        
    }
    catch(...)
    {
        result = ERR_IO_FAILURE;
        cout << "IO error!" << endl;
    }    
    
    return result;
}

void rotorsim::print_help_message(po::options_description *desc)
{
    cout << (*desc) << endl;
    cout << "Examples:" << endl;
    cout << "    rotorsim encrypt -f machine_config.ini -i in_file.txt -o out_file.txt -g 5" << endl;
    cout << "    rotorsim -c decrypt -f machine_config.ini -i in_file.txt -o out_file.txt" << endl;         
    cout << "    rotorsim encrypt -f machine_config.ini" << endl;            
    cout << endl;
}

int rotorsim::parse(int argc, char **argv)
{
    int return_code = RETVAL_OK;

    try
    {
        po::store(po::command_line_parser(argc, argv).options(desc).positional(p).run(), vm);
        po::notify(vm);    

        // Check if -h was specified
        if (vm.count("help")) 
        {
            print_help_message(&desc);
            
            return ERR_WRONG_COMMAND_LINE;
        }

        // Check if -f was specified
        if (vm.count("config-file") == 0) 
        {
            cout << "You have to specify a configuration file" << endl << endl;
            print_help_message(&desc);
            
            return ERR_WRONG_COMMAND_LINE;
        } 

        // Check if -c was specified either directly or through a positional parameter
        if (vm.count("command") == 0) 
        {
            cout << "You have to specify a command" << endl << endl;
            print_help_message(&desc);
            
            return ERR_WRONG_COMMAND_LINE;
        } 

        // Check if command is either encrypt or decrypt. No further commands are allowed.
        if ((vm["command"].as<string>() != "decrypt") and ((vm["command"].as<string>() != "encrypt")))
        {
            cout << "Unknown command " << vm["command"].as<string>() << endl;
            
            return ERR_WRONG_COMMAND_LINE;
        }

        // Command line specified by user is corret. Retrieve data from variable vm and store it
        // in the corresponding instance variables.
        config_file = vm["config-file"].as<string>();
        command = vm["command"].as<string>();

        if (vm.count("input-file")) 
        {
            input_file = vm["input-file"].as<string>();
        } 

        if (vm.count("output-file")) 
        {
            output_file = vm["output-file"].as<string>();
        } 
        
        if ((grouping_width < 0) or (grouping_width > 10))
        {
            grouping_width = 0;
        }        
    }
    catch (exception& e)
    {
        cout << e.what() << endl;
        return_code = ERR_WRONG_COMMAND_LINE;
    }
    
    if (return_code == RETVAL_OK)
    {
        // Parsing the command line was successful. Now do what the user requested.
        return_code = execute_command();
    }

    return return_code;
}

/*! \brief main function of rotorsim.
 */
int main(int argc, char **argv)
{
    rotorsim sim;   
    int ret_val = sim.parse(argc, argv);
        
    return ret_val;
}
