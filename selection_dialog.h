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

#ifndef __selection_dialog_h__
#define __selection_dialog_h__

/*! \file selection_dialog.h
 *  \brief Header file for the application class that is used to to display a dialog from which one can 
 *         select the machine which is to be simulated.
 */

#include<string>
#include<set>
#include<map>
#include<gtkmm.h>

using namespace std;

/*! \brief An application class that implements a dialog from which one can select the name of a machine which
 *         is to be simulated.
 *
 *  The selection is presented to the user as a set of RadioButtons from which a machine name can be selected.
 *  This class consciously derives from Gtk::Window and not Gtk::Dialog in order to make it possible to be used
 *  at times when the main application window has not yet been created.
 */
class selection_dialog : public Gtk::Window {
public:
    /*! \brief Constructor.
     *
     *  \param machine_names [in] Has to specifiy the set of machine names which are displayed to the user.
     *  \param main [in] Has to specifiy the application object which is used to run the dialog.     
     *  \param show_enigma_pic [in] If true a picture of an Enigma is shown. If false a Typex is depicted.
     */
    selection_dialog(set<string>& machine_names, Gtk::Main *main, bool show_enigma_pic = true);

    /*! \brief Returns the machine name which has been selected by the user.
     */    
    virtual Glib::ustring get_selected_name() { return name_selected; }

    /*! \brief Callback which is called when the selection in the group of RadioButtons is changed.
     */    
    virtual void on_radio_clicked(Glib::ustring selected_type) { name_selected = selected_type; }

    /*! \brief Callback which is called when the dialog is closed by clicking the OK button.
     */    
    virtual void on_ok_clicked() { hide(); }

    /*! \brief Callback which is called when the dialog is closed by clicking the Cancel button.
     */     
    virtual void on_cancel_clicked() { name_selected = "None"; hide(); }

    /*! \brief Callback which is called when the dialog is closed by the close button in the title bar of the window.
     */         
    virtual bool on_delete(GdkEventAny* event) { name_selected = "None"; return false; }

    /*! \brief Destructor.
     */
    virtual ~selection_dialog() { ; }
        
protected:
    /*! \brief Holds the name of the currently selected machine name. */
    Glib::ustring name_selected;
    
    /*! \brief Maps each machine name to the RadioButton that represents it. */    
    std::map<string, Gtk::RadioButton *> radio_buttons;    
    
    /*! \brief The OK button of the dialog. */        
    Gtk::Button ok_button;

    /*! \brief The Cancel button of the dialog. */        
    Gtk::Button cancel_button;

    /*! \brief The VBox, that holds all Widgets of this window. */
    Gtk::Box global_vbox;    
    
    /*! \brief The HButtonBox, that holds the "OK" and "Cancel" buttons. */    
    //Gtk::HButtonBox button_hbox;
    Gtk::Box button_hbox;
    
    /*! \brief Group identifier for the RadioButtons.  */            
    Gtk::RadioButton::Group group; 
    
    /*! \brief A box in wich all the RadioButtons are stacked upon each other. */    
    Gtk::Box radio_box;   

    /*! \brief Holds the application icon. */    
    Glib::RefPtr<Gdk::Pixbuf> enigma_icon;
    
    /*! \brief Points to the application object in use. */    
    Gtk::Main *m;

    /*! \brief An image widget that holds a picture of a rotor machine. */        
    Gtk::Image machine_pic;    
    
    /*! \brief Pixbuf used for selection_dialog::machine_pic. */            
    Glib::RefPtr<Gdk::Pixbuf> enigma_pixbuf;    
        
    /*! \brief Pixbuf used for selection_dialog::machine_pic. */            
    Glib::RefPtr<Gdk::Pixbuf> typex_pixbuf;    
    
    /*! \brief HBox holding the radio_box the image_box. */                
    Gtk::Box pic_box;

    /*! \brief HBox holding the the Image and its description. */                
    Gtk::Box image_box;
    
    /*! \brief Holds the descripton of the picture shown in the dialog. */                    
    Gtk::Label pic_label;
};

#endif /* __selection_dialog_h__ */

