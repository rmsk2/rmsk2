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

/*! \file rotorvis.cpp
 *  \brief This file contains a class that implements all the user interface stuff that is provided on top of the basic KL7, SIGABA, Typex, SG39 and Nema simulator functionality.
 */

#include<boost/scoped_ptr.hpp>
#include<gtkmm.h>
#include<iostream>
#include<rotor_machine.h>
#include<sigaba.h>
#include<sg39.h>
#include<typex.h>
#include<nema.h>
#include<kl7.h>
#include<enigma_draw.h>
#include<display_dialog.h>
#include<configurator.h>
#include<configurator_dialog.h>
#include<simple_state.h>
#include<app_helpers.h>
#include<machine_config.h>

#define ROTORVIS "rotorvis"

using namespace std;

/*! \brief An application class that allows to control the KL7, SIGABA, SG39, Typex and Nema simulators implemented by the GUI classes. 
 *         It contains the code that is used to configure the simulators, save and load their state, open and close the log viewer
 *         and so on. 
 */
class rotor_visual {
public:
    /*! \brief Constructor. The parameter main_win has to contain the application's main window and the parameter has to specifiy the name of the 
     *         machine that is to be simulated.
     */    
    rotor_visual(Gtk::Window *main_win, string machine_to_visualize);
    
    /*! \brief Callback that is executed, when the "Quit" menu entry is selected.
     */    
    virtual void on_quit_activate();
    
    /*! \brief Callback that is executed, when the "Show logs ..." menu entry is selected.
     */        
    virtual void on_output_activate();
    
    /*! \brief Callback that is executed, when the log window was closed.
     */            
    virtual void on_log_invisible();
    
    /*! \brief Callback that is executed, when the "Mode: Encryption" menu entry is toggled.
     */    
    virtual void on_enc_state_activate();
    
    /*! \brief Callback that is executed, when the "Rip paper strip" menu entry is selected.
     */    
    virtual void on_reset();    

    /*! \brief In rotorvis the mode of the machine is changed to Encryption when the machine is reconfigured. This method
     *         is used to update the "Mode: Encryption" menu item if the simulator_gui object emits the rotor_draw::signal_mode_changed() signal.
     */    
    virtual void on_mode_changed(); 

    /*! \brief Callback that is executed, when the "Configure machine" menu entry is selected.
     */    
    virtual void on_configure_machine();      

    /*! \brief Callback that is executed, when the "Randomize state" menu entry is selected.
     */    
    virtual void on_randomize_machine();      

    /*! \brief Callback that is executed, when the "Save rotor set data" menu entry is selected.
     */    
    virtual void on_save_rotor_set_activate();      
    
    /*! \brief Method that can be used to set the least recently used directory. The new value has to be specified by the string referenced by the
     *         l_dir parameter.
     */
    virtual void set_last_dir(Glib::ustring& l_dir) { last_dir = l_dir; }

    /*! \brief Method that can be used to retrieve the least recently used directory. The current value is written to the string referenced by the
     *         l_dir parameter.
     */
    virtual void get_last_dir(Glib::ustring& l_dir) { l_dir = last_dir; }    
    
    /*! \brief Method that can be used to retrieve the position the main window had when the application was closed.
     */
    virtual void get_last_pos(int& x, int& y) { x = pos_x; y = pos_y; }

    /*! \brief Method that is used to save the state of the application.
     */
    virtual void save_state();

    /*! \brief Method that is used as a callback when the user presses the close button in the title bar of the main window.
     */
    virtual bool on_my_delete_event(GdkEventAny* event);    

    /*! \brief Method that is used as a callback when saving the settings of the simulator.
     */    
    virtual bool do_save(Glib::ustring& desired_file_name);

    /*! \brief Method that is used as a callback when loading the settings of the simulator.
     */        
    virtual bool do_load(Glib::ustring& desired_file_name);    

    /*! \brief Destructor.
     */            
    virtual ~rotor_visual() { delete disp; }

protected:

