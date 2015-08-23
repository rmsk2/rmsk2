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

/*! \file rotor_draw.cpp
 *  \brief Implementation of the class that provides the graphical part of all simulators implemented
 *         by rmsk.
 */

#include<boost/scoped_ptr.hpp>
#include<rotor_draw.h>
#include<set>
#include<special_rotor_visualizer.h>

const int BUTTON_ROW_Y = 395;
const int COUNTER_ROW_Y = 170;
const int SIZE_Y_DEFAULT = 670;
const int SIZE_Y_SPACE_BAR = 710;

/*! \param r_names [in] Specifies the symbolic names of the rotor slots that are in use in the type of machine for which this rotor_draw object
 *                      is constructed. These names have to match the symbolic names that are in use in the underlying rotor_machine object.
 *  \param numeric_wheels [in] Used for Enigma variants. True if the markings on the rotors circumfences should be characters or numbers. The
 *                             only historically correct machine to use numeric markings is the Services Enigma (Wehrmachtsenigma).
 *  \param machine_to_visualize [in] Name of the machine to visualize. Is used to initialize data structures internal to rotor_draw. Currently the
 *                                   following names are valid: M3, M4, Services, Tirpitz, Abwehr, Railway, KD, CSP889, CSP2900, KL7, Typex, Nema, SG39.
 *  \param utilize_schreibmax [in] True if a Schreibmax printer is to be simulated. Only used when machine_to_visualize is an Enigma variant.                                    
 *  \param r_most_pos [in] Specifies the x coordinate of the position where the rightmost rotor of the rotor stack should be drawn.
 */
rotor_draw::rotor_draw(vector<string>& r_names, bool numeric_wheels, string machine_to_visualize, bool utilize_schreibmax, int r_most_pos)
{
    the_machine = NULL; 
    
    set_size_request(SIZE_X_DEFAULT, SIZE_Y_DEFAULT);
    
    // Default: No character counter present
    is_counter_active = false;
    // Default: No letters and figures buttons present
    typex_buttons_active = false;
    // Default: No step button present
    use_kl7_step_button = false;
    // Default: No figure lamp present
    use_figure_lamp = false;   
    // Default: No Schreibmax used
    use_schreibmax = false;      
    
    // Initialize internal data structures according to machine type
    if ((machine_to_visualize == "M3") || (machine_to_visualize == "M4") || (machine_to_visualize == "Services") || (machine_to_visualize == "Tirpitz") ||
        (machine_to_visualize == "Abwehr") || (machine_to_visualize == "Railway") || (machine_to_visualize == "KD"))
    {
        build_enigma(r_names, numeric_wheels, machine_to_visualize, utilize_schreibmax, r_most_pos);
    }
    else if ((machine_to_visualize == "CSP889") || (machine_to_visualize == "CSP2900"))
    {
        build_sigaba(r_names);
    }
    else if (machine_to_visualize == "KL7")
    {
        build_kl7(r_names);
    }
    else if (machine_to_visualize == "Typex")
    {
        build_typex(r_names);
    }
    else if (machine_to_visualize == "Nema")
    {
        build_nema(r_names);
    }
    else
    {
        build_sg39(r_names);
    }
    
    // Initialize clickable_elements member according to which elements are needed
    fill_data_structures();
        
    set_can_focus(true);
    // Make sure we receive mouse button events
    add_events(Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK | Gdk::KEY_PRESS_MASK | Gdk::KEY_RELEASE_MASK); 
    // Set callback for mouse button events
    signal_event().connect( sigc::mem_fun(*this, &rotor_draw::on_clicked));
    // Set callbacks handling events for pressing and releasing keys on computer keyboard
    signal_key_press_event().connect( sigc::mem_fun(*this, &rotor_draw::on_key));
    signal_key_release_event().connect( sigc::mem_fun(*this, &rotor_draw::on_key));
    
    // Default mode: Encryption
    enc_flag = true;
    
    keys->set_output_device(lamps.get());  
    // Subscribe to the signal emitted by the simulated rotor machine keyboard that tells us what the last input and output keycodes were
    inout_callback_conn = keys->signal_inout_value().connect(sigc::mem_fun(this, &rotor_draw::inout_value_callback));
    // Subscribe to the signal emitted by the simulated rotor machine keyboard that tells us that the current (simulated) key has been released
    key_up_callback_conn = keys->signal_key_up().connect(sigc::mem_fun(this, &rotor_draw::key_up_callback)); 
}

