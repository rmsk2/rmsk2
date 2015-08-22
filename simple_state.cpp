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

#include<iostream>
#include<gtkmm.h>
#include<rmsk_globals.h>
#include<selection_dialog.h>
#include<simple_state.h>
#include<Enigma.xpm>

/*! \file simple_state.cpp
 *  \brief This file contains the the implementation for a class that provides a simplistic facility to store the state of an application
 *         like for instance the least recently used directory name.
 */

using namespace std;

simple_state::simple_state(const char *file_name)
{
    ini_name = Glib::get_home_dir() + "/." + Glib::ustring(file_name);
    last_dir = Glib::get_home_dir();
    pos_x = 0;
    pos_y = 0;
}

void simple_state::load()
{
    Glib::KeyFile ini_file;
    bool result = false;
    
    try
    {
        result = !ini_file.load_from_file(ini_name);
        
        if (!result)        
        {
            if (ini_file.has_key("state", "lastdir"))
            {
                last_dir = ini_file.get_string("state", "lastdir");
            }
            
            if (ini_file.has_key("state", "posx") && ini_file.has_key("state", "posy"))
            {
                pos_x = ini_file.get_integer("state", "posx");
                pos_y = ini_file.get_integer("state", "posy");                                
            }            
        }        
    }
    catch(...)
    {
        cout << "error loading config" << endl;
    }
}

void simple_state::save()
{
    Glib::KeyFile ini_file;        
    Glib::ustring ini_data;
    bool result = false;
    
    try
    { 
        // create ini file data       
        ini_file.set_string("state", "lastdir", last_dir);
        ini_file.set_integer("state", "posx", pos_x);
        ini_file.set_integer("state", "posy", pos_y);        
        ini_data = ini_file.to_data();        
        
        // save ini file 
        Glib::RefPtr<Glib::IOChannel> out_file = Glib::IOChannel::create_from_file(ini_name.raw(), "w");    
        result = out_file->write(ini_data) != Glib::IO_STATUS_NORMAL;
        out_file->close();        
    }
    catch(...)
    {
        result = true;
    }
    
    if (result)
    {
        cout << "error saving config" << endl;    
    }
}

simulator_app::simulator_app(const char *state_base_name, set<string>& valid_names, bool use_enigma_pic)
    : app_state(state_base_name)
{
    win = NULL;
    allowed_names = valid_names;
    enigma_icon = Gdk::Pixbuf::create_from_xpm_data(Enigma_xpm); 
    do_stop = false;   
    show_enigma_pic = use_enigma_pic;
}

int simulator_app::run(int argc, char **argv, Gtk::Main *m)
{   
    if (argc < 2)
    {
        // No arguments gave been specified -> Present selection dialog
        selection_dialog sel_dialog(allowed_names, m, show_enigma_pic);
        m->run(sel_dialog);
        m_name = sel_dialog.get_selected_name();
    }
    else
    {
        // First argument has to specify the machine name as defined in allowed_names
        m_name = Glib::ustring(argv[1]);     
    }
    
    do_stop = (m_name == "None");
    
    if (!do_stop)
    {
        // User selected a machine variant and did not click on the cancel button
        
        init(argc, argv);
        
        win->set_icon(enigma_icon);        
        
        app_state.load();        
        restore_state();
 
        // Enter message loop       
        m->run(*win);

        determine_state();                
        app_state.save();        
    }
    
    rmsk::clean_up();    
    
    return 0;
}
