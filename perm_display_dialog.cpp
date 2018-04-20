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

/*! \file perm_display_dialog.cpp
 *  \brief Implementation for the application class that allows to display the permutations currently in use in a rotor machine.
 */

#include<perm_display_dialog.h>

perm_display_dialog::perm_display_dialog(Gtk::Window& parent, vector<Glib::ustring>& permutation_msgs)
    : Dialog("Show rotor permutations", parent, true)
{
    auto buttons = get_action_area();
    buttons->set_layout(Gtk::ButtonBoxStyle::BUTTONBOX_EXPAND);
    
    ok_button = add_button(Gtk::Stock::OK, Gtk::RESPONSE_OK);
    
    scroll_bar.add(printer);
    Gtk::Box *v_box = get_content_area();
    scroll_bar.set_size_request(480, 450);
    
    printer.set_editable(false);
    printer.set_cursor_visible(false);
    printer.set_can_focus(true);
    printer.override_font(Pango::FontDescription("monospace"));     
     
    Glib::ustring message_text;
    
    for (const auto &count : permutation_msgs)
    {
        message_text += count + "\n";
    }    
    
    printer.get_buffer()->set_text(message_text);
    
    v_box->pack_start(scroll_bar);    
    
    ok_button->set_can_default(true);
    ok_button->grab_default();
    ok_button->grab_focus();
    
    show_all();
}
