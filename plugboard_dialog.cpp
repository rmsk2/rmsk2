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

/*! \file plugboard_dialog.cpp
 *  \brief Implements application classes that provide a dialog that lets the user select an involution.
 *         Based on that it additionally implements a dialog which can be used to configure an Enigma's plugboard.
 */

#include<plugboard_dialog.h>

#include<iostream>
#include<boost/lexical_cast.hpp>

void select_involution_dialog::setup(Gtk::Dialog *d, Glib::RefPtr<Gtk::Builder> r, Glib::ustring& name_postfix)
{
    dialog = d; 
    ref_xml = r;
    name_post_fix = name_postfix;
    plug_count_label_prefix = "Number of plugs: ";

    // Retrieve widgets from Dialog d: Here delete button, delete all button, ok button
    Gtk::Button *button_delete = NULL;
    ref_xml->get_widget("delete_plug_button" + name_postfix, button_delete);
    Gtk::Button *button_delete_all = NULL;
    ref_xml->get_widget("delete_all_plugs_button" + name_postfix, button_delete_all);
    Gtk::Button *ok_button = NULL;
    ref_xml->get_widget("plugboard_ok" + name_postfix, ok_button);    

    // Retrieve widgets from Dialog d: Here label for counting the character pairs    
    ref_xml->get_widget("plugcount_label" + name_postfix, plugcount_label);   
    
    last_button_toggled = " ";

    // Clear all checkboxes and "ungray" them
    for (char count = 'A'; count <= 'Z'; count++)
    {
        Gtk::CheckButton *b = NULL;
        ref_xml->get_widget("checkbutton_" + to_ustr(count) + name_postfix, b);
        b->set_active(false); // clear
        b->set_sensitive(true); // "ungray"
    }
    
    num_plugs = 0;
    fill_stecker_model();
    
    // Connect clicked signals for delete and delete all buttons
    delete_connection = button_delete->signal_clicked().connect(sigc::mem_fun(*this, &select_involution_dialog::on_delete_clicked));
    delete_all_connection = button_delete_all->signal_clicked().connect(sigc::mem_fun(*this, &select_involution_dialog::on_delete_all_clicked));
    
    
    // Connect clicked signals for character checkboxes
    for (char count = 'A'; count <= 'Z'; count++)
    {
        Gtk::CheckButton *b = NULL;
        ref_xml->get_widget("checkbutton_" + to_ustr(count) + name_postfix, b);
        check_button_connections[b] = b->signal_clicked().connect(sigc::bind<Glib::ustring>( sigc::mem_fun(*this, &select_involution_dialog::on_plug_changed), to_ustr(count)));
    }
    
    update_plug_counter();
    d->set_focus(*ok_button);
}

select_involution_dialog::select_involution_dialog(Gtk::Dialog *d, Glib::RefPtr<Gtk::Builder> r, vector<pair<char, char> >& steckers)
    : current_steckers(steckers)
{
    Glib::ustring postfix;
    
    setup(d, r, postfix);
}

select_involution_dialog::select_involution_dialog(Gtk::Dialog *d, Glib::RefPtr<Gtk::Builder> r, vector<pair<char, char> >& steckers, Glib::ustring& name_postfix) 
    : current_steckers(steckers) 
{ 
    setup(d, r, name_postfix);
}    

void select_involution_dialog::insert_stecker_in_model(Glib::ustring first, Glib::ustring second)
{
    Glib::RefPtr<Gtk::ListStore> m;
    Gtk::TreeView *t = NULL;
    ref_xml->get_widget("stecker_treeview" + name_post_fix, t);
    
    set_plug_state(first, second, false, true);
    
    m = m.cast_dynamic(t->get_model());
    Gtk::ListStore::iterator iter = m->append();
    (*iter)[plugboard_cols.first] = first;
    (*iter)[plugboard_cols.second] = second;  
    num_plugs++;
    update_plug_counter();          
}

