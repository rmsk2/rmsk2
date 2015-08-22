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

#ifndef __output_device_h__
#define __output_device_h__

/*! \file output_device.h
 *  \brief Header file for the GUI classes that implement the simulated lampboard and printer.
 */ 

#include<sigc++/functors/slot.h>
#include<boost/asio.hpp>
#include<base_elements.h>

/*! \brief Default value of the y-position of the first row of lamps in a lampboard. */        
const int LAMP_ROW_ONE_Y = 280;

/*! \brief Default value for the x-psoition of the leftmost lamp in the first row of a lampboard. */        
const int LAMP_ROW_ONE_X = 55;

/*! \brief An abstract GUI class that describes an interface of an object that knows how to draw a rotor machine
 *         output device into a Cairo drawing context.
 *  
 *  An output_device visualizes an output symbol in two phases. The first is the start phase and the second the stop
 *  phase. A symbol is considered to have been fully visualized if the second phase has been completed.
 *
 *  An output_device is intended to be placed between the simulated keyboard and the visualized rotor stack. The 
 *  output_device is separated from the rotor stack by the so called upper border. The upper border is visualized
 *  by a horizontal line.
 *
 *  This interface is implemented in three forms. The first one is a simulated lampboard, the second is a simulated
 *  lampboard that also sends the output symbol to a serial interface to which a lampboard with real lamps or LEDs 
 *  may be connected and the last one is a simulated printer, that prints on a paper strip.
 */
class output_device : public element {
public:
    /*! \brief Constructor. The parameters pos_x and pos_y have to specify the position where the output device is to be drawn.
     */
    output_device(int pos_x, int pos_y) : element(pos_x, pos_y) { upper_border = 223; width = SIZE_X_DEFAULT; }
    
    /*! \brief Draws the output device into the Cairo context specified by the parameter cr. 
     *
     *  In this base class implementation only the upper border is drawn.
     */    
    virtual void draw(Cairo::RefPtr<Cairo::Context> cr);

    /*! \brief This method implements the first phase of the visualization of the output character given in parameter 
     *         symbol. The result of this visualization is drawn into the drawing context specified in parameter cr.
     */
    virtual void output_symbol_start(Cairo::RefPtr<Cairo::Context> cr, gunichar symbol) = 0;

    /*! \brief This method implements the second phase of the visualization of the output character that has been determined
     *         by a previous call to output_device::output_symbol_start(). The result of this visualization is drawn into the
     *         drawing context specified in parameter cr.
     */
    virtual void output_symbol_stop(Cairo::RefPtr<Cairo::Context> cr) = 0;         

    /*! \brief This method is intended to perform the operations necessary to reset the output_device.
     */    
    virtual void reset() { ; }

    /*! \brief This method sets the width of the GtK::DrawingArea in which the visualization of the output character does
     *         happen.
     */    
    virtual void set_width(int new_val) { width = new_val; }

    /*! \brief This method returns the width of the GtK::DrawingArea in which the visualization of the output character does
     *         happen.
     */    
    virtual int get_width() { return width; }

    /*! \brief This method sets the y-position where the upper border is to be drawn.
     */    
    virtual void set_upper_border(int new_val) { upper_border = new_val; }

    /*! \brief This method returns the y-position where the upper border is to be drawn.
     */    
    virtual int get_upper_border() { return upper_border; }

    /*! \brief Destructor.
     */    
    virtual ~output_device() { ; }    

protected:
    /*! \brief Holds the y-position of the upper border. */    
    int upper_border;
    
    /*! \brief Holds the width of the whole DrawingArea. */        
    int width;    
};

/*! \brief A GUI class that knows how to draw an Enigma style lampboard into a Cairo drawing context.
 *
 *  An Enigma lampboard has n (historically correct is three) rows of lamps. The row with the smallest y-position (the one
 *  closest to the menu bar of the simulator's window) is called the first row. 
 *
 *  The member enigma_lamp_board::line_breaks specifies the number of lamps in the first, second, ..., second but last rows. 
 *  Default is the Enigma layout 9 (first row), 8 (second row), 9 (third row). This can be changed by calling the method 
 *  enigma_lamp_board::set_line_breaks(). 
 *  
 *  The current sequence of the lamps as they appear in the GUI is stored in the member variable keyboard_base::std_keys. 
 *  Default is "QWERTZUIOASDFGHJKPYXCVBNML".
 */
