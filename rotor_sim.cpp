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
#include<boost/shared_ptr.hpp>
#include<rotor_machine.h>
#include<sigaba.h>
#include<glibmm.h>

#define ROTORPOS_DEFAULT "xnoposx"

namespace po = boost::program_options;
using namespace std;

const int RETVAL_OK = 0;
const int ERR_WRONG_COMMAND_LINE = 1;
const int ERR_UNABLE_SET_ROTOR_POS = 2;
const int ERR_IO_FAILURE = 42;


/*! \brief A class which implements the generic command line simulator called rotorsim.
 */
class rotor_sim {
public:
    /*! \brief Constructor.
     */
    rotor_sim();

    /*! \brief This method parses the command line and processes the input data using the appropriate rotor machine.
     */
    int parse(int argc, char **argv);

    /*! \brief This method executes the command requested by the user.
     */
    int execute_command();
    
protected:

    /*! \brief This method prints a message that describes how to use rotorsim.
     */
    void print_help_message(po::options_description *desc);

    /*! \brief Creates the machine that is to be simulated on the basis of the name of the config file contained in parameter
     *         config_file. Returns NULL in case of an error.
     */
    rotor_machine *determine_machine(string& config_file);

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

    /*! \brief This method save the state of the rotor machine specified through parameter machine in the file named by the parameter
     *         file_name. Use "" as file_name in order to save the state to stdout. In case of success RETVAL_OK is returned.
     */
    int save_machine_state(string& file_name, rotor_machine *machine);

    /*! \brief This method returns the permutations the underlying rotor machine generates during the next num_iterations - 1 steps.
     * 
     *  \param [num_iterations] Has to specify the number of permutations that are to be returned. The machine is stepped num_iterations - 1
     *                          by this method. 
     *  \param [out_s] Has to specify the output stream used by this method.
     *  \param [machine] Has to specify the rotor machine that is to be queried for permutations.
     */
    void execute_perm_command(int num_iterations, ostream *out_s, rotor_machine *machine);

    /*! \brief This method performs the operations required for the step command which is to step the underlying rotor machine a given
     *         number of times.
     * 
     *  \param [num_iterations] Has to specify the number times the underlying machine is to be stepped
     *  \param [out_s] Has to specify the output stream used by this method.
     *  \param [machine] Has to specify the rotor machine that is to be stepped.
     */
    void execute_step_command(int num_iterations, ostream *out_s, rotor_machine *machine);

    /*! \brief This method performs the operations required for setup stepping a SIGABA instance.
     * 
     *  \param [num_iterations] Has to specify the number times the underlying machine is to be stepped
     *  \param [setup_step_rotor_num] Has to specify the rotor number (1-5) which is to be setup stepped
     *  \param [out_s] Has to specify the output stream used by this method.
     *  \param [machine] Has to specify the rotor machine that is to be setup stepped.
     */
    void execute_sigabasetup_command(int num_iterations, int setup_step_rotor_num, ostream *out_s, rotor_machine *machine);

    /*! \brief This method writes a visualization of the current rotor positions to the output stream.
     * 
     *  \param [out_s] Has to specify the output stream used by this method.
     *  \param [machine] Has to specify the rotor machine which rotor positions are to be visualized.
     */
    void execute_getpos_command(ostream *out_s, rotor_machine *machine);
    
    /*! \brief This method performs the actual en/decyption of the input data using the rotor machine specified
     *         on the command line.
     */
    int process_stream(istream *in_s, ostream *out_s, int output_grouping, sigc::slot<Glib::ustring, gunichar> proc_func, sigc::slot<bool, gunichar> symbol_is_ok);

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

    /*! \brief Holds the grouping width as specified on the command line by the -g option.
     */
    int grouping_width;

    /*! \brief Holds the number of the SIGABA rotor to "setup step". 0 means no setup stepping is requested.
     */
    int setup_step_rotor_num;

    /*! \brief Holds the number iterations a "perm" or "step" command should be executed.
     */
    int num_iterations;

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

    /*! \brief Holds the name of the file into which the final state of the machine is saved after processing when
     *         the -s option has been used.
     */
    string state_file;

