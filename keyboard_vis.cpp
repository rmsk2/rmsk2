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

/*! \file keyboard_vis.cpp
 *  \brief Implementation of the GUI classes that provide the keyboard visualizers which are used by enigma and rotorvis.
 */ 

#include<keyboard_vis.h>
#include<rotor_draw.h>

const int KEY_ROW_ONE_Y = 484;
const int KEY_ROW_THREE_X_OFFSET = 55;
const int KEY_STEP = 62;
const int KEY_ROW_DISTANCE = 73;

const unsigned int PRESS_NONE = 0;
const unsigned int PRESS_KEYBOARD = 1;
const unsigned int PRESS_MOUSE = 2;

keyboard_base::keyboard_base(int pos_x, int pos_y, sigc::slot<bool> enc_state_func, sigc::slot<rotor_machine *> get_machine,  sigc::slot<void, Cairo::RefPtr<Cairo::Context> > update_func)
    : element(pos_x, pos_y), keycode_alpha(), draw_param(KEY_ROW_DISTANCE, KEY_STEP)
{
    vector<gunichar> mapping = rmsk::std_uni_alpha()->get_mapping();
    boost::shared_ptr<alphabet<gunichar> > temp(new alphabet<gunichar>(mapping));

    enc_state = enc_state_func;
    machine = get_machine;
    rotor_update = update_func;

    current_key = NULL;    
    space = NULL;
    key_is_pressed = false; 
    press_event_source = PRESS_NONE;
    output = NULL;
    line_breaks.push_back(9); //on an Enigma keyboard the second line of keys begins at the 10th char of std_keys
    line_breaks.push_back(17); //on an Enigma keyboard the third line of keys begins at the 18th char of std_keys
    std_keys = "qwertzuioasdfghjkpyxcvbnml"; // Set default key sequence
    keycode_alpha = temp; // Set default keycode alphabet
    
    keys = create_std_key_board(keycode_alpha.get()); // Create all keys
    set_key_positions(); // Move them to the desired positions

}

std::map<unsigned int, key*> keyboard_base::create_std_key_board(alphabet<gunichar> *alpha)
{
    std::map<unsigned int, key*> result;
    
    // Iterate over all key labels
    for (unsigned int count = 0; count < std_keys.length(); count++)
    {
        // Create key with label std_keys[count] and keycode alpha->from_val(std_keys[count])
        result[alpha->from_val(std_keys[count])] = new key(0, 0, alpha->from_val(std_keys[count]), Glib::Unicode::toupper(std_keys[count]));
    }    
        
    return result;    
}

void keyboard_base::set_line_breaks(unsigned int break_line_1, unsigned int break_line_2)
{
    line_breaks.clear(); 
    line_breaks.push_back(break_line_1); 
    line_breaks.push_back(break_line_2);
    set_key_positions();
}

void keyboard_base::set_line_breaks(vector<unsigned int>& new_line_breaks)
{
    line_breaks.clear(); 
    line_breaks = new_line_breaks;
    set_key_positions();
}

void keyboard_base::set_key_positions()
{
    alphabet<gunichar> *alpha;
    vector<int> offset_helper;
    
    // Transform line_breaks into usable form
    for (unsigned int count = 1; count < draw_param.offset_row.size(); count++)
    {
        offset_helper.push_back(draw_param.offset_row[count]);
    }
    
    offset_helper.push_back(0);    
    
    // Make sure we have a usable keycode alphabet
    if (keycode_alpha.get() != NULL)
    {
        alpha = keycode_alpha.get();        
    }
    else
    {
        alpha = rmsk::std_uni_alpha();
    }
    
    // (Re)position all keys
    unsigned int line_pointer = 0;
    int row_counter = 0;    
    int current_x_pos = x + draw_param.offset_row[0];
    int current_y_pos = y;
    
    for (unsigned int key_code = 0; key_code < alpha->get_size(); key_code++)
    {
        if (key_code == line_breaks[line_pointer])
        {
            current_x_pos = x + offset_helper[line_pointer];
            current_y_pos += draw_param.row_distance_y;
            row_counter = 0;
            line_pointer++;
        }

        keys[alpha->from_val(std_keys[key_code])]->set_elem_pos(current_x_pos + row_counter * draw_param.elem_distance_x, current_y_pos);
        
        row_counter++;
    }
}

