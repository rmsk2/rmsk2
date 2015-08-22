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

/*! \file enigma_app.cpp
 *  \brief This file contains the main function for the Enigma simulators.
 */

#include<gtkmm/main.h>

#include<machine_config.h>
#include<enigma_app_window.h>
#include<simple_state.h>

/*! \brief An application class that knows how to set up and run an Enigma simulator application. 
 */
class enigma_simulator_app : public simulator_app {
public:
    /*! \brief Constructor. The parameter valid_names specifies the names of the Enigma variants this simulator is able to simulate.
     */    
    enigma_simulator_app(set<string>& valid_names);

    /*! \brief Creates and initializes the application main window.
     */    
    virtual void init(int argc, char **argv);

    /*! \brief Sets the least recently used directory as well as the window position of enigma_simulator_app::window1.
     */    
    virtual void restore_state();

    /*! \brief Queries enigma_simulator_app::window1 for the least recently used directory and its window position.
     */    
    virtual void determine_state();

    /*! \brief Destructor. 
     */    
    virtual ~enigma_simulator_app() { delete window1; }

protected:
    /*! \brief Holds the main window of the enigma simulator that has been derived from Gtk::Window. */
    enigma_app_window *window1;
    
    /*! \brief Holds the Enigma configuration that is used by this simulator.  */    
    machine_config my_conf;
};

enigma_simulator_app::enigma_simulator_app(set<string>& valid_names)
    : simulator_app("enigma_sim.ini", valid_names)
{
    window1 = NULL;
}

void enigma_simulator_app::restore_state()
{
    Glib::ustring l_dir;
    int pos_x, pos_y;
        
    // Retrieve and restore applicaton state        
    app_state.get_last_dir(l_dir);
    app_state.get_last_pos(pos_x, pos_y);
    window1->set_last_dir(l_dir);        
    window1->move(pos_x, pos_y);
}

void enigma_simulator_app::determine_state()
{
    Glib::ustring l_dir;
    int pos_x, pos_y;

    // Determine and save app state
    window1->get_last_dir(l_dir);
    window1->get_last_pos(pos_x, pos_y);
    app_state.set_last_dir(l_dir);
    app_state.set_last_pos(pos_x, pos_y);               
}

void enigma_simulator_app::init(int argc, char **argv)
{
    Glib::ustring l_dir = Glib::get_user_data_dir();
    string serial_port;
    my_conf.make_config(m_name);

    // Setup graphical simulator object
    window1 = new class enigma_app_window(my_conf, l_dir);

    if (argc > 2)
    {
        // The second parameter is optional and can be used to specify a serial port on which
        // a physical lampboard can be addressed. When a serial port is specified it has to
        // designate the device file i.e. something like /dev/ttyS0 or /dev/ttyUSB0, of the serial
        // port. See also enigma_real_lamp_board class.   
        serial_port = string(argv[2]);

        if (serial_port != "")
        {
            // A physical lampboard is to be used.
            window1->use_serial_port(serial_port);
        }
    }
    
    win = window1;
}

/*! \brief This is the main function for the Enigma simulators.
 */  
int main(int argc, char **argv)
{      
    Gtk::Main m(&argc, &argv);    
    set<string> allowed_names;
    
    // Set the names of the Enigma variants that are known to this simulator
    allowed_names.insert("M3");
    allowed_names.insert("M4");
    allowed_names.insert("M4 Schreibmax");    
    allowed_names.insert("Services");
    allowed_names.insert("KD");    
    allowed_names.insert("Railway");        
    allowed_names.insert("Abwehr");        
    allowed_names.insert("Tirpitz");
    
    enigma_simulator_app enigma_sim(allowed_names);    

    return enigma_sim.run(argc, argv, &m);
}

