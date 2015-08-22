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

/*! \file rotor_dialog.cpp
 *  \brief Application classes that implement a dialog that lets the user select a wheel setting.
 */

#include<rotor_dialog.h>
#include<enigma_sim.h>

rotor_processor_base::rotor_processor_base(Gtk::Dialog *d, Glib::RefPtr<Gtk::Builder> r, vector<rotor_family_descriptor>& desc) 
    : state_desc(desc)
{ 
    dialog = d; 
    ref_xml = r;

    // Setup the prefixes which are used to construct the names of the widgets we want to access
    control_prefixes.push_back("fast_");
    control_prefixes.push_back("middle_");
    control_prefixes.push_back("slow_");
    control_prefixes.push_back("greek_");
    control_prefixes.push_back("reflector_");
        
    for (unsigned int count = 0; count < control_prefixes.size(); count++)
    {
        Gtk::VScale *s = NULL;
        ref_xml->get_widget(control_prefixes[count] + "scale", s);
        // Connect callback which is called if the ring setting at position count has been changed by the user
        up_down_connections.push_back(s->signal_value_changed().connect(sigc::bind<Glib::ustring>( sigc::mem_fun(*this, &rotor_processor_base::on_scale_change), control_prefixes[count])));
        
        // Setup the ComboBox that lists all the alternative rotors which can be placed in slot at position count        
        fill_rotor_model(count);
        // Setup the scale that represents the ring setting at position count 
        set_ring_data(count);
    }

    Gtk::Button *ok_button = NULL;
    ref_xml->get_widget("button1", ok_button);    
    
    d->set_focus(*ok_button);
}

void rotor_processor_base::on_scale_change(Glib::ustring wheel_name)
{
    Glib::ustring temp;
    // determine label of ring setting for rotor named by wheel_name    
    Gtk::Label *l = NULL;
    ref_xml->get_widget(wheel_name + "label", l);
    // determine scale of ring seting for rotor named by wheel_name
    Gtk::VScale *s = NULL;
    ref_xml->get_widget(wheel_name + "scale", s);    
    
    // Transform scale position into an alphabetic ring setting
    temp += (char)(((int)(s->get_value())) - 1 + 'A');
    
    l->set_text(temp);
}


rotor_dialog_processor::rotor_dialog_processor(Gtk::Dialog *d, Glib::RefPtr<Gtk::Builder> r, vector<rotor_family_descriptor>& desc) 
    : rotor_processor_base(d, r, desc)
{ 
}

void rotor_processor_base::set_state_ring_controls(unsigned int count, bool new_state)
{
    // determine label of ring setting for rotor at position count    
    Gtk::Label *l = NULL;
    ref_xml->get_widget(control_prefixes[count] + "label", l);
    
    l->set_sensitive(new_state);

    // determine scale of ring setting for rotor at position count    
    Gtk::VScale *s = NULL;
    ref_xml->get_widget(control_prefixes[count] + "scale", s);
        
    s->set_sensitive(new_state);
}

void rotor_processor_base::set_visibility_ring_controls(unsigned int count, bool new_state)
{
    // determine label of ring setting for rotor at position count 
    Gtk::Label *l = NULL;
    ref_xml->get_widget(control_prefixes[count] + "label", l);
    
    // determine scale of ring setting for rotor at position count
    Gtk::VScale *s = NULL;
    ref_xml->get_widget(control_prefixes[count] + "scale", s);    

    if (new_state)
    {
        l->show();
        s->show();
    }
    else
    {
        l->hide();
        s->hide();
    }    
}

void rotor_processor_base::set_ring_data(unsigned int count)
{
    // determine scale of ring setting for rotor at position count
    Gtk::VScale *s = NULL;
    ref_xml->get_widget(control_prefixes[count] + "scale", s);    
    
    Glib::ustring temp;

    // Transform alphabetic ring setting into numeric scale setting and apply it to the scale
    s->set_value((double)(state_desc[count].ring_setting -'A' + 1));
    
    set_state_ring_controls(count, state_desc[count].ring_selection_state);
}