class enigma_lamp_board : public output_device {
public:
    /*! \brief Constructor. The parameters pos_x and pos_y have to specify the position of the leftmost lamp in the first row.
     */
    enigma_lamp_board(int pos_x = LAMP_ROW_ONE_X, int pos_y = LAMP_ROW_ONE_Y);

    /*! \brief This method draws the lampboard in the drawing context specified by the parameter cr. At most one lamp can be
     *         illuminated.
     */    
    virtual void draw(Cairo::RefPtr<Cairo::Context> cr);

    /*! \brief This method puts the lamp, that is specified through the parameter symbol, into its illuminated state and redraws the
     *         lampboard utilizing the drawing context given in parameter cr.
     *
     *  This method is called if a key is pressed on the real keyboard of the computer or the simulated keyboard.
     */    
    virtual void output_symbol_start(Cairo::RefPtr<Cairo::Context> cr, gunichar symbol);

    /*! \brief This method switches the currently illumionated lamp (if there is any) off and redraws the lampboard utilizing
     *         the drawing context given in parameter cr.
     *
     *  This method is called when a key on the real keyboard of the computer or the simulated keyboard is released.
     */    
    virtual void output_symbol_stop(Cairo::RefPtr<Cairo::Context> cr);

    /*! \brief Returns the ::draw_parameters that are currently in use in this instance of enigma_lamp_board. 
     */    
    virtual draw_parameters get_draw_parameters() { return draw_param; }

    /*! \brief Allows to set the ::draw_parameters that are currently in use in this instance of enigma_lamp_board to the new value
     *         given in parameter new_params.
     */
    virtual void set_draw_parameters(draw_parameters new_params) { draw_param = new_params; }

    /*! \brief Allows to change the position where the enigma_lamp_board is drawn. The new position has to be specified by the parameters
     *         new_x and new_y.
     */
    virtual void set_elem_pos(int new_x, int new_y);

    /*! \brief Allows to change the sequence of lamps as they appear in the simulator's GUI. The new sequence is given by the parameter
     *         sequence_of_lamps. This method creates a new set of lamps.
     */
    virtual void set_lamp_sequence(ustring sequence_of_lamps);

    /*! \brief Allows to change the positions in the sequence of lamps that signify the start of the second (parameter line_break_1) and
     *         third (parameter line_break_2) rows in which the lamps are arranged.
     *
     *  For an Enigma style keyboard the line breaks are at positions 9 and 17.
     */
    virtual void set_line_breaks(unsigned int break_line_1, unsigned int break_line_2); 

    /*! \brief Allows to change the positions in the sequence of lamps that signify the start of the rows in which the lamps are arranged.
     *         The parameter new_line_breaks has to specifiy the new line breaks to use.
     */                
    virtual void set_line_breaks(vector<unsigned int>& new_line_breaks);    

    /*! \brief Destructor. Deletes all the lamp objects that are referenced through the member variable enigma_lamp_board::lamps.
     */        
    virtual ~enigma_lamp_board();
    
protected:
    /*! \brief Positions the lamps referenced in enigma_lamp_board::lamps according to the position of the lampboard and the drawing
     *         parameters held in enigma_lamp_board::draw_param,
     */        
    void set_lamp_positions();

    /*! \brief Returns a new set of lamps as specified by the member variable enigma_lamp_board::std_lamps.
     */        
    std::map<gunichar, lamp*>  create_std_lamps();

    /*! \brief Holds pointers to the current set of lamps in use in this lampboard. */        
    std::map<gunichar, lamp*> lamps; 
    
    /*! \brief Points to the currently illuminated lamp or is NULL if no lamp is illuminated. */            
    lamp *illuminated_symbol;
    
    /*! \brief Holds the ::draw_parameters in use in this enigma_lamp_board object. */                
    draw_parameters draw_param;
    
    /*! \brief Holds the current sequence of the lamps. */                    
    ustring std_lamps;
    