void keyboard_base::set_elem_pos(int new_x, int new_y)
{
    element::set_elem_pos(new_x, new_y);
    set_key_positions();
    set_space_bar_pos();
}

void keyboard_base::set_keys(std::map<unsigned int, key*>& new_keys)
{
    std::map<unsigned int, key*>::iterator iter;
    
    // Delete current keys
    for (iter = keys.begin(); iter != keys.end(); ++iter)
    {
        delete iter->second;
    }    

    keys = new_keys; // "Activate" the keys
    set_key_positions(); // Reposition them
    space = NULL; // After having done all that an eventual space_bar is lost => space has to be set to NULL
}

void keyboard_base::permute_key_sequence(ustring sequence_of_keys)
{
    std_keys = sequence_of_keys;
    set_key_positions(); 
} 

void keyboard_base::set_keycode_alpha(alphabet<gunichar> *alpha)
{
    std::map<unsigned int, key*> new_keys;

    if (alpha != NULL)
    {
        // Clone alpha
        vector<gunichar> mapping = alpha->get_mapping(); 
        boost::shared_ptr<alphabet<gunichar> > temp(new alphabet<gunichar>(mapping));
               
        keycode_alpha = temp;
        new_keys = create_std_key_board(alpha); // Create a new set of keys
        set_keys(new_keys); // Make the new keys active and position them
    }                        
}

void keyboard_base::set_key_sequence(ustring sequence_of_keys, alphabet<gunichar> *keycode_alpha)
{
    std_keys = sequence_of_keys;
    set_keycode_alpha(keycode_alpha);
}

bool keyboard_base::test(int pos_x, int pos_y)
{
    std::map<unsigned int, key*>::iterator iter;
    bool found = false;
    
    // Ask all the keys that make up the keyboard whether they want to handle the click
    for (iter = keys.begin(); !found && (iter != keys.end()); ++iter)
    {
        found = iter->second->test(pos_x, pos_y);        
    }                    
    
    return found;
}

void keyboard_base::on_mouse_button_down(Cairo::RefPtr<Cairo::Context> cr, int pos_x, int pos_y)
{
    std::map<unsigned int, key*>::iterator iter;
    bool found = false;
    
    // Find the key that has been clicked on
    for (iter = keys.begin(); !found && (iter != keys.end()); ++iter)
    {
        found = iter->second->test(pos_x, pos_y);
     
        // Is already another key pressed? In that case do nothing.   
        if (found && !key_is_pressed)
        {            
            process_key_press(cr, iter->second->get_key_code()); // handle mouse click
            press_event_source = PRESS_MOUSE; // store the fact that the last key event was due to a mouse click
        }                 
    }   
}

void keyboard_base::draw(Cairo::RefPtr<Cairo::Context> cr)
{
    std::map<unsigned int, key*>::iterator iter;
    int current_y = y + (keys[0]->get_radius()) + 12;
    
    // Draw the individual keys
    for (iter = keys.begin(); iter != keys.end(); ++iter)
    {
        iter->second->draw(cr);
    } 
    
    // Draw horizontal separator lines
    cr->save();
    
        cr->set_source_rgb(DARK_GREY);
        cr->set_line_width(2.0);        
        
        // Draw separators between rows
        for (unsigned int count = 0; count < line_breaks.size(); count++)
        {
            cr->move_to(0, current_y);
            cr->line_to(draw_param.width, current_y);
            cr->stroke();
            current_y += draw_param.row_distance_y;
        }

        // Draw separator between keyboard and output device (lampboard/printer)
        cr->set_line_width(10.0);
        cr->move_to(0, y - (keys[0]->get_radius()) - 12 - 5);
        cr->line_to(draw_param.width, y - (keys[0]->get_radius()) - 12 - 5);                
        cr->stroke();        
    
    cr->restore();   
}

