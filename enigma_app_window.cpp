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

/*! \file enigma_app_window.cpp
 *  \brief This file contains a class that implements all the user interface stuff that is provided on top of the basic Enigma simulator functionality.
 */

#include<enigma_app_window.h>
#include<boost/lexical_cast.hpp>
#include<enigma_uhr.h>


enigma_app_window::enigma_app_window(machine_config& c, Glib::ustring& l_dir)
    : conf(c), help_menu_manager(ENIGMA), file_helper(ENIGMA), clip_helper(ENIGMA), loghelp(ENIGMA), messages(ENIGMA), pos_helper(ENIGMA), rand_helper(ENIGMA), rand_rotor_set_helper(ENIGMA)
{
    Glib::ustring window_title(" Enigma");
    
    messages.set_parent_window(this);
    
    // Setup object to manage help menu events
    help_menu_manager.set_parent_window(this);
    help_menu_manager.set_text("/enigma/index.page", "An accurate simulator for several Enigma variants.\nWritten in 2008-2018", "1.0", "Martin Grap");
    
    // Setup object to manage file handling menu events    
    file_helper.set_parent_window(this);
    file_helper.set_state_variables(&last_dir, &last_file_opened);
    file_helper.set_callbacks(sigc::mem_fun(*this, &enigma_app_window::do_load),  sigc::mem_fun(*this, &enigma_app_window::do_save));    

    // Determine rotor names to use when constructing the simulator GUI object
    for (unsigned int count = 0; count < conf.get_all_descriptors().size(); count++)
    {
        if (conf.get_desc_at(count).has_rotor_window and conf.get_desc_at(count).rotor_selection_state)
        {
            rotor_names.push_back(conf.get_desc_at(count).wheel_identifier);
        }
    }
    
    set_resizable(false);
    
    ref_xml = Gtk::Builder::create_from_string(rmsk::get_glade_data());
    Gtk::ComboBox *combo_help = NULL;
    Gtk::TreeView *tree_help = NULL;
    
    // Create simulator GUI object
    simulator_gui = Gtk::manage(new rotor_draw(rotor_names, conf.get_wheels_are_numeric(), conf.get_machine_type(), conf.get_uses_schreibmax())); 
        
    // Setup object to handle clipboard processing menu events
    clip_helper.set_parent_window(this);    
    clip_helper.set_simulator(simulator_gui);    

    // Setup object to manage rotor set randomization menu events    
    rand_rotor_set_helper.set_parent_window(this);
    
    // Setup main window (stacking menu bar and simulator GUI object on top of each other)
    vbox1 = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_VERTICAL));
    menu_action = Gio::SimpleActionGroup::create();
    setup_menus();
    
    vbox1->pack_start(*menu_bar, Gtk::PACK_SHRINK, 0);
    vbox1->pack_start(*simulator_gui);
    add(*vbox1);
    simulator_gui->show();    
    vbox1->show_all();

    
    set_last_dir(l_dir);
    last_file_opened = "";
    app_name = ENIGMA;
    
    // Create rotor dialog object
    rotor_dialog = NULL;
    ref_xml->get_widget("rotor_dialog", rotor_dialog);
    rotor_dialog->set_transient_for(*this);
    
    // Create pugboard dialog object
    plugs_dialog = NULL;
    ref_xml->get_widget("plugboard_dialog", plugs_dialog);
    plugs_dialog->set_transient_for(*this);

    // Create object to manage the selection of a UKW D wiring
    ukw_d_wiring_dialog = NULL;
    ref_xml->get_widget("ukwdialog2", ukw_d_wiring_dialog);
    ukw_d_wiring_dialog->set_transient_for(*this);

    // Create object that contains widgets of the log window
    display_window = NULL;
    ref_xml->get_widget("display_dialog", display_window);

    // Create the actual Enigma object 
    enigma = conf.make_machine(conf.get_machine_type());
        
    simulator_gui->set_machine(enigma);
    update_rotors();
    update_stecker_brett();

    // Setup object to handle Set rotor positions menu events
    pos_helper.set_parent_window(this);    
    pos_helper.set_simulator(simulator_gui);

    // Setup models of combo boxes in the rotor selection dialog
    ref_xml->get_widget("reflector_wheel", combo_help);
    combo_help->pack_start(model_cols.text);
    
    ref_xml->get_widget("greek_wheel", combo_help);
    combo_help->pack_start(model_cols.text);
    
    ref_xml->get_widget("slow_wheel", combo_help);
    combo_help->pack_start(model_cols.text);
    
    ref_xml->get_widget("middle_wheel", combo_help);
    combo_help->pack_start(model_cols.text);
    
    ref_xml->get_widget("fast_wheel", combo_help);
    combo_help->pack_start(model_cols.text);

    // Setup model of treeview used to display already selected plugs in plugboard dialog
    ref_xml->get_widget("stecker_treeview", tree_help);
    tree_help->append_column("First", plugboard_cols.first);
    tree_help->append_column("Second", plugboard_cols.second);

    // Setup model of treeview used to display already selected plugs in UKW D wiring dialog
    ref_xml->get_widget("stecker_treeview_ukw", tree_help);
    tree_help->append_column("First", plugboard_cols_ukw.first);
    tree_help->append_column("Second", plugboard_cols_ukw.second);

    // Create object to manage log window
    disp = new display_dialog(this, display_window, ref_xml);
    
    // Setup object to manage log menu events
    loghelp.set_parent_window(this);
    loghelp.set_simulator(disp, simulator_gui);    
    
    rand_helper.set_parent_window(this);
    
    disp->signal_become_invisible().connect(sigc::mem_fun(*this, &enigma_app_window::on_log_invisible));
    log_style_menuitem->change_state(simulator_gui->get_enc_flag());

    simulator_gui->signal_mode_changed().connect(sigc::mem_fun(*this, &enigma_app_window::on_mode_changed));     
    signal_delete_event().connect(sigc::mem_fun(*this, &enigma_app_window::on_my_delete_event));
    
    window_title = conf.get_machine_type() + window_title;    
    set_title(window_title);        
    sync_log_grouping();
}

