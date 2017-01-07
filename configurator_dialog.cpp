/***************************************************************************
 * Copyright 2017 Martin Grap
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

/*! \file configurator_dialog.cpp
 *  \brief Implementation for the application class that provide the configuration dialog of rotorvis.
 */

#include<configurator_dialog.h>

configurator_dialog::configurator_dialog(Gtk::Window& parent, vector<key_word_info>& infos, std::map<string, string>& config_data)
    : Dialog("Configuration", parent, true), inf(infos), conf_data(config_data)
{
    add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
    ok_button = add_button(Gtk::Stock::OK, Gtk::RESPONSE_OK);    
    vector<key_word_info>::iterator iter;
    int count = 0, size_x, size_y;
    
    Gtk::Box *v_box = get_vbox();    
    v_box->pack_start(config_items);
    
    // For each keyword/config parameter a line containing a label and an entry or checkbutton is
    // added to the grid config_items.
    for (iter = infos.begin(); iter != infos.end(); ++iter)
    {
        // Add label
        Gtk::Label *l = manage(new Gtk::Label(iter->descriptive_text));
        config_items.attach(*l, 0, count, 1, 1);
        
        if (iter->type == KEY_STRING)
        {
            // Add Entry            
            Gtk::Entry *e = new Gtk::Entry();
            e->set_text(config_data[iter->keyword]);
            e->set_hexpand(true);
            e->get_size_request(size_x, size_y);
            e->set_size_request(size_x + 250, size_y);
            string_items[iter->keyword] = e;
            config_items.attach(*manage(e), 1, count, 1, 1);
        }
        else /* KEY_BOOL */
        {
            // Add CheckButton
            Gtk::CheckButton *c = new Gtk::CheckButton();
            bool_items[iter->keyword] = c;            
            c->set_active(config_data[iter->keyword] == CONF_TRUE);
            config_items.attach(*manage(c), 1, count, 1, 1);                    
        }
        
        count++; // line count
    }
    
    // Add an additional empty label to get some space between the configuraton controls and
    // the dialog buttons
    Gtk::Label *l = manage(new Gtk::Label(""));
    config_items.attach(*l, 0, count, 1, 1);
    
    
    config_items.set_column_spacing(5);
    config_items.set_row_spacing(4);
    ok_button->set_can_default(true);
    ok_button->grab_default();
    ok_button->grab_focus();
    ok_button->signal_clicked().connect(sigc::mem_fun(*this, &configurator_dialog::on_ok_clicked));
    
    show_all();
}


void configurator_dialog::on_ok_clicked()
{
    vector<key_word_info>::iterator iter;
    
    // Iterate over all known config parameters/keywords
    for (iter = inf.begin(); iter != inf.end(); ++iter)
    {
        if (iter->type == KEY_STRING)
        {
            // Extract current value of string parameter
            conf_data[iter->keyword] = string_items[iter->keyword]->get_text();
        }
        else /* KEY_BOOL */
        {                        
            // Extract current value of bool parameter       
            if (bool_items[iter->keyword]->get_active())
            {
                conf_data[iter->keyword] = CONF_TRUE;
            }
            else
            {
                conf_data[iter->keyword] = CONF_FALSE;
            }        
        }
    }
}