void keyboard_base::simulate_key_press(Cairo::RefPtr<Cairo::Context> cr, gunichar simulated_key)
{
    if (is_symbol_valid(simulated_key))        
    {
        process_key_release(cr); // Release any key that might be pressed at the moment        
        process_key_press(cr, get_key_code(simulated_key)); // Simulate key press
        press_event_source = PRESS_KEYBOARD; // Pretend a key on the real keyboard has been pressed
        process_key_release(cr); // Simulate key release
    }
}

bool keyboard_base::is_symbol_valid(gunichar in)
{
    bool result = false;
    in = Glib::Unicode::tolower(in);    
    
    if (enc_state())
    {
        result = machine()->get_keyboard()->is_valid_input_encrypt(in);
    }
    else
    {
        result = machine()->get_keyboard()->is_valid_input_decrypt(in);    
    }    
    
    return result;    
}

unsigned int keyboard_base::get_key_code(gunichar in)
{
    unsigned int result = 0;
    in = Glib::Unicode::tolower(in);

    if (enc_state())
    {
        result = machine()->get_keyboard()->get_key_code_encrypt(in, false);
    }
    else
    {
        result = machine()->get_keyboard()->get_key_code_decrypt(in);
    }        
    
    return result;
}

void keyboard_base::on_key_press(Cairo::RefPtr<Cairo::Context> cr, gunichar key)
{
    if (is_symbol_valid(key))
    {
        if (!key_is_pressed)
        {
            process_key_press(cr, get_key_code(key));
            press_event_source = PRESS_KEYBOARD;
        }
    }     
}

void keyboard_base::on_key_up(Cairo::RefPtr<Cairo::Context> cr, gunichar key)
{
    if (is_symbol_valid(key))
    {        
        unsigned pressed_key_code = get_key_code(key);
    
        // Remark 1: A key press that started on the real keyboard can not be finished by releasing the mouse button.
        // Remark 2: As it is possible to press serveral real keys on the computer keyboard at the same time we have to        
        //           make sure that we react only when the correct (current_key->get_key_code() == pressed_key_code) key 
        //           has been released.
        if ((press_event_source != PRESS_MOUSE) && (current_key != NULL) && (current_key->get_key_code() == pressed_key_code))
        {
            process_key_release(cr);
        }
    }
    else
    {
        // Using the left or right Shift key can be used to "unstuck" any currently stuck key
        if ((key == GDK_KEY_Shift_L) || (key == GDK_KEY_Shift_R))
        {
            process_key_release(cr);
        }
    }    
}

void keyboard_base::process_key_press(Cairo::RefPtr<Cairo::Context> cr, unsigned int key_code)
{
    gunichar cipher_c;
    Glib::ustring cip_help;
    gunichar key_pressed;
    
    // Send keycode through the underlying rotor machine
    if (enc_state())
    {
        // At this moment we can not add the key_code to the rotor_keyboard's state because this 
        // would make it impossible to correctly check for symbol validity in keyboard_base::on_key_up().
        key_pressed = machine()->get_keyboard()->map_key_code_encrypt(key_code, false);
        cip_help = machine()->get_printer()->print_encrypt(key_code);
    }
    else
    {
        key_pressed = machine()->get_keyboard()->map_key_code_decrypt(key_code);    
        cip_help = machine()->get_printer()->print_decrypt(key_code);          
    }
    
    // Tell our subscribers which keycode we have received and produced
    inout_value.emit(key_code, machine()->get_printer()->get_last_cipher_result());

    // Manage and redraw the key that has been pressed
    key_pressed = Glib::Unicode::toupper(key_pressed);
    current_key = keys[key_code];
    current_key->set_is_depressed(true);
    current_key->draw(cr);
        
    key_is_pressed = true;
    
    // Check whether input key was letter shift or figure shift    
    if ((key_pressed != '<') and (key_pressed != '>'))
    {
        // In case of a dual printer also print input symbol
        output->keyboard_symbol_start(cr, key_pressed);
        // Only emit an input char if it was neither letter shift nor figure shift
        input_char.emit(key_pressed);    
    }

    // When producing the letter or figure shift keycode upon decryption the call
    // machine()->get_printer()->print_decrypt(key_code) returns an empty string    
    if (cip_help.length() > 0)
    {        
        cipher_c = Glib::Unicode::toupper(cip_help[0]);
        // Tell our subscribers about the output symbol we produced
        output_char.emit(cipher_c); 
        // Make a lamp light up or cause a character to be printed
        output->output_symbol_start(cr, cipher_c); 
    }                
    
    // The rotors have moved. Redraw them.    
    rotor_update(cr);        
}