void enigma_app_window::setup_menus()
{
    // File menu
    menu_action->add_action("loadsettings", sigc::mem_fun(file_helper, &file_operations_helper::on_file_open));
    menu_action->add_action("savesettings", sigc::mem_fun(file_helper, &file_operations_helper::on_file_save));
    menu_action->add_action("savesettingsas", sigc::mem_fun(file_helper, &file_operations_helper::on_file_save_as));    


    show_log_menuitem = menu_action->add_action_bool("showlogs", sigc::mem_fun(*this, &enigma_app_window::on_output_activate), false);    
    log_style_menuitem = menu_action->add_action_bool("logstyleencrypt", sigc::mem_fun(*this, &enigma_app_window::on_enc_state_activate), true);  

    if (conf.get_uses_schreibmax())
    {
        menu_action->add_action("ripstrip", sigc::mem_fun(*this, &enigma_app_window::on_rip_schreibmax_activate));    
    }

    menu_action->add_action("processclipboard", sigc::mem_fun(clip_helper, &clipboard_helper::process_clipboard));
    
    menu_action->add_action("Quit", sigc::mem_fun(*this, &enigma_app_window::on_quit_activate));

    // Machine settings menu
    menu_action->add_action("rotorpos", sigc::mem_fun(*this, &enigma_app_window::on_set_rotor_positions_activate));    
    menu_action->add_action("rotorsettings", sigc::mem_fun(*this, &enigma_app_window::on_settings_activate));
    menu_action->add_action("plugboard", sigc::mem_fun(*this, &enigma_app_window::on_plugboard_activate));            
    menu_action->add_action("reset", sigc::mem_fun(*this, &enigma_app_window::on_reset_activate));            
    menu_action->add_action("randomize", sigc::mem_fun(*this, &enigma_app_window::on_randomize_activate));
    menu_action->add_action("ukwd", sigc::mem_fun(*this, &enigma_app_window::on_ukwd_activate));                        

    // Rotor set menu
    menu_action->add_action("saverotorset", sigc::mem_fun(*this, &enigma_app_window::on_save_rotor_set_data_activate));
    menu_action->add_action("randomizerotorset", sigc::mem_fun(*this, &enigma_app_window::on_randomize_rotor_set_data_activate));
    menu_action->add_action("loadrotorset", sigc::mem_fun(*this, &enigma_app_window::on_load_rotor_set_data_activate));
    menu_action->add_action("showrotors", sigc::mem_fun(*this, &enigma_app_window::on_show_rotors_activate));    

    // Help menu
    menu_action->add_action("howtouse", sigc::mem_fun(help_menu_manager, &help_menu_helper::on_help_activate));
    menu_action->add_action("about", sigc::mem_fun(help_menu_manager, &help_menu_helper::on_about_activate));

    insert_action_group("enigma", menu_action); 

    Glib::ustring ui_info_first =
    "<interface>"
    "  <menu id='menubar'>"
    "    <submenu>"
    "      <attribute name='label' translatable='no'>_Machine</attribute>"
    "      <section>" 
    "      <item>"
    "        <attribute name='label' translatable='no'>_Load settings ...</attribute>"
    "        <attribute name='action'>enigma.loadsettings</attribute>"
    "      </item>"
    "      <item>"
    "        <attribute name='label' translatable='no'>S_ave settings as ...</attribute>"
    "        <attribute name='action'>enigma.savesettingsas</attribute>"
    "      </item>"
    "      <item>"
    "        <attribute name='label' translatable='no'>Sa_ve settings ...</attribute>"
    "        <attribute name='action'>enigma.savesettings</attribute>"
    "      </item>"
    "      <item>"
    "        <attribute name='label' translatable='no'>Sh_ow logs ...</attribute>"
    "        <attribute name='action'>enigma.showlogs</attribute>"
    "      </item>"
    "      <item>"
    "        <attribute name='label' translatable='no'>Log style: Encryption</attribute>"
    "        <attribute name='action'>enigma.logstyleencrypt</attribute>"
    "      </item>";
    
    
    if (conf.get_uses_schreibmax())
    {
        ui_info_first +=  "      <item>";
        ui_info_first +=  "        <attribute name='label' translatable='no'>Rip _paper strip</attribute>";
        ui_info_first +=  "        <attribute name='action'>enigma.ripstrip</attribute>";
        ui_info_first +=  "      </item>";
    }    
        
    Glib::ustring ui_info_second = 
    "      <item>"
    "        <attribute name='label' translatable='no'>Process _clipboard</attribute>"
    "        <attribute name='action'>enigma.processclipboard</attribute>"
    "      </item>"            
    "      </section>"    
    "      <section>"        
    "      <item>"    
    "        <attribute name='label' translatable='no'>_Quit</attribute>"
    "        <attribute name='action'>enigma.Quit</attribute>"
    "      </item>"    
    "      </section>"    
    "    </submenu>"
    "    <submenu>"    
    "      <attribute name='label' translatable='no'>_Machine settings</attribute>"
    "      <item>"
    "        <attribute name='label' translatable='no'>Set rotor pos_itions ...</attribute>"
    "        <attribute name='action'>enigma.rotorpos</attribute>"
    "      </item>"
    "      <item>"
    "        <attribute name='label' translatable='no'>Rotor _settings ...</attribute>"
    "        <attribute name='action'>enigma.rotorsettings</attribute>"
    "      </item>"
    "      <item>"
    "        <attribute name='label' translatable='no'>_Plugboard ...</attribute>"
    "        <attribute name='action'>enigma.plugboard</attribute>"
    "      </item>"
    "      <item>"
    "        <attribute name='label' translatable='no'>_Reset</attribute>"
    "        <attribute name='action'>enigma.reset</attribute>"
    "      </item>"
    "      <item>"
    "        <attribute name='label' translatable='no'>R_andomize ...</attribute>"
    "        <attribute name='action'>enigma.randomize</attribute>"
    "      </item>"
    "      <item>"
    "        <attribute name='label' translatable='no'>UKW Dora _wiring ...</attribute>"
    "        <attribute name='action'>enigma.ukwd</attribute>"
    "      </item>"
    "    </submenu>"
    "    <submenu>"
    "      <attribute name='label' translatable='no'>Rotor set</attribute>"
    "      <item>"
    "        <attribute name='label' translatable='no'>Save rotor se_t data ...</attribute>"
    "        <attribute name='action'>enigma.saverotorset</attribute>"
    "      </item>"
    "      <item>"
    "        <attribute name='label' translatable='no'>Randomize rotor sets ...</attribute>"
    "        <attribute name='action'>enigma.randomizerotorset</attribute>"
    "      </item>"    
    "      <item>"
    "        <attribute name='label' translatable='no'>Load a rotor set ...</attribute>"
    "        <attribute name='action'>enigma.loadrotorset</attribute>"
    "      </item>"
    "      <item>"
    "        <attribute name='label' translatable='no'>Show active rotors ... </attribute>"
    "        <attribute name='action'>enigma.showrotors</attribute>"
    "      </item>"    
    "    </submenu>"    
    "    <submenu>"
    "      <attribute name='label' translatable='no'>_Help</attribute>"
    "      <item>"
    "        <attribute name='label' translatable='no'>How to use the simulato_r ...</attribute>"
    "        <attribute name='action'>enigma.howtouse</attribute>"
    "      </item>"
    "      <item>"
    "        <attribute name='label' translatable='no'>A_bout ...</attribute>"
    "        <attribute name='action'>enigma.about</attribute>"
    "      </item>"    
    "    </submenu>"
    "  </menu>"
    "</interface>";

    Glib::ustring ui_info = ui_info_first + ui_info_second;
    
    ref_xml->add_from_string(ui_info);

    auto object = ref_xml->get_object("menubar");
    auto gmenu = Glib::RefPtr<Gio::Menu>::cast_dynamic(object);    
    
    menu_bar = new Gtk::MenuBar(gmenu);    
}

