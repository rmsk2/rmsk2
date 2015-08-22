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

/*! \file rotor_window.cpp
 *  \brief Implementation of GUI classes that know how to draw rotor windows for the Enigma, KL7 and several other machines.
 */

#include<rotor_window.h>
#include<sg39.h>

void rotor_window_base::set_machine(rotor_machine *m, string &identifier) 
{ 
    the_machine = m; 
    rotor_identifier = identifier;
    update();     
}

gunichar rotor_window_base::get_ring_pos()
{
    ustring ring_pos = the_machine->visualize_rotor_pos(rotor_identifier);
    gunichar result;
    
    result = Glib::Unicode::toupper(ring_pos[0]);
    
    return result;
}

void rotor_window_base::update()
{    
    if (the_machine != NULL)
    {
        wheel_pos = get_ring_pos();
    }
}

void rotor_window_base::on_mouse_button_down(Cairo::RefPtr<Cairo::Context> cr, int x_pos, int y_pos)
{
    if (test(x_pos, y_pos))
    {
        if (y_pos >= y)
        {
            // Clicked below the middle -> advance one position 
            the_machine->get_stepping_gear()->advance_rotor(rotor_identifier);
        }
        else
        {
            // Clicked above the middle -> step one position back         
            the_machine->get_stepping_gear()->step_rotor_back(rotor_identifier);
        }
        
        update(cr);
    }
}

void rotor_window_base::update(Cairo::RefPtr<Cairo::Context> cr)
{
    gunichar help;
    
    if (the_machine != NULL)
    {
        help = get_ring_pos();
        
        if (wheel_pos != help)
        {
            wheel_pos = help;
            draw(cr);
        }
    }
}

/* ------------------------------------------------------------------ */

enigma_rotor_window::enigma_rotor_window(int pos_x, int pos_y, bool orient)
    : rotor_window_base(pos_x, pos_y)
{ 
    screw_orientation = orient;
    upper = new screw(0, 0, screw_orientation);
    lower = new screw(0, 0, !screw_orientation);
    set_defaults();
}

void enigma_rotor_window::set_defaults()
{
    wheel_pos = 'A'; 
    is_greek = false; 
    the_machine = NULL; 
    is_numeric = false; 
    has_ellipse = true;
    window_size = 40;
    calc_limits();     
    set_bkg_col(LIGHT_GREY);
    set_dash_col(DARK_GREY);
    set_rotor_bkg_col(WHITE);
    set_dash_bkg_col(BACKGROUND_GREY);
}

void enigma_rotor_window::calc_limits()
{
    // Derive drawing parameters from window_size
    ellipse_width = ((double)window_size) + 5.0;
    ellipse_height = ((double)window_size) * 3.0;
    rotor_rim_width = (double)(window_size / 3);
    // Make all digits after decimal point disappear. Prevents drawing bug in Ubuntu 14.04 LTS.
    padded_size = (int)(window_size - (rotor_rim_width / 2));
    screw_size = (double)(window_size / 6);
    font_size_char = (int)(((double)window_size) * 0.6);
    font_size_numeric = (int)(((double)window_size) * 0.55); 
    char_width_numeric = (font_size_numeric / 2) + 1;
    
    upper->set_radius(screw_size);
    lower->set_radius(screw_size);    
    upper->set_elem_pos(x, y - window_size);
    lower->set_elem_pos(x, y + window_size);
}

bool enigma_rotor_window::test(int pos_x, int pos_y)
{
    bool result = false;

    if (is_active)
    {
        // Return true if (pos_x, pos_y) is within the rotor rim/handle
        result = ((pos_x >= (x + padded_size)) && (pos_x <= (x + padded_size + rotor_rim_width + 2))) && ((pos_y >= (y - (2 * window_size))) && (pos_y <= (y + (2 * window_size))));
    }
    
    return result;
}

