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

/*! \file output_device.cpp
 *  \brief GUI classes that implement the simulated lampboard and printer.
 */ 

#include<output_device.h>
#include<rotor_draw.h>

const int LAMP_STEP = 62;
const int LAMP_ROW_DISTANCE = 60;
const int PRINTER_Y = 320;
const unsigned int FONT_SIZE_OFFSET = 4;
const int RIGHT_BORDER_OFFSET = 4;

void output_device::draw(Cairo::RefPtr<Cairo::Context> cr)
{
    cr->save();
    
        // Draw "upper border" which separates the output_device from the simulated rotor stack
        cr->set_source_rgb(DARK_GREY);
        cr->set_line_width(10.0);
        cr->move_to(0, upper_border);
        cr->line_to(width, upper_border);                
        cr->stroke();        
    
    cr->restore();   
}

/* ----------------------------------------------------------- */

enigma_lamp_board::enigma_lamp_board(int pos_x, int pos_y)
    : output_device(pos_x, pos_y), draw_param(LAMP_ROW_DISTANCE, LAMP_STEP)
{
    std_lamps = "QWERTZUIOASDFGHJKPYXCVBNML"; // Default is Enigma lamp sequence
    
    line_breaks.push_back(9);  //on an enigma lamp board the second line of keys begins at the 10th char of std_lamps
    line_breaks.push_back(17); //on an enigma lamp board the third line of keys begins at the 18th char of std_lamps
    
    lamps = create_std_lamps();
        
    illuminated_symbol = NULL;
    is_active = true;
    set_lamp_positions();
}

void enigma_lamp_board::set_lamp_sequence(ustring sequence_of_lamps)
{ 
    std::map<gunichar, lamp*>::iterator iter2;
    
    // Delete current lamp objects
    for (iter2 = lamps.begin(); iter2 != lamps.end(); ++iter2)
    {
        delete iter2->second;
    }

    // Create a new set of lamps and make them active
    std_lamps = sequence_of_lamps;     
    lamps = create_std_lamps();    
    set_lamp_positions(); 
} 

std::map<gunichar, lamp*>  enigma_lamp_board::create_std_lamps()
{
    std::map<gunichar, lamp*> result;
    ustring::iterator iter;
    
    // Create a new set of lamps
    for (iter = std_lamps.begin(); iter != std_lamps.end(); ++iter)
    {
        result[Glib::Unicode::toupper(*iter)] = new lamp(0, 0, Glib::Unicode::toupper(*iter));
    }
    
    return result;
}

void enigma_lamp_board::set_lamp_positions()
{
    vector<int> offset_helper;

    // Transform line_breaks into usable form    
    for (unsigned int count = 1; count < draw_param.offset_row.size(); count++)
    {
        offset_helper.push_back(draw_param.offset_row[count]);
    }    
    
    offset_helper.push_back(0);    

    // Set up counter variables
    unsigned int line_pointer = 0;
    int row_counter = 0;    
    int current_x_pos = x + draw_param.offset_row[0];
    int current_y_pos = y;
    
    // Iterate of lamp sequence
    for (unsigned int key_code = 0; key_code < std_lamps.length(); key_code++)
    {
        // line break reached?
        if (key_code == line_breaks[line_pointer])
        {
            current_x_pos = x + offset_helper[line_pointer];
            current_y_pos += draw_param.row_distance_y;
            row_counter = 0;
            line_pointer++;
        }

        // Reposition current lamp
        lamps[Glib::Unicode::toupper(std_lamps[key_code])]->set_elem_pos(current_x_pos + row_counter * draw_param.elem_distance_x, current_y_pos);
        
        row_counter++;
    }
}

void enigma_lamp_board::set_line_breaks(unsigned int break_line_1, unsigned int break_line_2)
{
    line_breaks.clear(); 
    line_breaks.push_back(break_line_1); 
    line_breaks.push_back(break_line_2);
    set_lamp_positions();
}

void enigma_lamp_board::set_line_breaks(vector<unsigned int>& new_line_breaks)
{
    line_breaks.clear(); 
    line_breaks = new_line_breaks;
    set_lamp_positions();
}


void enigma_lamp_board::set_elem_pos(int new_x, int new_y)
{
    element::set_elem_pos(new_x, new_y);
    set_lamp_positions();
}

