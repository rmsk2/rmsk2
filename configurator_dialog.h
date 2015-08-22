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

#ifndef __configurator_dialog_h__
#define __configurator_dialog_h__

/*! \file configurator_dialog.h
 *  \brief Header file for the application class that is used to implement the configuration dialog of rotorvis.
 */

#include<gtkmm.h>
#include<string>
#include<vector>
#include<map>
#include<rmsk_globals.h>

using namespace std;

/*! \brief An application class that implements a dialog which allows to enter configuration information in rotorvis. 
 *         The necessary configuration parameters and their types are specified by a vector of ::key_word_info objects.
 */
class configurator_dialog : public Gtk::Dialog {
public:
    /*! \brief Constructor.
     *
     *  \param parent [in] Has to point to the parent window, i.e. the main window of the application.
     *  \param infos [in] Specifies the number and names of the expected configuration parameters and their types.
     *  \param config_data [inout] Has to provided the current values for the configuration parameters. When the
     *                             dialog is closed it contains the changed configuration parameters as entered
     *                             by the user.
     *
     *  config_data is a map that maps the name of a configuration parameter (as specified in parameter infos) to its
     *  current value. If the dialog is not closed via OK, then config_data remains unchanged.
     */
    configurator_dialog(Gtk::Window& parent, vector<key_word_info>& infos, std::map<string, string>& config_data);

    /*! \brief Callback which is executed when the user closed the dialog by clicking on the OK button.
     */
    virtual void on_ok_clicked();

    /*! \brief Destructor.
     */    
    virtual ~configurator_dialog() { ; }

protected:
    /*! \brief Holds a grid with inf.size() rows and two colums. */    
    Gtk::Grid config_items;

    /*! \brief Points to the OK button of the dialog. */    
    Gtk::Button *ok_button;
    
    /*! \brief Maps the name of each string parameter to a pointer to an Entry object that holds its value. */        
    std::map<string, Gtk::Entry*> string_items;
    
    /*! \brief Maps the name of each bool parameter to a pointer to a CheckButton object that holds its value. */
    std::map<string, Gtk::CheckButton*> bool_items;    
    
    /*! \brief Specifiacaton of the known parameters/keywords and their types. */    
    vector<key_word_info>& inf;
    
    /*! \brief Current values of the  parameters/keywords. */    
    std::map<string, string>& conf_data;
};




#endif /* __configurator_dialog_h__ */