    /*! \brief This method can be used to construct a rotor_machine of the type specified in parameter name. Allowed values are
     *         at the moment: CSP2900, CSP889, Typex, KL7, SG39 and Nema. The parameter rotor_identifiers is filled with the
     *         rotor identifiers needed to construct a corresponding rotor_draw object.
     *
     *  The configuration of the contructed rotor_machine object is currently hardcoded.
     */            
    rotor_machine *machine_factory(string name, vector<string>& rotor_identifiers);

    /*! \brief Method that is used to construct the application's menu.
     */            
    void setup_menus();

    /*! \brief Method that sets the titles of the main window (rotor_visual::win) and the log dialog (rotor_visual::display_window)
     *         to correctly reflect the simulated machine and (optionally) the name of the settings file on which the simulated machine
     *         is based. The last file name used has to be specified in the parameter last_file_name.
     */            
    void set_titles(Glib::ustring& last_file_name);

    /*! \brief Holds the object that represents the menu action group of this application. */    
    Glib::RefPtr<Gtk::ActionGroup> menu_action;
    
    /*! \brief Holds the object that is used to construct the menu of this application from a textual representation. */        
    Glib::RefPtr<Gtk::UIManager> ui_manager;
    
    /*! \brief Holds the layout object that is used to stack the GUI elements (menu_bar and simulator_gui) of this application on top of each other. */    
    Gtk::Box *vbox1;
    
    /*! \brief Holds the menu item that can be used to switch the log window on or off. */    
    Glib::RefPtr<Gtk::ToggleAction> log_item;
    
    /*! \brief Holds the menu item that can be used to switch between encryption and decryption. Also affects the presentation of data in the log dialog. */    
    Glib::RefPtr<Gtk::ToggleAction> enc_dec_item;

    /*! \brief Holds the menu that is in use in this application. */
    Gtk::Widget *menu_bar;

    /*! \brief Holds the name of the application. */
    Glib::ustring app_name;
    
    /*! \brief Holds the least recently used directory. */    
    Glib::ustring last_dir;
    
    /*! \brief Holds the names of the rotor slots that are in use by the machine that is currently simulated. */         
    vector<string> rotor_names;       
    
    /*! \brief Points to the machine that is currently simulated. */    
    boost::shared_ptr<rotor_machine> the_machine;
    
    /*! \brief Points to the GUI simulator that is in use in this application. */    
    rotor_draw *simulator_gui;    
    
    /*! \brief Points to the main window of this application. */    
    Gtk::Window *win; 
    
    /*! \brief Points to the window unserlying the log dialog. */    
    Gtk::Window *display_window;

    /*! \brief Holds the Gtj::Builder object that can be used to gain access to GUI objects defined in the glade file. */
    Glib::RefPtr<Gtk::Builder> ref_display_xml; 
    
    /*! \brief Points to the object that manages the events of the log dialog. */    
    display_dialog *disp;
    
    /*! \brief Holds the description of the machine that is currently simulated as returned by rotor_machine::get_description(). */    
    string machine_id; 
    
    /*! \brief Holds the least recently used file. */    
    Glib::ustring last_file_name_used;  
    
    /*! \brief Holds the x-position of this object when it was closed or hidden. This position is saved and restored when 
     *         restarting the application.
     */                            
    int pos_x;
    
    /*! \brief Holds the y-position of this object when it was closed or hidden. This position is saved and restored when 
     *         restarting the application.
     */                            
    int pos_y;       
    
    /*! \brief Object that manages the events occuring when the user selects entries in the Help menu. */    
    help_menu_helper help_menu_manager;  
    
    /*! \brief Object that manages the events occuring when the user selects Load or Save settings entries from the menu. */    
    file_operations_helper file_helper;
    
    /*! \brief Object that manages the events occuring when the user selects "Process clipboard" from the menu. */    
    clipboard_helper clip_helper;
    
    /*! \brief Object that manages the events occuring when the user selects to show or hide the log window. */    
    log_helper loghelp;

    /*! \brief Helper object that is used to display simple messages. */    
    menu_helper messages;
};