/*! \param r_names [in] Specifies the symbolic names of the rotor slots that are to be used with the simulated Schluesselgeraet 39.
 */
void rotor_draw::build_sg39(vector<string>& r_names)
{
    keys = boost::shared_ptr<keyboard_base>(new enigma_keyboard(sigc::mem_fun(this, &rotor_draw::get_enc_flag), 
                                                                sigc::mem_fun(this, &rotor_draw::get_machine), sigc::mem_fun(this, &rotor_draw::update_rotors)));  
    set_triangular_keyboard_layout(keys.get(), "qwertzuiopasdfghjklyxcvbnm", SIZE_Y_DEFAULT);
    add_dual_printer();
    visualizer = boost::shared_ptr<rotor_visualizer>(new sg39_rotor_visualizer(r_names, 420));
    add_counter(530, COUNTER_ROW_Y);
}

/*! \param r_names [in] Specifies the symbolic names of the rotor slots that are to be used with the simulated Nema.
 */
void rotor_draw::build_nema(vector<string>& r_names)
{
    keys = boost::shared_ptr<keyboard_base>(new enigma_keyboard(sigc::mem_fun(this, &rotor_draw::get_enc_flag), 
                                                                sigc::mem_fun(this, &rotor_draw::get_machine), sigc::mem_fun(this, &rotor_draw::update_rotors)));  
    set_triangular_keyboard_layout(keys.get(), "qwertzuiopasdfghjklyxcvbnm", SIZE_Y_DEFAULT);
    add_lampboard();
    set_triangular_lampboard_layout(dynamic_cast<enigma_lamp_board *>(lamps.get()));
    visualizer = boost::shared_ptr<rotor_visualizer>(new thin_rotor_visualizer(r_names, true, 370));
    add_counter(530, COUNTER_ROW_Y);
    
    ustring nema_alpha = "ijklmnopqrstuvwxyzabcdefgh";
    vector<gunichar> alpha_help;
    
    printing_device::ustr_to_vec(nema_alpha, alpha_help);
    boost::scoped_ptr<alphabet<gunichar> >vis_alpha(new alphabet<gunichar>(alpha_help));
    
    set_keycode_alpha(vis_alpha.get());
}

/*! \param r_names [in] Specifies the symbolic names of the rotor slots that are to be used with the simulated Typex.
 */
void rotor_draw::build_typex(vector<string>& r_names)
{
    keys = boost::shared_ptr<keyboard_base>(new typex_keyboard(sigc::mem_fun(this, &rotor_draw::get_enc_flag), 
                                                               sigc::mem_fun(this, &rotor_draw::get_machine), sigc::mem_fun(this, &rotor_draw::update_rotors)));  
    set_triangular_keyboard_layout(keys.get(), "qwertyuiopasdfghjklzxcvbnm", SIZE_Y_SPACE_BAR);
    // Add space bar on specified position where the 'X' key doubles as space key
    keys->add_space_bar(300, 30, 'x');
    add_printer();
    visualizer = boost::shared_ptr<rotor_visualizer>(new enigma_visualizer(r_names, false, 380));
    add_counter(530, COUNTER_ROW_Y);       
    add_ltr_fig_gui();
}

/*! \param r_names [in] Specifies the symbolic names of the rotor slots that are to be used with the simulated KL7.
 */
void rotor_draw::build_kl7(vector<string>& r_names)
{
    keys = boost::shared_ptr<keyboard_base>(new kl7_keyboard(sigc::mem_fun(this, &rotor_draw::get_enc_flag), 
                                                             sigc::mem_fun(this, &rotor_draw::get_machine), sigc::mem_fun(this, &rotor_draw::update_rotors)));    
    set_triangular_keyboard_layout(keys.get(), "qwertyuiopasdfghjklzxcvbnm", SIZE_Y_SPACE_BAR);
    // Add space bar on specified position where the 'Z' key doubles as space key
    keys->add_space_bar(300, 30, 'z');        
    add_printer();
    visualizer = boost::shared_ptr<rotor_visualizer>(new thin_kl7_rotor_visualizer(r_names, 430));
    add_counter(530, BUTTON_ROW_Y);
    add_ltr_fig_gui();    
    add_step_button();
}

/*! \param r_names [in] Specifies the symbolic names of the rotor slots that are to be used with the simulated SIGABA variant.
 */