void enigma_rotor_window::draw_wheel_pos(Cairo::RefPtr<Cairo::Context> cr, gunichar new_pos)
{
    wheel_pos = new_pos; 
    const char *trans_2 = "00000000011111111112222222";
    const char *trans_1 = "12345678901234567890123456";
    int win_size = ((int)(padded_size)) - 1;
    
    cr->save();
        
        // Draw background of rotor window as a rectangle filled with the rotor background colour
        cr->set_source_rgb(rotor_r, rotor_g, rotor_b);
        cr->rectangle(x - win_size / 2, y - win_size / 2, win_size, win_size); // Set path
        cr->fill_preserve(); // Fill everyting inside the path and preserve the path
        
        // Draw black border around the path, i.e. the rectangle that represents the rotor window        
        cr->set_line_width(1.0);
        cr->set_source_rgb(BLACK);
        cr->stroke();
    
    cr->restore();
    
    cr->save();
        
        // Set colour in which to draw the rotor position
        if (!is_greek)
            cr->set_source_rgb(BLACK); // Default is black
        else
            cr->set_source_rgb(RED); // The greek wheel on M4 has red markings
        
        if (!is_numeric)
        {
            // rotor position is displayed as character A-Z
            print_char(cr, x, y, new_pos, font_size_char);
        }
        else
        {
            // rotor position is displayed as a number 01, 02, ..., 26
            print_char(cr, x - char_width_numeric / 2, y, trans_2[new_pos - 'A'], font_size_numeric);
            print_char(cr, x + char_width_numeric / 2, y, trans_1[new_pos - 'A'], font_size_numeric);   
        }
    
    cr->restore();    
}

void enigma_rotor_window::set_dash_col(double r, double g, double b)
{
    dash_r = r;
    dash_g = g;
    dash_b = b;
}

void enigma_rotor_window::set_dash_bkg_col(double r, double g, double b)
{
    dash_bkg_r = r;
    dash_bkg_g = g;
    dash_bkg_b = b;
}

void enigma_rotor_window::set_bkg_col(double r, double g, double b)
{
    bkg_r = r;
    bkg_g = g;
    bkg_b = b;
}

void enigma_rotor_window::set_rotor_bkg_col(double r, double g, double b)
{
    rotor_r = r;
    rotor_g = g;
    rotor_b = b;
}

void enigma_rotor_window::set_elem_pos(int new_x, int new_y)
{
    element::set_elem_pos(new_x, new_y);
    
    upper->set_elem_pos(x, y - window_size);
    lower->set_elem_pos(x, y + window_size);    
}

void enigma_rotor_window::draw(Cairo::RefPtr<Cairo::Context> cr)
{
    vector<double> dashes;
    // Pattern used to draw a dashed line (15 pixels of line followed by 15 "empty" pixels)
    dashes.push_back(15.0);
    dashes.push_back(15.0);
    
    if (has_ellipse)
    {
        cr->save();
        
            // Draw background ellipse
            cr->set_source_rgb(bkg_r, bkg_g, bkg_b);
            draw_ellipse(cr, x, y, ellipse_width, ellipse_height);
            cr->fill();
            // Draw black border of background ellipse
            cr->set_source_rgb(BLACK);
            cr->set_line_width(1.2);
            draw_ellipse(cr, x, y, ellipse_width, ellipse_height);
            cr->stroke();
        
        cr->restore();
    }
    
    cr->save();  
    
        // Draw a line of width rotor_rim_width in the dash background colour
        cr->set_line_width(rotor_rim_width);
        cr->set_source_rgb(dash_bkg_r, dash_bkg_g, dash_bkg_b);
        cr->move_to(x + window_size, y - (2 * window_size));
        cr->line_to(x + window_size, y + (2 * window_size));
        cr->stroke();
        
        // Draw a dashed line in the dash colour inside the previously drawn line
        // This creates the impression of "notches" on the handle/rim
        cr->set_source_rgb(dash_r, dash_g, dash_b);
        cr->set_dash(dashes, ((wheel_pos - 'A') & 1) * 15); // modifying the offset creates illusion of movement
        
        cr->move_to(x + window_size, y - (2 * window_size));
        cr->line_to(x + window_size, y + (2 * window_size));
        cr->stroke();
        
        // Draw border around handle/rim
        cr->set_line_width(2.0);
        cr->unset_dash();
        cr->set_source_rgb(DARK_GREY);
        cr->rectangle(x + padded_size, y - (2 * window_size), rotor_rim_width, (4 * window_size));
        cr->stroke();
    
    cr->restore();

    draw_wheel_pos(cr, wheel_pos);
    
    if (has_ellipse)
    {
        // Draw screws
        upper->draw(cr);
        lower->draw(cr);    
    }
}

