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

#ifndef __rotor_dialog_h__
#define __rotor_dialog_h__

/*! \file rotor_dialog.h
 *  \brief Header file for the application classes that implement a dialog that lets the user select a wheel setting.
 */

#include<gtkmm.h>
#include<machine_config.h>

using namespace std;
/*! \brief An application class that is used as a model for the ComboBoxes that are used to select the rotors in each
 *         rotor slot.
 *
 *  The model of a ComboBox in gtkmm consists of a list of things, where each of these things can have several components
 *  (called columns). The column text contains a human readable description of the rotor name (WALZE I, WALZE II, ...)
 *  and the column identifier specifies the numeric rotor id of the rotor named in column text. 
 */
class simple_text_cols : public Gtk::TreeModel::ColumnRecord {
public:
    /*! \brief text column is of type Glib::ustring.
     */    
    Gtk::TreeModelColumn<Glib::ustring> text;

    /*! \brief identifier column is of type unsigned int.
     */    
    Gtk::TreeModelColumn<unsigned int> identifier;

    /*! \brief Constructor. Adds columns.
     */    
    simple_text_cols() {  add(text); add(identifier);}
};

/*! \brief An application class that manages as a set of widgets that together provide a dialog that can be used to
 *         specifiy wheel settings for all simulated Enigma variants.
 *
 *  For each of the five rotor slots of an Enigma machine (UKW, greek wheel, slow wheel, middle wheel, fast wheel) one
 *  has to select a rotor (done via a Combobox) and a ring setting (done via a scale/slider widget). Some machines do
 *  not have all five slots (only M4 has the greek wheel) and some others allow to set the rotor ring for the UKW and
 *  some do not. These differences are taken into account when initializing the dialog through the method 
 *  rotor_processor_base::fill_rotor_model().
 */
class rotor_processor_base {
public:
    /*! \brief Constructor.
     * 
     * \param d [in] The dialog that contains the widgets this class has to manage.
     * \param r [in] A Gtk::Builder object that can be used to retrieve pointers to controls inside d.
     * \param desc [inout] A vector of ::rotor_family_descriptor objects that describe the wheel and ring settings
     *                     of an Enigma machine. When calling the rotor_processor_base::run() method this vector
     *                     has to contain the current wheel and ring settings. After closing the dialog by
     *                     clicking the OK button it is changed to the values selected by the user.
     */
    rotor_processor_base(Gtk::Dialog *d, Glib::RefPtr<Gtk::Builder> r, vector<rotor_family_descriptor>& desc);

    /*! \brief Callback that is called if the ring setting is changed by moving any of the scales/sliders.
     */    
    virtual void on_scale_change(Glib::ustring wheel_name);

    /*! \brief Executes the dialog. Has to return zero, if the user closed the dialog by clicking on OK.
     */    
    virtual int run();

    /*! \brief Destructor.
     */        
    virtual ~rotor_processor_base();
    
protected:

    /*! \brief Sets up the alternatives which can be selected by the ComboBoxes in different rotor slots. Additionally
     *         deactivates widgets that pertain to rotor slots that are not active in the Enigma variant currently
     *         simulated. The sliders/scales of rotor slots that do not allow to set the ring position for the
     *         currently simulated machine are also deactived.
     */    
    virtual void fill_rotor_model(unsigned int count);

    /*! \brief Sets the scale of the rotor slot identified by the parameter count to the value specfied in 
     *         rotor_processor_base::state_desc[count].ring_setting and activates or deactivates ("grays") the 
     *         GUI elements that allow to change the ring setting depending on the value 
     *         rotor_processor_base::state_desc[count].ring_selection_state.
     */
    virtual void set_ring_data(unsigned int count);

    /*! \brief Activates or deactivates ("grays") the GUI elements that allow to change the ring setting for a rotor slot 
     *         specified by parameter count depending on the value of parameter new_state. False means that the elements
     *         are "grayed".
     */
    virtual void set_state_ring_controls(unsigned int count, bool new_state);

    /*! \brief Shows or hides the GUI elements that allow to change the ring setting for a rotor slot 
     *         specified by parameter count depending on the value of parameter new_state. False means that the elements
     *         are hidden.
     */
    virtual void set_visibility_ring_controls(unsigned int count, bool new_state);

    /*! \brief Uses the currently selected values of the ComboBoxes to modify the object referenced by rotor_processor_base::state_desc
     *         according to the settings chosen by the user. This method creates the "return values" when the user has closed
     *         the dialog by pressing OK.
     */
    virtual void fill_result();

    /*! \brief Helper method that can be used to retrieve the id of the rotor that has been selected in the ComboBox named by parameter
     *         widget_name. The retrieved value is then stored in the set referenced by parameter test_set.
     */    
    virtual void retrieve_selected_rotor_id(const char *widget_name, set<unsigned int>& test_set);

    /*! \brief Holds the Gtk::Dialog in which the managed widgets reside. */        
    Gtk::Dialog *dialog;
    
    /*! \brief Holds the connection objects which result from connecting to the change signals of the scales used to modify
     *         the ring settings.
     */            
    vector<sigc::connection> up_down_connections;

    /*! \brief Holds the Gtk::Builder object which can be used to derive pointers to widgets in rotor_processor_base::dialog. */            
    Glib::RefPtr<Gtk::Builder> ref_xml;
    
    /*! \brief Description of the elements that can be selected through the ComboBoxes. */                
    simple_text_cols model_cols;
    
    /*! \brief Used to initialize the dialog and modified upon successfull completion of the dialog. */                    
    vector<rotor_family_descriptor>& state_desc;
    
    /*! \brief Prefixes for the names of the widgets which are used in the rotor slots. Is used together with 
     *         rotor_processor_base::ref_xml to gain access to the widgets inside of rotor_processor_base::dialog.
     */    
    vector<string> control_prefixes;
};

/*! \brief An application class that manages as a set of widgets that together provide a dialog that can be used to
 *         specifiy wheel settings for all simulated Enigma variants.
 *
 *  Inherits everything from rotor_processor_base.
 */
class rotor_dialog_processor : public rotor_processor_base {
public:
    /*! \brief Constructor.
     * 
     * \param d [in] The dialog that contains the widgets this class has to manage.
     * \param r [in] A Gtk::Builder object that can be used to retrieve pointers to controls inside d.
     * \param desc [inout] A vector of ::rotor_family_descriptor objects that describe the wheel and ring settings
     *                     of an Enigma machine. When calling the rotor_processor_base::run() method this vector
     *                     has to contain the current wheel and ring settings. After closing the dialog by
     *                     clicking the OK button it is changed to the values selected by the user.
     */
    rotor_dialog_processor(Gtk::Dialog *d, Glib::RefPtr<Gtk::Builder> r, vector<rotor_family_descriptor>& desc);

    /*! \brief Destructor.
     */        
    virtual ~rotor_dialog_processor() { ; }

};

#endif /* __rotor_dialog_h__*/

