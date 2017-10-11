/***************************************************************************
 * Copyright 2017 Martin Grap
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

/*! \file rotor_state.cpp
 *
 *  \brief Implements a generic command line program that allows to create state files for the rotor machines 
  *        simulated by rmsk2.
 */     

#include<boost/scoped_ptr.hpp>
#include<cmdline_base.h>
#include<configurator.h>


/*! \brief A class which implements a command line tool that allows to generate and save rotor machine states.
 */
class rotor_state : public cmdline_base {
public:
    /*! \brief Constructor.
     */
    rotor_state();
    
    /*! \brief This method parses the command line and processes the input data according to the purpose of this program.
     */
    virtual int parse(int argc, char **argv); 

    /*! \brief This method executes the operations specified by the user.
     */    
    virtual int execute_command();

    /*! \brief Destructor.
     */        
    virtual ~rotor_state() { ; }
    
protected:

    /*! \brief This method prints a message that describes how to use rotorsim.
     */
    virtual void print_help_message(po::options_description *desc, string appendix = "");

    /*! \brief This method modifies the parameter description stored in this.desc to
     *         include the machine specific configuration options. The parameter type
     *         has to specify the machine type.
     */    
    virtual void add_machine_specific_options(string& type);

    /*! \brief This method checks whether the command line parameters given on the command
     *         line for the machine specific parameters are valid or not. In case of an error
     *         the string referenced by parameter err_message is set to an error message.
     *
     *  Returns RETVAL_OK if the parameters are valid.
     */    
    virtual int check_machine_specific_options(string& err_message);

    /*! \brief This method returns a message that describes the randomizer parameters which can be used with
     *         the machines of the type specified by the parameter machine_name. The generated message is returned
     *         through the parameter randomizer_help.
     */
    virtual void generate_randomizer_help(string& machine_name, string& randomizer_help);
    
    virtual int randomize_machine(rotor_machine *machine);
    
    virtual int configure_machine(rotor_machine *machine, configurator *conf);

    /*! \brief Holds the configuration information as specified on the command line. */
    map<string, string> config_map;
    
    /*! \brief Holds the allowed machine names. */    
    set<string> allowed_machine_names;

    /*! \brief Stores the rotor position as specified on the command line. */
    string rotor_positions;
    
    /*! \brief Holds the machine type as specified on the command line. */    
    string machine_type;
    
    /*! \brief Holds the input file name as specified on the command line. */    
    string input_file;
    
    /*! \brief Holds the output file name as specified on the command line. */    
    string output_file;

    /*! \brief Holds the value of the --randparm parameter. */    
    string rand_parameter;
    
    /*! \brief Holds the value of the --rand-set parameter. */        
    string rand_set_name;
    
    /*! \brief Contains a flag that is true if the corresponding command line option is boolean valued. */
    map<string, bool> bool_config_map;     
};

rotor_state::rotor_state()
    : cmdline_base("Allowed options") 
{
    desc.add_options()
        ("help,h", "Produce help message")    
        ("random", "Generate random machine state. Optional.")
        ("randparm", po::value<string>(&rand_parameter), "Generate random machine state using the string supplied as the value of this option as a parameter. Optional.")
        ("positions,p", po::value<string>(&rotor_positions)->default_value(ROTORPOS_DEFAULT), "Desired positions of settable rotors.")
        ("input-file,i", po::value<string>(&input_file), "Read input data from this file and pipe it to stdout. Optional. stdin used if missing and --pipe specified.")
        ("stdout", "Force generated state to also be written to stdout. Has no effect if no output file was specified.")
        ("pipe", "Pipe input data from stdin to stdout. Has no effect if an input file was specified.")
        ("output-file,o", po::value<string>(&output_file), "Save generated state in this output file. Optional. stdout used if missing.") 
        ("rand-set", po::value<string>(&rand_set_name), "Create a random rotor set, use it to generate a machine state and store the rotor set in a file named as specified by parameter. Optional.") 
        ("load-set", po::value<string>(&rand_set_name), "Load a custom random rotor set from a file named as specified by parameter and use it to generate a machine state. Optional."); 
        
    allowed_machine_names.insert("M4");
    allowed_machine_names.insert("M3");
    allowed_machine_names.insert("Services");
    allowed_machine_names.insert("Tirpitz");
    allowed_machine_names.insert("Railway");
    allowed_machine_names.insert("Abwehr");
    allowed_machine_names.insert("KD");
    allowed_machine_names.insert("SIGABA");
    allowed_machine_names.insert("KL7");
    allowed_machine_names.insert("Typex");
    allowed_machine_names.insert("Nema");
    allowed_machine_names.insert("SG39");
}

