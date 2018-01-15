/***************************************************************************
 * Copyright 2018 Martin Grap
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

#ifndef __app_helpers_h__
#define __app_helpers_h__

/*! \file app_helpers.h
 *  \brief This file contains the header for a set of classes that know how to handle GUI events that occur in rotorvis
 *         and Enigma.
 */

#include<gtkmm.h>
#include<rotor_draw.h>
#include<display_dialog.h>


/*! \brief An application class that serves as the base class for a variety of other helper classes that are intended
 *         to factor out the menu event handling code in rotorvis and enigma into a common set of code.
 */    
class menu_helper {
public:
    /*! \brief Constructor. The parameter name_of_app has to contain the name of the app and is used to set the title
     *         of any dialog window that is presented to the user while processing the event.
     */    
    menu_helper(const char *name_of_app) { app_name = Glib::ustring(name_of_app); }    

    /*! \brief This method allows to tell this menu_helper instance about the main window of the application. This is
     *         needed when displaying a dialog.
     *
     *  Important: This method has to be called at least once for each menu_helper instance or any instance of a child
     *  of menu_helper. Probably this functionality should be part of the constructor.
     */    
    virtual void set_parent_window(Gtk::Window *w) {win = w; }

    /*! \brief Helper method that can be used to show an information dialog to the user displaying the message given
     *         in parameter message.
     */    
    virtual void info_message(const char *message) { Glib::ustring m(message); message_dialog(m, Gtk::MESSAGE_INFO); }

    /*! \brief Helper method that can be used to show an error dialog to the user displaying the message given
     *         in parameter message.
     */    
    virtual void error_message(const char *message) { Glib::ustring m(message); message_dialog(m, Gtk::MESSAGE_ERROR); }   

    /*! \brief Helper method that can be used to show an information dialog to the user displaying the message given
     *         in parameter message.
     */
    virtual void info_message(Glib::ustring& message) { message_dialog(message, Gtk::MESSAGE_INFO); }
    
    /*! \brief Helper method that can be used to show an information dialog to the user displaying the message given
     *         in parameter message.
     */        
    virtual void error_message(Glib::ustring& message) { message_dialog(message, Gtk::MESSAGE_ERROR); }   

    /*! \brief Destructor.
     */        
    virtual ~menu_helper() { ; }
    
protected:

    /*! \brief Method that displays the message contained in parameter message to the user. The type of dialog can be determined
     *         by the parameter type.
     */        
    virtual void message_dialog(Glib::ustring& message, Gtk::MessageType type);

    /*! \brief Holds the name of the appication. */        
    Glib::ustring app_name;
    
    /*! \brief Holds a pointer to the main window of the application. Has to be set by a call to set_parent_window(). */            
    Gtk::Window *win;    
};

/*! \brief An application class that allows to handle the events that can occur when the user clicks on the Help menu in
 *         Enigma or rotorvis.
 */    
class help_menu_helper : public menu_helper {
public:
    /*! \brief Constructor. The parameter name_of_app has to contain the name of the app and is used to set the title
     *         of any dialog window that is presented to the user while processing the event.
     */    
    help_menu_helper(const char *name_of_app) : menu_helper(name_of_app) { ; }

    /*! \brief This method shows the about dialog that has been defined for this application. The information displayed in
     *         this dialog can be set by the set_text() method.
     */        
    virtual void on_about_activate();

    /*! \brief This method opens the help file that has been defined for this application. The path of the help file to open
     *         can be set by the set_text() method.
     */        
    virtual void on_help_activate();

    /*! \brief This method allows to set the parameters that are used to customize the information shown to the user depending
     *         on the application.
     *
     *  \param [in] help_file Has to contain the path to the help file to open. In order to open the help file with the default
     *                        help file viewer the value of help_file is appended to the value returned by rmsk::get_doc_path().
     *  \param [in] comment Has to contain a text that described the application in broad terms.
     *  \param [in] version Has to contain a text that describes the application version.
     *  \param [in] author Has to contain a text that specified information about the author of this application.     
     */            
    virtual void set_text(const char *help_file, const char *comment, const char *version, const char *author);

