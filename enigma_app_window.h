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

#ifndef __enigma_app_window_h__
#define __enigma_app_window_h__

/*! \file enigma_app_window.h
 *  \brief Header file for the application class that implements the functionality to control the Enigma simulators provided by the GUI classes.
 *         This class implements all the user interface stuff that is provided on top of the basic simulator functionality.
 */

#include<gtkmm.h>
#include<enigma_draw.h>
#include<rotor_dialog.h>
#include<plugboard_dialog.h>
#include<ukwd_wiring_dialog.h>
#include<machine_config.h>
#include<enigma_sim.h>
#include<display_dialog.h>
#include<app_helpers.h>

#define ENIGMA "Enigma"

/*! \brief An application class that allows to control the Enigma simulators implemented by the GUI classes. It contains the code
 *         that is used to configure the Enigma simulators, save and load their state, open and close the log viewer and so on. 
 *
 *  The member enigma_app_window::conf references the simulator state that is managed by instances of this class. All configuration
 *  changes are primarily made to enigma_app_window::conf and are then synchronized to the real simulator object pointed to by 
 *  enigma_app_window::enigma through the methods enigma_app_window::update_rotors() and enigma_app_window::update_stecker_brett().
 */
class enigma_app_window : public Gtk::Window {
public:

    /*! \brief Constructor.
     * 
     * \param c [in] The machine_config that is to be used to initialize this instance of enigma_app_window.
     * \param l_dir [inout] When constructing an instance of enigma_app_window this variable has to contain the directory in which 
     *                      the last load or save operation has been execxuted. If further load or save operations are excuted via
     *                      this instance the ustring which is referenced by this parameter is updated accordingly. 
     */
    enigma_app_window(machine_config& c, Glib::ustring& l_dir);
    
    /*! \brief Destructor.
     */    
    ~enigma_app_window() { delete enigma; delete rotor_dialog; delete plugs_dialog; delete display_window; delete ukw_d_wiring_dialog; delete disp; }

    /*! \brief Callback that is executed, when the "Rotor settings ..." menu entry is selected.
     */    
    virtual void on_settings_activate();

    /*! \brief Callback that is executed, when the "Plugboard ..." menu entry is selected.
     */    
    virtual void on_plugboard_activate();

    /*! \brief Callback that is executed, when the "Reset" menu entry is selected.
     */
    virtual void on_reset_activate();

    /*! \brief Callback that is executed, when the "Rip paper strip" menu entry is selected.
     */
    virtual void on_rip_schreibmax_activate();
    
    /*! \brief Callback that is executed, when the "Quit" menu entry is selected.
     */
    virtual void on_quit_activate();

    /*! \brief Callback that is executed, when the "Set rotor positions ..." menu entry is selected.
     */        
    virtual void on_set_rotor_positions_activate();
    
    /*! \brief Callback that is executed, when the "Show logs ..." menu entry is selected.
     */    
    virtual void on_output_activate();

    /*! \brief Callback that is executed, when the "UKW Dora wiring ..." menu entry is selected.
     */
    virtual void on_ukwd_activate();    

    /*! \brief Callback that is executed, when the "Save rotor set data" menu entry is selected.
     */
    virtual void on_save_rotor_set_data_activate();
        
    /*! \brief Callback that is executed, when the "Randomize" menu entry is selected.
     */        
    virtual void on_randomize_activate();
    
    /*! \brief Callback that is executed, when the log window was closed.
     */        
    virtual void on_log_invisible();
        
    /*! \brief Callback that is executed, when the "Log style: Encryption" menu entry is toggled.
     */
    virtual void on_enc_state_activate();

    /*! \brief Callback that is executed, when the GUI simulator object changes its En/Decryption mode.
     */    
    virtual void on_mode_changed();  

    /*! \brief Method that can be used to set the least recently used directory. The new value has to be specified by the string referenced by the
     *         l_dir parameter.
     */
    virtual void set_last_dir(Glib::ustring& l_dir) { last_dir = l_dir; }

    /*! \brief Method that can be used to retrieve the least recently used directory. The current value is written to the string referenced by the
     *         l_dir parameter.
     */
    virtual void get_last_dir(Glib::ustring& l_dir) { l_dir = last_dir; }

    /*! \brief This method can be used to configure the underlying simulator to use a real lampboard that can be controlled by sending appropropriate
     *         commands to the serial port named by the contents of the parameter port (See ::enigma_real_lamp_board).
     */
    virtual void use_serial_port(string port);    

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
    
protected:

    /*! \brief This method configures the simulator object to which enigma_app_window::enigma points according to the values specified
     *         by enigma_app_window::conf. It is called each time the user made changes to the rotor configuration of the simulated machine
     *         via the menu entries and dialogs provided by this class.
     *
     *  This method updates the rotor selection (including use of UKW D), the ring setting and the rotor positions of the underlying simulator. 
     */
    virtual void update_rotors();