void rotor_visual::set_titles(Glib::ustring& last_file_name)
{
    string new_title;

    if (last_file_name != "")
    {
        new_title = "rotorvis " + machine_id + " [" + last_file_name + "]";
    }
    else
    {
        new_title = "rotorvis " + machine_id;
    }                    
    
    win->set_title(new_title);
    display_window->set_title("rotorvis " + machine_id + " log"); 
}

void rotor_visual::on_configure_machine()
{
    rotor_machine *the_machine =  simulator_gui->get_machine();
    string name_help = rmsk::get_config_name(the_machine);
    
    configurator *conf = configurator_factory::get_configurator(name_help);
    
    if (conf != NULL)
    {
        boost::scoped_ptr<configurator> conf_ptr(conf);
        vector<key_word_info> infos;
        std::map<string, string> config_data;
        int dlg_result;
        unsigned int config_result;
        
        // Query current machine for its configuration
        conf_ptr->get_keywords(infos);
        conf_ptr->get_config(config_data, the_machine);
        
        // Use that information to initialize the configuration dialog
        configurator_dialog dlg(*win, infos, config_data);
        
        do
        {
            dlg_result = dlg.run();        
            
            if (dlg_result == Gtk::RESPONSE_OK)
            {
                // User clicked OK
                
                // Try to configure machine with the new configuration data
                if ((config_result = conf_ptr->configure_machine(config_data, the_machine)) == CONFIGURATOR_OK)
                {
                    // Success! A correct configuration has been entered
                    machine_id = the_machine->get_description();
                    set_titles(last_file_name_used);                
                    simulator_gui->set_machine(the_machine);            
                }
                else
                {
                    messages.error_message("Configuration information incorrect");
                }        
            }
            // Try again if the user entered a wrong configuration but left the dialog by clicking OK    
        } while ((dlg_result == Gtk::RESPONSE_OK) && (config_result != CONFIGURATOR_OK));            
    }
    else
    {
        messages.info_message("Not yet implemented");
    }    
}

void rotor_visual::on_randomize_machine()
{
    string dummy;    
    bool result = the_machine->randomize(dummy);
    
    if (result)
    {
        messages.error_message("Randomization failed");
    }
    else
    {
        simulator_gui->set_machine(the_machine.get());
    }    
}

void rotor_visual::save_state()
{
    win->get_position(pos_x, pos_y);
}

bool rotor_visual::on_my_delete_event(GdkEventAny* event)
{
    save_state();
    
    return false;
}

void rotor_visual::on_quit_activate()
{
    save_state();
    win->hide();
}

void rotor_visual::on_output_activate()
{
    loghelp.display_log_window(log_item->get_active());
}

void rotor_visual::on_log_invisible()
{
    loghelp.block_connections();
    log_item->set_active(false);    
}

void rotor_visual::on_enc_state_activate()
{
    loghelp.set_grouping(FORMAT_GROUP5, enc_dec_item->get_active()); 
}

void rotor_visual::on_reset()
{
    simulator_gui->get_output_device()->reset();
}

bool rotor_visual::do_save(Glib::ustring& desired_file_name)
{
    bool result;
    string file_name = desired_file_name;

    result = the_machine->save(file_name);
    
    if (!result)
    {
        set_titles(desired_file_name);        
    }
    
    return result;
}

void rotor_visual::on_save_rotor_set_activate()
{
    if (simulator_gui->get_machine()->get_name() != "SIGABA")
    {
        file_helper.on_save_rotor_set_activate(simulator_gui->get_machine(), NULL);
    }
    else
    {
        sigaba *s = dynamic_cast<sigaba *>(simulator_gui->get_machine());
        file_helper.on_save_rotor_set_activate(s, s->get_sigaba_stepper()->get_index_bank());   
    }
}

bool rotor_visual::do_load(Glib::ustring& desired_file_name)
{
    bool result;
    boost::shared_ptr<rotor_machine> help_machine;    
    string file_name = desired_file_name;
    vector<string> temp_names;        

    // Create a new rotor_machine object of the type determined by machine_id
    help_machine = boost::shared_ptr<rotor_machine>(machine_factory(machine_id, temp_names));    

    // Try to load configuration into newly created machine
    result = help_machine->load(file_name);
    
    if (!result)
    {
        the_machine = help_machine;
        rotor_names = temp_names;

        // Now set help_machine as the new one in use in the simulator's GUI. This way we do not
        // change the currently used machine when loading the settings file fails.
        // Also indirectly rips paper strip, i.e. calls reset() on the output device
        simulator_gui->set_machine(the_machine.get());        
        
        machine_id = the_machine->get_description();

        set_titles(desired_file_name);    
    }
    
    return result;
}

