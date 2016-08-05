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

/*! \file special_rotor_visualizer.cpp
 *  \brief Implementation of the GUI classes that know how to draw simulated rotor stacks for the SIGABA and the SG39.
 */ 


#include<special_rotor_visualizer.h>

composite_rotor_visualizer::composite_rotor_visualizer(int r_most_pos, int y_pos)
    : rotor_visualizer(0, r_most_pos, y_pos)
{
    ;
}

void composite_rotor_visualizer::draw(Cairo::RefPtr<Cairo::Context> cr)
{
    vector<thin_rotor_visualizer *>::iterator iter3;
    
    // Iterate over sub visualizers
    for (iter3 = sub_visualizers.begin(); iter3 != sub_visualizers.end(); ++iter3)
    {
        (*iter3)->draw(cr); // draw current sub visualizer
    }    
}
    
void composite_rotor_visualizer::update_rotors(Cairo::RefPtr<Cairo::Context> cr)
{
    vector<thin_rotor_visualizer *>::iterator iter3;
    
    // Iterate over sub visualizers
    for (iter3 = sub_visualizers.begin(); iter3 != sub_visualizers.end(); ++iter3)
    {
        (*iter3)->update_rotors(cr); // sync rotor positions and redraw sub visualizer
    }    
}

void composite_rotor_visualizer::on_mouse_button_down(Cairo::RefPtr<Cairo::Context> cr, int pos_x, int pos_y)
{
    vector<thin_rotor_visualizer *>::iterator iter3;
    
    // Iterate over sub visualizers
    for (iter3 = sub_visualizers.begin(); iter3 != sub_visualizers.end(); ++iter3)
    {
        // Ask current sub visualizer whether it wants to handle the click
        if ((*iter3)->test(pos_x, pos_y))
        {
            (*iter3)->on_mouse_button_down(cr, pos_x, pos_y); // Let current sub visualizer handle the click
        }
    }
}

bool composite_rotor_visualizer::test(int pos_x, int pos_y)
{
    bool result = false;
    vector<thin_rotor_visualizer *>::iterator iter3;

    // Iterate over sub visualizers
    for (iter3 = sub_visualizers.begin(); iter3 != sub_visualizers.end(); ++iter3)
    {
        result |= (*iter3)->test(pos_x, pos_y); // Forward call to current sub visualizer
    }
    
    return result;    
}

composite_rotor_visualizer::~composite_rotor_visualizer()
{
    vector<thin_rotor_visualizer *>::iterator iter3;

    // Iterate over sub visualizers            
    for (iter3 = sub_visualizers.begin(); iter3 != sub_visualizers.end(); ++iter3)
    {
        delete *iter3; // Delete sub visualizer
    }        
}

void composite_rotor_visualizer::update_all_rotor_windows()
{
    vector<thin_rotor_visualizer *>::iterator iter;
    
    for (iter = sub_visualizers.begin(); iter != sub_visualizers.end(); ++iter)
    {
        (*iter)->update_all_rotor_windows();
    }
}


/* ------------------------------------------------------------------------------- */

sigaba_rotor_visualizer::sigaba_rotor_visualizer(vector<string>& r_names, int r_most_pos, int y_pos)
    : composite_rotor_visualizer(y_pos, r_most_pos)
{
    vector<string> r_name_help;
    
    // Symbolic names of cipher rotors
    r_name_help.push_back(r_names[0]);
    r_name_help.push_back(r_names[1]);
    r_name_help.push_back(r_names[2]);
    r_name_help.push_back(r_names[3]);
    r_name_help.push_back(r_names[4]); 
    
    // sub_visualizers[0] visualizes cipher rotors
    sub_visualizers.push_back(new thin_rotor_visualizer(r_name_help, false, 0, y_pos));
    r_name_help.clear();

    // Symbolic names of driver rotors
    r_name_help.push_back(r_names[5]);
    r_name_help.push_back(r_names[6]);
    r_name_help.push_back(r_names[7]);
    r_name_help.push_back(r_names[8]);
    r_name_help.push_back(r_names[9]); 
    
    // sub_visualizers[1] visualizes driver rotors    
    sub_visualizers.push_back(new thin_action_rotor_visualizer(r_name_help, sigc::mem_fun(this, &sigaba_rotor_visualizer::action_container), 0, y_pos));
    r_name_help.clear();

    // Symbolic names of index rotors
    r_name_help.push_back(r_names[10]);
    r_name_help.push_back(r_names[11]);
    r_name_help.push_back(r_names[12]);
    r_name_help.push_back(r_names[13]);
    r_name_help.push_back(r_names[14]); 
    
    // sub_visualizers[2] visualizes index rotors
    sub_visualizers.push_back(new thin_rotor_visualizer(r_name_help, false, 0, y_pos));
    sub_visualizers[2]->set_height(INDEX_ROTOR_HEIGHT);
    
    the_sigaba_machine = NULL;
    
    set_sub_vis_positions();               
}