void enigma_lamp_board::draw(Cairo::RefPtr<Cairo::Context> cr)
{
    output_device::draw(cr);
    
    std::map<gunichar, lamp*>::iterator iter2;
    
    // Draw all lamps
    for (iter2 = lamps.begin(); iter2 != lamps.end(); ++iter2)
    {
        iter2->second->draw(cr);
    }    
}

enigma_lamp_board::~enigma_lamp_board()
{
    std::map<gunichar, lamp*>::iterator iter2;
    
    // Delete all lamps
    for (iter2 = lamps.begin(); iter2 != lamps.end(); ++iter2)
    {
        delete iter2->second;
    }    
}

void enigma_lamp_board::output_symbol_start(Cairo::RefPtr<Cairo::Context> cr, gunichar symbol)
{
    if (is_active and (lamps.count(Glib::Unicode::toupper(symbol)) != 0))
    {
        illuminated_symbol = lamps[Glib::Unicode::toupper(symbol)];
        illuminated_symbol->is_illuminated = true; // Switch lamp on
        illuminated_symbol->draw(cr); // Redraw the lamp
    }
}

void enigma_lamp_board::output_symbol_stop(Cairo::RefPtr<Cairo::Context> cr)
{
    if ((illuminated_symbol != NULL) and is_active)
    {
        illuminated_symbol->is_illuminated = false; // Switch lamp off
        illuminated_symbol->draw(cr); // Redraw the lamp
        illuminated_symbol = NULL;
    }
}

/* ----------------------------------------------------------- */

enigma_real_lamp_board::enigma_real_lamp_board(string port, int pos_x, int pos_y) 
    : enigma_lamp_board(pos_x, pos_y), io()
{
    try
    {
        serial = new boost::asio::serial_port(io, port);  
        serial_port = port; 
        serial->set_option(boost::asio::serial_port_base::baud_rate(9600));
    }
    catch(...)
    {
        serial = NULL;
        cout << "error opening serial port " << port << endl;
    }    
}

enigma_real_lamp_board::~enigma_real_lamp_board()
{
    delete serial;
}

void enigma_real_lamp_board::output_symbol_start(Cairo::RefPtr<Cairo::Context> cr, gunichar symbol)
{
    // Make lamp light up in real lampboard that is attached to serial port
    char val = (unsigned char)(symbol & 0xFF);
    
    // Send symbol to serial port
    try
    {
        if ((serial != NULL) && (serial->is_open()))
        {
            boost::asio::write(*serial, boost::asio::buffer(&val, 1));
        }
    }
    catch(...)
    {
        delete serial;
        serial = NULL;
    }
    
    // Switch lamp on in simulated lampboard    
    enigma_lamp_board::output_symbol_start(cr, symbol);        
}

void enigma_real_lamp_board::output_symbol_stop(Cairo::RefPtr<Cairo::Context> cr)
{
    // The symbol 0x80 is intended to signify that the lamp which is currently illuminated (if any)
    // has to be switched off    
    unsigned char val = 0x80;

    // Send symbol to serial port
    try
    {    
        if ((serial != NULL) && (serial->is_open()))
        {
            boost::asio::write(*serial, boost::asio::buffer(&val, 1));        
        }
    }
    catch(...)
    {
        delete serial;
        serial = NULL;
    }

    // Switch lamp off in simulated lampboard    
    enigma_lamp_board::output_symbol_stop(cr);
}

/* ----------------------------------------------------------- */

printer_base::printer_base(sigc::slot<bool> enc_state_func, sigc::slot<void> redraw_func, int pos_x, int pos_y)
    : output_device(pos_x, pos_y) 
{ 
    enc_state = enc_state_func;
    redraw = redraw_func;
}

/* ----------------------------------------------------------- */

void printer_visualizer::init_data()
{
    grouping_width = 5;
    use_lower_case = false;
}

printer_visualizer::printer_visualizer(sigc::slot<bool> enc_state_func, sigc::slot<void> redraw_func, sigc::signal<void>& sig_mode_change, int pos_x, int pos_y)
    : printer_base(enc_state_func, redraw_func, pos_x, pos_y) 
{ 
    init_data();
    connect_signal(sig_mode_change);    
}

printer_visualizer::printer_visualizer(sigc::slot<bool> enc_state_func, sigc::slot<void> redraw_func, sigc::signal<void>& sig_mode_change)
    : printer_base(enc_state_func, redraw_func, 0, PRINTER_Y) 
{
    init_data();    
    connect_signal(sig_mode_change);    
}

