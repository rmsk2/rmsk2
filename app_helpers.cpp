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

/*! \file app_helpers.cpp
 *  \brief This file contains the implementation of a set of classes that know how to handle GUI events that occur in rotorvis
 *         and Enigma.
 */

#include<algorithm>
#include<app_helpers.h>
#include<rmsk_globals.h>
#include<Enigma.xpm>
#include<rotorpos_dialog.h>
#include<randomizer_dialog.h>

void menu_helper::message_dialog(Glib::ustring& message, Gtk::MessageType type)
{
    Gtk::MessageDialog msg(*win, message, false, type, Gtk::BUTTONS_OK, true);
    msg.set_title(app_name);
    msg.run();                
}

/* ------------------------------------------------------------------ */

const char *apache_license = "Copyright 2018 Martin Grap\n"
    "\n"
    "Licensed under the Apache License, Version 2.0 (the \"License\");\n"
    "You may obtain a copy of the License at\n"
    "\n"
    "http://www.apache.org/licenses/LICENSE-2.0\n"
    "\n"
    "Unless required by applicable law or agreed to in writing, software\n"
    "distributed under the License is distributed on an \"AS IS\" BASIS,\n"
    "WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.\n"
    "See the License for the specific language governing permissions and\n"
    "limitations under the License.\n"
    "\n";

void help_menu_helper::set_text(const char *help_file, const char *comment, const char *version, const char *author)
{
    help_file_name = Glib::ustring(help_file);
    about_comment = Glib::ustring(comment);
    about_version = Glib::ustring(version);
    about_author = Glib::ustring(author);        
}

void help_menu_helper::on_help_activate()
{
    if (!Gio::AppInfo::launch_default_for_uri("ghelp://" + rmsk::get_doc_path() + help_file_name))
    {
        error_message("Unable to display help file");
    }                    
}

void help_menu_helper::on_about_activate()
{
    Gtk::AboutDialog about;
    vector<Glib::ustring> authors;
    Glib::RefPtr<Gdk::Pixbuf> enigma_icon = Gdk::Pixbuf::create_from_xpm_data(Enigma_xpm);
    
    authors.push_back(about_author);
    
    about.set_authors(authors);
    about.set_comments(about_comment);
    about.set_name(app_name);
    about.set_version(about_version);
    about.set_transient_for(*win);
    about.set_license(apache_license);
    about.set_copyright("Copyright 2018 " + about_author);
    about.set_logo(enigma_icon);
    
    about.run();
}

/* ------------------------------------------------------------------ */

void file_operations_helper::on_save_rotor_set_activate(rotor_machine *the_machine, rotor_machine *index_machine)
{
    Gtk::FileChooserDialog file_dialog(*win, "Save default rotor set", Gtk::FILE_CHOOSER_ACTION_SAVE);
    Glib::RefPtr<Gtk::FileFilter> filter = Gtk::FileFilter::create();
    Glib::ustring temp_file_name;
    
    filter->add_pattern("*.ini");
    filter->set_name("Rotor set file");
    file_dialog.add_filter(filter);
    file_dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
    file_dialog.add_button(Gtk::Stock::OK, Gtk::RESPONSE_OK);
    file_dialog.set_transient_for(*win); 
    string default_set_name = the_machine->get_default_set_name(); 
    string chosen_file_name;   
    bool save_failed = false;

    // If last_dir is not empty then set the file selection dialog to open this directory.
    if ((*last_dir) != "")
    {
        file_dialog.set_current_folder(*last_dir);
    }    

    if (file_dialog.run() == Gtk::RESPONSE_OK)
    {
        temp_file_name = file_dialog.get_filename();
        
        if (temp_file_name.rfind(".ini") == Glib::ustring::npos)
        {
            temp_file_name = temp_file_name + ".ini";
        }
        
        file_dialog.hide();
        
        chosen_file_name = temp_file_name;
        
        // Save default rotor set used in the_machine
        save_failed = the_machine->get_rotor_set(default_set_name)->save(chosen_file_name);
        
        // Save index rotor set if index_machine is not NULL
        if ((!save_failed) and (index_machine != NULL))
        {
            default_set_name = index_machine->get_default_set_name();
            chosen_file_name = chosen_file_name.substr(0, chosen_file_name.length() - 4) + "_index.ini";
            save_failed = index_machine->get_rotor_set(default_set_name)->save(chosen_file_name);
        }

        if (save_failed)
        {
            error_message("Saving rotor set data failed");
        }
        else
        {
            *last_dir = file_dialog.get_current_folder();
            info_message("rotor set data successfully saved");
        }                        
    }
    else
    {
        file_dialog.hide();
    }
}


