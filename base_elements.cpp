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

/*! \file base_elements.cpp
 *  \brief Contains the implementation of the basic graphical elements used by the class ::rotor_draw.
 */

#include<base_elements.h>

using namespace std;

const double SCREW_SIZE = 6.0;
const int ROW_TWO_OFFSET = 35, ROW_ONE_OFFSET = 5;


void element::on_mouse_button_down(Cairo::RefPtr<Cairo::Context> cr, int pos_x, int pos_y)
{
    ;
}

draw_parameters::draw_parameters(int row_dist, int elem_dist)
{
    row_distance_y = row_dist;
    elem_distance_x = elem_dist;    
    offset_row.push_back(ROW_ONE_OFFSET);
    offset_row.push_back(ROW_TWO_OFFSET);   
    width = SIZE_X_DEFAULT;
}

draw_parameters::draw_parameters(int row_dist, int elem_dist, int row1_offset, int row2_offset, int wdth)
{
    row_distance_y = row_dist;
    elem_distance_x = elem_dist;    
    offset_row.push_back(row1_offset);
    offset_row.push_back(row2_offset);   
    width = wdth;
}

/* ----------------------------------------------------------- */

screw::screw(int pos_x, int pos_y, bool hor_orient) 
    : element(pos_x, pos_y)
{ 
    horizontal_orientation = hor_orient; 
    radius = SCREW_SIZE; 
    col_r = 0.0;
    col_g = 0.0;
    col_b = 0.0;
}

void screw::set_line_color(double r, double g, double b)
{
    col_r = r;
    col_g = g;
    col_b = b;
}

void screw::draw(Cairo::RefPtr<Cairo::Context> cr)
{
    cr->save();
        
        // Draw circular outline of the screw top
        cr->set_source_rgb(col_r, col_g, col_b);
        cr->arc(x, y, radius, 0.0, 2.0 * M_PI);
        cr->set_line_width(1.0);
        cr->stroke();
        
        // Draw slot of the screw top, either horizontally or in a 45 degree angle
        if (horizontal_orientation)
        {
            cr->move_to(x - radius, y);
            cr->line_to(x + radius, y);
            cr->stroke();
        }
        else // 45 degree angle
        {
            double span = radius - 2.0;
            cr->move_to(x - span, y + span);
            cr->line_to(x + span, y - span);
            cr->stroke();
        }        
        
    cr->restore();
}

/* ----------------------------------------------------------- */

lamp::lamp(int pos_x, int pos_y, gunichar c) 
    : element(pos_x, pos_y) 
{ 
    set_defaults();
    character = c;
}

lamp::lamp() 
    : element(0, 0) 
{ 
    set_defaults();
}

void lamp::set_defaults()
{
    character = 'A'; 
    is_illuminated = false; 
    radius = LAMP_SIZE;
    set_cover_col(BACKGROUND_GREY);
    set_lamp_bkg_col(DARK_GREY);
    set_lamp_on_col(YELLOW);
    set_lamp_off_col(LIGHT_GREY);
    char_size = 40;
}

void lamp::set_cover_col(double r, double g, double b)
{
    cover_r = r;
    cover_g = g;
    cover_b = b;
}

void lamp::set_lamp_bkg_col(double r, double g, double b)
{
    bkg_r = r;
    bkg_g = g;
    bkg_b = b;
}

void lamp::set_lamp_off_col(double r, double g, double b)
{
    off_r = r;
    off_g = g;
    off_b = b;
}

void lamp::set_lamp_on_col(double r, double g, double b)
{
    on_r = r;
    on_g = g;
    on_b = b;
}