void rotor_draw::build_sigaba(vector<string>& r_names)
{
    keys = boost::shared_ptr<keyboard_base>(new enigma_keyboard(sigc::mem_fun(this, &rotor_draw::get_enc_flag), 
                                                                sigc::mem_fun(this, &rotor_draw::get_machine), sigc::mem_fun(this, &rotor_draw::update_rotors)));      
    set_triangular_keyboard_layout(keys.get(), "qwertyuiopasdfghjklzxcvbnm", SIZE_Y_SPACE_BAR);
    // Add space bar on specified position where the 'Z' key doubles as space key
    keys->add_space_bar(300, 30, 'z');  
    add_printer();  
    visualizer = boost::shared_ptr<rotor_visualizer>(new sigaba_rotor_visualizer(r_names, 565)); 
    add_counter(530, BUTTON_ROW_Y);                                                                            
}

void rotor_draw::add_schreibmax()
{
    int current_width, current_height;
    
    use_schreibmax = true;
    add_printer();
    
    // Schreibmax printed four letter groups in lower case
    get_size_request(current_width, current_height);
    printer_visualizer *prt = dynamic_cast<printer_visualizer *>(lamps.get());
    prt->set_grouping_width(4);
    prt->set_use_lower_case(true);
    prt->set_width(current_width);
    
    // Schreibmax had two additional keys that allowed to print a '+' or ' '
    plus_button = boost::shared_ptr<button>(new button(20, BUTTON_ROW_Y, " + ", 55, 30));
    plus_button->signal_pressed().connect(sigc::bind(sigc::mem_fun(*this, &rotor_draw::print_char), '+'));
    blank_button = boost::shared_ptr<button>(new button(532, BUTTON_ROW_Y, "SPC", 60, 30));             
    blank_button->signal_pressed().connect(sigc::bind(sigc::mem_fun(*this, &rotor_draw::print_char), ' ')); 
}

/*! \param r_names [in] Specifies the symbolic names of the rotor slots that are to be used with the simulated Enigma variant.
 *  \param numeric_wheels [in] True if the markings on the rotors circumfences should be numbers. False when characters are desired.The
 *                             only historically correct machine to use numeric markings is the Services Enigma (Wehrmachtsenigma).
 *  \param machine_to_visualize [in] Name of the machine to visualize. Currently the following values are supported: 
 *                                   M3, M4, Services, Abwehr, Tirpitz, Railway and KD.
 *  \param utilize_schreibmax [in] True if a Schreibmax printer is to be simulated.
 *  \param r_most_pos [in] Specifies the x coordinate of the position where the rightmost rotor of the rotor stack should be drawn.
 */
void rotor_draw::build_enigma(vector<string>& r_names, bool numeric_wheels, string machine_to_visualize, bool utilize_schreibmax, int r_most_pos)
{
    keys = boost::shared_ptr<keyboard_base>(new enigma_keyboard(sigc::mem_fun(this, &rotor_draw::get_enc_flag), 
                                                                sigc::mem_fun(this, &rotor_draw::get_machine), sigc::mem_fun(this, &rotor_draw::update_rotors)));      
    if (!utilize_schreibmax)    
    {
        add_lampboard();    
    }
    else
    {
        add_schreibmax();
    }
    
    visualizer = boost::shared_ptr<rotor_visualizer>(new enigma_visualizer(r_names, numeric_wheels, r_most_pos));
    
    // Abwehr was the only Enigma variant with a counter
    if (machine_to_visualize == "Abwehr")
    {
        add_counter(510, COUNTER_ROW_Y);
    }
}

void rotor_draw::add_lampboard()
{
    enigma_lamp_board *lamps_help = new enigma_lamp_board();
    lamps = boost::shared_ptr<output_device>(lamps_help);
}

void rotor_draw::add_printer()
{
    printer_visualizer *temp = new printer_visualizer(sigc::mem_fun(this, &rotor_draw::get_enc_flag), sigc::mem_fun(this, &rotor_draw::redraw), signal_mode_changed());
    lamps = boost::shared_ptr<output_device>(temp);
    lamps->set_width(SIZE_X_BIG_KEYBOARD);
}

void rotor_draw::add_dual_printer()
{
    int current_width, current_height;
    
    get_size_request(current_width, current_height);
    printer_base *temp = new dual_printer(sigc::mem_fun(this, &rotor_draw::get_enc_flag), sigc::mem_fun(this, &rotor_draw::redraw), signal_mode_changed(), 0, 320 - 50);
    lamps = boost::shared_ptr<output_device>(temp);
    lamps->set_width(current_width);
}