    /*! \brief Destructor.
     */            
    virtual ~help_menu_helper() { ; }
    
protected:
    /*! \brief Holds the relative path (with respect to the value returned by rmsk::get_doc_path()) of the help file to open. */        
    Glib::ustring help_file_name;

    /*! \brief Holds a short text that describes the purpose of this application. */            
    Glib::ustring about_comment;    
    
    /*! \brief Holds a string that describes the version of this application. */                
    Glib::ustring about_version;
    
    /*! \brief Holds a short text containing information about the author of this application. */                
    Glib::ustring about_author;        
};

/*! \brief An application class that allows to handle the events that can occur when the user clicks on the "Load settings ..." 
 *         "Save settings ..." or "Save settings as ..." menu items in Enigma or rotorvis.
 *
 *  This class makes use of two so called state variables. These point to a string that contains the last file that has been opened 
 *  or saved (last_file_opened) and the path of the directory in which this file lives (last_dir). This information is part of the state
 *  of the application. On top of that two callbacks are needed by objects of this class (load_settings and save_settings) which do
 *  the actual loading or saving of settings in the context of the applicaton in question.
 */    
class file_operations_helper : public menu_helper {
public:

    /*! \brief Constructor. The parameter name_of_app has to contain the name of the app and is used to set the title
     *         of any dialog window that is presented to the user while processing the event.
     */
    file_operations_helper(const char *name_of_app) : menu_helper(name_of_app) { ; }

    /*! \brief This method allows to handle the event when the user clicked on the "Load settings ..." menu item. Through
     *         the parameter load_data you can specify a callback that actually handles the loading of data.
     */            
    virtual void on_file_open_with_callback(sigc::slot<bool, Glib::ustring&> load_data);

    /*! \brief This method allows to handle the event when the user clicked on the "Load settings ..." menu item.
     */                
    virtual void on_file_open() { on_file_open_with_callback(load_settings); }

    /*! \brief This method allows to handle the event when the user clicked on the "Save settings ..." menu item. 
     *
     *  When the string to which last_file_opened points is empty then on_file_save() and on_file_save_as() behave in the
     *  same way. Calls on_do_save() to do most of the work.
     */            
    virtual void on_file_save();    

    /*! \brief This method allows to save the default rotor set data which is in use in the rotor machines specified by the parameters
     *         the_machine and index_machine. The parameter index_machine may has to be NULL if there is no index machine.
     */        
    virtual void on_save_rotor_set_activate(rotor_machine *the_machine, rotor_machine *index_machine);

    /*! \brief This method allows to load a rotor set saved in a file into the machine to which the_machine points.
     */            
    virtual void on_load_rotor_set_activate(rotor_machine *the_machine);

    /*! \brief This method allows to handle the event when the user clicked on the "Save settings as ..." menu item.
     *
     *  Calls on_do_save() to do most of the work.
     */            
    virtual void on_file_save_as(); 

    /*! \brief This method allows to set the state variables that point the path of last file loaded or saved (l_file) and the path of the
     *         directory where this file lives (l_dir).
     */            
    virtual void set_state_variables(Glib::ustring *l_dir, Glib::ustring *l_file) { last_dir = l_dir; last_file_opened = l_file; }

    /*! \brief This method allows to set callbacks to which the work of actually loading or saving the settigs is delegated.
     */            
    virtual void set_callbacks(sigc::slot<bool, Glib::ustring&> load, sigc::slot<bool, Glib::ustring&> save) { load_settings = load; save_settings = save; }   

    /*! \brief Destructor.
     */    
    virtual ~file_operations_helper() { ; }
    
protected:

    /*! \brief This class is a helper for loading a rotor set.
     */                    
     class rotor_set_loader {
     public:
        /*! \brief Constructor. The parameter machine has to point to the machine into which the rotor set will be loaded.
         */         
        rotor_set_loader(rotor_machine* machine) { the_machine = machine; }

        /*! \brief Implements the actual loading.
         */                 
        bool load_set(Glib::ustring& file_name);
     
     protected:
        /*! \brief Holds the rotor machine into which a rotor set will be loaded.
         */                 
        rotor_machine *the_machine;
     };


    /*! \brief This method does the actual saving of the settings. If the parameter desired_file_name contains a non empty string then
     *         the file selection dialog is not shown ("Save settings ..."). When it is empty the user is prompted to select a file name
     *         ("Save settings as ...").
     */                
    virtual void on_do_save(const Glib::ustring& desired_file_name);

