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

#ifndef __simple_state_h__
#define __simple_state_h__

/*! \file simple_state.h
 *  \brief This file contains the the header for a class that provides a simplistic facility to store the state of an application
 *         like for instance the least recently used directory name.
 */

#include<set>
#include<glibmm.h>

using namespace std;

/*! \brief An application class that knows how to store and load state information of a simulator application like for instance
 *         the path of the least recently used directory. 
 *
 *  Objects of this class serialize their data into an INI file the name of which has to be specified in the constructor. It is 
 *  stored as a hidden file in the home directory of the current user.
 */
class simple_state {
public:
    /*! \brief Constructor. The parameter file_name has to specify the base name of the INI file. A "." is prepended to the actual
     *         file name in order to make the file hidden.
     *
     *  Sets last_dir to the user's home directory.
     */
    simple_state(const char *file_name);
    
    /*! \brief Loads the INI file. In case of an error a message is printed to stdout.
     *
     *  If the INI file does not exist or does not contain the necessary keys then the member variables that could
     *  not be read remain unchanged. In the case of a freshly constructed object this means that memeber variables 
     *  retain the value set in the constructor.
     */    
    virtual void load();

    /*! \brief Stores the INI file. In case of an error a message is printed to stdout 
     */    
    virtual void save();

    /*! \brief Sets the path of the least recently used directory.
     */        
    virtual void set_last_dir(Glib::ustring& l_dir) { last_dir = l_dir; }

    /*! \brief Returns the path of the recently used directory.
     */        
    virtual void get_last_dir(Glib::ustring& l_dir) { l_dir = last_dir; }    

    /*! \brief Saves the position of the application main window.
     */        
    virtual void set_last_pos(int x, int y) { pos_x = x; pos_y = y; }

    /*! \brief Returns the saved window positon.
     */        
    virtual void get_last_pos(int& x, int& y) { x = pos_x; y = pos_y; }    


    /*! \brief Destructor.
     */        
    virtual ~simple_state() { ; }
    
protected:
    /*! \brief Holds the name of the INI file to which the data is serialized.
     */        
    Glib::ustring ini_name;

    /*! \brief Holds the path of the least recently used directory.
     */        
    Glib::ustring last_dir;

    /*! \brief Holds the x-position of the application main window.
     */            
    int pos_x;

    /*! \brief Holds the y-position of the application main window.
     */            
    int pos_y;
};

/*! \brief An application class that knows how to set up and run a generic rotor machine simulator application. 
 */
class simulator_app {
public:
    /*! \brief Constructor. The parameter state_base_name has to specify the name of the hidden file which is used to store the
     *         application state. The parameter valid_names specifies the names of the machines that this simulator is able to
     *         simulate. The parameter use_enigma_pic influences the picture that is shown when the user is presented with a
     *         machine selection dialog. If true a picture of an Enigma is shown. In case use_enigma_pic is false a picture of
     *         another rotor machine is depicted.
     *
     *  The set of names given in valid_names is used to parse the first parameter given to simulator_app::run(). If no parameter
     *  was specified by the user this set is used to display a dialog that lets the user select the desired machine.
     */    
    simulator_app(const char *state_base_name, set<string>& valid_names, bool use_enigma_pic = true);

    /*! \brief This method sets the application up and runs it. It returns a status code that can be returned by the
     *         application's main function. 
     *
     *  It is expected that argc and argv contain the parameters given to the main function via the program's command line.
     */    
    virtual int run(int argc, char **argv, Gtk::Main *m);    

    /*! \brief This method is intended to initialize the application. 
     */    
    virtual void init(int argc, char **argv) { ; }

    /*! \brief This method is intended to restore the application state before the application is actually run.
     *
     *  This method should not call add_state.load(). This is already done by simulator_app::run(). 
     */    
    virtual void restore_state() { ; }    

    /*! \brief This method is intended to determine the application state after the main window has been closed. 
     *
     *  This method should not call add_state.save(). This is already done by simulator_app::run(). 
     */    
    virtual void determine_state() { ; }        

    /*! \brief Destructor. 
     */        
    virtual ~simulator_app() { ; }
    
protected:
    /*! \brief Holds the main window of the application. */            
    Gtk::Window *win;

    /*! \brief Holds a set of machine names which are known to this simulator application. These names are allowed
     *         to be used as the first argument to the main program.
     */            
    set<string> allowed_names;
    
    /*! \brief Holds the name of the machine that the user wishes to simulate. */                
    Glib::ustring m_name;
    
    /*! \brief Flag that is true when the user cancelled the selection dialog. */                    
    bool do_stop;    

    /*! \brief Is true if the selection dialog is to contain a picture of an Enigma. If set to false a picture of a Typex is shown. */                    
    bool show_enigma_pic;    

    /*! \brief A reference to the icon that is used for this application. */                    
    Glib::RefPtr<Gdk::Pixbuf> enigma_icon;
    
    /*! \brief Holds the state (window position, least recently used directory, ....) of the application. */    
    simple_state app_state;            
};

#endif /* __simple_state_h__ */ 