void select_involution_dialog::set_plug_state(Glib::ustring first, Glib::ustring second, bool is_sensitive, bool is_active)
{
    Gtk::CheckButton *check_first = NULL;
    // Retrieve affected checkbox widgets
    ref_xml->get_widget("checkbutton_" + first + name_post_fix, check_first);
    Gtk::CheckButton *check_second = NULL;
    ref_xml->get_widget("checkbutton_" + second + name_post_fix, check_second);
    
    // Block signals from the affected checkboxes
    check_button_connections[check_first].block();
    check_button_connections[check_second].block();
    
    // Check or Uncheck affected checkboxes        
    check_first->set_active(is_active);
    check_second->set_active(is_active);
    
    // "Gray" or "Ungray" affected checkboxes
    check_first->set_sensitive(is_sensitive);
    check_second->set_sensitive(is_sensitive);        

    // Unblock signals from the affected checkboxes    
    check_button_connections[check_first].unblock();
    check_button_connections[check_second].unblock();            
}

void select_involution_dialog::fill_stecker_model()
{
    Gtk::TreeView *t = NULL;
    ref_xml->get_widget("stecker_treeview" + name_post_fix, t);
    Glib::RefPtr<Gtk::ListStore> m = Gtk::ListStore::create(plugboard_cols);    
    vector<pair<char, char> >::iterator stecker_iter;
    
    t->set_model(m);
    
    for (stecker_iter = current_steckers.begin(); stecker_iter != current_steckers.end(); ++stecker_iter)
    {
        insert_stecker_in_model(to_ustr(stecker_iter->first), to_ustr(stecker_iter->second));
    }        
}

void select_involution_dialog::on_plug_changed(Glib::ustring plug_name)
{ 
    if (last_button_toggled != " ")
    {
        // First checkbox of a plug/character pair has been clicked 
        // so this click may comlete the plug if the checkbox that was
        // clicked is different from last_button_toggled
        if (plug_name == last_button_toggled)
        {
            // Plug/Pair not completed. 
            last_button_toggled = " ";
        }
        else
        {
            // Plug/Pair completed. 
            insert_stecker_in_model(last_button_toggled, plug_name);
            last_button_toggled = " ";
        }
    }
    else
    {
        // This is the first checkbox of a plug/character pair. Store it.
        last_button_toggled = plug_name;
    }
}

void select_involution_dialog::update_plug_counter()
{
    plugcount_label->set_text(plug_count_label_prefix + boost::lexical_cast<string>(num_plugs));
}

void  select_involution_dialog::on_delete_all_clicked()
{
    Glib::RefPtr<Gtk::ListStore> m;
    Gtk::TreeView *t = NULL;
    ref_xml->get_widget("stecker_treeview" + name_post_fix, t);
    Gtk::TreeModel::Children::iterator iter;
    
    m = m.cast_dynamic(t->get_model());
    Gtk::TreeModel::Children children = m->children();
    
    // Clear and "ungray" checkboxes that are associated with the plugs/character pairs that are
    // currently contained in  "stecker_treeview".
    for (iter = children.begin(); iter != children.end(); ++iter)
    {
        remove_plug((*iter)[plugboard_cols.first], (*iter)[plugboard_cols.second]);   
    }

    num_plugs = 0;
    update_plug_counter();

    // Clear Treeview
    m->clear();
}

select_involution_dialog::~select_involution_dialog()
{
    map<Gtk::CheckButton *, sigc::connection>::iterator iter;
    
    // Remove connections to checkbuttons
    for (iter = check_button_connections.begin(); iter != check_button_connections.end(); ++iter)
    {
        iter->second.disconnect();
    }
    
    // Remove connections to buttons
    delete_connection.disconnect();
    delete_all_connection.disconnect();
}

/* ------------------------------------------------------------------------------- */

plugboard_dialog::plugboard_dialog(Gtk::Dialog *d, Glib::RefPtr<Gtk::Builder> r, vector<pair<char, char> >& steckers, bool& use_uhr, unsigned int& uhr_dial_pos, bool uhr_capable)
    : select_involution_dialog(d, r, steckers), uhr_state(use_uhr), uhr_pos(uhr_dial_pos)
{
    uhr_state_temp = use_uhr;
    
    // Retrieve widgets managing the Enigma Uhr from Dialog d:   
    Gtk::CheckButton *uhr_check_button = NULL;
    ref_xml->get_widget("uhrcheckbutton", uhr_check_button);
    Gtk::VScale *uhr_scale = NULL;
    ref_xml->get_widget("uhrscale", uhr_scale);        

    // Connect to checkbox that switches Enigma Uhr on and off
    uhr_connection = uhr_check_button->signal_clicked().connect(sigc::mem_fun(*this, &plugboard_dialog::on_uhr_change));
    
    // Initialize values of Uhr widgets
    uhr_scale->set_value((double)uhr_dial_pos);
    uhr_check_button->set_active(uhr_state_temp);
    uhr_check_button->set_sensitive(uhr_capable);
    update_uhr_state(uhr_state_temp);
}

