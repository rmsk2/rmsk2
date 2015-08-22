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

/*! \file display_dialog.cpp
 *  \brief Implementation for the application classes that together provide the log dialog.
 */

#include<boost/lexical_cast.hpp>
#include<display_dialog.h>

void textview_logger::format_text()
{
    Glib::ustring view_contents;
    const unsigned int  NUM_GROUPS_PER_LINE = 10;
    const unsigned int  NUM_LINE_WIDTH = 40;
    // Counts the number of groups that are part of the current line of formatted text
    unsigned int group_count = 0; 
    
    // Prepare text formatted according to the formatting (grouping) policy
    if (format_type == FORMAT_NONE)
    {
        // No grouping but line breaks are inserted after NUM_LINE_WIDTH characters
        for (unsigned count = 0; count < log_contents.length(); count++)
        {            
            if ((count != 0) and ((count % NUM_LINE_WIDTH) == 0))
            {
                view_contents += "\n";
            }
            
            view_contents += log_contents[count];
        }
    }
    else
    {
        // Grouping is done. Additionally line breaks are inserted after NUM_GROUPS_PER_LINE groups
        for (unsigned count = 0; count < log_contents.length(); count++)
        {            
            if ((count != 0) and ((count % format_type) == 0))
            {
                // Group is completed -> Add a space character.
                view_contents += " ";
                group_count++;
            }
            
            if ((group_count != 0) and (group_count == NUM_GROUPS_PER_LINE))
            {
                // Line contains NUM_GROUPS_PER_LINE groups -> insert line feed and reset group_counter
                group_count = 0;
                view_contents += "\n";
            }
            
            view_contents += log_contents[count];
        }    
    }   
    
    if (view != NULL)
    {
        // Set formatted text        
        view->get_buffer()->set_text(view_contents);
        
        // Construct label of clear button, including character count
        Glib::ustring new_label = message + " [";
        new_label += boost::lexical_cast<string>(log_contents.length());
        new_label += "]";

        // Set label on clear button
        if (button != NULL)
        {
            button->set_label(new_label);
        }
        
        // Scroll to the end of the TextView contents. Makes sure that the latest characters are always
        // visible
        Gtk::TextBuffer::iterator iter = view->get_buffer()->begin();
        
        if (log_contents.length() != 0)
        {  
            view->move_visually(iter, view_contents.length());
            Glib::RefPtr<Gtk::TextBuffer::Mark> insert_mark = view->get_buffer()->get_insert();
            view->scroll_to(insert_mark);
        }
    }
    
}

void textview_logger::report_char(gunichar c)
{
    log_contents += c;
    format_text();
}

void textview_logger::clear()
{
    log_contents = "";
    
    format_text();
}

display_dialog::display_dialog(Gtk::Window *p, Gtk::Window *w, Glib::RefPtr<Gtk::Builder> r)
    : in("Clear input log"), out("Clear output log")
{
    ref_xml = r;
    display_window = w;
    parent = p;
    
    ref_xml->get_widget("textview_in", in_view);
    ref_xml->get_widget("textview_out", out_view);
    
    ref_xml->get_widget("scrolledwindow2", scroll_in);
    ref_xml->get_widget("scrolledwindow3", scroll_out);    
    
    in.set_view(in_view, scroll_in);
    in.set_format_type(FORMAT_GROUP5); 
    out.set_view(out_view, scroll_out);
    out.set_format_type(FORMAT_GROUP5);
    
    ref_xml->get_widget("input_clear", clear_input);
    ref_xml->get_widget("output_clear", clear_output);
    
    // Connect to the clicked signals of the clear buttons
    clear_input->signal_clicked().connect(sigc::mem_fun(in, &textview_logger::clear));
    clear_output->signal_clicked().connect(sigc::mem_fun(out, &textview_logger::clear));
    
    in.set_button(clear_input);
    out.set_button(clear_output);    
    
    in_view->override_font(Pango::FontDescription("monospace")); 
    out_view->override_font(Pango::FontDescription("monospace")); 

    ref_xml->get_widget("radio_group4", group_4);
    ref_xml->get_widget("radio_group5", group_5);
    ref_xml->get_widget("radio_group_none", group_none);

    // Connect to the clicked signal of the output grouping RadioButtons
    group_4->signal_clicked().connect(sigc::bind<unsigned int>( sigc::mem_fun(out, &textview_logger::set_format_type), FORMAT_GROUP4));
    group_5->signal_clicked().connect(sigc::bind<unsigned int>( sigc::mem_fun(out, &textview_logger::set_format_type), FORMAT_GROUP5));
    group_none->signal_clicked().connect(sigc::bind<unsigned int>( sigc::mem_fun(out, &textview_logger::set_format_type), FORMAT_NONE));    

    ref_xml->get_widget("radiobutton1", group_4_in);
    ref_xml->get_widget("radiobutton2", group_5_in);
    ref_xml->get_widget("radiobutton3", group_none_in);

    // Connect to the clicked signal of the input grouping RadioButtons    
    group_4_in->signal_clicked().connect(sigc::bind<unsigned int>( sigc::mem_fun(in, &textview_logger::set_format_type), FORMAT_GROUP4));
    group_5_in->signal_clicked().connect(sigc::bind<unsigned int>( sigc::mem_fun(in, &textview_logger::set_format_type), FORMAT_GROUP5));
    group_none_in->signal_clicked().connect(sigc::bind<unsigned int>( sigc::mem_fun(in, &textview_logger::set_format_type), FORMAT_NONE));    
    
    display_window->set_transient_for(*parent);  
    
    // Connect to the delete signal, which is emitted when the log is closed    
    display_window->signal_delete_event().connect(sigc::mem_fun(*this, &display_dialog::on_delete_log));                  
}

sigc::connection display_dialog::connect_input(sigc::signal<void, gunichar>& in_signal)
{
    sigc::connection result = in_signal.connect(sigc::mem_fun(in, &textview_logger::report_char));
    
    return result;
}

sigc::connection display_dialog::connect_output(sigc::signal<void, gunichar>& out_signal)
{
    sigc::connection result = out_signal.connect(sigc::mem_fun(out, &textview_logger::report_char));
    
    return result;
}


bool display_dialog::on_delete_log(GdkEventAny* event)
{
    become_invisible.emit();    
    
    return false;
}

void display_dialog::set_grouping_state_in(unsigned int id)
{
    switch(id)
    {
        case FORMAT_NONE:
            group_none_in->set_active(true);
            break;
        case FORMAT_GROUP4:
            group_4_in->set_active(true);
            break;
        default:
            group_5_in->set_active(true);
            break;        
    }        
}

void display_dialog::set_grouping_state_out(unsigned int id)
{
    switch(id)
    {
        case FORMAT_NONE:
            group_none->set_active(true);
            break;
        case FORMAT_GROUP4:
            group_4->set_active(true);
            break;
        default:
            group_5->set_active(true);
            break;        
    }
}      