    /*! \brief This method configures the simulator object to which enigma_app_window::enigma points according to the values specified
     *         by enigma_app_window::conf. It is called each time the user made changes to the plugboard configuration of the simulated machine
     *         via the menu entries and dialogs provided by this class.
     *
     *  This method updates the plugboard configuration (including the use of the Enigma Uhr) of the underlying simulator. 
     */
    virtual void update_stecker_brett();

    /*! \brief This method retrieves the rotor positions from the underlying simulator object to which enigma_app_window::enigma points and
     *         uses this data to update the corresponding values in enigma_app_window::conf.
     *
     *  This method is used to synchronize the rotor positions stored in the state variable enigma_app_window::conf used by this class with
     *  the actual rotor positions of the simulator object. Not calling this method before allowing the user to make changes to the configuration
     *  of the underlying machine can result in the loss of the rotor positions that were reached before making the configuration changes as  
     *  enigma_app_window::update_rotors() uses the state recorded in enigma_app_window::conf as its source. It should also be called before storing
     *  the state of a simulator object.   
     */
    virtual void sync_rotor_pos();
    
    /*! \brief Helper method that sets up the data structures which are used to represent the menus in gtkmm. Not virtual as intended to be called
     *         from constructor.
     */    
    void setup_menus();

    /*! \brief This method queries the current state of the underlying rotor machine and set the grouping value in the log dialog
     *         accordingly.
     */    
    void sync_log_grouping();
    
    /*! \brief Points to the GUI simulator that is in use in this application. */
    rotor_draw *simulator_gui;
    
    /*! \brief Holds the name of the application. */    
    Glib::ustring app_name;

    /*! \brief Holds the object that represents the menu action group of this application. */    
    Glib::RefPtr<Gio::SimpleActionGroup> menu_action;
        
    /*! \brief Holds the layout object that is used to stack the GUI elements (menu_bar and simulator_gui) of this application on top of each other. */    
    Gtk::Box *vbox1;
    
    /*! \brief Holds the menu item that can be used to switch between encryption and decryption style when logging machine output. */    
    Glib::RefPtr<Gio::SimpleAction> log_style_menuitem;
    
    /*! \brief Holds the menu item that can be used to switch the log window on or off. */        
    Glib::RefPtr<Gio::SimpleAction> show_log_menuitem;
    
    /*! \brief Holds the menu that is in use in this application. */        
    Gtk::Widget *menu_bar;
    
    /*! \brief Holds the names of the rotor slots that are in use by the Enigma variant that is currently simulated. */    
    vector<string> rotor_names;
    
    /*! \brief Holds the configuration (and the full state) of the Enigma variant that is currently simulated. */    
    machine_config& conf;    
    
    /*! \brief Holds the least recently used directory. */    
    Glib::ustring last_dir;
    
    /*! \brief Holds the Gtj::Builder object that can be used to gain access to GUI objects defined in the glade file. */    
    Glib::RefPtr<Gtk::Builder> ref_xml;
    
    /*! \brief Holds the least recently used file. */    
    Glib::ustring last_file_opened;
    
    /*! \brief Points the rotor_dialog_processor object that is in use in this application. */    
    Gtk::Dialog *rotor_dialog;
    
    /*! \brief Points the plugboard_dialog object that is in use in this application. */        
    Gtk::Dialog *plugs_dialog;
    
    /*! \brief Points the ukwd_wiring_dialog object that is in use in this application. */
    Gtk::Dialog *ukw_d_wiring_dialog;
    
    /*! \brief Points the Gtk::Window object underlying managed by the enigma_app_window::disp member. */
    Gtk::Window *display_window;
    
    /*! \brief Points to the object that manages the events of the log dialog. */    
    display_dialog *disp;
    
    /*! \brief Points to the object that actually holds the simulated Enigma machine. */        
    enigma_base *enigma;  
    
    /*! \brief Defines the columns used in the combo boxes that allow rotor selection in the rotor dialog. */            
    simple_text_cols model_cols;
    
    /*! \brief Defines the columns used in the list view that displays the already selected plugs in the plugboard dialog. */                
    plug_cols plugboard_cols;
    
    /*! \brief Defines the columns used in the list view that displays the already selected plugs in the UKW D wiring dialog. */                    
    plug_cols plugboard_cols_ukw;    
    
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
    
    /*! \brief Helper object that is used to manage the events that occur when the user select the "Set rotor positions" 
     *         entry from the menu. 
     */    
    rotor_position_helper pos_helper;
    
    /*! \brief Helper object that is used to manage the events that occur when the user requests to randomize machine settings. 
     */    
    randomizer_param_helper rand_helper;
          
};
#endif /* __enigma_app_window_h__ */