    /*! \brief Holds the current line breaks in use in this instance of enigma_lamp_board. */                        
    vector<unsigned int> line_breaks;    
};

/*! \brief A GUI class that is identical to ::enigma_lamp_board but that also sends the output symbol to a serial interface.
 *
 *  This can be used to control a lampboard with real lamps or LEDs via a serial interface. This is really experimental
 *  and has been barely tested. Tests were performed in a proof of concept scenario with an Arduino based lampboard of
 *  four LEDs.
 */
class enigma_real_lamp_board : public enigma_lamp_board {
public:
    /*! \brief Constructor. The parameters pos_x and pos_y have to specify the position of the leftmost lamp in the first row.
     *
     *  The port parameter has to specify the device file (for instance /dev/ttyUSB0) which can be used to access the serial port.
     */
    enigma_real_lamp_board(string port, int pos_x = LAMP_ROW_ONE_X, int pos_y = LAMP_ROW_ONE_Y);

    /*! \brief This method puts the lamp, that is specified through the parameter symbol, into its illuminated state and redraws the
     *         lampboard utilizing the drawing context given in parameter cr.
     *
     *  This method is called if a key is pressed on the real keyboard of the computer or the simulated keyboard. The output character
     *  is also sent to the serial port specified in the member variable enigma_real_lamp_board::serial_port. 
     * 
     *  Caveat: Only one byte is sent to the serial port so this will most probably not work for Unicode characters whose UTF-8 
     *  representation is longer than one byte. 
     */    
    virtual void output_symbol_start(Cairo::RefPtr<Cairo::Context> cr, gunichar symbol);
    
    /*! \brief This method switches the currently illuminated lamp (if there is any) off and redraws the lampboard utilizing
     *         the drawing context given in parameter cr.
     *
     *  This method is called when a key on the real keyboard of the computer or the simulated keyboard is released. A special
     *  character (0x80) is also sent to the serial port specified in the member variable enigma_real_lamp_board::serial_port. 
     */        
    virtual void output_symbol_stop(Cairo::RefPtr<Cairo::Context> cr);

    /*! \brief Destructor. Deletes all the lamp objects that are referenced through the member variable enigma_lamp_board::lamps.
     */    
    virtual ~enigma_real_lamp_board();
    
protected:
    /*! \brief Holds a specification of the serial port to use, i.e. something like "/dev/ttyUSB0". */
    string serial_port;    
    
    /* From the boost docs: The io_service represents your program's link to the operating system's I/O services. */
    boost::asio::io_service io;
    
    /*! \brief Boost object that abstracts the serial port to use. */    
    boost::asio::serial_port *serial;    
};

/*! \brief A struct that contains parameters that influence how a printer_visualizer is drawn.
 */
struct printer_params {
    /*! \brief Default constructor.
     */
    printer_params() { height = 30; }
    
    /*! \brief Holds the height of the simulated paper strip in pixels.
     */    
    unsigned int height;
};

/*! \brief A GUI class that knows how to draw a simulated printer into a Cairo drawing context.
 *
 *  A printer_visualizer draws a white strip on which the symbols to be printed appear character by character. Depending on
 *  whether the underlying machine is in en- or decryption mode the characters can be grouped or not. After printing
 *  a symbol the paper strip is moved one character to the left. 
 *
 *  A printer_visualizer can be forced to convert all symbols to lower case before they are printed. This is controlled by 
 *  the printer_visualizer::use_lower_case flag. This flag can be set by the method printer_visualizer::set_use_lower_case().
 */
class printer_visualizer : public output_device {
public:
    /*! \brief Constructor. The parameters pos_x and pos_y have to specify the upper left corner of the white paper strip. The
     *         callback enc_state_func has to return a boolean which is used to determine if the underlying rotor machine is
     *         doing encryptios (True) or not (False). The callback redraw_func is intended to initiate a redraw of the simulator's
     *         GUI. A printer_visualizer subscribes to the sig_mode_change signal in order to reset itself when the mode (en-/
     *         decryption) of the underlying rotor machine changes.
     */
    printer_visualizer(sigc::slot<bool> enc_state_func, sigc::slot<void> redraw_func, sigc::signal<void>& sig_mode_change, int pos_x, int pos_y);