    /*! \brief Holds the new positions of the visible rotors. Has to be present when the setpos command is used.
     */
    string new_rotor_positions;

    /*! \brief Holds the command that is to be executed as specified on the command line by the -c option or as
     *         a positional parameter.
     */
    string command;

    /*! \brief This flag is true if the state reached at the end of processing should be stored. If state_file
     *         is not equal to the empty string the state is stored in the file named by state_file. Else the
     *         state is written to stdout. 
     */    
    bool state_progression;

};

rotor_sim::rotor_sim()
    :   desc("Allowed options")
{
    p.add("command", 1);    

    desc.add_options()
        ("help,h", "Produce help message")
        ("state-progression", "Write state reached after processing to stdout. Optional.")
        ("rotor-num,r", po::value<int>(&setup_step_rotor_num)->default_value(-1), "Setup step the SIGABA control rotor with the given number 1-5.")            
        ("config-file,f", po::value<string>(&config_file), "Configuration file to read")
        ("input-file,i", po::value<string>(&input_file), "Input file to read. Optional. stdin used if missing.")
        ("positions,p", po::value<string>(&new_rotor_positions)->default_value(ROTORPOS_DEFAULT), "New rotor positions. Optional. Only used with the encrypt or decrypt commands.")        
        ("output-file,o", po::value<string>(&output_file), "Output file to produce. Optional. stdout used if missing.")                    
        ("command,c", po::value<string>(&command), "Command to execute. Can be used without -c or --command. Allowed commands: encrypt, decrypt, step, perm, getpos, sigabasetup.")
        ("save-state,s", po::value<string>(&state_file), "Save state of machine in specified file after processing. Optional.")        
        ("grouping,g", po::value<int>(&grouping_width)->default_value(0), "Grouping to use for output. Optional. No grouping if missing.")
        ("num-iterations,n", po::value<int>(&num_iterations)->default_value(1), "Number of iterations to execute perm or step commands.")            
    ;
    
    state_progression = false;
}

istream *rotor_sim::determine_input_stream(string& file_name, ifstream& file_in)
{
    istream *result = NULL;

    if (file_name != "")
    {        
        file_in.open(file_name, ifstream::in | ifstream::binary);
        
        if (file_in.good())
        {
            result = &file_in;
        }
    }
    else
    {
        result = &cin;
    }
    
    return result;
}

ostream *rotor_sim::determine_output_stream(string& file_name, ofstream& file_out)
{
    ostream *result = NULL;
    
    if (file_name != "")
    {
        file_out.open(file_name, ofstream::out | ofstream::binary);
        
        if (file_out.good())
        {
            result = &file_out;
        }
    }            
    else
    {
        result = &cout;
    }
    
    return result;
}

rotor_machine *rotor_sim::determine_machine(string& config_file)
{
    rotor_machine *machine = NULL;
    string config_data;
    int error_code;    

    if (config_file != "")
    {
        machine = rmsk::restore_from_file(config_file);        
    }
    else
    {
        if ((error_code = read_delimited_stream(&cin, config_data, 0xFF)) == RETVAL_OK)
        {
            machine = rmsk::restore_from_data(config_data);        
        }
    }
        
    return machine;
}

int rotor_sim::save_machine_state(string& file_name, rotor_machine *machine)
{
    Glib::KeyFile ini_file;
    int result = RETVAL_OK;
    Glib::ustring data_temp; 
    string ini_data;       

    if (file_name != "")
    {
        if (machine->save(file_name))
        {
            result = ERR_IO_FAILURE;
        }
    }
    else
    {
        // Write state to stdout using 0xFF as delimiter between the state and the output data
        machine->save_ini(ini_file);
                        
        cout << (char)(255);
        
        data_temp = ini_file.to_data();
        ini_data = data_temp;
        cout << ini_data;                                
    }            
    
    return result;
}