/*! Machine name has already been checked for validity by main function. No further validity check performed in
 *  this method.     
 */
rotor_machine *rotor_visual::machine_factory(string name, vector<string>& rotor_identifiers)
{
    rotor_machine *result = NULL;
    map<string, string> machine_conf;
    string configurator_name = name;
    
    rotor_identifiers.clear();
    
    if ((name == "CSP2900") || (name == "CSP889"))
    {
        configurator_name = MNAME_SIGABA;        
    }
    
    boost::scoped_ptr<configurator> c(configurator_factory::get_configurator(configurator_name)); 
    
    do
    {
        // Construct SIGABA machine with default settings
        if (configurator_name == MNAME_SIGABA)
        {            
            machine_conf[KW_CIPHER_ROTORS] = "0N1N2R3N4N";
            machine_conf[KW_CONTROL_ROTORS] = "5N6N7R8N9N";
            machine_conf[KW_INDEX_ROTORS] = "0N1N2R3N4N";
            machine_conf[KW_CSP_2900_FLAG] = CONF_FALSE;
            
            if (name == "CSP2900")
            {
                machine_conf[KW_CSP_2900_FLAG] = CONF_TRUE;
            }
                       
            result = c->make_machine(machine_conf);            
            
            rotor_identifiers.push_back(R_FOUR);
            rotor_identifiers.push_back(R_THREE);
            rotor_identifiers.push_back(R_TWO);
            rotor_identifiers.push_back(R_ONE);
            rotor_identifiers.push_back(R_ZERO);
            
            rotor_identifiers.push_back(STATOR_R);
            rotor_identifiers.push_back(S_MIDDLE);
            rotor_identifiers.push_back(S_FAST);
            rotor_identifiers.push_back(S_SLOW);
            rotor_identifiers.push_back(STATOR_L);            

            rotor_identifiers.push_back(I_FOUR);
            rotor_identifiers.push_back(I_THREE);
            rotor_identifiers.push_back(I_TWO);
            rotor_identifiers.push_back(I_ONE);
            rotor_identifiers.push_back(I_ZERO);                 
            
            break;
        
        } 

        // Construct Typex machine with default settings                
        if (name == MNAME_TYPEX)
        {              
            machine_conf[KW_TYPEX_ROTORS] = "aNbNcRdNeN";
            machine_conf[KW_TYPEX_RINGS] = "aaaaa";
            machine_conf[KW_TYPEX_REFLECTOR] = "arbycudheqfsglixjpknmotwvz";
            result = c->make_machine(machine_conf);                      

            rotor_identifiers.push_back(STATOR1);
            rotor_identifiers.push_back(STATOR2);
            rotor_identifiers.push_back(FAST);
            rotor_identifiers.push_back(MIDDLE);
            rotor_identifiers.push_back(SLOW);
            break;        
            
        }

        // Construct Nema machine with default settings        
        if (name == MNAME_NEMA)
        {                        
            machine_conf[KW_NEMA_ROTORS] = "abcd";
            machine_conf[KW_NEMA_RINGS] = "12 13 14 15";
            machine_conf[KW_NEMA_WAR_MACHINE] = CONF_TRUE;
            result = c->make_machine(machine_conf);                      

            nema_stepper *s = dynamic_cast<nema_stepper *>(result->get_stepping_gear());    
            s->set_all_positions("mqqpzqsoev");            
            
            rotor_identifiers.push_back(DRIVE_RED_1);
            rotor_identifiers.push_back(CONTACT_2);
            rotor_identifiers.push_back(DRIVE_3);
            rotor_identifiers.push_back(CONTACT_4);
            rotor_identifiers.push_back(DRIVE_5);            
            rotor_identifiers.push_back(CONTACT_6);
            rotor_identifiers.push_back(DRIVE_7);
            rotor_identifiers.push_back(CONTACT_8);
            rotor_identifiers.push_back(DRIVE_9);
            rotor_identifiers.push_back(CONTACT_UKW_10);            
        
            break;
        }

        // Construct KL7 machine with default settings        
        if (name == MNAME_KL7)
        {                                            
            machine_conf[KW_KL7_ROTORS] = "lfcgabhd";
            machine_conf[KW_KL7_ALPHA_POS] = "17 1 1 23 1 36 1 1";
            machine_conf[KW_KL7_NOTCH_RINGS] = "2 4 3 11 7 1 10";
            machine_conf[KW_KL7_NOTCH_POS] = "eaaag+aa";
            result = c->make_machine(machine_conf);
            kl7 *enc = dynamic_cast<kl7 *>(result);   
            
            enc->get_kl7_stepper()->move_to_letter_ring_pos(KL7_ROT_1, 9);
            enc->get_kl7_stepper()->move_to_letter_ring_pos(KL7_ROT_6, 0);            
                    
            enc->step_rotors();                        

            rotor_identifiers.push_back(KL7_ROT_8);
            rotor_identifiers.push_back(KL7_ROT_7);
            rotor_identifiers.push_back(KL7_ROT_6);
            rotor_identifiers.push_back(KL7_ROT_5);
            rotor_identifiers.push_back(KL7_ROT_4);            
            rotor_identifiers.push_back(KL7_ROT_3);
            rotor_identifiers.push_back(KL7_ROT_2);
            rotor_identifiers.push_back(KL7_ROT_1);
            
            result = enc;
            
            break;
        }

        // Construct Enigma M3 machine with default settings        
        if (name == "M3")
        {
            // Walzenlage UKW B I VI VIII
            machine_conf[KW_ENIG_ROTOR_SELECTION] = "1168";
            machine_conf[KW_ENIG_RINGSTELLUNG] = "abc";
            machine_conf[KW_ENIG_STECKERBRETT] = "14:adcnetflgijvkzpuqywx";
            machine_conf[KW_USES_UHR] = CONF_TRUE;
            machine_conf[KW_UKW_D_PERM] = "azbpcxdqetfogshvirknlmuw";
            
            result = c->make_machine(machine_conf);
            
            rotor_identifiers.push_back(FAST);
            rotor_identifiers.push_back(MIDDLE);
            rotor_identifiers.push_back(SLOW);
            
            break;
        }

        // Construct Enigma M4 machine with default settings        
        if (name == "M4")
        {   
            // Walzenlage: UKW B-Dünn Beta II IV I                                         
            machine_conf[KW_ENIG_ROTOR_SELECTION] = "11241";
            machine_conf[KW_ENIG_RINGSTELLUNG] = "aaav";
            machine_conf[KW_ENIG_STECKERBRETT] = "atbldfgjhmnwopqyrzvx";
            
            result = c->make_machine(machine_conf);
            
            rotor_identifiers.push_back(FAST);
            rotor_identifiers.push_back(MIDDLE);
            rotor_identifiers.push_back(SLOW);
            rotor_identifiers.push_back(GRIECHENWALZE);            
            
            break;
        }

        // Construct Enigma Services machine with default settings        
        if (name == "Services")
        {
            // Walzenlage UKW B I II III                                            
            machine_conf[KW_ENIG_ROTOR_SELECTION] = "1123";
            machine_conf[KW_ENIG_RINGSTELLUNG] = "abc";
            machine_conf[KW_ENIG_STECKERBRETT] = "adcnetflgijvkzpuqywx";
            machine_conf[KW_USES_UHR] = CONF_FALSE;
            machine_conf[KW_UKW_D_PERM] = "azbpcxdqetfogshvirknlmuw";
            
            result = c->make_machine(machine_conf);
            
            rotor_identifiers.push_back(FAST);
            rotor_identifiers.push_back(MIDDLE);
            rotor_identifiers.push_back(SLOW);
            
            break;
        }


        // Construct SG39 machine with default settings        
        {                    
            machine_conf[KW_SG39_ROTORS] = "3415";
            machine_conf[KW_SG39_RING_POS] = "aaaa";            
            machine_conf[KW_SG39_ENTRY_PLUGS] = "ldtrmihoncpwjkbyevsaxgfzuq";
            machine_conf[KW_SG39_REFLECTOR_PLUGS] = "awbicvdketfmgnhzjulopqrysx";
            machine_conf[KW_SG39_PINS_WHEEL_1] = "";
            machine_conf[KW_SG39_PINS_WHEEL_2] = "abcdefghijklmnopqrstuvw";            
            machine_conf[KW_SG39_PINS_WHEEL_3] = "cfilorux";  
            machine_conf[KW_SG39_PINS_ROTOR_1] = "";
            machine_conf[KW_SG39_PINS_ROTOR_2] = "";            
            machine_conf[KW_SG39_PINS_ROTOR_3] = "aeimquy";                        
            
            result = c->make_machine(machine_conf);
            
            schluesselgeraet39 *sg39 = dynamic_cast<schluesselgeraet39 *>(result);        
            
            sg39->get_sg39_stepper()->set_wheel_pos(ROTOR_2, 15);
            sg39->get_sg39_stepper()->set_wheel_pos(ROTOR_1, 7);
            sg39->get_sg39_stepper()->set_rotor_displacement(ROTOR_1, 3);
            sg39->get_sg39_stepper()->set_rotor_displacement(ROTOR_2, 16);
            sg39->get_sg39_stepper()->set_rotor_displacement(ROTOR_3, 17);
            sg39->get_sg39_stepper()->set_rotor_displacement(ROTOR_4, 5);            
            
            rotor_identifiers.push_back(ROTOR_1);
            rotor_identifiers.push_back(ROTOR_2);
            rotor_identifiers.push_back(ROTOR_3);
            rotor_identifiers.push_back(ROTOR_4);        
        }
        
    } while(0);        
    
    return result;
}