void sigaba_rotor_visualizer::action_container(string& arg, Cairo::RefPtr<Cairo::Context> cr)
{
    if (the_sigaba_machine != NULL)
    {
        the_sigaba_machine->get_sigaba_stepper()->setup_step(arg); // Perform setup stepping of rotor named by arg
        update_rotors(cr); // Sync and redraw all rotor windows
    }
}

void sigaba_rotor_visualizer::set_sub_vis_positions()
{
    int offset_driver_bank = 5 * (sub_visualizers[0]->get_width() + THIN_WHEEL_SPACE) + THIN_WHEEL_BANK_SPACE;
    int offset_index_bank = offset_driver_bank + (5 * (sub_visualizers[1]->get_width() + THIN_WHEEL_SPACE) + THIN_WHEEL_BANK_SPACE);

    sub_visualizers[0]->set_rightmost_rotor_pos(rightmost_rotor_pos);
    sub_visualizers[1]->set_rightmost_rotor_pos(rightmost_rotor_pos - offset_driver_bank);  
    sub_visualizers[2]->set_rightmost_rotor_pos(rightmost_rotor_pos - offset_index_bank);
}

void sigaba_rotor_visualizer::set_machine(rotor_machine *machine)
{
    sigaba *the_sigaba = dynamic_cast<sigaba *>(machine);
    rmsk::simple_assert(the_sigaba == NULL, "programmer error: type of machine must be sigaba");
    
    sub_visualizers[0]->set_machine(the_sigaba);
    // Set the SIGABA driver machine as underlying machine for the driver rotor sub visualizer
    sub_visualizers[1]->set_machine(the_sigaba->get_sigaba_stepper()->get_driver_machine());  
    // Set the SIGABA index machine as underlying machine for the index rotor sub visualizer      
    sub_visualizers[2]->set_machine(the_sigaba->get_sigaba_stepper()->get_index_bank()); 
    
    the_sigaba_machine = the_sigaba;       
}
    
void sigaba_rotor_visualizer::set_rightmost_rotor_pos(unsigned int new_pos)
{
    rightmost_rotor_pos = new_pos;
    
    set_sub_vis_positions();
}

/* ------------------------------------------------------------------ */

sg39_drive_wheel::sg39_drive_wheel(int pos_x, int pos_y, int tr_width, int tr_height)
    : thin_rotor(pos_x, pos_y, tr_width, tr_height)
{
    ;
}

void sg39_drive_wheel::draw(Cairo::RefPtr<Cairo::Context> cr)
{
    thin_rotor::draw(cr);
    int pinion_x = x - width / 2, pinion_x2 = x + width / 2, pinion_y = y - height / 2;
    
    cr->save();
    
        cr->set_source_rgb(BACKGROUND_GREY);
        
        // Draw pinion pattern on rim of the rotor windw
        while(pinion_y < y + height / 2)
        {
            // On the left side
            cr->rectangle(pinion_x, pinion_y, 2, 8);
            cr->fill();
            cr->stroke();

            // On the right side
            cr->rectangle(pinion_x2 - 2, pinion_y, 2, 8);
            cr->fill();
            cr->stroke();
            
            pinion_y += 20;            
        }
    
    cr->restore();
}