void enigma_app_window::update_rotors()
{   
    // Iterate over all rotor slots in conf
    for (unsigned int count = 0; count < conf.get_all_descriptors().size(); count++)
    {
        // Do something only if the rotor slot number count is in use in this machine as determined by conf
        if (conf.get_desc_at(count).rotor_selection_state)
        {
            // Replace rotor in slot number count by new one as prescribed by the configuration contained in conf
            enigma->prepare_rotor(conf.get_active_rotor_id(conf.get_desc_at(count)), conf.get_desc_at(count).wheel_identifier);
            
            // If UKW D is in use as the reflector replace the reflector with a new one constructed from the current value in conf
            if (conf.get_active_rotor_id(conf.get_desc_at(count)) == UKW_D)
            {
                boost::shared_ptr<permutation> new_reflector(new permutation(conf.get_ukw_d_perm()));                
                enigma->get_stepping_gear()->get_descriptor(UMKEHRWALZE).r->set_perm(new_reflector);                            
            }
            
            // Change ringstellung to the one determined by conf if the ring in slot number count is settable           
            if (conf.get_desc_at(count).ring_selection_state)
            {
                enigma->get_enigma_stepper()->set_ringstellung(conf.get_desc_at(count).wheel_identifier, tolower(conf.get_desc_at(count).ring_setting));
            }
            
            // Change rotor position to the one specified in conf
            enigma->get_enigma_stepper()->set_rotor_pos(conf.get_desc_at(count).wheel_identifier, tolower(conf.get_desc_at(count).rotor_pos));
        }        
    }
    
    simulator_gui->set_machine(enigma);    
}