void printer_visualizer::connect_signal(sigc::signal<void>& signal)
{
    mode_change_conn.disconnect();    
    mode_change_conn = signal.connect(sigc::mem_fun(*this, &printer_visualizer::reset));        
    reset();
}

double printer_visualizer::measure_string(Cairo::RefPtr<Cairo::Context> cr, ustring& to_measure)
{
    Cairo::TextExtents extent;
    
    cr->get_text_extents(to_measure, extent);    
    
    return extent.x_advance;
}

void printer_visualizer::draw(Cairo::RefPtr<Cairo::Context> cr)
{
    printer_base::draw(cr);
    
    double text_width;
    unsigned int font_size = (params.height / 2) + FONT_SIZE_OFFSET;
    ustring out_help;
    
    cr->save();
    
        // Draw (empty) paper strip
        cr->set_source_rgb(WHITE);
        cr->rectangle(x, y, width, params.height);
        cr->fill();
        cr->stroke();

        // Colour for printing characters is dark grey
        cr->set_source_rgb(DARK_GREY);

        cr->save();
    
            // Set the default Monospace font
            cr->select_font_face("Monospace", Cairo::FONT_SLANT_NORMAL, Cairo::FONT_WEIGHT_NORMAL);
            cr->set_font_size(font_size);

            Cairo::FontOptions font_options;

            font_options.set_hint_style(Cairo::HINT_STYLE_NONE);
            font_options.set_hint_metrics(Cairo::HINT_METRICS_OFF);
            font_options.set_antialias(Cairo::ANTIALIAS_GRAY);

            cr->set_font_options(font_options);

            // Get pixel width of the current contents of text_buffer
            text_width = measure_string(cr, text_buffer);                                      
            
            // If necessary chop off some characters on the left of the paper strip until
            // the characters in text_buffer fit the width of the simulated paper strip
            while(text_width + RIGHT_BORDER_OFFSET > width)
            {
                text_buffer = text_buffer.substr(1);
                text_width = measure_string(cr, text_buffer);
            }            
            
            // Finally print text
            cr->move_to(x + width - text_width - RIGHT_BORDER_OFFSET, y + params.height - (font_size / 2));            
            cr->show_text(text_buffer);
            cr->stroke();            
            
        cr->restore();                
    
    cr->restore();    
}

void printer_visualizer::reset() 
{ 
    text_buffer.clear(); 
        
    if (!enc_state())
    {
        grouping_count = 0; // no grouping while doing decryptions
    }
    else
    {
        grouping_count = grouping_width; // grouping during encryptions
    }
    
    redraw();    
}    

void printer_visualizer::output_symbol_start(Cairo::RefPtr<Cairo::Context> cr, gunichar symbol)
{
    // Force conversion to lower case if desired
    if (use_lower_case)
    {
        symbol = Glib::Unicode::tolower(symbol);
    }
   
    text_buffer += symbol; // Append symbol to text_buffer
    grouping_count--;        
    
    // Insert an additional space if a group has been completed
    if ((grouping_count == 0) && (enc_state()))
    {
        text_buffer.push_back(' ');
        grouping_count = grouping_width;
    }    
    
    draw(cr);    
}

void printer_visualizer::output_symbol_stop(Cairo::RefPtr<Cairo::Context> cr)
{
    ;
}

/* ----------------------------------------------------------- */

dual_printer::dual_printer(sigc::slot<bool> enc_state_func, sigc::slot<void> redraw_func, sigc::signal<void>& sig_mode_change, int pos_x, int pos_y)
    : printer_base(enc_state_func, redraw_func, pos_x, pos_y) 
{       
    input_printer = boost::shared_ptr<printer_visualizer>(new printer_visualizer(sigc::mem_fun(*this, &dual_printer::always_enc), redraw_func, dummy_signal, pos_x, pos_y));    
    output_printer = boost::shared_ptr<printer_visualizer>(new printer_visualizer(enc_state_func, redraw_func, dummy_signal, pos_x, pos_y + 90));        
    
    connect_signal(sig_mode_change);
}

void dual_printer::connect_signal(sigc::signal<void>& signal)
{
    mode_change_conn.disconnect();    
    mode_change_conn = signal.connect(sigc::mem_fun(*this, &dual_printer::reset));        
    reset();
}