void rotor_visual::on_mode_changed()
{
    enc_dec_item->set_active(simulator_gui->get_enc_flag());
}

rotor_visual::rotor_visual(Gtk::Window *main_win, string machine_to_visualize)
    : help_menu_manager(ROTORVIS), file_helper(ROTORVIS), clip_helper(ROTORVIS), loghelp(ROTORVIS), messages(ROTORVIS)
{    
    last_file_name_used = "";
    win = main_win;
    app_name = ROTORVIS;
    
    messages.set_parent_window(win);
    
    // Construct a rotor_machine of the desrired type using default settings
    the_machine = boost::shared_ptr<rotor_machine>(machine_factory(machine_to_visualize, rotor_names));
    machine_id = machine_to_visualize;
    
    // Setup object to handle help menu events
    help_menu_manager.set_parent_window(win);
    help_menu_manager.set_text("/rotorvis/rotorvis.page", "An accurate simulator for the SIGABA, KL7, Nema, Typex and Schlüsselgerät 39 rotor machines.\nWritten in 2008-2015", "1.0", "Martin Grap");

    // Setup object to manage file handling menu events    
    file_helper.set_parent_window(win);
    file_helper.set_state_variables(&last_dir, &last_file_name_used);
    file_helper.set_callbacks(sigc::mem_fun(*this, &rotor_visual::do_load),  sigc::mem_fun(*this, &rotor_visual::do_save));
    
    win->set_resizable(false);
    
    vbox1 = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_VERTICAL));
    menu_action = Gtk::ActionGroup::create();
    ui_manager = Gtk::UIManager::create();       

    // Create simulator GUI object
    if (dynamic_cast<enigma_base *>(the_machine.get()) == NULL)
    {
        // Not an Enigma variant. Use 530 for rightmost rotor position.
        simulator_gui = Gtk::manage(new class rotor_draw(rotor_names, false, machine_to_visualize, false, 530));
    }
    else
    {
        // We have an Enigma variant. Use default for rightmost rotor position.
        simulator_gui = Gtk::manage(new class rotor_draw(rotor_names, (machine_to_visualize == "Services"), machine_to_visualize, false));
    }

    // Setup object to handle clipboard processing menu events
    clip_helper.set_parent_window(win);    
    clip_helper.set_simulator(simulator_gui);

    // Setup main window (stacking menu bar and simulator GUI object on top of each other)    
    setup_menus();
    vbox1->pack_start(*menu_bar, Gtk::PACK_SHRINK, 0);
    vbox1->pack_start(*simulator_gui);
    win->add(*vbox1);
    simulator_gui->show();
    vbox1->show_all();
    
    ref_display_xml = Gtk::Builder::create_from_string(rmsk::get_glade_data());

    // Create object that contains widgets of the log window    
    display_window = NULL;
    ref_display_xml->get_widget("display_dialog", display_window); 

    // Create object to manage log window
    disp = new display_dialog(win, display_window, ref_display_xml);    
            
    disp->signal_become_invisible().connect(sigc::mem_fun(*this, &rotor_visual::on_log_invisible));

    set_titles(last_file_name_used);

    simulator_gui->set_machine(the_machine.get());

    // Setup object to manage log menu events
    loghelp.set_parent_window(win);
    loghelp.set_simulator(disp, simulator_gui);
    
    on_mode_changed();
    simulator_gui->signal_mode_changed().connect(sigc::mem_fun(*this, &rotor_visual::on_mode_changed)); 
    on_enc_state_activate();
    win->signal_delete_event().connect(sigc::mem_fun(*this, &rotor_visual::on_my_delete_event));              
}