void rotor_state::print_help_message(po::options_description *desc, string appendix)
{
    string allowed_names;
    set<string>::iterator iter;
    
    for (iter = allowed_machine_names.begin(); iter != allowed_machine_names.end(); ++iter)
    {
        allowed_names += *iter + " ";
    }    
    
    cout << "First parameter has to be machine type. Valid values are:" << endl << endl;
    cout << allowed_names << endl << endl;
    cout << (*desc) << endl;

    if (appendix != "")
    {
        cout << appendix <<endl;
        cout << endl;
    }

    cout << "Examples:" << endl << endl;
    cout << "echo vonvonjl | rotorstate M4 --pipe --rotors 11241 --rings aaav --plugs atbldfgjhmnwopqyrzvx | rotorsim encrypt -g 4 -p vjna" << endl;                        
    cout << "rotorstate M4 -o m4_verification_test.ini --rotors 11241 --rings aaav --plugs atbldfgjhmnwopqyrzvx -p fgtu" << endl;
    cout << "rotorstate Services -o services_verification_test.ini --randparm uhr" << endl;
    cout << "rotorstate M4 -i test_data.txt --rotors 11241 --rings aaav --plugs atbldfgjhmnwopqyrzvx | rotorsim encrypt -g 4 -p vjna" << endl;
    cout << "echo thisisatest | rotorstate KL7 --random -o egal.ini --pipe --stdout | rotorsim encrypt -g 5" << endl;
    cout << "rotorstate KL7 -h" << endl;    
    cout << endl;
}

void rotor_state::add_machine_specific_options(string& type)
{
    vector<key_word_info> configurator_keywords;
    vector<key_word_info>::iterator iter_kw;
    boost::scoped_ptr<configurator> c(configurator_factory::get_configurator(type));                
    c->get_keywords(configurator_keywords);        
    config_map.clear();
    bool_config_map.clear();  
    string desc_help;      
    
    for (iter_kw = configurator_keywords.begin(); iter_kw != configurator_keywords.end(); ++iter_kw)
    {
        desc_help = iter_kw->descriptive_text;
        bool_config_map[iter_kw->keyword] = (iter_kw->type == KEY_BOOL);
        
        if (iter_kw->type == KEY_BOOL)
        {
            desc_help += " (true/false)";
        }
                        
        config_map[iter_kw->keyword] = "";
        desc.add_options()((iter_kw->keyword).c_str(), po::value<string>(&config_map[iter_kw->keyword]), (desc_help).c_str());
    }
}

int rotor_state::check_machine_specific_options(string& err_message)
{
    int return_code = RETVAL_OK;
    map<string, string>::iterator iter_conf_map;
    
    // Verify dynamic per machine options
    for (iter_conf_map = config_map.begin(); (iter_conf_map != config_map.end()) && (return_code == RETVAL_OK); ++iter_conf_map)
    {
        // Check that true or false is used as a value for a boolean option
        if (bool_config_map[iter_conf_map->first])
        {
            if ((iter_conf_map->second != CONF_TRUE) && (iter_conf_map->second != CONF_FALSE))
            {
                err_message = "Value given for option " + iter_conf_map->first + " has to be either true or false";
                return_code = ERR_WRONG_COMMAND_LINE;
            }
        }
    }      
    
    return return_code;
}

void rotor_state::generate_randomizer_help(string& machine_name, string& randomizer_help)
{
    boost::scoped_ptr<rotor_machine> machine(rmsk::make_default_machine(machine_name));
    vector<struct randomizer_descriptor> descriptors;
    vector<struct randomizer_descriptor>::iterator iter;
    
    randomizer_help.clear();
    
    if (machine.get() != NULL)
    {                
        descriptors = machine->get_randomizer_descriptors();
        
        for (iter = descriptors.begin(); iter != descriptors.end(); ++iter)
        {
            randomizer_help += (iter->keyword + ": " + iter->description + "\n");
        }
        
        if (randomizer_help != "")
        {
            randomizer_help = "Allowed randomizer parameters: \n" + randomizer_help;
        }
    }
}