    /*! \brief This method allows to save all rotor sets that are currently known to the machine to which the parameter the_machine points. 
     *         The parameter base_name has to contain a prefix to which the name of each rotor set is appended in order to generate the file
     *         name used to save this set.
     */                
    virtual bool save_all_sets(rotor_machine *the_machine, Glib::ustring& base_name);

    /*! \brief Points to a string that holds the path of directory in which *last_file_opened lives. */        
    Glib::ustring *last_dir;
    
    /*! \brief Points to a string that holds the path of the last file that has been loaded or saved. */    
    Glib::ustring *last_file_opened;    
    
    /*! \brief Holds the callback to which loading settings is delegated. */        
    sigc::slot<bool, Glib::ustring&> load_settings;
    
    /*! \brief Holds the callback to which saving settings is delegated. */            
    sigc::slot<bool, Glib::ustring&> save_settings;    
};

/*! \brief An application class that allows to handle the event that occurs when the user clicks on the "Process clipboard" menu item
 *         in Enigma or rotorvis.
 */    
class clipboard_helper : public menu_helper {
public:
    /*! \brief Constructor. The parameter name_of_app has to contain the name of the app and is used to set the title
     *         of any dialog window that is presented to the user while processing the event.
     */
    clipboard_helper(const char *name_of_app) : menu_helper(name_of_app) { ; }

    /*! \brief This method allows to handle the event when the user clicked on the "Process clipboard" menu item.
     */                
    virtual void process_clipboard();

    /*! \brief As objects of this class have to perform encryptions and decryptions they have to know the simulator object that
     *         is currently in use in this applicaton. This method can be used to tell a clipboard_helper instance about the
     *         simulator to use (obviously contained in parameter simulator).
     */                
    virtual void set_simulator(rotor_draw *simulator) { simulator_gui = simulator; }

    /*! \brief Destructor.
     */    
    virtual ~clipboard_helper() { ; }
    
protected:

    /*! \brief This method actually performs the en/decyptions.
     *
     *  It is not called directly by process_clipboard(). Instead process_clipboard() initializes a timer and uses this method
     *  to handle the event when the timer fires. This prevents that the simulator GUI is blocked while performing the en/decryptions. 
     */    
    virtual bool timer_func();        

    /*! \brief Holds the connection used for the timer callback. */    
    sigc::connection timer_conn;
    
    /*! \brief Points to the simulator object in use in this application. */    
    rotor_draw *simulator_gui;    
};

/*! \brief An application class that allows to handle the events that occur when the user opens or closes the log window in Enigma or
 *         rotorvis.
 *
 *  In order to fulfill its intended tasks an object of this class has to know the log dialog (member log_helper::disp) that it
 *  manages and the simulator object that is actually in use in the application (member log_helper::simulator_gui).
 */    
class log_helper : public menu_helper {
public:
    /*! \brief Constructor. The parameter name_of_app has to contain the name of the app and is used to set the title
     *         of any dialog window that is presented to the user while processing the event.
     */
    log_helper(const char *name_of_app) : menu_helper(name_of_app) { ; }

    /*! \brief This method allows to set the display_dialog and rotor_simulator instances that this log_helper instance is intended
     *         to manage.
     */                
    virtual void set_simulator(display_dialog *d, rotor_draw *simulator);

    /*! \brief This method shows (is_visible = true) or hides (is_visible = false) the log dialog to which log_helper::disp points.
     */                
    virtual void display_log_window(bool is_visible);

    /*! \brief This method makes sure that log_helper::disp is blocked from receiving information about input or output characters
     *         generated by log_helper::simulator_gui. It is intended to be called after log_helper::disp has been hidden.
     */                
    virtual void block_connections();

    /*! \brief This method allows to change the grouping settings (4 or 5 letter groups or no grouping at all) of the log dialog to
     *         which log_helper::disp points. This method is intended to be called when the user changes the en/decryption mode of the
     *         simulator.
     *
     *  The parameter group_type determines whether 4 or 5 letter groups are produced and the parameter enc_state determines in which
     *  output window the grouping happens. If enc_state = true then the input characters are not grouped but the output characters are.
     *  If enc_state = false it is the other way round.
     */                
    virtual void set_grouping(unsigned int group_type, bool enc_state);