void rotor_visual::setup_menus()
{
    menu_action->add(Gtk::Action::create("MenuFile", "_Machine"));
    menu_action->add(Gtk::Action::create("loadsettings", "_Load settings ..."), sigc::mem_fun(file_helper, &file_operations_helper::on_file_open));
    menu_action->add(Gtk::Action::create("savesettingsas", "S_ave settings as ..."), sigc::mem_fun(file_helper, &file_operations_helper::on_file_save_as));    
    menu_action->add(Gtk::Action::create("savesettings", "Sa_ve settings ..."), sigc::mem_fun(file_helper, &file_operations_helper::on_file_save));        


    menu_action->add(Gtk::Action::create("configure", "Confi_gure machine ..."), sigc::mem_fun(*this, &rotor_visual::on_configure_machine));        
    log_item = Gtk::ToggleAction::create("showlogs", "Sh_ow logs ...");
    menu_action->add(log_item, sigc::mem_fun(*this, &rotor_visual::on_output_activate));    
    enc_dec_item = Gtk::ToggleAction::create("logstyleencrypt", "Mode: Encryption");
    menu_action->add(enc_dec_item, sigc::mem_fun(*this, &rotor_visual::on_enc_state_activate));
    menu_action->add(Gtk::Action::create("outputreset", "Rip _paper strip "), sigc::mem_fun(*this, &rotor_visual::on_reset));    
    menu_action->add(Gtk::Action::create("randomize", "Ran_domize state ..."), sigc::mem_fun(*this, &rotor_visual::on_randomize_machine));            
    menu_action->add(Gtk::Action::create("processclipboard", "Process _clipboard"), sigc::mem_fun(clip_helper, &clipboard_helper::process_clipboard));    
    
    menu_action->add(Gtk::Action::create("Quit", Gtk::Stock::QUIT), sigc::mem_fun(*this, &rotor_visual::on_quit_activate));
    
    menu_action->add(Gtk::Action::create("MenuHelp", "_Help"));
    
    menu_action->add(Gtk::Action::create("howtouse", "How to use the simulato_r ..."), sigc::mem_fun(help_menu_manager, &help_menu_helper::on_help_activate));
    menu_action->add(Gtk::Action::create("saverotorset", "Save rotor se_t data ..."), sigc::mem_fun(*this, &rotor_visual::on_save_rotor_set_activate));    
    menu_action->add(Gtk::Action::create("about", "A_bout ..."), sigc::mem_fun(help_menu_manager, &help_menu_helper::on_about_activate));


    ui_manager->insert_action_group(menu_action);
    win->add_accel_group(ui_manager->get_accel_group());    

    Glib::ustring ui_info =
        "<ui>"
        "  <menubar name='MenuBar'>"
        "    <menu action='MenuFile'>"
        "      <menuitem action='loadsettings'/>"
        "      <menuitem action='savesettingsas'/>"       
        "      <menuitem action='savesettings'/>"                 
        "      <menuitem action='configure'/>"                
        "      <menuitem action='showlogs'/>"
        "      <menuitem action='logstyleencrypt'/>"
        "      <menuitem action='randomize'/>"        
        "      <menuitem action='outputreset'/>"   
        "      <menuitem action='processclipboard'/>"                        
        "      <separator/>"                
        "      <menuitem action='Quit'/>"
        "    </menu>" 
        "    <menu action='MenuHelp'>"
        "      <menuitem action='howtouse'/>"
        "      <menuitem action='saverotorset'/>"        
        "      <menuitem action='about'/>"
        "    </menu>"                         
        "  </menubar>"
        "</ui>";

    ui_manager->add_ui_from_string(ui_info);
    menu_bar = ui_manager->get_widget("/MenuBar");    
}