int rotor_state::configure_machine(rotor_machine *machine, configurator *conf)
{
    int result = RETVAL_OK;
    
    do
    {
        // Use custom rotor set
        if (((vm.count("rand-set") != 0) || (vm.count("load-set") != 0)))
        {
            string rotor_set_name;
            bool rotor_set_known = conf->determine_rotor_set_name(config_map, rotor_set_name);
            
            if (!rotor_set_known)
            {
                result = ERR_ROTOR_MACHINE;
                cout << "Unable to determine rotor set name to use" << endl;
                break;
            }
            
            // Randomize set
            if (vm.count("rand-set") != 0)
            {                
                machine->get_rotor_set(rotor_set_name)->replace_permutations();

                if (machine->get_rotor_set(rotor_set_name)->save(rand_set_name))
                {
                    result = ERR_ROTOR_MACHINE;
                    cout << "Unable to save randomized rotor set data" << endl;
                    break;
                }
            }
            else // It is now guaranteed that vm.count("load-set") != 0. Load custom rotor set.
            {
                if (machine->get_rotor_set(rotor_set_name)->load(rand_set_name))
                {
                    result = ERR_ROTOR_MACHINE;
                    cout << "Unable to load randomized rotor set data" << endl;
                    break;
                }
            }
        }
                
        // Change state according to command line parameters
        if (conf->configure_machine(config_map, machine) != CONFIGURATOR_OK)
        {
            result = ERR_ROTOR_MACHINE;
            cout << "Unable to configure machine with command line parameters. Most probably a syntax error!" << endl;
            break;
        }
            
    } while(0);
    
    return result;

}

int rotor_state::randomize_machine(rotor_machine *machine)
{
    int result = RETVAL_OK;
    string randomize_parm = "dummy";
    
    do
    {        
        // If user supplied an argument for the random generator use it
        if (vm.count("randparm") != 0)
        {
            randomize_parm = rand_parameter;
        }                   
                    
        // Use custom rotor set
        if ((vm.count("rand-set") != 0) || (vm.count("load-set") != 0))
        {
            string rotor_set_name = machine->map_rand_parm_to_set_name(randomize_parm);
            
            // randomize rotor set
            if (vm.count("rand-set") != 0)
            {
                machine->get_rotor_set(rotor_set_name)->replace_permutations();
                if (machine->get_rotor_set(rotor_set_name)->save(rand_set_name))
                {
                    result = ERR_ROTOR_MACHINE;
                    cout << "Unable to save randomized rotor set data" << endl;
                    break;
                }
            }
            else // It is now guaranteed that vm.count("load-set") != 0. Load custom rotor set. 
            {
                if (machine->get_rotor_set(rotor_set_name)->load(rand_set_name))
                {
                    result = ERR_ROTOR_MACHINE;
                    cout << "Unable to load randomized rotor set data" << endl;
                    break;
                }
            }
        }
        
        // Randomize state
        if (machine->randomize(randomize_parm))
        {
            result = ERR_ROTOR_MACHINE;
            cout << "Unable to randomize rotor machine" << endl;
            break;
        }
        
    } while(0);
    
    return result;
}