void rotor_draw::set_triangular_lampboard_layout(enigma_lamp_board *lamps_help)
{
    int org_pos_x, org_pos_y;    
    
    lamps_help->set_lamp_sequence("QWERTZUIOPASDFGHJKLYXCVBNM");        

    // Draw rows one and two further to the left    
    draw_parameters org_parms = lamps_help->get_draw_parameters();
    org_parms.offset_row.clear();
    org_parms.offset_row.push_back(-100);
    org_parms.offset_row.push_back(-80);
    lamps_help->set_draw_parameters(org_parms);

    // move the whole lampboard further to the right
    lamps_help->get_elem_pos(org_pos_x, org_pos_y);
    org_pos_x += 80;
    lamps_help->set_elem_pos(org_pos_x, org_pos_y);

    lamps_help->set_line_breaks(10, 19);
    // Triangular lampboard layout needs more space in x-direction than Enigma layout
    lamps_help->set_width(SIZE_X_BIG_KEYBOARD);
}

/*! \param keys [in] The keyboard object that is to be configured.
 *  \param sequence [in] Specifies the keys as shown on the keyboard when read from the upper left to the lower right.
 *  \param new_y_size [in] Specifies the new height of the simulator's GUI. This is intended to make room for the optional
 *                         space bar that is used by some machines.
 */
void rotor_draw::set_triangular_keyboard_layout(keyboard_base *keys, ustring sequence, int new_y_size)
{
    int org_pos_x, org_pos_y; 
    
    keys->permute_key_sequence(sequence);  
    
    // Triangular keyboard layout needs more space in x-direction than Enigma layout. Enlarge DrawingArea accordingly.
    // Also adapt size in y-direction (needed to make room for otional space bar) if necessary.   
    set_size_request(SIZE_X_BIG_KEYBOARD, new_y_size);             

    // Draw rows one and two further to the left    
    draw_parameters org_parms = keys->get_draw_parameters();
    org_parms.offset_row.clear();
    org_parms.offset_row.push_back(-100);
    org_parms.offset_row.push_back(-80);
    org_parms.width = SIZE_X_BIG_KEYBOARD;
    keys->set_draw_parameters(org_parms);

    // move the whole keyboard further to the right    
    keys->get_elem_pos(org_pos_x, org_pos_y);
    org_pos_x += 80;
    keys->set_elem_pos(org_pos_x, org_pos_y);
    
    keys->set_line_breaks(10, 19);
}

void rotor_draw::add_step_button()
{
    use_kl7_step_button = true;
    
    step_button = boost::shared_ptr<button>(new button(160, BUTTON_ROW_Y, "STEP", 70, 30));             
    step_button->signal_pressed().connect(sigc::mem_fun(*this, &rotor_draw::step_machine));
}

void rotor_draw::add_counter(int x_pos, int y_pos)
{
    char_counter = boost::shared_ptr<counter>(new counter(x_pos, y_pos, 70, 30));
    is_counter_active = true;
}

/*! Add "buttons" for letter and figure shift for some machines as well as a "lamp" that glows when figure shift
 *  is active
 */
void rotor_draw::add_ltr_fig_gui()
{
    // Add buttons
    typex_buttons_active = true;
    
    lettrs_button = boost::shared_ptr<button>(new button(20, BUTTON_ROW_Y, "LTR", 60, 30));
    lettrs_button->signal_pressed().connect(sigc::bind(sigc::mem_fun(*this, &rotor_draw::simulate_key_press), '<'));
    figs_button = boost::shared_ptr<button>(new button(90, BUTTON_ROW_Y, "FIG", 60, 30));             
    figs_button->signal_pressed().connect(sigc::bind(sigc::mem_fun(*this, &rotor_draw::simulate_key_press), '>')); 

    // Add figures lamp
    use_figure_lamp = true; 
    
    figure_lamp = boost::shared_ptr<lamp>(new lamp(260, BUTTON_ROW_Y + 15, 'f'));
    figure_lamp->set_lamp_on_col(BLACK);
    figure_lamp->set_lamp_bkg_col(WHITE);
    figure_lamp->set_lamp_off_col(WHITE);
    figure_lamp->set_radius(15.0);         
    figure_lamp->set_char_size(20);
    figure_lamp->is_illuminated = false;
}

