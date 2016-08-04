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

#ifndef __rotorpos_dialog_h__
#define __rotorpos_dialog_h__

/*! \file rotorpos_dialog.h
 *  \brief Header file for the application class that is used to implement the set rotor position dialog of rotorvis and enigma.
 */

#include<gtkmm.h>
#include<string>
#include<vector>
#include<map>
#include<rmsk_globals.h>

using namespace std;

/*! \brief An application class that implements a dialog which allows to enter new rotor positions in rotorvis and enigma. 
 */
class rotorpos_dialog : public Gtk::Dialog {
public:
    /*! \brief Constructor.
     *
     *  \param parent [in] Has to point to the parent window, i.e. the main window of the application.
     *  \param rotor_positions [inout] Specifies the rotor positions displayed to the user on entering the dialog. Changed to new
     *                                 value entered by the user when dialog is closed with OK.
     */
    rotorpos_dialog(Gtk::Window& parent, Glib::ustring& rotor_positions);

    /*! \brief Callback which is executed when the user closed the dialog by clicking on the OK button.
     */
    virtual void on_ok_clicked();

    /*! \brief Destructor.
     */    
    virtual ~rotorpos_dialog() { ; }

protected:
    /*! \brief Holds a grid with two rows and two colums. */    
    Gtk::Grid config_items;

    /*! \brief Points to the OK button of the dialog. */    
    Gtk::Button *ok_button;
    
    /*! \brief Holds the rotor positions displayed to the user on entering the dialog. Changed to new value entered by the user when
     *         dialog is closed with OK. 
     */        
    Glib::ustring& positions;
    
    /*! \brief Holds entry box used to enter new rotor positions. */
    Gtk::Entry pos_entry;
    
    /*! \brief Holds Label left of pos_entry. */        
    Gtk::Label pos_label;
    
    /*! \brief Empty label used as a spacer below pos_label. */    
    Gtk::Label empty_label;
};

#endif /* __rotorpos_dialog_h__ */
