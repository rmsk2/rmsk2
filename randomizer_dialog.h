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

#ifndef __randomizer_dialog_h__
#define __randomizer_dialog_h__

/*! \file randomizer_dialog.h
 *  \brief Header file for the application class that is used to implement the randomize dialog of rotorvis and enigma.
 */

#include<gtkmm.h>
#include<string>
#include<vector>
#include<map>
#include<rmsk_globals.h>

using namespace std;

/*! \brief An application class that implements a dialog which allows the user to choose a randomizer parameter in
 *         rotorvis and enigma.
 */
class randomizer_dialog : public Gtk::Dialog {
public:
    /*! \brief Constructor.
     *
     *  \param parent [in] Has to point to the parent window, i.e. the main window of the application.
     *  \param randomizer_parameter [out] Changed to new value chosen by the user when the dialog is closed with OK.
     */
    randomizer_dialog(Gtk::Window& parent, string& randomizer_parameter, vector<randomizer_descriptor>& param_choices);

    /*! \brief Callback which is executed when the user closed the dialog by clicking on the OK button.
     */
    virtual void on_ok_clicked();

    /*! \brief Destructor.
     */    
    virtual ~randomizer_dialog() { ; }

protected:
    /*! \brief Holds a grid with two rows and two colums. */    
    Gtk::Grid randomizer_items;

    /*! \brief Points to the OK button of the dialog. */    
    Gtk::Button *ok_button;
    
    /*! \brief Changed to the randomizer parameter value chosen by the user when dialog is closed with OK. 
     */        
    string& parameter;
    
    /*! \brief Combobox that holds the selection of andomizer parameters the user can choose from. */
    Gtk::ComboBoxText param_combo;
        
    /*! \brief Holds Label left of param_combo. */        
    Gtk::Label param_label;
    
    /*! \brief Empty label used as a spacer below param_label. */    
    Gtk::Label empty_label;

    /*! \brief This mapping maps the human understandable description to the keyword known to the randomize method. */    
    std::map<string, string> description_keyword_mapping;

};

#endif /* __randomizer_dialog_h__ */
