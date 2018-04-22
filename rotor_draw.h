/***************************************************************************
 * Copyright 2018 Martin Grap
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

#ifndef __rotor_draw_h__
#define __rotor_draw_h__

/*! \file rotor_draw.h
 *  \brief Header file for the class that implements the graphical part of all simulators implemented
 *         by rmsk.
 */ 

#include<keyboard_vis.h>
#include<output_device.h>
#include<rotor_visualizer.h>

class keyboard_base;
class output_device;

/*! \brief A GUI class that provides the graphical part of all simulators implemented by rmsk. 
 *
 *  The Enigma simulator and rotorvis main programs use this class as their interface to all aspects of the simulator that are drawn "by hand" 
 *  and are so isolated from the low level details.         
 *
 *  On top of that this class orchestrates the cooperation between the required and optional subelements. As a convention all children of the ::element  
 *  class delegate direct interaction with the event system of gtkmm to this class. 
 *  The main graphical elements of a rotor_draw object are the keyboard (member rotor_draw::keys), the 
 *  rotor stack (member rotor_draw::visualizer) and the lampboard or printer (member rotor_draw::lamps). On top of that additional (optional) elements are simulated 
 *  for instance a counter that counts the number of characters processed by the machine (member rotor_draw::char_counter), a "figure lamp" that indicates whether 
 *  the machine is in figures or letters mode (member rotor_draw::figure_lamp) and several buttons (members rotor_draw::lettrs_button, rotor_draw::figs_button, 
 *  rotor_draw::step_button). All graphical elements are children of the ::element class.
 *
 *  A rotor_draw object can either be in Encryption or Decryption mode. This mode can be queried by get_enc_flag() and set by set_enc_flag(). True means that the
 *  rotor_draw object is in Encryption mode. 
 *
 *  The output device (member rotor_draw::lamps) can be switched off or on. The current state (on/off) can be queried by get_lampboard_state() and set by
 *  set_lampboard_state(). This functionality has been implemented to speed up the encryption/decryption of longer texts, as not having to draw the output 
 *  symbols speeds things up.
 */
class rotor_draw : public Gtk::DrawingArea {
public:
    
    /*! \brief Constructor.
     */
    rotor_draw(vector<string>& r_names, bool numeric_wheels, string machine_to_visualize, bool utilize_schreibmax, int r_most_pos = RIGHTMOST_ROTOR_POS);
    
    /*! \brief Destructor.
     */    
    virtual ~rotor_draw();

    /*! \brief Sets the rotor_machine object, contained in parameter machine, that processes input and output data. Also resets mode to Encryption.
     */        
    virtual void set_machine(rotor_machine *machine);

    /*! \brief Returns the rotor_machine object currently in use by this rotor_draw object.
     */        
    virtual rotor_machine *get_machine() { return the_machine; }

    /*! \brief Returns the keyboard object currently in use by this rotor_draw object.
     */        
    virtual boost::shared_ptr<keyboard_base> get_key_board() { return keys; }

    /*! \brief Sets the keyboard object that is to be used by this rotor_draw object.
     */        
    virtual void set_key_board(boost::shared_ptr<keyboard_base> new_key_board);

    /*! \brief Returns the lampboard or printer object currently in use by this rotor_draw object.
     */        
    virtual boost::shared_ptr<output_device> get_output_device() { return lamps; }

    /*! \brief Sets the lampboard or printer object that is to be used by this rotor_draw object.
     */        
    virtual void set_output_device(boost::shared_ptr<output_device> new_output_device);

    /*! \brief Returns the object that visualizes the rotor stack currently in use by this rotor_draw object.
     */        
    virtual rotor_visualizer *get_rotor_visualizer() { return visualizer.get(); }
    
    /*! \brief Sets this rotor_draw object to be either in Encryption (do_enc = True) or Decryption (do_enc = False) mode.
     */
    virtual void set_enc_flag(bool do_enc);

    /*! \brief Returns the current mode (En/Decryption) of this rotor_draw object.
     */
    virtual bool get_enc_flag() { return enc_flag; }
    