void lamp::draw(Cairo::RefPtr<Cairo::Context> cr)
{
    cr->save();
        // Clear background
        cr->set_source_rgb(cover_r, cover_g, cover_b);
        cr->rectangle(x - radius, y - radius, 2 * radius, 2 * radius);
        cr->fill();
        cr->stroke();
    
        // Draw background
        cr->set_source_rgb(bkg_r, bkg_g, bkg_b);
        cr->arc(x, y, radius, 0.0, 2.0 * M_PI);
        cr->fill();
        cr->stroke();
        
        // Determine label colour
        if (!is_illuminated)
            cr->set_source_rgb(off_r, off_g, off_b);
        else
            cr->set_source_rgb(on_r, on_g, on_b);
        
        // Draw label
        print_char(cr, x, y, character, char_size);
    
    cr->restore();
}

/* ----------------------------------------------------------- */

key::key(int pos_x, int pos_y, unsigned int key_code, gunichar l)
    : element(pos_x, pos_y) 
{
    ustring help;
    
    help += l;
 
    set_defaults();
    this->key_code = key_code;
    set_label(help);     
}

key::key() 
    : element(0, 0) 
{ 
    set_defaults();
}

void key::set_defaults()
{
    key_code = 0;
    is_depressed = false;     
    labeling.clear();
    labeling += 'A';
    radius = KEY_SIZE;
    char_size = 2 * radius - 10;  
    set_cover_col(BACKGROUND_GREY);
    set_key_bkg_col(DARK_GREY); 
    set_border_col(WHITE);   
}

void key::set_cover_col(double r, double g, double b)
{
    cover_r = r;
    cover_g = g;
    cover_b = b;
}

void key::set_key_bkg_col(double r, double g, double b)
{
    bkg_r = r;
    bkg_g = g;
    bkg_b = b;
}

void key::set_border_col(double r, double g, double b)
{
    border_r = r;
    border_g = g;
    border_b = b;
}


bool key::test(int pos_x, int pos_y)
{
    bool result = false;

    if (is_active)
    {
        // Calculate the square of the distance between the key's center at (x, y)
        // and the point (pos_x, pos_y)
        int dx = x - pos_x, dy = y - pos_y;
        int dist = dx * dx + dy * dy;
        
        // (pos_x, pos_y) is inside the key if the square of the distance is less than
        // or equal the square of the radius
        result =  dist <= ((radius + 2) * (radius + 2));
    }
        
    return result;
}

void key::draw(Cairo::RefPtr<Cairo::Context> cr)
{
    int key_size;
    // offset in y-direction when drawing the label
    int offset;
    int font_size;
    double line_width;
    // Size of the characters in the label
    int base_char_size;
    // offset in x-direction that determines how many pixels to the left the drawing
    // of the label starts in relation to a one character label
    int offset_x;

    // Adapt character size in label and offset_x depending on the label length 
    switch(labeling.length())
    {
        case 0:
        case 1:
            base_char_size = char_size;
            offset_x = 0;
            break;
        case 2:
            base_char_size = (int)(((double)char_size) * 0.66);
            offset_x = base_char_size / 2 - 4;
            break;
        default:
            base_char_size = (int)(((double)char_size) * 0.5);
            offset_x = (base_char_size * 3) / 4;
            break;        
    }
    
    if (!is_depressed)
    {
        key_size = radius;
        offset = 0;
        line_width = 5.0;
        font_size = base_char_size;
    }
    else // draw a proportionally smaller key when it is depressed
    {
        // draw the label at a slightly bigger y position
        offset = 2;
        
        key_size = radius - 2.0;
        line_width = 3.0;
        font_size = base_char_size - 3;
        
        if (offset_x != 0)
        {
            offset_x -= 2;
        }
    }

    cr->save();
    
        // Clear background
        cr->set_source_rgb(cover_r, cover_g, cover_b);
        cr->rectangle(x - radius - 4, y - radius - 4, 2 * radius + 8, 2 * radius + 8);
        cr->fill();
        cr->stroke();
    
        // Draw background
        cr->set_source_rgb(bkg_r, bkg_g, bkg_b);
        cr->arc(x, y, key_size, 0.0, 2.0 * M_PI);
        cr->fill();
        cr->stroke();

        // Draw border
        cr->set_source_rgb(border_r, border_g, border_b);
        cr->set_line_width(line_width);
        cr->arc(x, y, key_size, 0.0, 2.0 * M_PI);
        cr->stroke();
        
        // Draw label        
        print_chars(cr, x - offset_x, y + offset, labeling, font_size);        
    
    cr->restore();
}

