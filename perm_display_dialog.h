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

#ifndef __perm_display_dialog_h__
#define __perm_display_dialog_h__

/*! \file perm_display_dialog.h
 *  \brief Header file for the application class that is used to implement a dialog to show the permutations currently in use in a rotor machine.
 */

#include<gtkmm.h>
#include<string>
#include<vector>
#include<map>

using namespace std;

/*! \brief An application class that implements a dialog to show the permutations currently in use in a rotor machine.
 */
class perm_display_dialog : public Gtk::Dialog {
public:
    /*! \brief Constructor.
     *
     *  \param parent [in] Has to point to the parent window, i.e. the main window of the application.
     *  \param permutation_msgs [in] Has to contain the permutation information that should be displayed.     
     */
    perm_display_dialog(Gtk::Window& parent, vector<Glib::ustring>& permutation_msgs);

    /*! \brief Destructor.
     */    
    virtual ~perm_display_dialog() { ; }

protected:    
    /*! \brief Points to the OK button of the dialog. */    
    Gtk::Button *ok_button;
    
    /*! \brief Holds a container that adds a scroll bar if the text in the Textview becomes too large. */    
    Gtk::ScrolledWindow scroll_bar;
    
    /*! \brief Holds the control that is actually used to display text. */
    Gtk::TextView printer;
};
 
#endif /* __perm_display_dialog_h__ */