void rotor_sim::execute_perm_command(int num_iterations, ostream *out, rotor_machine *machine)
{
    // perm command
    vector<unsigned int> current_perm;
    
    for (int count = 0; count < num_iterations; count++)
    {                    
        current_perm.clear();
        
        machine->get_current_perm(current_perm);
        
        (*out) <<  "[";

        // Write all but the last permutation component to output        
        for (unsigned int count = 0; count < current_perm.size() - 1; count++)
        {
            (*out) << current_perm[count] << ", ";
        }
                
        // Write last permutation component to output
        (*out) << current_perm[current_perm.size() - 1] << "]" << endl;   
        
        // Step machine if this is not the last iteration
        if (count < num_iterations - 1)
        {
            machine->step_rotors();
        }             
    }
    
    // num_iterations <= 0. Write at least an LF as output. The python side depends on it.
    if (num_iterations <= 0)
    {
        (*out) << endl;
    }                
}

void rotor_sim::execute_getpos_command(ostream *out, rotor_machine *machine)
{        
    string help = machine->visualize_all_positions();

    (*out) << help << endl;
}

void rotor_sim::execute_sigabasetup_command(int num_iterations, int setup_step_rotor_num, ostream *out, rotor_machine *machine)
{
    const char *rotor_names[5] = {STATOR_L, S_SLOW, S_FAST, S_MIDDLE, STATOR_R};
    
    sigaba *machine_as_sigaba = dynamic_cast<sigaba *>(machine);

    for (int count = 0; (count < num_iterations) and (setup_step_rotor_num > 0) and (machine_as_sigaba != NULL); count++)
    {
        // Setup stepping
        machine_as_sigaba->get_sigaba_stepper()->setup_step(rotor_names[(setup_step_rotor_num - 1) % 5]);        
        string help = machine->visualize_all_positions();    
        (*out) << help << endl;
    }
    
    if ((num_iterations <= 0) or (setup_step_rotor_num <= 0) or (machine_as_sigaba == NULL))
    {
        (*out) << endl;
    }                                
}

void rotor_sim::execute_step_command(int num_iterations, ostream *out, rotor_machine *machine)
{

    for (int count = 0; count < num_iterations; count++)
    {
        machine->step_rotors();
        string help = machine->visualize_all_positions();    
        (*out) << help << endl;
    }
    
    if (num_iterations <= 0)
    {
        (*out) << endl;
    }                                
}


int rotor_sim::execute_command()
{
    int result = RETVAL_OK;
    istream *in = NULL;
    ostream *out = NULL;
    ifstream file_in;
    ofstream file_out;
    // functor that allows to encrypt or decrypt a character
    sigc::slot<Glib::ustring, gunichar> processor;
    // functor that allows to check whether a given symbol is a valid input character for
    // the underlying rotor machine at the time when it is called
    sigc::slot<bool, gunichar> verifier;
    boost::shared_ptr<rotor_machine> the_machine;
    
    do
    {        
        // Create rotor machine object
        the_machine = boost::shared_ptr<rotor_machine>(determine_machine(config_file));        
        if (the_machine.get() == NULL)
        {
            result = ERR_IO_FAILURE;
            cout << "Unable to load machine configuration " << config_file << endl;
            break;
        }

        // Open input stream        
        in = determine_input_stream(input_file, file_in);        
        if (in == NULL)
        {
            result = ERR_IO_FAILURE;
            cout << "Unable to open input file " << input_file << endl;
            break;
        }
        
        // Open output stream
        out = determine_output_stream(output_file, file_out);        
        if (out == NULL)
        {
            result = ERR_IO_FAILURE;
            cout << "Unable to open output file " << input_file << endl;
            break;
        }
                        
        if ((command == "encrypt") or (command == "decrypt"))
        {
            if (new_rotor_positions != ROTORPOS_DEFAULT)
            {
                Glib::ustring new_pos((char *)new_rotor_positions.c_str());
            
                if (the_machine->move_all_rotors(new_pos))
                {
                    (*out) << "Not a valid rotor position" << endl;
                    result = ERR_UNABLE_SET_ROTOR_POS;
                }
            }
            
            if (result == RETVAL_OK)
            {            
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
            }
        }
        else            
        {
            if (command == "sigabasetup")
            {
                execute_sigabasetup_command(num_iterations, setup_step_rotor_num, out, the_machine.get());
            }
            else
            {
                if (command == "getpos")
                {
                    execute_getpos_command(out, the_machine.get());
                }
                else
                {
                    if (command == "step")
                    {
                        execute_step_command(num_iterations, out, the_machine.get());
                    }
                    else
                    {
                        // perm command
                        execute_perm_command(num_iterations, out, the_machine.get());                
                    }
                }                
            }
        }
        
        // Save rotor machine state if required        
        if ((result == RETVAL_OK) and state_progression)
        {
            if ((result = save_machine_state(state_file, the_machine.get())) != RETVAL_OK)
            {
                cout << "Unable to save state information" << endl;
            } 
        }
    
    } while(0);
    
    file_in.close();
    file_out.close();
    
    return result;
}

