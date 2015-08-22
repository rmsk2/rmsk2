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

#ifndef __plugboard_dialog_h__
#define __plugboard_dialog_h__

/*! \file plugboard_dialog.h
 *  \brief Header file for the application classes that are used to implement a dialog that lets the user select an involution.
 *         On top of that it contains the header for a class that implements a dialog which allows to configure an Enigma's
 *         plugboard.
 */

#include<set>
#include<vector>
#include<utility>

#include<gtkmm.h>
#include<glibmm.h>

using namespace std;

/*! \brief An application class that is needed by a TreeView which displays the pairs of characters that have already been
 *         selected to build an involution.
 *
 *  It simply describes the data types of the two columns which are used in the TreeView. Yes TreeView. In gtkmm the kind of control
 *  that displays its data in a table with headers is an appropriately configured TreeView.
 */
class plug_cols : public Gtk::TreeModel::ColumnRecord {
public:

    /*! \brief First column is of type ustring. */
    Gtk::TreeModelColumn<Glib::ustring> first;
    
    /*! \brief Second columns is of type ustring. */    
    Gtk::TreeModelColumn<Glib::ustring> second;

    /*! \brief Constructor.
     */
    plug_cols() { add(first); add(second); }
};

/*! \brief An application class that knows how to manage a set of controls which together allow the user to construct an involution
 *         by specifiying a variable number of character pairs.
 *
 *  This class is the base class for two different dialogs: The dialog to configure an Enigma plugboard and the dialog for speicifying
 *  a wiring for the UKW D.
 *
 *  The controls which are managed by objects of this class have been specified in a glade file, where each control has a name. During
 *  object construction the necessary controls are extracted by their name from the managed dialog. As names have to be unique in a 
 *  glade file controls in different dialogs (Plugboard, UKW D wiring) that have the same purpose still have to have different names.
 *
 *  The names of equivalent controls in different dialogs can be distinguished by their postfix. For instance the checkbox representing
 *  the character Z is called checkbutton_Z_ukw in the UKW wiring dialog and checkbutton_Z in the plugboard dialog. In the first case the 
 *  prefix is "_ukw" in the second it is "".
 *
 *  The controls which are managed consist of a list of previuosly added character pairs, a button to delete a pair, a button to delete
 *  all pairs, a label which specifies the current number of pairs (the plug count), 26 checkboxes (one for each character) and an OK
 *  button.
 */
class select_involution_dialog {
public:
    /*! \brief Constructor.
     * 
     * \param d [in] The dialog that contains the controls this class has to manage.
     * \param r [in] A Gtk::Builder object that can be used to retrieve pointers to controls inside d.
     * \param steckers [inout] Data structure which has to specifiy the pairs which are to be used to initialize
     *                         the selection process. When the dialog has been completed by pressing OK, the object
     *                         which steckers references contains the character pairs selected by the user.
     *
     * The default prefix is the empty string.
     */
    select_involution_dialog(Gtk::Dialog *d, Glib::RefPtr<Gtk::Builder> r, vector<pair<char, char> >& steckers);

    /*! \brief Constructor.
     * 
     * \param d [in] The dialog that contains the controls this class has to manage.
     * \param r [in] A Gtk::Builder object that can be used to retrieve pointers to controls inside d.
     * \param steckers [inout] Data structure which has to specifiy the pairs which are to be used to initialize
     *                         the selection process. When the dialog has been completed by pressing OK, the object
     *                         which steckers references contains the character pairs selected by the user.
     * \param name_postfix [in] Specifies a postfix which is appended to the names of the controls which are retrieved 
     *                          through the parameter r.
     */
    select_involution_dialog(Gtk::Dialog *d, Glib::RefPtr<Gtk::Builder> r, vector<pair<char, char> >& steckers, Glib::ustring& name_postfix);

    /*! \brief Pure virtual method that children of this class have to implement in order to execute the dialog. Has to return zero
     *         to inidicate that the user left the dialog by clikcking OK.
     */    
    virtual int run() = 0;