/*! \brief An application class that knows how to set up and run a rotor machine simulator application for the KL7, Typex, SIGABA, SG39 and Nema.
 */
class rotorvis_simulator_app : public simulator_app {
public:
    /*! \brief Constructor. The parameter valid_names specifies the machine names that this simulator is able to simulate.
     */    
    rotorvis_simulator_app(set<string>& valid_names);

    /*! \brief Creates and initializes the rotor_viz object.
     */         
    virtual void init(int argc, char **argv);     
    
    /*! \brief Sets the least recently used directory as well as the window position of rotorvis_simulator_app::win.
     */    
    virtual void restore_state();

    /*! \brief Queries rotorvis_simulator_app::rotor_viz for the least recently used directory and the window position.
     */    
    virtual void determine_state();    

    /*! \brief Destructor. 
     */     
    virtual ~rotorvis_simulator_app() { delete rotor_viz; delete win; }
    
protected:
    /*! \brief Holds the object that manages the application GUI. */         
    rotor_visual *rotor_viz;
};

rotorvis_simulator_app::rotorvis_simulator_app(set<string>& valid_names)
    : simulator_app("rotorvis_sim.ini", valid_names, false)
{
    rotor_viz = NULL;
}
     
void rotorvis_simulator_app::init(int argc, char **argv)
{
    if (allowed_names.count(m_name) == 0)
    {
        m_name = MNAME_SG39;
    }
           
    win = new Gtk::Window();
    rotor_viz = new rotor_visual(win, m_name);
}