    /*! \brief Constructor. Sets a default position. The callbacks have the same semantics as in the constructor that takes
     *         an explicit position as additional parameters.
     *
     *  The default position is (0, PRINTER_Y).
     */
    printer_visualizer(sigc::slot<bool> enc_state_func, sigc::slot<void> redraw_func, sigc::signal<void>& sig_mode_change);    
    
    /*! \brief Draws the simulated printer into the drawing contex specified by parameter cr.
     */        
    virtual void draw(Cairo::RefPtr<Cairo::Context> cr);
    
    /*! \brief This method prints the character given in parameter symbol. For this it utilizes the drawing context
     *         specified in parameter cr.
     */            
    virtual void output_symbol_start(Cairo::RefPtr<Cairo::Context> cr, gunichar symbol);

    /*! \brief This method does nothing.
     */            
    virtual void output_symbol_stop(Cairo::RefPtr<Cairo::Context> cr); 

    /*! \brief This method can be use to set the number of characters in a group. The new group size gas to be specified
     *         through the parameter new_width.
     */                
    virtual void set_grouping_width(unsigned int new_width) { grouping_width = new_width; }

    /*! \brief This method returns the number of characters that make up a group in this instance of printer_visualizer.
     */                
    virtual unsigned int get_grouping_width() { return grouping_width; }

    /*! \brief Calling this method with parameter new_value set to True forces this printer_visualizer to convert all characters
     *         to lower case before they are printed. Calling this method with new_value = False instructs this printer_visualizer
     *         to perform no forced conversion to lower case.
     *
     *  Calling this method with new_value = True does not change the case of letters that have already been printed.
     */                
    virtual void set_use_lower_case(bool new_value) { use_lower_case = new_value; }

    /*! \brief This method returns the value of the flag that determines if characters a converted to lower case before
     *         being printed.
     */                
    virtual bool get_use_lower_case() { return use_lower_case; }
        
    /*! \brief This method allows to set the printer_params that are in use in this instance of printer_visualizer to the new value
     *         given in parameter parms.
     */                
    virtual void set_printer_params(printer_params parms) { params = parms; }

    /*! \brief This method returns the printer_params that are currently in use in this printer_visualizer.
     */                
    virtual printer_params get_printer_params() { return params; }

    /*! \brief Clears the paper strip, empties printer_visualizer::text_buffer and redraws the simulator's GUI.
     */                    
    virtual void reset();
    
    /*! \brief Destructor.
     */                    
    virtual ~printer_visualizer() { ; }   

protected:
    /*! \brief Initializes some member variables. Intended to be called by the two constructors.
     */                    
    void init_data();

    /*! \brief Returns the number of pixels it takes to draw the text specified in parameter to_measure into the drawing
     *         context given in parameter cr.
     */                    
    double measure_string(Cairo::RefPtr<Cairo::Context> cr, ustring& to_measure);

    /*! \brief Connects the printer_visualizer::reset() method to the signal specified in paramete signal. This method is
     *         intended to be called by the two constructors.
     */                    
    void connect_signal(sigc::signal<void>& signal);

    /*! \brief Holds the printer_params that are in use in this instance of printer_visualizer. */                        
    printer_params params;
    
    /*! \brief If this flag is true then this printer_visualizer converts all symbols to lower case before they are printed. */                            
    bool use_lower_case;
    
    /*! \brief Holds the characters that currently appear on the simulated paper strip. */                                
    Glib::ustring text_buffer;
    
    /*! \brief Holds the number of characters that make up a group. */                                    
    unsigned int grouping_width;
    
    /*! \brief Holds the number of characters that are currently missing to a full group. */    
    unsigned int grouping_count;
    
    /*! \brief Holds the connection object by which this printer_visualizer subscribes to the sig_mode_change signal 
     *         specified in the constructor. 
     */        
    sigc::connection mode_change_conn;

    /*! \brief Holds the callback that can be used to determine the current state (en/decryption) of the underlying rotor machine. */    
    sigc::slot<bool> enc_state;

    /*! \brief Holds the callback that can be used to redraw the simulator's GUI. */    
    sigc::slot<void> redraw;       
};


#endif /* __output_device_h__ */