    /*! \brief Pure virtual method that children of this class have to implement. It is used as a callback when the user opted to delete
     *         a previously added character pair by selecting the pair in the list of character pairs and pressing the delete button.
     */    
    virtual void on_delete_clicked() = 0;

    /*! \brief Callback that is called when the user clicks on the "Remove all plugs" or "Remove all connections" button. This clears
     *         the list of current character pairs.
     */    
    virtual void on_delete_all_clicked();

    /*! \brief Callback that is called when the user selects one of the checkboxes that represent single characters. The parameter plug_name
     *         contains the name of the checkbox that was clicked. Possible names are 'A', 'B', .... 'Z'.
     */    
    virtual void on_plug_changed(Glib::ustring plug_name);

    /*! \brief The plug count label text is the prefix that is printed before the value that specifies how many character pairs have already
     *         been selected. In the plugboard dialog this is for instance "Number of plugs: ". This method can be used to change this prefix to
     *         value specified in parameter plug_count_prefix.
     */    
    virtual void set_plug_count_label_text(Glib::ustring& plug_count_prefix) { plug_count_label_prefix = plug_count_prefix; }

    /*! \brief Destructor.
     */        
    virtual ~select_involution_dialog();
    
protected:
    /*! \brief Helper method that transofrms a character specified as a char through parameter in into a ustring of length one.
     */        
    virtual Glib::ustring to_ustr(char in) { Glib::ustring temp; temp += (char)(toupper(in)); return temp; }

    /*! \brief Helper method that inserts the character pair (first, second) into the list of already selected characters. Also
     *         updates elements like the plug count.
     */        
    virtual void insert_stecker_in_model(Glib::ustring first, Glib::ustring second);

    /*! \brief Helper method that updates the state of the two checkboxes that make up a character pair after the pair has been selected or
     *         deselected.
     *
     *  \param first [in] First character of new pair.
     *  \param second [in] Second character of new pair.     
     *  \param is_sensitive [in] New state of the checkboxes. If False the checkboxes are "grayed".
     *  \param is_active [in] Specifies of the boxes should appear checked or not. If False the checkboxes are not checked.     
     */        
    virtual void set_plug_state(Glib::ustring first, Glib::ustring second, bool is_sensitive, bool is_active);

    /*! \brief Helper method that updates the state of the two checkboxes that make up a character pair after the pair has been
     *         deselected.
     *
     *  \param first [in] First character of new pair.
     *  \param second [in] Second character of new pair.     
     *
     * Simply ungrays the checkboxes and unchecks them.
     */        
    virtual void remove_plug(Glib::ustring first, Glib::ustring second) { set_plug_state(first, second, true, false); }

    /*! \brief Helper method that fills the List of already selected pairs based on the value of current_steckers.
     */
    virtual void fill_stecker_model();

    /*! \brief Helper method that sets the text of the plug counter label by combining the number of the currently
     *         selected pairs with select_involution_dialog::plug_count_label_prefix.
     */
    virtual void update_plug_counter();

    /*! \brief Helper method that initializes most the member variables. Non virtual so that ot can be called safely by the
     *         construdtors.
     */
    void setup(Gtk::Dialog *d, Glib::RefPtr<Gtk::Builder> r, Glib::ustring& name_postfix);

    /*! \brief Holds the number of currently selected character pairs. */
    unsigned int num_plugs;
    
    /*! \brief Points to the plug count label. */    
    Gtk::Label *plugcount_label;
    
    /*! \brief Holds the name ('A', ..., 'Z') of the last checkbox that was checked or "" when the last checking completed a pair. */        
    Glib::ustring last_button_toggled;
    
    /*! \brief Points to the dialog which contains the controls managed by objects of this class. */            
    Gtk::Dialog *dialog;
    
    /*! \brief Data structure that maps the name ('A', ..., 'Z') of a checkbox to a pointer to the checkbox of that name. */                
    map<char, Gtk::CheckButton *> buttons;
    
    /*! \brief Data structure that maps a pointer to a checkbox to the connection object that is used to receive the signal_clicked()
     *         signal from that checkbox. 
     */                    
    map<Gtk::CheckButton *, sigc::connection> check_button_connections;
    