void enigma_app_window::update_stecker_brett()
{
    steckered_enigma *e = NULL;
    
    if (conf.get_has_plugboard())
    {
        e = dynamic_cast<steckered_enigma *>(enigma);
        
        if (e == NULL)
        {
            // This should not happen.
            messages.error_message("Programmer error! Dynamic cast failed!");
        }
        else
        {
            e->set_stecker_brett(conf.get_inserted_plugs(), conf.get_uses_uhr());
                    
            if (conf.get_uses_uhr())
            {
                e->get_uhr()->set_dial_pos(conf.get_uhr_dial_pos());
            }
        }
    }        
}

void enigma_app_window::on_save_rotor_set_data_activate()
{
    file_helper.on_save_rotor_set_activate(enigma, NULL);
}

void enigma_app_window::on_load_rotor_set_data_activate()
{
    file_helper.on_load_rotor_set_activate(enigma);
}

void enigma_app_window::on_set_rotor_positions_activate()
{
    sigc::slot<void> sync_functor = sigc::mem_fun(*this, &enigma_app_window::sync_rotor_pos);
    
    pos_helper.set_rotor_positions(&sync_functor);
}

void enigma_app_window::on_ukwd_activate()
{
    vector<pair<char, char> > steckers = ukw_d_wiring_helper::perm_to_plugs(conf.get_ukw_d_perm());
    Glib::ustring name_postfix = "_ukw";
    
    ukwd_wiring_dialog d(ukw_d_wiring_dialog, ref_xml, steckers, name_postfix);
    
    if (d.run() == 0)
    {
        conf.get_ukw_d_perm() = ukw_d_wiring_helper::plugs_to_perm(steckers);
        sync_rotor_pos();        
        update_rotors();
    }    
}