    /*! \brief Sets the output device in use with this rotor_draw object to be either on (new_lampboard_state = True) or off (new_lampboard_state = False).
    */    
    virtual void set_lampboard_state(bool new_lampboard_state);

    /*! \brief Returns the current state (on/off) of the output device currently in use in this rotor_draw object.
    */    
    virtual bool get_lampboard_state();
    
    /*! \brief This method processes the input symbol specified in parameter simulated_key as if it would have been entered through the simulated keyboard.
     *
     *  Invalid characters are ignored.         
     */
    virtual void simulate_key_press(gunichar simulated_key);

    /*! \brief This method forces the keyboard to release a currently pressed key.
     */
    virtual void simulate_key_release();

    /*! \brief This method causes the symbol specified in parameter letter to be printed on the paper strip of the simulated printer.
     *
     *  If no printer is in use, this method does nothing.
     */
    virtual void print_char(gunichar letter);
    
    /*! \brief Causes the visualizer object that represents the rotor stack of the simulated machine to be redrawn.
     */        
    virtual void update_rotors(Cairo::RefPtr<Cairo::Context> cr);

    /*! \brief Causes the whole simulated machine to be redrawn.
     */        
    virtual void redraw();

    /*! \brief A signal that is emitted when the mode of the machine (En/Decryption) has changed.
     */            
    virtual sigc::signal<void>& signal_mode_changed() { return mode_setting_changed; }    

    /*! \brief A signal that is emitted when the plugboard of an Enigma has been clicked.
     */            
    virtual sigc::signal<void>& signal_plugboard_clicked();

    /*! \brief Callback for mouse click events.
     */                    
    bool on_clicked(GdkEvent *event);

    /*! \brief Callback for computer keyboard events.
     */                    
    bool on_key(GdkEventKey *event);

protected:

    /*! \brief Sets the keycode alphabet of the underlying keyboard.
     *
     *  The keycode alphabet is the mapping of the characters in the output alphabet of the simulated machine (usually rmsk::std_uni_alpha()) to keycodes.
     *  Nema is the exception: Its keycode alphabet is "ijklmnopqrstuvwxyzabcdefgh".
     */
    void set_keycode_alpha(alphabet<gunichar> *new_keycode_alpha);    

    /*! \brief Causes the element to which the paramter elem points to be redrawn.
     */                    
    virtual void redraw_element(element *elem);    

    /*! \brief Draws the simulators graphical representation, i.e. its GUI.
     */                    
    virtual bool on_draw(const Cairo::RefPtr<Cairo::Context>& cr);
    
    /*! \brief Fills the clickable_elements member. If is_counter_active, typex_buttons_active, use_kl7_step_button or use_figure_lamp is False the
     *         corresponding element is not included in clickable_elements.
     */                        
    void fill_data_structures();

    /*! \brief Callback that receives the input and output keycodes of each character processed by the underlying rotor_machine.
     */                        
    virtual void inout_value_callback(unsigned int in_value, unsigned int out_value);

    /*! \brief Causes the underlying rotor_machine to advance its rotors and redraws the visualizer representing the rotor stack.
     */                        
    virtual void step_machine();

    /*! \brief Callback that is called when a simulated key is released.
     */                        
    virtual void key_up_callback(); 

    /*! \brief Causes a letter and a figures button to be shown in the simulator's GUI.
     */                        
    void add_ltr_fig_gui(); 

    /*! \brief Causes a step button to be shown in the simulator's GUI.
     */                        
    void add_step_button();

    /*! \brief Causes a character counter to be shown in the simulator's GUI. The parameters x_pos and y_pos specify the coordinates where
     *         the counter is to be drawn.
     */                        
    void add_counter(int x_pos, int y_pos); 

    /*! \brief Causes a symbolized plugboard to be added to the GUI.
     */                        
    void add_plugboard(); 
    
    /*! \brief Configures the keyboard object (specified by the paramter keys) in such a way that the keys are shown in a triangular
     *         layout (first row 10, second row 9 and third row 7 keys) as opposed to the Enigma standard of (1st row 9, 2nd row 8, 3rd row 9).
     *         All simulated non-Enigma machines use this keyboard layout.
     */                            
    void set_triangular_keyboard_layout(keyboard_base *keys, ustring sequence, int new_y_size); 