void keyboard_base::on_mouse_button_up(Cairo::RefPtr<Cairo::Context> cr)
{
    if (press_event_source != PRESS_KEYBOARD)
    {                
        process_key_release(cr);                
    }                    
}

void keyboard_base::process_key_release(Cairo::RefPtr<Cairo::Context> cr)
{
    if ((current_key != NULL))
    {   
        // Switch lamp off
        output->output_symbol_stop(cr);

        // Redraw the current key in its released state
        current_key->set_is_depressed(!current_key->get_is_depressed());        
        current_key->draw(cr);        
        
        // Finally add the keycode to the rotor_keyboard's state
        if (enc_state())
        {
            machine()->get_keyboard()->add_to_state(current_key->get_key_code()); 
        }
        
        current_key = NULL;
        
        // Tell our subcribers that the current key has been released
        key_up.emit();
    }
    
    press_event_source = PRESS_NONE;
    key_is_pressed = false;    
}

void keyboard_base::set_space_bar_pos()
{
    int radius = keys[0]->get_radius();
    int space_x = x;
    int space_y = y + 2 * draw_param.row_distance_y + radius + 15;

    if (space != NULL)
    {
        space->set_space_pos(space_x, space_y);
    }    
}

void keyboard_base::add_space_bar(int width, int height, char alternate_key)
{
    key *alt_key = keys[keycode_alpha->from_val(alternate_key)];
    space = new space_bar(0, 0, width, height, alt_key, enc_state);
    keys[keycode_alpha->from_val(alternate_key)] = space;      
    set_space_bar_pos();
}

keyboard_base::~keyboard_base()
{
    // Delete all the keys
    std::map<unsigned int, key*>::iterator iter;
    
    for (iter = keys.begin(); iter != keys.end(); ++iter)
    {
        delete iter->second;
    }
    
    keys.clear();   
}

/* ------------------------------------------------------------------------------- */

enigma_keyboard::enigma_keyboard(int pos_x, int pos_y, sigc::slot<bool> enc_state_func, sigc::slot<rotor_machine *> get_machine,  sigc::slot<void, Cairo::RefPtr<Cairo::Context> > update_func)
    : keyboard_base(pos_x, pos_y, enc_state_func, get_machine, update_func)
{
    ;
}

enigma_keyboard::enigma_keyboard(sigc::slot<bool> enc_state_func, sigc::slot<rotor_machine *> get_machine,  sigc::slot<void, Cairo::RefPtr<Cairo::Context> > update_func)
    : keyboard_base(KEY_ROW_THREE_X_OFFSET, KEY_ROW_ONE_Y, enc_state_func, get_machine, update_func)
{
    ;
}

/* ------------------------------------------------------------------------------- */

kl7_keyboard::kl7_keyboard(int pos_x, int pos_y, sigc::slot<bool> enc_state_func, sigc::slot<rotor_machine *> get_machine,  sigc::slot<void, Cairo::RefPtr<Cairo::Context> > update_func)
    : keyboard_base(pos_x, pos_y, enc_state_func, get_machine, update_func)
{
    modify_labelling();
}

kl7_keyboard::kl7_keyboard(sigc::slot<bool> enc_state_func, sigc::slot<rotor_machine *> get_machine,  sigc::slot<void, Cairo::RefPtr<Cairo::Context> > update_func)
    : keyboard_base(KEY_ROW_THREE_X_OFFSET, KEY_ROW_ONE_Y, enc_state_func, get_machine, update_func)
{
    modify_labelling();
}