    /*! \brief Connection object that resulted from connecting to the signal_clicked() signal of the delete button. */                        
    sigc::connection delete_connection;
    
    /*! \brief Connection object that resulted from connecting to the signal_clicked() signal of the delete all button. */                            
    sigc::connection delete_all_connection;

    /*! \brief Holds a reference to the Gtk::Builder object that is used to retreive controls from select_involution_dialog::dialog. */                            
    Glib::RefPtr<Gtk::Builder> ref_xml;

    /*! \brief Holds the vector of plugs which are used to initialize the dialogs and is filled with the vector of plugs the 
     *         user has selected. 
     */                            
    vector<pair<char, char> >& current_steckers;
    
    /*! \brief Specified the columns in the list of selected pairs and their types. */    
    plug_cols plugboard_cols;
    
    /*! \brief Holds the prefix which is appended to the name of controls in order to retrive them through 
     *         select_involution_dialog::ref_xml. 
     */    
    Glib::ustring name_post_fix;
    
    /*! \brief Holds prefix for the plug count label. */    
    Glib::ustring plug_count_label_prefix;    
};

/*! \brief An application class that knows how to manage a set of controls which together allow the user to configure all parameters of
 *         an Enigma plubgoard including a possible Enigma Uhr.
 *
 *  Character pairs are called plugs in the context of this dialog.
 */
class plugboard_dialog : public select_involution_dialog {
public:
    /*! \brief Constructor.
     * 
     * \param d [in] The dialog that contains the controls this class has to manage.
     * \param r [in] A Gtk::Builder object that can be used to retrieve pointers to controls inside d.
     * \param steckers [inout] Data structure which has to specifiy the plugs which are to be used to initialize
     *                         the selection process. When the dialog has been completed by pressing OK, the object
     *                         which the parameter steckers references contains the plugs selected by the user.
     * \param use_uhr [inout] References a bool that specifies if the Uhr is currently in use in the plugboard that is
     *                        to be configured. When the dialog is completed by clicking OK it is set to reflect the users
     *                        choice with respect to usage of the Uhr. 
     * \param uhr_dial_pos [inout] References an unsigned int that contains the dial pos of the Uhr. If use_uhr is False
     *                             this value is effectively ignored. After completing the dialog by clicking OK this
     *                             variable is set to the new dial position or to zero if use_uhr is set to False.
     * \param uhr_capable [in] If True then the controls to configure the Enigma Uhr are enabled in the plugbaord dialog.                           
     */
    plugboard_dialog(Gtk::Dialog *d, Glib::RefPtr<Gtk::Builder> r, vector<pair<char, char> >& steckers, bool& use_uhr, unsigned int& uhr_dial_pos, bool uhr_capable);

    /*! \brief Execute the dialog. Returns zero to indicate that the user left the dialog by clikcking OK.
     */        
    virtual int run();

    /*! \brief Callback that is called when the user opted to delete a previously added plug by selecting it in the list of plugs
     *         and pressing the delete button.
     */
    virtual void on_delete_clicked();

    /*! \brief Callback that is called when the user turns the Uhr on or off by clicking on the corresponding checkbox.
     */
    virtual void on_uhr_change();

    /*! \brief Destructor.
     */    
    virtual ~plugboard_dialog();
    
protected:
    /*! \brief Turns Uhr controls on or off depending on the parameter current_state.
     */
    virtual void update_uhr_state(bool current_state);

    /*! \brief Holds the connection object which resulted from subscribing to the signal_clicked() of the Uhr checkbutton. */
    sigc::connection uhr_connection;
    
    /*! \brief Set to new value after dialog has been closed by clicking on OK.
     */    
    bool& uhr_state;

    /*! \brief Set to new value after dialog has been closed by clicking on OK.
     */    
    unsigned int& uhr_pos;
    
    /*! \brief Holds the current value of uhr_state while the dialog is running. Is used to set the variable which
     *         plugboard_dialog::uhr_state references to a new value after user has clicked on OK button.
     */    
    bool uhr_state_temp;
};

#endif /* __plugboard_dialog_h__ */