    /*! \brief Configures the lampboard object (specified by the paramter lamps_help) in such a way that the lamps are shown in a triangular
     *         layout (first row 10, second row 9 and third row 7 lamps) as opposed to the Enigma standard of (1st row 9, 2nd row 8, 3rd row 9).
     *         This is needed to simulate the Nema.
     */                            
    void set_triangular_lampboard_layout(enigma_lamp_board *lamps_help); 

    /*! \brief Causes a simulated Enigma printer (Schreibmax) to be shown in the simulator's GUI.
     */                        
    void add_schreibmax();

    /*! \brief Causes a printer (simulated paper strip) to be shown in the simulator's GUI.
     */                        
    void add_printer();

    /*! \brief Causes a dual printer (simulated paper strips for input and output characters) to be shown in the simulator's GUI. The parameters
     *         pos_x and pos_y specify the upper left corner where the dual printer is drawn.
     */                        
    void add_dual_printer(int pos_x, int pos_y);

    /*! \brief Causes a lampboard to be shown in the simulator's GUI.
     */                        
    void add_lampboard();

    /*! \brief Initializes the internal data structures and configures the created objects in such a way that the simulator's GUI shows an Enigma variant
     *         corresponding to the parameter machine_to_visualize parameter.
     */                            
    void build_enigma(vector<string>& r_names, bool numeric_wheels, string machine_to_visualize, bool utilize_schreibmax, int r_most_pos);

    /*! \brief Initializes the internal data structures and configures the created objects in such a way that the simulator's GUI shows a SIGABA variant.
     */                            
    void build_sigaba(vector<string>& r_names);

    /*! \brief Initializes the internal data structures and configures the created objects in such a way that the simulator's GUI shows a KL7.
     */                            
    void build_kl7(vector<string>& r_names);    

    /*! \brief Initializes the internal data structures and configures the created objects in such a way that the simulator's GUI shows a Typex.
     */                            
    void build_typex(vector<string>& r_names);

    /*! \brief Initializes the internal data structures and configures the created objects in such a way that the simulator's GUI shows a Nema.
     */                            
    void build_nema(vector<string>& r_names);

    /*! \brief Initializes the internal data structures and configures the created objects in such a way that the simulator's GUI shows a
     *         Schluesselgeraet 39.
     */                            
    void build_sg39(vector<string>& r_names);    

    /*! \brief This shared_ptr holds a pointer to the output_device currently in use in this instance. Most commonly an enigma_lamp_board or a 
     *         printer_visualizer object are used. All simulated machines have to have an output_device.
     */     
    boost::shared_ptr<output_device> lamps;

    /*! \brief This shared_ptr holds a pointer to the graphical ::element that represents the keyboard of the simulated rotor machine.
     *         All simualted machines have to have a keyboard.
     */     
    boost::shared_ptr<keyboard_base> keys;

    /*! \brief This member holds a pointer to the graphical ::element that represents the rotor stack the simulated rotor machine.
     *         All simualted machines have to have a rotor_visualizer.
     */     
    boost::shared_ptr<rotor_visualizer> visualizer;
    
    /*! \brief This shared_ptr holds a pointer to a ::counter ::element. This element is optional. When it is not initialized the member
     *         variable rotor_draw::is_counter_active has to be false.
     */      
    boost::shared_ptr<counter> char_counter;

    /*! \brief This shared_ptr holds a pointer to a ::button ::element. Clicking on the button results in the machine switching into letters mode.
     *         This element is optional. When it is not initialized the member variable rotor_draw::typex_buttons_active has to be false.
     */      
    boost::shared_ptr<button> lettrs_button;

    /*! \brief This shared_ptr holds a pointer to a ::button ::element. Clicking on the button results in the machine switching into figures mode.
     *         This element is optional. When it is not initialized the member variable rotor_draw::typex_buttons_active has to be false.
     */
    boost::shared_ptr<button> figs_button;

    /*! \brief This shared_ptr holds a pointer to a ::button ::element. Clicking on the button results in the machine advancing its rotors one step.
     *         This element is optional. When it is not initialized the member variable rotor_draw::use_kl7_step_button has to be false.
     */
    boost::shared_ptr<button> step_button; 