int rotor_sim::read_delimited_stream(istream *in, string& data_read, int delimiter)
{
    int result = RETVAL_OK;
    int char_read = 0;

    data_read.clear();
    
    try
    {
        while (in->good() and (char_read != delimiter))
        {
            // Read input character
            char_read = in->get();
            
            if ((char_read != char_traits<char>::eof()) and (char_read != delimiter))
            {
                data_read += (char)char_read;
            }
        }
        
        // Check if input stream is still ok. If the failbit is set on 
        // the stream then flag an error. If the failbit is set but the
        // stream has also reached EOF, then everything is still OK.
        if (in->fail() and !in->eof())
        {
            result = ERR_IO_FAILURE;
            cout << "IO error!" << endl;        
        }        
    }
    catch(...)
    {
        result = ERR_IO_FAILURE;
        cout << "IO error!" << endl;
    }    
    
    return result;
}

int rotor_sim::process_stream(istream *in, ostream *out, int output_grouping, sigc::slot<Glib::ustring, gunichar> proc_func, sigc::slot<bool, gunichar> symbol_is_ok)
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

void rotor_sim::print_help_message(po::options_description *desc)
{
    cout << (*desc) << endl;
    cout << "Examples:" << endl;
    cout << "    rotorsim encrypt -f machine_config.ini -i in_file.txt -o out_file.txt -g 5 -p vjna" << endl;
    cout << "    rotorsim -c decrypt -f machine_config.ini -i in_file.txt -o out_file.txt" << endl;         
    cout << "    rotorsim encrypt -f machine_config.ini" << endl;            
    cout << "    rotorsim step -f machine_config.ini -n 2" << endl;                
    cout << "    rotorsim perm -f machine_config.ini -n 3" << endl;    
    cout << "    rotorsim getpos -f machine_config.ini" << endl;        
    cout << "    rotorsim sigabasetup -f machine_config.ini -r 1 -n 4" << endl;            
    cout << endl;
}

int rotor_sim::parse(int argc, char **argv)
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

        // Check if -c was specified either directly or through a positional parameter
        if (vm.count("command") == 0) 
        {
            cout << "You have to specify a command" << endl << endl;
            print_help_message(&desc);
            
            return ERR_WRONG_COMMAND_LINE;
        } 

        // Check if the value of the command parameter is among the allowed values.
        if ((command != "decrypt") and (command != "encrypt") and 
            (command != "step")  and (command != "perm") and 
            (command != "getpos") and (command != "sigabasetup"))
        {
            cout << "Unknown command " << command << endl;
            
            return ERR_WRONG_COMMAND_LINE;
        }

        // Command line specified by user is correct. Save some additional data.
        
        if (vm.count("save-state")) 
        {
            state_progression = true;
        } 

        if (vm.count("state-progression")) 
        {          
            state_progression = true;
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
    
    return return_code;
}

/*! \brief main function of rotorsim.
 */
int main(int argc, char **argv)
{
    rotor_sim sim;   
    int ret_val = sim.parse(argc, argv);
 
    if (ret_val == RETVAL_OK)
    {
        // Parsing the command line was successful. Now do what the user requested.
        ret_val = sim.execute_command();
    }
        
    return ret_val;
}