void kl7_keyboard::modify_labelling()
{
    keys[rmsk::std_uni_alpha()->from_val('q')]->set_label("Q1");    
    keys[rmsk::std_uni_alpha()->from_val('w')]->set_label("W2");    
    keys[rmsk::std_uni_alpha()->from_val('e')]->set_label("E3");    
    keys[rmsk::std_uni_alpha()->from_val('r')]->set_label("R4");    
    keys[rmsk::std_uni_alpha()->from_val('t')]->set_label("T5");    
    keys[rmsk::std_uni_alpha()->from_val('y')]->set_label("Y6");    
    keys[rmsk::std_uni_alpha()->from_val('u')]->set_label("U7");    
    keys[rmsk::std_uni_alpha()->from_val('i')]->set_label("I8");    
    keys[rmsk::std_uni_alpha()->from_val('o')]->set_label("O9");    
    keys[rmsk::std_uni_alpha()->from_val('p')]->set_label("P0");    
}

/* ------------------------------------------------------------------------------- */

typex_keyboard::typex_keyboard(int pos_x, int pos_y, sigc::slot<bool> enc_state_func, sigc::slot<rotor_machine *> get_machine,  sigc::slot<void, Cairo::RefPtr<Cairo::Context> > update_func)
    : keyboard_base(pos_x, pos_y, enc_state_func, get_machine, update_func)
{
    modify_labelling();
}

typex_keyboard::typex_keyboard(sigc::slot<bool> enc_state_func, sigc::slot<rotor_machine *> get_machine,  sigc::slot<void, Cairo::RefPtr<Cairo::Context> > update_func)
    : keyboard_base(KEY_ROW_THREE_X_OFFSET, KEY_ROW_ONE_Y, enc_state_func, get_machine, update_func)
{
    modify_labelling();
}

void typex_keyboard::modify_labelling()
{
    keys[rmsk::std_uni_alpha()->from_val('q')]->set_label("Q1");    
    keys[rmsk::std_uni_alpha()->from_val('w')]->set_label("W2");    
    keys[rmsk::std_uni_alpha()->from_val('e')]->set_label("E3");    
    keys[rmsk::std_uni_alpha()->from_val('r')]->set_label("R4");    
    keys[rmsk::std_uni_alpha()->from_val('t')]->set_label("T5");    
    keys[rmsk::std_uni_alpha()->from_val('y')]->set_label("Y6");    
    keys[rmsk::std_uni_alpha()->from_val('u')]->set_label("U7");    
    keys[rmsk::std_uni_alpha()->from_val('i')]->set_label("I8");    
    keys[rmsk::std_uni_alpha()->from_val('o')]->set_label("O9");    
    keys[rmsk::std_uni_alpha()->from_val('p')]->set_label("P0");    

    keys[rmsk::std_uni_alpha()->from_val('a')]->set_label("A-");    
    keys[rmsk::std_uni_alpha()->from_val('s')]->set_label("S/");    
    keys[rmsk::std_uni_alpha()->from_val('d')]->set_label("DZ");    
    keys[rmsk::std_uni_alpha()->from_val('f')]->set_label("F%");    
    keys[rmsk::std_uni_alpha()->from_val('g')]->set_label("GX");    
    keys[rmsk::std_uni_alpha()->from_val('h')]->set_label("H£");    
    keys[rmsk::std_uni_alpha()->from_val('j')]->set_label("J*");    
    keys[rmsk::std_uni_alpha()->from_val('k')]->set_label("K(");    
    keys[rmsk::std_uni_alpha()->from_val('l')]->set_label("L)"); 
    
    keys[rmsk::std_uni_alpha()->from_val('c')]->set_label("CV");    
    keys[rmsk::std_uni_alpha()->from_val('b')]->set_label("B'");    
    keys[rmsk::std_uni_alpha()->from_val('n')]->set_label("N,");    
    keys[rmsk::std_uni_alpha()->from_val('m')]->set_label("M.");    
}

