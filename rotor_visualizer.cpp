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

/*! \file rotor_visualizer.cpp
 *  \brief GUI classes that implement the simulated rotor stack.
 */ 

#include<rotor_visualizer.h>
#include<kl7.h>


rotor_visualizer::rotor_visualizer(int pos_x, int pos_y, int r_most_pos)
    : element(pos_x, pos_y) 
{ 
    rightmost_rotor_pos = r_most_pos; 
    rotor_step = ROTOR_STEP;
}

void rotor_visualizer::set_rightmost_rotor_pos(unsigned int new_pos)
{
    rightmost_rotor_pos = new_pos;
    
    set_positions_base();
}

void rotor_visualizer::set_positions_base()
{
    // Iterate over the symbolic names of the rotor slots
    for (unsigned int count = 0; count < rotor_names.size(); count++)
    {
        // The rotor window with index 0 is drawn in the rightmost position and the others
        // follow to the left in ascending order rotor_step pixels apart from each other
        rotors[rotor_names[count]]->set_elem_pos(rightmost_rotor_pos - (count * rotor_step), y);
    }
}

void rotor_visualizer::set_positions()
{
    set_positions_base();
}

bool rotor_visualizer::test(int pos_x, int pos_y)
{
    bool found = false;

    // Iterate over the symbolic names of the rotor slots
    for (unsigned int count = 0; (count < rotor_names.size()) && !found; count++)
    {
        string name_help = rotor_names[count];                
        found = rotors[name_help]->test(pos_x, pos_y); // Ask current object if it wants to handle the click
    }                                        
    
    return found;
}

void rotor_visualizer::on_mouse_button_down(Cairo::RefPtr<Cairo::Context> cr, int pos_x, int pos_y)
{
    bool found = false;

    // Iterate over the symbolic names of the rotor slots
    for (unsigned int count = 0; (count < rotor_names.size()) && !found; count++)
    {
        string name_help = rotor_names[count];
        
        // Ask current object if it wants to handle the click
        if ((found = rotors[name_help]->test(pos_x, pos_y)))
        {
            rotors[name_help]->on_mouse_button_down(cr, pos_x, pos_y); // Finally handle the click
        }
    }
}

void rotor_visualizer::draw(Cairo::RefPtr<Cairo::Context> cr)
{
    std::map<string, rotor_window_base *>::iterator iter3;    

    // Iterate of the managed rotor_window_base objects
    for (iter3 = rotors.begin(); iter3 != rotors.end(); ++iter3)
    {
        iter3->second->draw(cr); // draw current object
    }        
}

void rotor_visualizer::update_all_rotor_windows()
{
    // Iterate over the symbolic names of the rotor slots
    for (unsigned int count = 0; count < rotor_names.size(); count++)
    {
        string name_help = rotor_names[count];        
        rotors[name_help]->update(); // Sync rotor position and redraw           
    }                                                       
}


void rotor_visualizer::set_machine(rotor_machine *the_machine)
{
    // Iterate over the symbolic names of the rotor slots
    for (unsigned int count = 0; count < rotor_names.size(); count++)
    {
        string name_help = rotor_names[count];
        
        rotors[name_help]->set_machine(the_machine, name_help); // Tell rotor window which machine to use
        rotors[name_help]->update(); // Sync rotor position and redraw           
    }                                                       
}
    
void rotor_visualizer::update_rotors(Cairo::RefPtr<Cairo::Context> cr)
{
    // Iterate over the symbolic names of the rotor slots
    for (unsigned int count = 0; count < rotor_names.size(); count++)
    {
        string name_help = rotor_names[count];
        
        // Sync rotor position and redraw   
        rotors[name_help]->update(cr);
    }
}
    
rotor_visualizer::~rotor_visualizer()
{
    std::map<string, rotor_window_base *>::iterator iter3;

    // Iterate of the managed rotor_window_base objects            
    for (iter3 = rotors.begin(); iter3 != rotors.end(); ++iter3)
    {
        // delete current object
        delete iter3->second;
    }
    
    rotors.clear();
}

/*------------------------------------------------------------*/