void rotor_draw::key_up_callback()
{
    unsigned int shifting_state = 0;
    
    // Determine whether the machine is in letters or figures mode. When doing encryptions only the input side
    // can be in one of the two modes. When doing decryptions only the output side is relevant.  
    if (get_enc_flag())
    {
        shifting_state = the_machine->get_keyboard()->get_state();
    }
    else
    {
        shifting_state = the_machine->get_printer()->get_state();    
    }
    
    // Set state of figure lamp (if present)
    if (use_figure_lamp)
    {
        switch(shifting_state)
        {
            case FUNC_FIGURE_SHIFT:
                figure_lamp->is_illuminated = true;
                break;
            case FUNC_LETTER_SHIFT:
                figure_lamp->is_illuminated = false;
                break;
            default:
                 break;
        }
        
        redraw_element(figure_lamp.get());                
    }
}

rotor_draw::~rotor_draw()
{
    inout_callback_conn.disconnect();
    key_up_callback_conn.disconnect();
}

bool rotor_draw::on_clicked(GdkEvent *event)
{ 
    Glib::RefPtr<Gdk::Window> window = get_window();
    vector<element *>::iterator click_iter;
  
    // We only look for mouse button down or up events
    if( ((event->type == GDK_BUTTON_PRESS) || (event->type == GDK_BUTTON_RELEASE)))
    {
        GdkEventButton *button_event = (GdkEventButton *)event;
        
        // Was it a click with the left mouse button?
        if(window && (button_event->button == 1))
        {  
            // Yes          
            Cairo::RefPtr<Cairo::Context> cr = window->create_cairo_context();

            // mouse button down
            if ((event->type == GDK_BUTTON_PRESS))
            {
                for (click_iter = clickable_elements.begin(); click_iter != clickable_elements.end(); ++click_iter)
                {
                    // ask each element whether it wants to handle the click 
                    if ((*click_iter)->test(button_event->x, button_event->y))
                    {
                        // Make the element react on the click
                        (*click_iter)->on_mouse_button_down(cr, button_event->x, button_event->y);
                    }
                }
            }
            else // mouse buton up
            {
                // Currently only the keyboard reacts on mouse button up events
                keys->on_mouse_button_up(cr);
            }
        }
    }
        
    return false;
}

void rotor_draw::simulate_key_release()
{
    Glib::RefPtr<Gdk::Window> window = get_window();
      
    if (window)        
    {
        Cairo::RefPtr<Cairo::Context> cr = window->create_cairo_context();        
        keys->on_key_up(cr, GDK_KEY_Shift_L); // Using the left shift key "unstucks" any stuck key
    }
}

void rotor_draw::simulate_key_press(gunichar simulated_key)
{
    Glib::RefPtr<Gdk::Window> window = get_window();
      
    if (window)        
    {
        Cairo::RefPtr<Cairo::Context> cr = window->create_cairo_context();        
        keys->simulate_key_press(cr, simulated_key);
    }
}

void rotor_draw::step_machine()
{
    Glib::RefPtr<Gdk::Window> window = get_window();
        
    the_machine->step_rotors(); 
      
    if (window)        
    {
        Cairo::RefPtr<Cairo::Context> cr = window->create_cairo_context();        
        visualizer->update_rotors(cr);
    }
}

void rotor_draw::update_rotors(Cairo::RefPtr<Cairo::Context> cr)
{
    visualizer->update_rotors(cr);
}

/*! Only the simulated keyboard reacts to key press events generated by the real keyboard of the computer.
*/  
bool rotor_draw::on_key(GdkEventKey *event)
{ 
    Glib::RefPtr<Gdk::Window> window = get_window();
  
    if (window)        
    {
        Cairo::RefPtr<Cairo::Context> cr = window->create_cairo_context();
        
        if ((event->type == GDK_KEY_PRESS))
        {
            keys->on_key_press(cr, event->keyval);
        }
        else
        {
            keys->on_key_up(cr, event->keyval);
        }
    }       
    
    return false;
}

void rotor_draw::set_lampboard_state(bool new_lampboard_state) 
{
    if (!new_lampboard_state)    
    {
        // Switch lampboard off
        
        Glib::RefPtr<Gdk::Window> window = get_window(); 
        
        if (window)
        {
            Cairo::RefPtr<Cairo::Context> cr = window->create_cairo_context();
            // Switch off any lamps that "glow" at the moment
            lamps->output_symbol_stop(cr);
        }
    }
    
    lamps->set_is_active(new_lampboard_state); 
}