    /*! \brief This shared_ptr holds a pointer to a ::lamp ::element. If the lamp "glows" the simulated machine is currently in figures mode.
     *         This element is optional. When it is not initialized the member variable rotor_draw::use_figure_lamp has to be false.
     */
    boost::shared_ptr<lamp> figure_lamp;   

    /*! \brief This shared_ptr holds a pointer to a ::button ::element. Clicking on the button results in printig a plus onto the simulated paper strip
     *         of a simulated Schreibmax printer. When it is not initialized the member variable rotor_draw::use_schreibmax has to be false.
     */
    boost::shared_ptr<button> plus_button; 

    /*! \brief This shared_ptr holds a pointer to a ::button ::element. Clicking on the button results in printig a blank onto the simulated paper strip
     *         of a simulated Schreibmax printer. When it is not initialized the member variable rotor_draw::use_schreibmax has to be false.
     */
    boost::shared_ptr<button> blank_button; 

    /*! \brief This shared_ptr holds a pointer to a ::enigma_plugboard ::element. Clicking on the plugboard emits a signal. When it is not initialized 
     *         the member variable rotor_draw::use_plugboard has to be false.
     */
    boost::shared_ptr<enigma_plugboard> plugboard; 

    /*! \brief Holds the underlying rotor_machine object that actually implements the cryptographic functionality.
     */                            
    rotor_machine *the_machine;

    /*! \brief Is True, if the simulator is currently in Encryption mode. In Decryption mode enc_flag is False.
     */                            
    bool enc_flag;

    /*! \brief A signal that is emitted, if the mode (Encryption/Decryption) changes. This is used to make the menus correctly reflect the
     *         the simulator's state.
     */                            
    sigc::signal<void> mode_setting_changed;

    /*! \brief A signal that is returned by signal_plugboard_clicked() when there is no Enigma plugboard.
     */                            
    sigc::signal<void> plugboard_dummy;

    /*! \brief This member holds all ::element objects that need to be drawn. It is therefore used by the on_draw method to iterate over
     *         all objects that make up the simulators GUI. 
     */                            
    vector<element *> clickable_elements;

    /*! \brief Helps the rotor_draw::fill_data_structures method to fill rotor_draw::clickable_elements. When True rotor_draw::char_counter has to point to a
     *         suitable ::element.
     */  
    bool is_counter_active;

    /*! \brief Helps the rotor_draw::fill_data_structures method to fill rotor_draw::clickable_elements. When True rotor_draw::lettrs_button and
     *         rotor_draw::figs_button have to point to a suitable ::element.
     */  
    bool typex_buttons_active;

    /*! \brief Helps the rotor_draw::fill_data_structures method to fill rotor_draw::clickable_elements. When True rotor_draw::step_button has to point to a
     *         suitable ::element.
     */  
    bool use_kl7_step_button;

    /*! \brief Helps the rotor_draw::fill_data_structures method to fill rotor_draw::clickable_elements. When True rotor_draw::figure_lamp has to point to a
     *         suitable ::element.
     */  
    bool use_figure_lamp;

    /*! \brief Helps the rotor_draw::fill_data_structures method to fill rotor_draw::clickable_elements. When True rotor_draw::plus_button and rotor_draw::blank_button    
     *         has to point to a suitable ::element.
     */  
    bool use_schreibmax;

    /*! \brief Helps the rotor_draw::fill_data_structures method to fill rotor_draw::clickable_elements. When True rotor_draw::plugboard    
     *         has to point to a suitable ::element.
     */  
    bool use_plugboard;
    
    /*! \brief Connection resulting from subscribing to the signal of the simulated rotor machine's keyboard that tells us what the last input
     *         and output keycodes were.
     */      
    sigc::connection inout_callback_conn;    

    /*! \brief Connection resulting from subscribing to the signal of the simulated rotor machine's keyboard that tells us that the current
     *         (simulated) key has been released.
     */      
    sigc::connection key_up_callback_conn;    
};


#endif /* __rotor_draw_h__ */