enigma_visualizer::enigma_visualizer(vector<string>& r_names, bool numeric_wheels, int r_most_pos, int y_pos)
    : rotor_visualizer(0, y_pos, r_most_pos)
{
    rotor_names =  r_names;
    
    // Iterate over the symbolic names of the rotor slots
    for (unsigned int count = 0; count < rotor_names.size(); count++)
    {
        // For each slot create an enigma_rotor_window object
        // Remark: The (count % 2) determines whether the top screw in the window is oriented
        //         horzontally or in a 45 degree angle
        enigma_rotor_window *temp = new enigma_rotor_window(0, 0, (count % 2) == 0);
        temp->set_wheel_pos('A');
        temp->is_numeric = numeric_wheels;
        // Only Services Enigma uses numeric wheels. Only Services has no background ellipse.
        temp->has_ellipse = !numeric_wheels; 

        if (rotor_names[count] == "griechenwalze")
        {
            temp->is_greek = true;
        }
        
        rotors[rotor_names[count]] = temp;
    }
    
    // Position the enigma_rotor_window objects
    set_positions_base();
}

/*------------------------------------------------------------*/

thin_rotor_visualizer::thin_rotor_visualizer(vector<string>& r_names, bool is_nema, int r_most_pos, int y_pos)
    : rotor_visualizer(0, y_pos, r_most_pos)
{
    rotor_names =  r_names;
    thin_rotor *temp = NULL;
    
    is_nema_machine = is_nema;

    // Iterate over the specified rotor slot names    
    for (unsigned int count = 0; count < rotor_names.size(); count++)
    {
        // Rightmost wheel in Nema is the so called "red wheel"
        bool paint_red = ((count == 0) and (is_nema_machine));
        
        // For each slot create an thin_rotor object
        temp = new thin_rotor(0, 0);
        temp->set_wheel_pos('A');
        
        if (paint_red)
        {
            temp->set_bkg_col(RED);
        }
        
        rotors[rotor_names[count]] = temp;
    }
    
    // Set distance in pixels between two thin_rotor objects
    rotor_step = temp->get_width() + THIN_WHEEL_SPACE;

    // Position the thin_rotor objects    
    set_positions_base();
}

void thin_rotor_visualizer::set_width(int new_width)
{
    // Iterate over the symbolic names of the rotor slots
    for (unsigned int count = 0; count < rotor_names.size(); count++)
    {
        string name_help = rotor_names[count];
        
        (dynamic_cast<thin_rotor *>(rotors[name_help]))->set_width(new_width);
    }
}

void thin_rotor_visualizer::set_height(int new_height)
{
    // Iterate over the symbolic names of the rotor slots
    for (unsigned int count = 0; count < rotor_names.size(); count++)
    {
        string name_help = rotor_names[count];
        
        (dynamic_cast<thin_rotor *>(rotors[name_help]))->set_height(new_height);
    }    
}

int thin_rotor_visualizer::get_width()
{
    string name_help = rotor_names[0];
        
    return (dynamic_cast<thin_rotor *>(rotors[name_help]))->get_width();
}

int thin_rotor_visualizer::get_height()
{
    string name_help = rotor_names[0];
        
    return (dynamic_cast<thin_rotor *>(rotors[name_help]))->get_height();
}

/*------------------------------------------------------------*/

thin_kl7_rotor_visualizer::thin_kl7_rotor_visualizer(vector<string>& r_names, int r_most_pos, int y_pos)
    : thin_rotor_visualizer(r_names, false, r_most_pos, y_pos)
{
    rotor_names =  r_names;
    thin_rotor *temp = NULL;

    // Iterate over the symbolic names of the rotor slots    
    for (unsigned int count = 0; count < rotor_names.size(); count++)
    {
        // Delete objects which were created by parent constructor
        delete rotors[rotor_names[count]];
        
        // Replace previously created thin_rotor object with a thin_kl7_rotor object
        temp = new thin_kl7_rotor(0, 0);
        temp->set_wheel_pos('A');
        
        // The stationary wheel is to be drawn with a dark grey background
        if (count == 4)
        {
            temp->set_bkg_col(DARK_GREY);
        }
        
        rotors[rotor_names[count]] = temp;
    }    
    
    set_positions_base();        
}

/*------------------------------------------------------------*/

thin_action_rotor_visualizer::thin_action_rotor_visualizer(vector<string>& r_names, sigc::slot<void, string&, Cairo::RefPtr<Cairo::Context> > action_func, int r_most_pos, int y_pos)
    : thin_rotor_visualizer(r_names, false, r_most_pos, y_pos)
{
    rotor_names =  r_names;
    thin_rotor *temp = NULL;
    
    for (unsigned int count = 0; count < rotor_names.size(); count++)
    {
        // delete objects which were created by parent constructor
        delete rotors[rotor_names[count]];

        // Replace previously created thin_rotor object with a thin_action_rotor object        
        temp = new thin_action_rotor(0, 0, action_func);
        temp->set_wheel_pos('A');
        rotors[rotor_names[count]] = temp;
    }
    
    set_positions_base();    
}