void file_operations_helper::on_file_open()
{
    Glib::ustring temp_file_name;
    Gtk::FileChooserDialog file_dialog(*win, "Open Settings file");
    Glib::RefPtr<Gtk::FileFilter> filter = Gtk::FileFilter::create();
    
    filter->add_pattern("*.ini");
    filter->set_name("Simulator settings file");
    file_dialog.add_filter(filter);
    file_dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
    file_dialog.add_button(Gtk::Stock::OK, Gtk::RESPONSE_OK);
    file_dialog.set_transient_for(*win);    
    
    // If last_dir is not empty then set the file selection dialog to open this directory.
    if ((*last_dir) != "")
    {
        file_dialog.set_current_folder(*last_dir);
    }    
    
    if (file_dialog.run() == Gtk::RESPONSE_OK)
    {
        temp_file_name = file_dialog.get_filename();
        file_dialog.hide();
        
        if (load_settings(temp_file_name))
        {
            error_message("Loading settings file failed");
        }
        else
        {
            // Update state variables
            *last_file_opened = temp_file_name;
            *last_dir = file_dialog.get_current_folder();
            info_message("Settings successfully loaded");
        }
    }
    else
    {    
        file_dialog.hide();
    }
}

void file_operations_helper::on_file_save()
{
    Glib::ustring help = *last_file_opened;

    on_do_save(help);
}

void file_operations_helper::on_file_save_as()
{
    on_do_save("");
}

void file_operations_helper::on_do_save(const Glib::ustring& desired_file_name)
{
    Gtk::FileChooserDialog file_dialog(*win, "Save Settings file", Gtk::FILE_CHOOSER_ACTION_SAVE);
    Glib::RefPtr<Gtk::FileFilter> filter = Gtk::FileFilter::create();
    Glib::ustring temp_file_name;
    
    filter->add_pattern("*.ini");
    filter->set_name("Simulator settings file");
    file_dialog.add_filter(filter);
    file_dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
    file_dialog.add_button(Gtk::Stock::OK, Gtk::RESPONSE_OK);
    file_dialog.set_transient_for(*win);     

    // If last_dir is not empty then set the file selection dialog to open this directory.    
    if ((*last_dir) != "")
    {
        file_dialog.set_current_folder(*last_dir);
    }
    
    if (desired_file_name == "")
    {
        // If desired_file_name is empty then display file selection dialog. "Save settings as ..."
        if (file_dialog.run() == Gtk::RESPONSE_OK)
        {
            temp_file_name = file_dialog.get_filename();
            
            if (temp_file_name.rfind(".ini") == Glib::ustring::npos)
            {
                temp_file_name = temp_file_name + ".ini";
            }
            
            file_dialog.hide();

            if (save_settings(temp_file_name))
            {
                error_message("Saving settings file failed");
            }
            else
            {
                // Update state variables.                
                *last_file_opened = temp_file_name;
                *last_dir = file_dialog.get_current_folder();                
                info_message("Settings successfully saved");
            }                        
        }
        else
        {
            file_dialog.hide();
        }
    }
    else
    {
        // If desired_file_name is not empty then use this filename and do not display file selection dialog. "Save settings ..."
        temp_file_name = desired_file_name;
        
        if (save_settings(temp_file_name))
        {
            error_message("Saving settings file failed");
        }
        else
        {
            // Updating state variables is not necessary as the have not changed.
            info_message("Settings successfully saved");
        }
    }
}

/* ------------------------------------------------------------------ */

void clipboard_helper::process_clipboard()
{
    timer_conn = Glib::signal_timeout().connect(sigc::mem_fun(*this, &clipboard_helper::timer_func), 50);
}

/*! Under X11 there is more than one clipboard. This method makes use of the "normal" clipboard which can be filled
 *  by using CTRL+C and the primary clipboard which contains the currently selected text. This method makes use of both.
 *  Output is always provided in the "normal" clipboard. Input is first read from "primary" clipboard. If this clipboard
 *  is empty then the "normal" clipboard is queried.
 */
bool clipboard_helper::timer_func()
{
    Glib::ustring clip_data, result_data;
    storage_logger storage(result_data);
    sigc::connection conn;
    // Set ref_clipboard to the primary clipboard
    Glib::RefPtr<Gtk::Clipboard> ref_clipboard = Gtk::Clipboard::get(GDK_SELECTION_PRIMARY);
    
    clip_data = ref_clipboard->wait_for_text();
    
    // Switch to the "normal" clipboard
    ref_clipboard = Gtk::Clipboard::get();    
    
    if (clip_data == "")
    {
        // If the "primary" clipboard did not contain usable text query the "normal" clipboard
        clip_data = ref_clipboard->wait_for_text();
    }
    
    clip_data = clip_data.uppercase();

    // Set up things in such a way that the storage_logger object storage is called back for each output character produced
    // by the rotor machine. The string result_data then contains the produced output. 
    conn = simulator_gui->get_key_board()->signal_output_char().connect(sigc::mem_fun(storage, &storage_logger::report_char));
    
    // Send input characters through rotor machine
    for_each(clip_data.begin(), clip_data.end(), [this](gunichar c){ simulator_gui->simulate_key_press(c); });
    
    // Disconnect our storage_logger again.
    conn.disconnect();

    // Set "normal" clipboard to output produced by rotor machine.    
    ref_clipboard->set_text(result_data);
    
    info_message("Processing result stored in clipboard");
        
    return false;        
}