void rotor_processor_base::fill_result()
{
    Glib::ustring help;
    
    for (unsigned int count = 0; count < state_desc.size(); count++)
    {
        // Has the rotor at position count a settable ring?
        if (state_desc[count].ring_selection_state)
        {
            // Transfer current value of the label that specifes the ring setting at position count to the state variable rotor_processor_base::state_desc
            Gtk::Label *l = NULL;
            ref_xml->get_widget(control_prefixes[count] + "label", l);
            help = l->get_text();
            state_desc[count].ring_setting = (char)(help[0]);
        }
        
        // Is the rotor at position count in use in the enigma variant that is currently simulated?
        if (state_desc[count].rotor_selection_state)
        {
            // Transfer the currently selected value of the ComboBox that specifes the selected rotor at position count to the state variable
            // rotor_processor_base::state_desc        
            Gtk::ComboBox *cb = NULL;
            ref_xml->get_widget(control_prefixes[count] + "wheel", cb);
            state_desc[count].rotor_index_active = cb->get_active_row_number();
        }
    }
}

void rotor_processor_base::retrieve_selected_rotor_id(const char *widget_name, set<unsigned int>& test_set)
{
    Gtk::ComboBox *cb = NULL;
    Gtk::TreeModel::iterator iter;

    ref_xml->get_widget(widget_name, cb);
    iter = cb->get_active();                        
    if (iter)
    {
        test_set.insert((*iter)[model_cols.identifier]);
    }    
}

int rotor_processor_base::run()
{
    int result = 42, temp_res;
    // Set of rotor identifiers which is used to make sure that three different rotors have been selected
    set<unsigned int> test_set;
    
    do
    {
        temp_res = dialog->run();
        
        test_set.clear();
        
        // User closed the dialog by clicking on OK
        if (temp_res == 0)
        {
            // Retrieve selected rotor ids and store them in test_set
            retrieve_selected_rotor_id("fast_wheel", test_set);
            retrieve_selected_rotor_id("middle_wheel", test_set);
            retrieve_selected_rotor_id("slow_wheel", test_set);
            
            // If the number of elements in test_set is three, then no rotor has been selected more than once            
            if (test_set.size() == 3)
            {
                result = temp_res;
                fill_result(); // Retrieve the selected rotor settings and store them in output parameter
            }
            else
            {
                // At least one rotor has been duplicated -> Tell the user that this is not allowed
                Gtk::MessageDialog msg(*dialog, "Wheel setting not allowed.", false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK, true);
                msg.set_title("Enigma");
                msg.run();
            }
        }
        else // User closed the dialog by clicking on CANCEL
        {
            result = temp_res;
        }
        
    } while(result == 42);
    
    dialog->hide();
    
    return result;
}

void rotor_processor_base::fill_rotor_model(unsigned int count)
{
    // determine ComboBox used for rotor selection at position count
    Gtk::ComboBox *cb = NULL;
    ref_xml->get_widget(control_prefixes[count] + "wheel", cb);
    
    // Create and fill ListStore model for ComboBox at position count using the values specified in
    // rotor_processor_base::state_desc[count]
    Glib::RefPtr<Gtk::ListStore> m = Gtk::ListStore::create(model_cols);    
    Gtk::TreeModel::iterator iter;
    
    for (unsigned int i = 0; i < state_desc[count].rotor_names.size(); i++)
    {
        iter = m->append(); 
        (*iter)[model_cols.text] = state_desc[count].rotor_names[i];
        (*iter)[model_cols.identifier] = state_desc[count].available_rotors[state_desc[count].rotor_names[i]];        
    }
    
    cb->set_model(m); // Set prepared model
    cb->set_active(state_desc[count].rotor_index_active); // Select the rotor id specified by state_desc[count].rotor_index_active in the ComboBox
    
    // Rotor slot active?    
    if (!state_desc[count].rotor_selection_state)
    {
        // No: Hide all controls because the rotor slot is not in use in the enigma variant that is currently simulated
        cb->hide();
        set_visibility_ring_controls(count, false);
        Gtk::Label *l = NULL;
        ref_xml->get_widget(control_prefixes[count] + "head", l);        
        l->hide();
    }
}

rotor_processor_base::~rotor_processor_base()
{
    vector<sigc::connection>::iterator iter;
    
    for (iter = up_down_connections.begin(); iter != up_down_connections.end(); ++iter)
    {
        iter->disconnect();
    }
}