/* ----------------------------------------------------------- */

space_bar::space_bar(int pos_x, int pos_y, int wdth, int hght, key *org_key, sigc::slot<bool> enc_state_func)
{
    x = pos_x;
    y = pos_y;
    
    width = wdth;
    height = hght;
    
    enc_state = enc_state_func;
    original_key = org_key;
}    
    
void space_bar::draw(Cairo::RefPtr<Cairo::Context> cr)
{
    original_key->draw(cr);
    
    // Clear background
    cr->save();

        cr->set_source_rgb(BACKGROUND_GREY);
        cr->rectangle(x, y, width, height + 5);
        cr->fill();
        cr->stroke();
         
    cr->restore();
    
    if (enc_state() && original_key->get_is_depressed())
    {
        // Draw a slightly smaller, depressed, space bar
        cr->save();
        
            cr->set_source_rgb(DARK_GREY);
            cr->rectangle(x, y + 5, width, height - 3);
            cr->fill();
            cr->stroke();    
        
        cr->restore();
    }
    else
    {
        // Draw normal space bar
        cr->save();
        
            cr->set_source_rgb(DARK_GREY);
            cr->rectangle(x, y, width, height);
            cr->fill();
            cr->stroke();    
        
        cr->restore();
    }
}

void space_bar::set_radius(double r)
{
    original_key->set_radius(r);
}

double space_bar::get_radius()
{ 
    return original_key->get_radius(); 
}

bool space_bar::test(int pos_x, int pos_y)
{
    bool is_in_space_bar = ((pos_x >= x) && (pos_x <= (x + width)) && (pos_y >= y) && (pos_y <= (y + height))) && enc_state();

    return (original_key->test(pos_x, pos_y) || is_in_space_bar) && is_active;
}

unsigned int space_bar::get_key_code()
{ 
    return original_key->get_key_code(); 
}

void space_bar::set_label(ustring& label)
{ 
    original_key->set_label(label); 
}

void space_bar::set_label(const char *label)
{ 
    original_key->set_label(label); 
}    

ustring space_bar::get_label() 
{
    return original_key->get_label(); 
}
    
void space_bar::set_is_depressed(bool new_val)
{ 
    original_key->set_is_depressed(new_val); 
}

bool space_bar::get_is_depressed() 
{ 
    return original_key->get_is_depressed(); 
}

/* ----------------------------------------------------------- */

counter::counter(int pos_x, int pos_y, int wdth, int hght)
    : element(pos_x, pos_y)
{
    char_counter = 0;
    width = wdth;
    height = hght;
}

void counter::draw(Cairo::RefPtr<Cairo::Context> cr)
{
    string help  = "0123456789";
    ustring out_string;
    unsigned int conv_help = char_counter;
    
    // convert char_counter value to string
    for (unsigned int count = 0; count < 4; count++)
    {
        out_string = help[conv_help % 10] + out_string;
        conv_help = conv_help / 10;
    }
    
    cr->save();
    
        // Draw background
        cr->set_source_rgb(DARK_GREY);
        cr->rectangle(x, y, width, height);
        cr->fill();
        cr->stroke();
        
        // Draw counter value
        cr->set_source_rgb(WHITE);
        
        cr->select_font_face("Monospace", Cairo::FONT_SLANT_NORMAL, Cairo::FONT_WEIGHT_NORMAL);
        cr->set_font_size(height - 3);        

        Cairo::FontOptions font_options;

        font_options.set_hint_style(Cairo::HINT_STYLE_NONE);
        font_options.set_hint_metrics(Cairo::HINT_METRICS_OFF);
        font_options.set_antialias(Cairo::ANTIALIAS_GRAY);

        cr->set_font_options(font_options);
        
        cr->move_to(x + 3, y + height - 4);            
        cr->show_text(out_string);
        
        cr->stroke();                    
        
    cr->restore();

}

