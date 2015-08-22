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

#ifndef __ukwd_wiring_dialog_h__
#define __ukw_dwiring_dialog_h__

/*! \file ukwd_wiring_dialog.h
 *  \brief Header file for the application class that is used to implement a dialog that lets the user select a UKW D wiring.
 */

#include<set>
#include<utility>
#include<string>
#include<gtkmm.h>
#include<glibmm.h>
#include<permutation.h>
#include<plugboard_dialog.h>

/*! \brief An application class that knows how to manage a set of controls which together allow the user to select a wiring for the
 *         Umkehrwalze D (a.k.a UKW D).
 *
 *  Character pairs are called connections in the context of this dialog. An Involution that specifies a valid UKW D wiring is
 *  special in that it is made up of exactly 13 connections and that the connection ('J', 'Y') always has to be part
 *  of the wiring because that connection was built into the UKW D and could not be changed.
 */
class ukwd_wiring_dialog : public select_involution_dialog {
public:
    /*! \brief Constructor.
     * 
     * \param d [in] The dialog that contains the controls this class has to manage.
     * \param r [in] A Gtk::Builder object that can be used to retrieve pointers to controls inside d.
     * \param steckers [inout] Data structure which has to specifiy the connections which are to be used to initialize
     *                         the selection process. When the dialog has been completed by pressing OK, the object
     *                         which steckers references contains the connections selected by the user.
     * \param name_postfix [in] Specifies a postfix which is appended to the names of the controls which are retrieved 
     *                          through the parameter r.
     */
    ukwd_wiring_dialog(Gtk::Dialog *d, Glib::RefPtr<Gtk::Builder> r, vector<pair<char, char> >& steckers, Glib::ustring& name_postfix);

    /*! \brief Execute the dialog. Returns zero to inidicate that the user left the dialog by clikcking OK.
     */    
    virtual int run();
    
    /*! \brief Callback that is called when the user opted to delete a previously added connection by selecting it in the list of
     *         connections and pressing the delete button.
     */    
    virtual void on_delete_clicked();

    /*! \brief Callback that is called when the user opted to delete all previously added connections by pressing the delete all
     *         button.
     */    
    virtual void on_delete_all_clicked();    

    /*! \brief Method that can be used to parse the permutation given in parameter perm and which specifies an involution into
     *         the character pairs that make up that involution. 
     */    
    static vector<pair<char, char> > perm_to_plugs(permutation& perm);

    /*! \brief Does the inverse of ukwd_wiring_dialog::perm_to_plugs(). Transforms the vector of character pairs referenced by
     *         parameter plugs into a permutation.
     */    
    static permutation plugs_to_perm(vector<pair<char, char> >& plugs);

    /*! \brief Destructor.
     */        
    ~ukwd_wiring_dialog() { ; }
    
protected:  
    /*! \brief Helper method to compare two character pairs by their first element.
     */        
    static bool less_than(const pair<char, char>& l, const pair<char, char>& r);    
};

#endif /* __ukwd_wiring_dialog_h__ */