void rotorvis_simulator_app::restore_state()
{
    Glib::ustring l_dir;
    int pos_x, pos_y;
        
    // Retrieve and restore applicaton state  
    app_state.get_last_dir(l_dir);      
    app_state.get_last_pos(pos_x, pos_y);
    win->move(pos_x, pos_y);
    rotor_viz->set_last_dir(l_dir);
}

void rotorvis_simulator_app::determine_state()
{
    Glib::ustring l_dir;
    int pos_x, pos_y;

    // Determine and save app state
    rotor_viz->get_last_dir(l_dir);    
    rotor_viz->get_last_pos(pos_x, pos_y);
    app_state.set_last_dir(l_dir);
    app_state.set_last_pos(pos_x, pos_y);
}

int main(int argc, char *argv[])
{
    Gtk::Main m(&argc, &argv);    
    set<string> allowed_names;
    
    // Set the names of the machine variants that are known to this simulator
    allowed_names.insert("CSP889");
    allowed_names.insert("CSP2900");
    allowed_names.insert(MNAME_SG39);
    allowed_names.insert(MNAME_TYPEX);    
    allowed_names.insert(MNAME_NEMA);        
    allowed_names.insert(MNAME_KL7); 
#ifdef INCLUDE_ENIGMA
    allowed_names.insert("M3"); 
    allowed_names.insert("Services");
    allowed_names.insert("M4");
#endif
        
    rotorvis_simulator_app rotorvis_sim(allowed_names);  
      
    return rotorvis_sim.run(argc, argv, &m);
}