int rotor_state::parse(int argc, char **argv)
{
    int return_code = RETVAL_OK;
    string err_message;
    
    try
    {
        do
        {
            if (argc < 2)
            {
                print_help_message(&desc);                
                cout << "Use rotorstate <machine_type> -h to also get additional information about machine specific options." << endl << endl;
                return_code = ERR_WRONG_COMMAND_LINE;
                break;
            }
        
            machine_type = string(argv[1]);
            
            // Check if first parameter is a valid machine name
            if (allowed_machine_names.count(machine_type) == 0)
            {
                // Is it at least -h or --help?
                if ((machine_type != "-h") && (machine_type != "--help"))
                {
                    cout << "Unknown machine type " << machine_type << endl << endl;
                }
                
                print_help_message(&desc);                           
                cout << "Use rotorstate <machine_type> -h to also get additional information about machine specific options." << endl << endl;
                     
                return_code = ERR_WRONG_COMMAND_LINE;
                break;
            }
            
            // Append machine specific options dynamically to parameter description
            add_machine_specific_options(machine_type);
            
            // parse command line    
            po::store(po::command_line_parser(argc - 1, argv + 1).options(desc).positional(p).run(), vm);
            po::notify(vm);    

            // Check if -h was specified and if yes show help message
            if (vm.count("help")) 
            {
                string rand_help;
                
                generate_randomizer_help(machine_type, rand_help);                
                print_help_message(&desc, rand_help);                
                return_code = ERR_WRONG_COMMAND_LINE;
                break;
            }
            
            // Only check dynamic parameters if a randomized state has not been requested
            if ((vm.count("random") == 0) && (vm.count("randparm") == 0))
            {
                if ((return_code = check_machine_specific_options(err_message)) != RETVAL_OK)
                {
                    cout << err_message << endl;
                }
            }
            
            // Either load a random rotor set, generate a new one or use the default
            if ((vm.count("rand-set") != 0) && (vm.count("load-set") != 0))
            {
                cout << "The parameters --rand-set and --load-set can not be used together." << endl;
                return_code = ERR_WRONG_COMMAND_LINE;
                break;            
            }
            
        } while(0); 
    }
    catch (exception& e)
    {
        cout << e.what() << endl;
        return_code = ERR_WRONG_COMMAND_LINE;
    }
    
    return return_code;
}

int rotor_state::execute_command()
{
    int result = RETVAL_OK;
    istream *in = NULL;
    ifstream file_in;
    string data_in;
    const int DONT_CARE = 0x4747;
    ustring new_positions = rotor_positions;
    
    do
    {        
        // Open input stream        
        in = determine_input_stream(input_file, file_in);        
        if (in == NULL)
        {
            result = ERR_IO_FAILURE;
            cout << "Unable to open input file " << input_file << endl;
            break;
        }
        
        boost::scoped_ptr<rotor_machine> machine(rmsk::make_default_machine(machine_type));
        boost::scoped_ptr<configurator> conf(configurator_factory::get_configurator(machine_type));
        
        // Read input data if an input file or --pipe has been specified
        if (vm.count("pipe") || (input_file != ""))
        {
            result = read_delimited_stream(in, data_in, DONT_CARE);
            if (result != RETVAL_OK)
            {
                cout << "Unable to read data from stdin" << endl;
                break;
            }
        }
        
        // Configure state of machine
        if ((vm.count("random") != 0) || (vm.count("randparm") != 0))
        {
            result = randomize_machine(machine.get());
            
            if (result != RETVAL_OK)
            {
                break;
            } 
        }
        else
        {
            result = configure_machine(machine.get(), conf.get());
            
            if (result != RETVAL_OK)
            {
                break;
            }
        }
        
        // Modify rotor positions if requested by user
        if (rotor_positions != ROTORPOS_DEFAULT)
        {
            if (machine->move_all_rotors(new_positions))
            {
                result = ERR_UNABLE_SET_ROTOR_POS;
                cout << "Unable to set rotor positions" << endl;
                break;
            }
        }
        
        // Save machine state reached by now
        result = save_machine_state(output_file, machine.get(), false);
        if (result != RETVAL_OK)
        {
            cout << "Unable to save generated state" << endl;
            break;
        }
        
        // If user explicitly requests it and if an output file has been specified *also* write state to stdout
        if (vm.count("stdout") && (output_file != ""))
        {
            string temp_empty;
            result = save_machine_state(temp_empty, machine.get(), false);
            if (result != RETVAL_OK)
            {
                cout << "Unable to save generated state" << endl;
                break;
            }
        }
        
        // Write input data to stdout if user has given us input data
        if (vm.count("pipe") || (input_file != ""))
        {        
            cout << data_in;    
        }
                            
    } while(0);
    
    file_in.close();
    
    return result;
}

/*! \brief main function of rotor_state.
 */
int main(int argc, char **argv)
{
    rotor_state sim;   
    int ret_val = sim.parse(argc, argv);
 
    if (ret_val == RETVAL_OK)
    {
        // Parsing the command line was successful. Now do what the user requested.
        ret_val = sim.execute_command();
    }
        
    return ret_val;
}