void sg39_drive_wheel::on_mouse_button_down(Cairo::RefPtr<Cairo::Context> cr, int x_pos, int y_pos)
{
    schluesselgeraet39 *the_sg39 = dynamic_cast<schluesselgeraet39 *>(the_machine);
    
    if (test(x_pos, y_pos))
    {
        if (y_pos >= y)
        {
            // Advance drive wheel one position
            the_sg39->get_sg39_stepper()->get_descriptor(rotor_identifier).mod_int_vals["wheelpos"].increment();
        }
        else
        {
            // Step drive wheel one position backwards        
            the_sg39->get_sg39_stepper()->get_descriptor(rotor_identifier).mod_int_vals["wheelpos"].decrement();
        }
        
        update(cr);
    }
}

void sg39_drive_wheel::set_machine(rotor_machine *m, string &identifier)
{
    schluesselgeraet39 *the_sg39 = dynamic_cast<schluesselgeraet39 *>(m);
    rmsk::simple_assert(the_sg39 == NULL, "programmer error: type of machine must be SG39");
    
    thin_rotor::set_machine(m, identifier);
}

gunichar sg39_drive_wheel::get_ring_pos()
{
    gunichar result;    
    schluesselgeraet39 *the_sg39 = dynamic_cast<schluesselgeraet39 *>(the_machine);
    
    result = the_sg39->get_sg39_stepper()->get_wheel_pos(rotor_identifier.c_str()) + 'A';
    
    return result;
}
/* ------------------------------------------------------------------ */

sg39_drive_wheel_visualizer::sg39_drive_wheel_visualizer(vector<string>& r_names, int r_most_pos, int y_pos)
    : thin_rotor_visualizer(r_names, false, r_most_pos, y_pos)
{
    rotor_names =  r_names;
    thin_rotor *temp = NULL;
    
    for (unsigned int count = 0; count < rotor_names.size(); count++)
    {
        // delete objects which were created by parent constructor
        delete rotors[rotor_names[count]];
        
        temp = new sg39_drive_wheel(0, 0);
        temp->set_wheel_pos('A');
                
        rotors[rotor_names[count]] = temp;
    }    
    
    set_positions_base();        
}

/* ------------------------------------------------------------------ */

sg39_rotor_visualizer::sg39_rotor_visualizer(vector<string>& r_names, int r_most_pos, int y_pos)
    : composite_rotor_visualizer(y_pos, r_most_pos)
{
    vector<string> r_name_help;

    // There only three drive wheels. The fourth rotor does not step.
    r_name_help.push_back(r_names[0]);
    r_name_help.push_back(r_names[1]);
    r_name_help.push_back(r_names[2]);

    // Create drive wheel visualizer
    sub_visualizers.push_back(new sg39_drive_wheel_visualizer(r_name_help, 0, y_pos));    

    // There are four wired rotors, therefore add fourth name.
    r_name_help.push_back(r_names[3]);

    // Create rotor stack visualizer
    sub_visualizers.push_back(new thin_rotor_visualizer(r_name_help, false, 0, y_pos));    

    set_sub_vis_positions();
}

void sg39_rotor_visualizer::set_machine(rotor_machine *machine)
{
    schluesselgeraet39 *the_sg39 = dynamic_cast<schluesselgeraet39 *>(machine);
    rmsk::simple_assert(the_sg39 == NULL, "programmer error: type of machine must be SG39");

    // the_sg39 is the underlying rotor machine for both sub visualizers.
    sub_visualizers[0]->set_machine(the_sg39);
    sub_visualizers[1]->set_machine(the_sg39);    
}

void sg39_rotor_visualizer::set_rightmost_rotor_pos(unsigned int new_pos)
{
    rightmost_rotor_pos = new_pos;
    
    set_sub_vis_positions();
}

void sg39_rotor_visualizer::set_sub_vis_positions()
{
    int offset_cipher_bank = 3 * (sub_visualizers[0]->get_width() + THIN_WHEEL_SPACE) + THIN_WHEEL_BANK_SPACE;

    sub_visualizers[0]->set_rightmost_rotor_pos(rightmost_rotor_pos);
    sub_visualizers[1]->set_rightmost_rotor_pos(rightmost_rotor_pos - offset_cipher_bank);  
}

