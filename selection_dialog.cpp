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

/*! \file selection_dialog.cpp
 *  \brief Implementation for the selection_dialog class.
 */

#include<selection_dialog.h>
#include<Enigma.xpm>
#include<services_enigma.xpm>
#include<typex.xpm>

selection_dialog::selection_dialog(set<string>& machine_names, Gtk::Main *main, bool show_enigma_pic)
    : global_vbox(Gtk::ORIENTATION_VERTICAL, 4), button_hbox(Gtk::ORIENTATION_HORIZONTAL, 0), radio_box(Gtk::ORIENTATION_VERTICAL, 0), 
      pic_box(Gtk::ORIENTATION_HORIZONTAL, 8), image_box(Gtk::ORIENTATION_VERTICAL, 0)
{
    set<string>::iterator iter;   
    m = main; 
    
    enigma_pixbuf = Gdk::Pixbuf::create_from_xpm_data(services_enigma_xpm);
    typex_pixbuf = Gdk::Pixbuf::create_from_xpm_data(typex_xpm);

    if (show_enigma_pic)
    {
        machine_pic.set(enigma_pixbuf);
        pic_label.set_text("A Services Enigma");
        
    }
    else
    {
        machine_pic.set(typex_pixbuf);
        pic_label.set_text("A late model Typex");        
    }

    
    enigma_icon = Gdk::Pixbuf::create_from_xpm_data(Enigma_xpm); 
    set_icon(enigma_icon);
    set_position(Gtk::WIN_POS_CENTER);
    set_title("Machine to simulate");
    ok_button.set_label("OK");      
    cancel_button.set_label("Cancel");
    
    // Create a radio button for each machine name that can be selected and place them in VBox radio_box    
    for (iter = machine_names.begin(); iter != machine_names.end(); ++iter)
    {
        radio_buttons[*iter] = manage(new Gtk::RadioButton(group, *iter));
        radio_box.pack_start(*radio_buttons[*iter], false, false, 0);
        radio_buttons[*iter]->signal_clicked().connect(sigc::bind<Glib::ustring>( sigc::mem_fun(*this, &selection_dialog::on_radio_clicked), *iter));
    }
    
    iter = machine_names.begin();
    
    if (iter != machine_names.end())
    {
        name_selected = *iter; // initialize member that can be used to retrieve the selected name
        radio_buttons[*iter]->set_active(true);
    }

    // Place buttons in their Button box
    button_hbox.pack_start(cancel_button, true, true, 1);
    button_hbox.pack_start(ok_button, true, true, 1);
    
    // Place radio buttons and machine_pic in pic_box    
    pic_box.pack_start(radio_box, true, true, 4);
    pic_box.pack_start(image_box, true, true, 4);            
    
    image_box.pack_start(machine_pic, true, true, 4);        
    image_box.pack_start(pic_label, false, false, 0);            
    
    // Put the pic_box in the global vbox
    global_vbox.pack_start(pic_box, true, true, 4);
    // Put the box containing the OK and Cancel buttons in the global vbox
    global_vbox.pack_start(button_hbox, false, false, 1);     
    // Add the global vbox to the window
    add(global_vbox);

    // Setup OK button     
    ok_button.set_can_default(true);
    ok_button.grab_default();
    ok_button.grab_focus();
    ok_button.signal_clicked().connect(sigc::mem_fun(*this, &selection_dialog::on_ok_clicked));
    
    // Connect signals for handling the events when the user clicks Cancel or simply closes the window
    cancel_button.signal_clicked().connect(sigc::mem_fun(*this, &selection_dialog::on_cancel_clicked)); 
    signal_delete_event().connect(sigc::mem_fun(*this, &selection_dialog::on_delete));    
    
    set_resizable(false);
    show_all();
}