/* ------------------------------------------------------------------ */

void log_helper::set_simulator(display_dialog *d, rotor_draw *simulator)
{
    disp = d;
    simulator_gui = simulator;
    
    out_log = disp->connect_output(simulator_gui->get_key_board()->signal_output_char());
    in_log = disp->connect_input(simulator_gui->get_key_board()->signal_input_char());
    out_log.block();
    in_log.block();    
}

void log_helper::display_log_window(bool is_visible)
{
    int current_width, current_height, current_x, current_y;
    
    simulator_gui->get_size_request(current_width, current_height);
    win->get_position(current_x, current_y);

    if (is_visible)
    {
        // Make sure we receive input and output characters from the simulator.
        out_log.unblock();
        in_log.unblock();        
        // Show log dialog twenty pixels to the right of the main window.
        disp->get_window()->move(current_x + current_width + 20, current_y);
        disp->get_window()->show();
    }
    else
    {
        // Block input and output characters from the simulator.    
        out_log.block();
        in_log.block();  
        // Hide log dialog.      
        disp->get_window()->hide();
    }    
}

void log_helper::block_connections()
{
    out_log.block();
    in_log.block();        
}

void log_helper::set_grouping(unsigned int group_type, bool enc_state)
{
    // Update processing mode of simulator according the value of parameter enc_state.
    simulator_gui->set_enc_flag(enc_state);

    // Set grouping information in input and output TextViews.            
    if (simulator_gui->get_enc_flag())
    {
        // Encryption mode: Output is grouped.
        disp->set_grouping_state_in(FORMAT_NONE);
        disp->set_grouping_state_out(group_type);
    }
    else
    {
        // Decryption mode: Input is grouped.    
        disp->set_grouping_state_out(FORMAT_NONE);
        disp->set_grouping_state_in(group_type);    
    }   
}

/* ------------------------------------------------------------------ */

void rotor_position_helper::set_rotor_positions(sigc::slot<void> *set_pos_success)
{
    Glib::ustring current_positions = simulator_gui->get_machine()->visualize_all_positions();    
    rotorpos_dialog dlg(*win, current_positions);
    int dlg_result;
    bool move_result;
    
    do
    {
        dlg_result = dlg.run();        
        
        if (dlg_result == Gtk::RESPONSE_OK)
        {
            // User clicked OK
            current_positions = current_positions.lowercase();
            // Try to configure machine with the new rotor positions
            if (!(move_result = simulator_gui->get_machine()->move_all_rotors(current_positions)))
            {
                // Success! Correct rotor positions have been entered
                if (set_pos_success != NULL)
                { 
                    (*set_pos_success)();
                }
            }
            else
            {
                error_message("Rotor positions incorrect");
            }        
        }
        // Try again if the user entered wrong rotor positions but left the dialog by clicking OK    
    } while ((dlg_result == Gtk::RESPONSE_OK) && move_result);                

    // Redraw rotor windows
    simulator_gui->get_rotor_visualizer()->update_all_rotor_windows();    
}

/* ------------------------------------------------------------------ */

void randomizer_param_helper::randomize_machine(rotor_machine *machine)
{
    string randomizer_parameter;
    vector<randomizer_descriptor> known_parameters = machine->get_randomizer_descriptors();
    
    was_cancelled = false;
    has_errors = false;    
    
    if (known_parameters.size() == 0)
    {
        if (machine->randomize(randomizer_parameter))
        {
            has_errors = true;
            error_message("Randomization failed");
        }            
    }
    else
    {        
        randomizer_dialog dlg(*win, randomizer_parameter, known_parameters);
        int dlg_result;
        bool rand_result;
    
        do
        {
            dlg_result = dlg.run();        
            was_cancelled = (dlg_result != Gtk::RESPONSE_OK);
            
            if (dlg_result == Gtk::RESPONSE_OK)
            {
                // Try to randomize machine
                if ((rand_result = machine->randomize(randomizer_parameter)))
                {
                    has_errors = true;
                    error_message("Randomization failed");
                }        
            }
          // Try again if randomization failed even though the user closed the dialog by clicking OK    
        } while ((dlg_result == Gtk::RESPONSE_OK) && rand_result);                
    }
}