/* ------------------------------------------------------------------ */

thin_rotor::thin_rotor(int pos_x, int pos_y, int tr_width, int tr_height)
    : rotor_window_base(pos_x, pos_y)
{
    width = tr_width;
    height = tr_height;
    
    set_col_base(LIGHT_GREY);    
}

void thin_rotor::set_col_base(double r, double g, double b)
{
    red = r;
    green = g;
    blue = b;
}

void thin_rotor::draw(Cairo::RefPtr<Cairo::Context> cr)
{
    cr->save();
    
        // Draw rotor window rectangle
        cr->set_source_rgb(red, green, blue);
        cr->set_line_width(1.0);
        cr->rectangle(x - width / 2, y - height / 2, width, height);
        cr->fill();
        cr->stroke();
    
    cr->restore();    
    
    cr->save();
    
        // Draw rotor position
        cr->set_source_rgb(BLACK);
        print_char(cr, x, y, wheel_pos, width - 2);
        
    cr->restore();    
}

bool thin_rotor::test(int pos_x, int pos_y)
{
    // Return true if (pos_x, pos_y) is within the rotor window rectangle
    return (pos_x >= (x - width / 2)) && (pos_y >= y - height / 2) && (pos_x <= (x + width / 2)) && (pos_y <= y + height / 2);
}


/* ------------------------------------------------------------------ */

gunichar thin_kl7_rotor::get_ring_pos()
{
    ustring ring_pos = the_machine->visualize_rotor_pos(rotor_identifier);
    gunichar result = ring_pos[0];
    
    if ((result >= '0') && (result <= '9'))
    {
        result = ' ';
    }
    
    result = Glib::Unicode::toupper(result);
    
    return result;    
}

/* ------------------------------------------------------------------ */

thin_action_rotor::thin_action_rotor(int pos_x, int pos_y, sigc::slot<void, string&, Cairo::RefPtr<Cairo::Context> > action_func, int tr_width, int tr_height)
    : thin_rotor(pos_x, pos_y, tr_width, tr_height)
{
    action = action_func;
}

void thin_action_rotor::draw(Cairo::RefPtr<Cairo::Context> cr)
{
    int help_y = y + height / 2;
    
    thin_rotor::draw(cr);
    
    cr->save();
    
        // Draw action rectangle
        cr->set_source_rgb(DARK_GREY);
        cr->rectangle(x - width / 2, help_y, width, (height / 6));
        cr->fill();
        cr->stroke();
        
        // Draw border in background colour around action rectangle to make it appear smaller
        cr->set_line_width(3.0);
        cr->set_source_rgb(BACKGROUND_GREY);
        cr->move_to(x - width / 2, help_y + 1);
        cr->line_to(x + width / 2, help_y + 1);
        cr->stroke();
    
    cr->restore();    
}

bool thin_action_rotor::test(int pos_x, int pos_y)
{
    // Return true if (pos_x, pos_y) is either within the rotor window or the action rectangle
    return thin_rotor::test(pos_x, pos_y) || ((pos_x >= (x - width / 2)) && (pos_y >= y + height / 2) && (pos_x <= (x + width / 2)) && (pos_y <= y + (height / 2) + (height / 6)));
}

void thin_action_rotor::on_mouse_button_down(Cairo::RefPtr<Cairo::Context> cr, int x_pos, int y_pos)
{    
    int help_y = y + (height / 2) + 1; // y-position of the top of the action rectangle
    
    if (test(x_pos, y_pos) && (y_pos > help_y))
    {
        // Click appeared in action rectangle
        action(rotor_identifier, cr);
    }
    else
    {
        thin_rotor::on_mouse_button_down(cr, x_pos, y_pos);
    }
}

