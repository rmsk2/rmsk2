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

/*! \file randomizer_dialog.cpp
 *  \brief Implementation for the application class that provides the the randomize dialog of rotorvis and enigma.
 */

#define DEFAULT_CHOICE "Default"

#include<randomizer_dialog.h>

randomizer_dialog::randomizer_dialog(Gtk::Window& parent, string& randomizer_parameter, vector<randomizer_descriptor>& param_choices)
    : Dialog("Randomize settings", parent, true), parameter(randomizer_parameter), param_label("Possible randomizer parameters"), empty_label("")
{
    add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
    ok_button = add_button(Gtk::Stock::OK, Gtk::RESPONSE_OK);
    int size_x, size_y;
    vector<randomizer_descriptor>::iterator iter;
    
    Gtk::Box *v_box = get_vbox();    
    v_box->pack_start(randomizer_items);
    randomizer_items.attach(param_label, 0, 0, 1, 1);
    
    // Initialize combo box with randomizer parameters
    randomizer_items.attach(param_combo, 1, 0, 1, 1);
    param_combo.set_hexpand(true);
    param_combo.get_size_request(size_x, size_y);
    param_combo.set_size_request(size_x + 250, size_y);
    
    param_combo.append(DEFAULT_CHOICE);
    description_keyword_mapping[DEFAULT_CHOICE] = "";
    
    for (iter = param_choices.begin(); iter != param_choices.end(); ++iter)
    {
        param_combo.append(iter->description);
        description_keyword_mapping[iter->description] = iter->keyword;
    }
        
    param_combo.set_active_text(DEFAULT_CHOICE);
    
    randomizer_items.attach(empty_label, 0, 1, 1, 1);    
    
    randomizer_items.set_column_spacing(5);
    randomizer_items.set_row_spacing(4);
    ok_button->set_can_default(true);
    ok_button->grab_default();
    ok_button->grab_focus();
    ok_button->signal_clicked().connect(sigc::mem_fun(*this, &randomizer_dialog::on_ok_clicked));
    
    show_all();
}


void randomizer_dialog::on_ok_clicked()
{
    parameter = description_keyword_mapping[param_combo.get_active_text()];
}