void enigma_app_window::on_settings_activate()
{
    rotor_dialog_processor proc(rotor_dialog, ref_xml, conf.get_all_descriptors());
    
    if (proc.run() == 0)
    {
        sync_rotor_pos();
        update_rotors();
    }
}

void enigma_app_window::on_rip_schreibmax_activate()
{
    simulator_gui->get_output_device()->reset();
}

void enigma_app_window::on_reset_activate()
{    
    conf.get_inserted_plugs().clear();
    conf.get_uses_uhr() = false;
    conf.get_uhr_dial_pos() = 0;

    // Iterate over all rotor slots in conf
    for (unsigned int count = 0; count < conf.get_all_descriptors().size(); count++)
    {
        // Do something only if the rotor slot number count is in use in this machine as determined by conf    
        if (conf.get_desc_at(count).rotor_selection_state)
        {   
            // Set rotor position to A         
            conf.get_desc_at(count).rotor_pos = 'A';
            
            // Set ring setting to A, in case the rotor is in use in this machine
            if (conf.get_desc_at(count).rotor_selection_state)
            {            
                conf.get_desc_at(count).ring_setting = 'A';
            }
        }        
    }
    
    update_rotors();
    update_stecker_brett();
    
    messages.info_message("Rotor positions, ring settings and plugboard (if applicable) have been reset");
}

bool enigma_app_window::do_save(Glib::ustring& desired_file_name)
{
    bool result = false;
    
    sync_rotor_pos();    
    result = conf.save_settings(desired_file_name, enigma);
    
    if (!result)
    {
        set_title(conf.get_machine_type() + "Enigma [" + desired_file_name + "]");        
    }
    
    return result;
}

bool enigma_app_window::do_load(Glib::ustring& desired_file_name)
{
    bool result = false;
    
    // Reconstruct machine configuration from state file. Does not change the machine.
    result = conf.load_settings(desired_file_name);
    
    if (!result)
    {
        set_title(conf.get_machine_type() + "Enigma [" + desired_file_name + "]");
        // Also load state file into machine
        (void)enigma->load(desired_file_name.c_str()); 
        // Update GUI among other things
        simulator_gui->set_machine(enigma); 
    }
    
    return result;
}

