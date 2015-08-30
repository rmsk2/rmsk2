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

/*! \file ukwd_wiring_dialog.cpp
 *  \brief Implementation of a dialog that lets the user select a UKW D wiring.
 */

#include<ukwd_wiring_dialog.h>
#include<string>
#include<iostream>
#include<algorithm>
#include<rmsk_globals.h>
#include<locale>

alphabet<char> ukwd_alpha("yzxwvutsrqponjmlkihgfedcba", 26);

ukwd_wiring_dialog::ukwd_wiring_dialog(Gtk::Dialog *d, Glib::RefPtr<Gtk::Builder> r, vector<pair<char, char> >& steckers, Glib::ustring& name_postfix)
    : select_involution_dialog(d, r, steckers, name_postfix)
{
    Glib::ustring new_text = "Number of connections: ";
    
    set_plug_count_label_text(new_text);
    update_plug_counter();
    d->set_title("Enigma");
}

bool ukwd_wiring_dialog::less_than(const pair<char, char>& l, const pair<char, char>& r)
{
    return l.first < r.first;
}

vector<pair<char, char> > ukwd_wiring_dialog::perm_to_plugs(permutation& perm)
{
    vector<pair<char, char> > result;
    set<pair<unsigned int, unsigned int> > involution;
    set<pair<unsigned int, unsigned int> >::iterator iter;
    pair<unsigned int, unsigned int> fixed_connection(UKWD_FIXED_CONTACT_Y, UKWD_FIXED_CONTACT_J);
    char f, s;
    vector<pair<char, char> > data_sorted;

    // If any of the tests below fails, return an involution that contains only the fixed connection    
    result.push_back(pair<char, char>('J', 'Y'));
    perm.test_for_involution(involution); // The set involution is cleared if perm is no involution
    
    if (involution.find(fixed_connection) != involution.end())
    {
        //  Required connection is included. Good!
        
        // An UKW D requires 13 pairs to be correct.
        if (involution.size() == (rmsk::std_alpha()->get_size() / 2))
        {
            // iterate over the pairs returned by perm.test_for_involution(involution)
            for (iter = involution.begin(); iter != involution.end(); ++iter)
            {
                // transform the unsigned ints from the pair into characterts 
                f = ukwd_alpha.to_val(iter->first);
                s = ukwd_alpha.to_val(iter->second);
                
                // The pairs (f, s) and (s, f) are functionally equivalent. We
                // prefer the form where f < s.
                if (f > s)
                {
                    data_sorted.push_back(pair<char, char>(s, f));
                }
                else
                {
                    data_sorted.push_back(pair<char, char>(f, s));                    
                }
            }
            
            // Normalize sequence of pairs through sorting by the first elements of the pairs
            sort(data_sorted.begin(), data_sorted.end(), less_than);

            result = data_sorted;            
        }
    }
        
    return result;
}

permutation ukwd_wiring_dialog::plugs_to_perm(vector<pair<char, char> >& plugs)
{
    permutation result = ukwd_alpha.make_involution(plugs);
    
    return result;
}

void ukwd_wiring_dialog::on_delete_clicked()
{
    Glib::RefPtr<Gtk::ListStore> m;
    Gtk::TreeView *t = NULL;
    ref_xml->get_widget("stecker_treeview" + name_post_fix, t);
    Glib::RefPtr<Gtk::TreeSelection> selection = t->get_selection();
    
    if (selection->count_selected_rows() != 0)
    {
        // A connection is selected
        m = m.cast_dynamic(t->get_model());
        Gtk::TreeModel::iterator iter = selection->get_selected();
  
        // Test is fixed connection is selected for removal  
        if ( (((*iter)[plugboard_cols.first]) != "J") and  (((*iter)[plugboard_cols.second]) != "Y") )
        {
            // No! -> delete the selected connection
            remove_plug((*iter)[plugboard_cols.first], (*iter)[plugboard_cols.second]);
            num_plugs--;
            update_plug_counter();
            m->erase(iter);
        }
        else
        {
            // Yes! -> error message
            Gtk::MessageDialog msg(*dialog, "The connection J, Y is fixed and cannot be removed.", false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK, true);
            msg.set_title("Enigma");
            msg.run();                
        }
    }
    else
    {
        // No connection is selected    
        Gtk::MessageDialog msg(*dialog, "Nothing selected to delete.", false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK, true);
        msg.set_title("Enigma");
        msg.run();        
    }
}

void ukwd_wiring_dialog::on_delete_all_clicked()
{
    Glib::ustring fixed_first("J"), fixed_second("Y");
    
    // Clear all connections
    select_involution_dialog::on_delete_all_clicked();
    // Reinsert fixed connection
    insert_stecker_in_model(fixed_first, fixed_second);
}

int ukwd_wiring_dialog::run()
{
    int result = 42, res_temp;
    Glib::RefPtr<Gtk::ListStore> m;
    set<pair<char, char> > plugs;
    vector<pair<char, char> > steckers_temp;
    Gtk::TreeModel::Children::iterator iter;    
    Glib::ustring first_help, second_help;     
    Gtk::TreeView *t = NULL;
    ref_xml->get_widget("stecker_treeview" + name_post_fix, t);
    
    do
    {
        res_temp = dialog->run();

        // OK clicked?        
        if (res_temp == 0)
        {
            // Yes!
            steckers_temp.clear();
            m = m.cast_dynamic(t->get_model());
            Gtk::TreeModel::Children children = m->children();
            
            // Iterate over all connections in TreeView
            for (iter = children.begin(); iter != children.end(); ++iter)
            {
                pair<char, char> help;
                first_help = (*iter)[plugboard_cols.first];
                second_help = (*iter)[plugboard_cols.second];
                
                help.first = tolower((char)(first_help[0]));
                help.second = tolower((char)(second_help[0]));
                
                steckers_temp.push_back(help);                
            }        
        
            // Check if user selected exactly 13 connections
            if (steckers_temp.size() == 13)
            {
                // Yes! -> set result variables
                current_steckers = steckers_temp;                
                result = res_temp;
            }
            else
            {
                // No! -> error message
                Gtk::MessageDialog msg(*dialog, "There have to be exactly 13 connections.", false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK, true);
                msg.set_title("Enigma");
                msg.run();                            
            }            
        }
        else
        {
            // No!
            result = res_temp;
        }
    
    } while(result == 42); 
       
    dialog->hide();
    
    return 0;
}