bool counter::test(int pos_x, int pos_y)
{
    return ((pos_x >= x) && (pos_x <= (x + width)) && (pos_y >= y) && (pos_y <= (y + height))) && is_active;
}

void counter::on_mouse_button_down(Cairo::RefPtr<Cairo::Context> cr, int pos_x, int pos_y)
{
    if (test(pos_x, pos_y))
    {
        reset();
        draw(cr);
    }
}

/* ----------------------------------------------------------- */

button::button(int pos_x, int pos_y, string text, int wdth, int hght)
    : element(pos_x, pos_y)
{
    width = wdth;
    height = hght;
    label = text;    
}

void button::draw(Cairo::RefPtr<Cairo::Context> cr)
{
    cr->save();
    
        // Draw background
        cr->set_source_rgb(DARK_GREY);
        cr->rectangle(x, y, width, height);
        cr->fill();
        cr->stroke();
        
        // Draw button label
        cr->set_source_rgb(WHITE);
        
        cr->select_font_face("Monospace", Cairo::FONT_SLANT_NORMAL, Cairo::FONT_WEIGHT_NORMAL);
        cr->set_font_size(height - 3);        

        Cairo::FontOptions font_options;

        font_options.set_hint_style(Cairo::HINT_STYLE_NONE);
        font_options.set_hint_metrics(Cairo::HINT_METRICS_OFF);
        font_options.set_antialias(Cairo::ANTIALIAS_GRAY);

        cr->set_font_options(font_options);
        
        cr->move_to(x + 3, y + height - 4);            
        cr->show_text(label);
        
        cr->stroke();                    
        
    cr->restore();

}

void button::on_mouse_button_down(Cairo::RefPtr<Cairo::Context> cr, int pos_x, int pos_y)
{
    if (test(pos_x, pos_y))
    {
        pressed.emit();
        draw(cr);
    }    
}

bool button::test(int pos_x, int pos_y)
{
    return ((pos_x >= x) && (pos_x <= (x + width)) && (pos_y >= y) && (pos_y <= (y + height))) && is_active;    
}

/* ----------------------------------------------------------- */

void draw_ellipse(Cairo::RefPtr<Cairo::Context> cr, int x, int y, int width, int height)
{   
    cr->save();
    cr->translate(x, y);
    cr->scale(width / 2.0, height / 2.0);
    cr->arc(0.0, 0.0, 1.0, 0.0, 2 * M_PI);
    cr->restore();    
}

void print_char(Cairo::RefPtr<Cairo::Context> cr, int x, int y, gunichar character, int font_size)
{
    ustring temp;
    
    temp += character;
    print_chars(cr, x, y, temp, font_size);
}

void print_chars(Cairo::RefPtr<Cairo::Context> cr, int x, int y, ustring& characters, int font_size)
{    
    cr->save();
    
        cr->select_font_face("Monospace", Cairo::FONT_SLANT_NORMAL, Cairo::FONT_WEIGHT_NORMAL);
        cr->set_font_size(font_size);

        Cairo::FontOptions font_options;

        font_options.set_hint_style(Cairo::HINT_STYLE_NONE);
        font_options.set_hint_metrics(Cairo::HINT_METRICS_OFF);
        font_options.set_antialias(Cairo::ANTIALIAS_GRAY);

        cr->set_font_options(font_options);
        
        cr->move_to(x - (font_size / 3), y + (font_size / 3));
        cr->show_text(characters);
        cr->stroke();
        
    cr->restore();
}