    /*! \brief Destructor.
     */    
    virtual ~log_helper() { ; }

protected:
    /*! \brief Points to the simulator object in use in this application. */    
    rotor_draw *simulator_gui;
    
    /*! \brief Points to the log dialog that is managed by this instance of log_helper. */    
    display_dialog *disp;
    
    /*! \brief Holds the connection which is used to receive input characters. */    
    sigc::connection in_log;
    
    /*! \brief Holds the connection which is used to receive output characters. */    
    sigc::connection out_log;        
};

/*! \brief An application class that allows to handle the events that occur when the user opens the dialog which allows
 *         to set a new rotor position.
 *
 *  In order to fulfill its intended tasks an object of this class has to know the simulator object that is actually in use in the application
 *  (member log_helper::simulator_gui).
 */    
class rotor_position_helper : public menu_helper {
public:
    /*! \brief Constructor. The parameter name_of_app has to contain the name of the app and is used to set the title
     *         of any dialog window that is presented to the user while processing the event.
     */
    rotor_position_helper(const char *name_of_app) : menu_helper(name_of_app) { ; }
    
    /*! \brief This method allows to set the rotor_simulator instances that this log_helper instance is intended
     *         to manage.
     */                
    virtual void set_simulator(rotor_draw *simulator) { simulator_gui = simulator; }

    /*! \brief This method allows to handle calling the rotorpos_dialog in order to change to rotor positions of the machine underlying
     *         the rotor_position_helper_simulator_gui.
     */                    
    virtual void set_rotor_positions(sigc::slot<void> *set_pos_success);    

    /*! \brief Destructor.
     */                    
    virtual ~rotor_position_helper() { ; }

protected:
    /*! \brief Points to the simulator object in use in this application. */    
    rotor_draw *simulator_gui;
};

/*! \brief An application class that allows to handle the events that occur when the user opens the dialog which allows
 *         to randomize machine settings.
 */    
class randomizer_param_helper : public menu_helper {
public:
    /*! \brief Constructor. The parameter name_of_app has to contain the name of the app and is used to set the title
     *         of any dialog window that is presented to the user while processing the event.
     */
    randomizer_param_helper(const char *name_of_app) : menu_helper(name_of_app) { has_errors = false; was_cancelled = false; }

    /*! \brief This method shows a dialog which allows the user to select a randomizer parameter and subsequently calls the randomize()
     *         method of the rotor_machine object to which the parameter machine points using that randomizer parameter.
     */    
    virtual void randomize_machine(rotor_machine *machine);    

    /*! \brief This method returns false, if no errors were encountered during randomization. Else true is returned.
     */                    
    virtual bool get_has_error() { return has_errors; }

    /*! \brief This method returns true, if the randomization dialog was cancelled. Else it returns true.
     */                    
    virtual bool get_was_cancelled() { return was_cancelled; }

    /*! \brief Destructor.
     */                    
    virtual ~randomizer_param_helper() { ; }

protected:
    /*! \brief Is true when the previous call of the randomize_machine() resulted in an error. Else holds false. */  
    bool has_errors;

    /*! \brief Is true when the dialog to select the randomizer parameter was closed by clicking cancel in the 
     *         previous call of the randomize_machine() method. Else holds false. 
     */      
    bool was_cancelled;
};

/*! \brief An application class that allows to handle the event that occurs when the user activates the Rotor set|Randomize rotor sets ...
 *         menu entry.
 */    
class rotor_set_rand_helper : public menu_helper {
public:
    /*! \brief Constructor. The parameter name_of_app has to contain the name of the app and is used to set the title
     *         of any dialog window that is presented to the user while processing the event.
     */
    rotor_set_rand_helper(const char *name_of_app) : menu_helper(name_of_app) { ; }

    /*! \brief This method randomizes all rotor sets known to the rotor_machine object to which the parameter machine points.
     */    
    virtual void randomize_rotor_sets(rotor_machine *machine);    

    /*! \brief Destructor.
     */                    
    virtual ~rotor_set_rand_helper() { ; }
};

#endif /* __app_helpers_h__ */