void plugboard_dialog::update_uhr_state(bool current_state)
{
    Gtk::VScale *uhr_scale = NULL;
    ref_xml->get_widget("uhrscale", uhr_scale);
    
    // "Gray" or ungray checkbox that can be used to switch Uhr on and off
    uhr_scale->set_sensitive(current_state);
    
    // When Enigma Uhr is deactived set dial position to 0
    if (!current_state)
    {
        uhr_scale->set_value(0.0);
    }
}

void plugboard_dialog::on_uhr_change()
{
    Gtk::CheckButton *uhr_check_button = NULL;
    ref_xml->get_widget("uhrcheckbutton", uhr_check_button);
    
    uhr_state_temp = uhr_check_button->get_active();
    update_uhr_state(uhr_state_temp);
}

void  plugboard_dialog::on_delete_clicked()
{
    Glib::RefPtr<Gtk::ListStore> m;
    Gtk::TreeView *t = NULL;
    ref_xml->get_widget("stecker_treeview" + name_post_fix, t);
    Glib::RefPtr<Gtk::TreeSelection> selection = t->get_selection();
    
    if (selection->count_selected_rows() != 0)
    {
        m = m.cast_dynamic(t->get_model());
        Gtk::TreeModel::iterator iter = selection->get_selected(); // Find the currently selected plug/character pair
    
        // Uncheck and "Ungray" checkboxes
        remove_plug((*iter)[plugboard_cols.first], (*iter)[plugboard_cols.second]);
        num_plugs--;
        update_plug_counter();
        m->erase(iter); // Finally delete the pair
    }
    else
    {
        Gtk::MessageDialog msg(*dialog, "Nothing selected to delete.", false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK, true);
        msg.set_title("Enigma");
        msg.run();        
    }

}

int plugboard_dialog::run()
{ 
    Glib::RefPtr<Gtk::ListStore> m;
    Gtk::TreeView *t = NULL;
    ref_xml->get_widget("stecker_treeview" + name_post_fix, t);
    Gtk::VScale *uhr_dial = NULL;
    ref_xml->get_widget("uhrscale", uhr_dial);
    Gtk::TreeModel::Children::iterator iter;
    int temp, result = 42;
    Glib::ustring first_help, second_help; 
    vector<pair<char, char> > steckers_temp;

    do
    {
        temp = dialog->run();
        
        steckers_temp.clear();
            
        if (temp == 0)
        {
            // OK was clicked            
            m = m.cast_dynamic(t->get_model());
            Gtk::TreeModel::Children children = m->children();
            
            // Retrieve character pairs/plugs selected by the user and store them in steckers_temp
            // Iterate over character pairs in TreeView
            for (iter = children.begin(); iter != children.end(); ++iter)
            {
                pair<char, char> help;
                first_help = (*iter)[plugboard_cols.first];
                second_help = (*iter)[plugboard_cols.second];
                
                help.first = tolower((char)(first_help[0]));
                help.second = tolower((char)(second_help[0]));
                
                steckers_temp.push_back(help);                
            }
            
            //Uhr used?
            if (uhr_state_temp)
            {
                // Yes!
                // When using the Uhr exactly 10 plugs have to be used
                if (steckers_temp.size() == 10)
                {
                    // Correct number of plugs -> set output variables
                    uhr_pos = (unsigned int)uhr_dial->get_value();
                    uhr_state = uhr_state_temp;
                    current_steckers = steckers_temp;
                    result = temp;                    
                }
                else
                {
                    // More or less than 10 plugs -> error message
                    Gtk::MessageDialog msg(*dialog, "When using the Uhr exactly 10 plugs must be used.", false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK, true);
                    msg.set_title("Enigma");
                    msg.run();                
                }
            }
            else
            {
                // No! Uhr not used
                // Set output variables                
                uhr_pos = (unsigned int)uhr_dial->get_value();
                uhr_state = uhr_state_temp;                                
                current_steckers = steckers_temp;
                result = temp;
            }                    
        }
        else
        {
            // Cancel was clicked
            result = temp;
        }
        
    }  while(result == 42);
    
    dialog->hide(); 
    
    return result; 
}

plugboard_dialog::~plugboard_dialog()
{
    uhr_connection.disconnect();
}



