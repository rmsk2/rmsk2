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

/*! \file rotorpos_dialog.cpp
 *  \brief Implementation for the application class that provides the rotor positions dialog of rotorvis and enigma.
 */

#include<rotorpos_dialog.h>

rotorpos_dialog::rotorpos_dialog(Gtk::Window& parent, Glib::ustring& rotor_positions)
    : Dialog("Rotor positions", parent, true), pos_label("Positions"), empty_label(""), positions(rotor_positions)
{
    add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
    ok_button = add_button(Gtk::Stock::OK, Gtk::RESPONSE_OK);    
    vector<key_word_info>::iterator iter;
    int size_x, size_y;
    
    Gtk::Box *v_box = get_vbox();    
    v_box->pack_start(config_items);
    config_items.attach(pos_label, 0, 0, 1, 1);
    
    // Set up pos_entry
    config_items.attach(pos_entry, 1, 0, 1, 1);
    pos_entry.set_hexpand(true);
    pos_entry.get_size_request(size_x, size_y);
    pos_entry.set_size_request(size_x + 250, size_y);    
    pos_entry.set_text(positions);
    
    config_items.attach(empty_label, 0, 1, 1, 1);    
    
    config_items.set_column_spacing(5);
    config_items.set_row_spacing(4);
    ok_button->set_can_default(true);
    ok_button->grab_default();
    ok_button->grab_focus();
    ok_button->signal_clicked().connect(sigc::mem_fun(*this, &rotorpos_dialog::on_ok_clicked));
    
    show_all();
}


void rotorpos_dialog::on_ok_clicked()
{
    positions = pos_entry.get_text();
}