void enigma_app_window::sync_rotor_pos()
{
    char temp;

    // Iterate over all rotor slots in conf
    for (unsigned int count = 0; count < conf.get_all_descriptors().size(); count++)
    {
        // Do something only if the rotor slot number count is in use in this machine as determined by conf    
        if (conf.get_desc_at(count).rotor_selection_state)
        {
            // Determine current rotor position of the underlying enigma simulator
            temp = toupper(enigma->get_enigma_stepper()->get_rotor_pos(conf.get_desc_at(count).wheel_identifier));
            // Store this position in configuration
            conf.get_desc_at(count).rotor_pos = temp;
        }        
    }    
}

void enigma_app_window::save_state()
{
    get_position(pos_x, pos_y);
}

bool enigma_app_window::on_my_delete_event(GdkEventAny* event)
{
    save_state();
    
    return false;
}

void enigma_app_window::on_quit_activate()
{
    save_state();
    hide();   
}

void enigma_app_window::on_plugboard_activate()
{
    if (conf.get_has_plugboard())
    {
        plugboard_dialog plugs(plugs_dialog, ref_xml, conf.get_inserted_plugs(), conf.get_uses_uhr(), conf.get_uhr_dial_pos(), conf.get_uhr_capable());
        
        if (plugs.run() == 0)
        {        
            update_stecker_brett();
        }
    }
    else
    {
        messages.info_message("This Enigma variant did not have a plugboard!");
    }
}

void enigma_app_window::use_serial_port(string port)
{
    boost::shared_ptr<output_device> real_lampboard(new enigma_real_lamp_board(port));
    simulator_gui->set_output_device(real_lampboard);
}

void enigma_app_window::sync_log_grouping()
{
    bool current_state;
    unsigned int group_type = FORMAT_GROUP5;
    
    log_style_menuitem->get_state(current_state);
    
    if ((conf.get_machine_type() == "M3") || (conf.get_machine_type() == "M4"))
    {
        group_type = FORMAT_GROUP4;
    }

    loghelp.set_grouping(group_type, current_state);       
}

void enigma_app_window::on_enc_state_activate()
{
    bool current_state; 
    
    log_style_menuitem->get_state(current_state);
    current_state = !current_state;
    log_style_menuitem->change_state(current_state);

    sync_log_grouping();
}

void enigma_app_window::on_output_activate()
{
    bool current_state;

    show_log_menuitem->get_state(current_state);
    current_state = !current_state;
    show_log_menuitem->change_state(current_state);    
    loghelp.display_log_window(current_state);
}

void enigma_app_window::on_log_invisible()
{
    loghelp.block_connections();
    show_log_menuitem->change_state(false);
}

void enigma_app_window::on_mode_changed()
{
    log_style_menuitem->change_state(simulator_gui->get_enc_flag());
}

void enigma_app_window::on_randomize_activate()
{
    const char *model = enigma->get_machine_type().c_str();
    string model_str(model);
    std::map<string, string> randomized_settings;

    sync_rotor_pos();
    rand_helper.randomize_machine(enigma);    
    
    if (!rand_helper.get_has_error())
    {
        // randomization of the underlying machine was successfull or no randomization was done at all
        
        // Retrieve settings from underlying machine
        enigma_configurator conf_obj(model);
        conf_obj.get_config(randomized_settings, enigma);
        
        if (!rand_helper.get_was_cancelled())
        {
            // User did not cancel the randomization dialog           
            // Sync config with randomized settings (This never should fail)
            if (!conf.from_keywords(randomized_settings, model_str))
            {
                // Retrieve rotor positions from randomized machine
                sync_rotor_pos(); 
                // Update GUI with new settings
                simulator_gui->set_machine(enigma);
                sync_log_grouping();
            }
            else
            {
                messages.error_message("Syncing random settings failed");
            }    
        }
    }
}

void enigma_app_window::on_randomize_rotor_set_data_activate()
{
    rand_rotor_set_helper.randomize_rotor_sets(enigma);
}

void enigma_app_window::on_show_rotors_activate()
{
    ustring message_intro = "Current rotor permutations:\n";
    
    auto visualized_perms = enigma->visualize_active_permutations();
    visualized_perms.insert(std::begin(visualized_perms), message_intro);
        
    messages.info_message(visualized_perms);
}