void rotor_draw::set_machine(rotor_machine *machine)
{
    the_machine = machine;
    
    visualizer->set_machine(machine);
    set_enc_flag(true);    
    redraw();    
}

void rotor_draw::set_keycode_alpha(alphabet<gunichar> *new_keycode_alpha)
{    
    keys->set_keycode_alpha(new_keycode_alpha);
    redraw();        
}

void rotor_draw::inout_value_callback(unsigned int in_value, unsigned int out_value)
{        
    if (is_counter_active)
    {
        char_counter->increment();
        redraw_element(char_counter.get());
    }
}

void rotor_draw::set_enc_flag(bool do_enc)
{ 
    enc_flag = do_enc; 

    // Switch to letters mode when en/decryption state changed
    the_machine->get_keyboard()->set_state(FUNC_LETTER_SHIFT);
    the_machine->get_printer()->set_state(FUNC_LETTER_SHIFT);
    
    key_up_callback();        
    
    mode_setting_changed.emit(); 
}

void rotor_draw::print_char(gunichar letter)
{
    // Test if we are using a printer as output device
    if (dynamic_cast<printer_visualizer *>(lamps.get()) != NULL)
    {
        // Yes! A printer is used
        // Create drawing context
        Glib::RefPtr<Gdk::Window> window = get_window(); 
        
        if (window)
        {
            Cairo::RefPtr<Cairo::Context> cr = window->create_cairo_context();
            // Print letter on paper strip
            lamps->output_symbol_start(cr, letter);
            lamps->output_symbol_stop(cr);
            // Tell log window about the generated output character 
            keys->signal_output_char().emit(letter);       
        }    
    }
}

void rotor_draw::redraw_element(element *elem)
{
    Glib::RefPtr<Gdk::Window> window = get_window(); 
    
    if (window)
    {
        Cairo::RefPtr<Cairo::Context> cr = window->create_cairo_context();
        elem->draw(cr);
    }    
}

void rotor_draw::redraw()
{
    Glib::RefPtr<Gdk::Window> window = get_window();
    
    if(window)
    {
        window->invalidate(true);
    }
}

bool rotor_draw::on_draw(const Cairo::RefPtr<Cairo::Context>& cr)
{
    vector<element *>::iterator click_iter;
    
    Gtk::Allocation allocation = get_allocation();
    const int width = allocation.get_width();
    const int height = allocation.get_height();

    // Draw background first
    cr->set_source_rgb(BACKGROUND_GREY);
    cr->rectangle(0, 0, width, height);
    cr->fill();
    cr->stroke();
    
    // Draw all known elements, i.e. all elements that are referenced through clickable_elements
    for (click_iter = clickable_elements.begin(); click_iter != clickable_elements.end(); ++click_iter)
    {
        (*click_iter)->draw(cr);
    }

    return true;
}

void rotor_draw::fill_data_structures()
{
    clickable_elements.clear();

    // Include required elements
    clickable_elements.push_back(visualizer.get());
    clickable_elements.push_back(keys.get());    
    clickable_elements.push_back(lamps.get()); 
    
    // Include optional elements as prescribed by the relevant flags 
    if (is_counter_active)
    {
        clickable_elements.push_back(char_counter.get()); 
    }    
    
    if (typex_buttons_active)
    {
        clickable_elements.push_back(lettrs_button.get());
        clickable_elements.push_back(figs_button.get());        
    }
        
    if (use_kl7_step_button)
    {
        clickable_elements.push_back(step_button.get());            
    }
    
    if (use_figure_lamp)
    {
        clickable_elements.push_back(figure_lamp.get());
    }  
    
    if (use_schreibmax)
    {
        clickable_elements.push_back(plus_button.get());    
        clickable_elements.push_back(blank_button.get());            
    }    
}

void rotor_draw::set_key_board(boost::shared_ptr<keyboard_base> new_key_board) 
{ 
    keys = new_key_board; keys->set_output_device(lamps.get()); 
    fill_data_structures();    
}

void rotor_draw::set_output_device(boost::shared_ptr<output_device> new_output_device)
{
    lamps = new_output_device; keys->set_output_device(lamps.get());
    fill_data_structures();
}

bool rotor_draw::get_lampboard_state() 
{ 
    return lamps->get_is_active(); 
}

