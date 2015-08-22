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

#ifndef __base_elements_h__
#define __base_elements_h__

/*! \file base_elements.h
 *  \brief Header file for the classes that implement the basic graphical elements used by the class ::rotor_draw.
 */ 

#include<gtkmm/main.h>
#include<gtkmm/drawingarea.h>
#include<cairomm/context.h>

// Some #defines that can be used with Cairo's set_source_rgb method to specify colours. I am usually not fond of
// macro trickery like this, but here it really helps. cr->set_source_rgb(BACKGROUND_GREY) is much more readable
// than something like cr->set_source_rgb(GREY_R, GREY_G, GREY_B)
#define BLACK 0.0, 0.0, 0.0
#define WHITE 1.0, 1.0, 1.0
#define RED 0.5, 0.0, 0.0
#define DARK_GREY 0.2, 0.2, 0.2
#define LIGHT_GREY 0.7, 0.7, 0.7
#define YELLOW 0.9, 1.0, 0.0
#define BACKGROUND_GREY 0.35, 0.35, 0.35

/*! \brief 25 pixels is the default radius of a lamp in a lampboard.*/
const double LAMP_SIZE = 25.0;

/*! \brief 23 pixels is the default radius of a key in a keyboard.*/
const double KEY_SIZE = 23.0;

/*! \brief Size in x-direction of the main DrawingArea in which ::rotor_draw does it's magic. 610 pixels is the
 *         default size of the Enigma simulators.  
 */
const int SIZE_X_DEFAULT = 610;

/*! \brief 630 pixels is the default x-size of all simulators that use a triangular keyboard layout.*/
const int SIZE_X_BIG_KEYBOARD = 630;

using namespace Glib;
using namespace std;

/*! \brief An abstract GUI class that encapsulates the notion of an object that can draw itself into a Cairo context. It serves as the base class
 *         of all graphical elements that are drawn "by hand". 
 *
 *  An element can be active or inactive. In its inactive state it is not interested in handling clicks and therefore the test method
 *  has to return False in that case.
 */
class element {
public:
    /*! \brief Constructor. The parameters pos_x and pos_y have to specify the position where the element is to be drawn.
     */
    element(int pos_x, int pos_y) { x = pos_x; y = pos_y; is_active = true; }

    /*! \brief This method draws the element (in its current state) into the Cairo context specified by the parameter cr. This
     *         method is a pure virtual method and has therefore to be reimplemented in all children of this class.
     */    
    virtual void draw(Cairo::RefPtr<Cairo::Context> cr) = 0;
    
    /*! \brief This method is intended to implement a function that is to be executed when the user clicks on the element. The parameters
     *         pos_x and pos_y have to specifiy the coordinates of the pixel that has been clicked. The parameter cr references a Cairo
     *         context into which the element can redraw itself, if the implemented special function has changed the state of the object in
     *         such a way that it needs to redraw itself in order to reflect that changed state.
     *
     *  The default implementation that is provided in this class does nothing. Children have to reimplement this method in order to override
     *  this behaviour.
     */    
    virtual void on_mouse_button_down(Cairo::RefPtr<Cairo::Context> cr, int pos_x, int pos_y);

    /*! \brief This method is intended to ask an element whether it wants to handle the click that happened at the coordinates (pos_x, pos_y) or
     *         not. Returning True means that the object is interested in handling the click.
     *
     *  The default implementation that is provided in this class always returns False and therefore signifies that the object does not want to 
     *  handle the click. Children have to reimplement this method in order to override this behaviour.
     */    
    virtual bool test(int pos_x, int pos_y) { return false; }
 
     /*! \brief Queries the active flag.
     */
    virtual bool get_is_active() { return is_active; }

     /*! \brief Sets the active flag.
     */
    virtual void set_is_active(bool active_f) { is_active = active_f; }

     /*! \brief Changes the element's position to the coordinates specified by the parameters new_x and new_y.
     */
    virtual void set_elem_pos(int new_x, int new_y) { x = new_x, y = new_y; }

     /*! \brief Returns the element's current position through the variables referenced by the parameters pos_x and pos_y.
     */
    virtual void get_elem_pos(int& pos_x, int& pos_y) { pos_x = x; pos_y = y; }

    /*! \brief Destructor.
     */    
    virtual ~element() { ; }

protected:
    /*! \brief Holds the active flag. True means the element is ready to react to clicks.*/    
    bool is_active;
    
    /*! \brief Holds the x-coordinate of the element's current position. */    
    int x;

    /*! \brief Holds the y-coordinate of the element's current position. */    
    int y;
};

/*! \brief A GUI class that knows how to draw a screw top into a Cairo drawing context.
 *
 *  Elements of this type are used to draw Enigma rotor windows (see class ::enigma_rotor_window). The screw has an orientation which can be
 *  either horizontal (slot drawn horizontally) or not (slot drawn in 45 degree angle). It also has a radius and can be drawn in specified
 *  colour.
 */
class screw : public element {
public:
    /*! \brief Constructor. The parameters pos_x and pos_y have to specify the position where the screw is to be drawn. The
     *         parameter hor_orient has to be true, if the screw is to be drawn in horizontal position.
     */
    screw(int pos_x, int pos_y, bool hor_orient = true);
    
    /*! \brief Draws the screw into the drawing context specified by parameter cr.
     */        
    virtual void draw(Cairo::RefPtr<Cairo::Context> cr);

    /*! \brief Allows to set the radius (in pixels) of the screw. Default is 6 pixels.
     */        
    virtual void set_radius(double r) { radius = r; }

    /*! \brief Allows to set the colour of the screw. Default is BLACK.
     */      
    virtual void set_line_color(double r, double g, double b);

    /*! \brief Destructor.
     */        
    virtual ~screw() { ; }
    
protected:
    /*! \brief Holds orientation of the screw. */        
    bool horizontal_orientation;

    /*! \brief Holds radius of the screw. */        
    double radius;
    
    /*! \brief Holds the red component of the screw's colour. */
    double col_r;
    
    /*! \brief Holds the green component of the screw's colour. */    
    double col_g;
    
    /*! \brief Holds the blue component of the screw's colour. */    
    double col_b;
};

/*! \brief A GUI class that knows how to draw a lamp into a Cairo drawing context.
 *
 *  Elements of this type are used to draw the lamps in the lampboards of the Enigma and Nema simulators. Additionally
 *  they are used as figure lamps (see rotor_draw::figure_lamp). A lamp can be illuminated or not (see lamp::is_illuminated). 
 *  It is also marked with a character, that has to be specified during contruction and a lamp has a radius that determines
 *  its size. 
 *
 *  Any lamp can be configured with four colours.
 *   
 *  1. The colour of the rectangular bounding box of the lamp (the cover colour, default BACKGROUND_GREY).
 *  2. The colour of the circular background of the lamp (default DARK_GREY). 
 *  3. The colour of the character marking when the lamp is not illuminated (lamp off colour, default LIGHT_GREY) 
 *  4. The colur of the character marking when the lamp is illuminated (lamp on colour, default YELLOW).  
 */
class lamp : public element {
public:
    /*! \brief Constructor. The parameters pos_x and pos_y have to specify the position where the lamp is to be drawn. The
     *         parameter c has to specify the marking used for the lamp. This usually is a character.
     *
     *  The position given by pos_ and pos_y determines the center of the lamp.
     */
    lamp(int pos_x, int pos_y, gunichar c);
    
    /*! \brief Default constructor. Values are determined by the set_defaults() method.
     */    
    lamp();

    /*! \brief Draws the lamp into the drawing context specified by parameter cr.
     */      
    virtual void draw(Cairo::RefPtr<Cairo::Context> cr);
 
    /*! \brief Change the lamps radius to the new value given in parameter r. Default value is 25 pixels.
     */       
    virtual void set_radius(double r) { radius = r; }    

    /*! \brief Change the cover colour (see 1.) to the new value specified by the parameters r, g and b .
     */ 
    void set_cover_col(double r, double g, double b);

    /*! \brief Change the background colour (see 2.) to the new value specified by the parameters r, g and b .
     */ 
    void set_lamp_bkg_col(double r, double g, double b);
    
    /*! \brief Change the lamp off colour (see 3.) to the new value specified by the parameters r, g and b .
     */     
    void set_lamp_off_col(double r, double g, double b);

    /*! \brief Change the lamp on colour (see 4.) to the new value specified by the parameters r, g and b .
     */     
    void set_lamp_on_col(double r, double g, double b);
    
    /*! \brief Change the size in pixels of the character marking of this lamp. Default value is 40 pixels.
     */       
    void set_char_size(int new_size) { char_size = new_size; }
 
    /*! \brief Destructor.
     */     
    virtual ~lamp() { ; }
    
    /*! \brief Used to set and get the illumination status of the lamp.
     */       
    bool is_illuminated;

protected:
    /*! \brief Sets member variables to default values.
     */ 
    void set_defaults();

    /*! \brief Holds the character marking. */ 
    gunichar character;
    
    /*! \brief Holds the radius. */     
    double radius;
    
    /*! \brief Holds the cover colour red component. */ 
    double cover_r;

    /*! \brief Holds the cover colour green component. */     
    double cover_g;
    
    /*! \brief Holds the cover colour blue component. */     
    double cover_b;
    
    /*! \brief Holds the background colour red component. */     
    double bkg_r;

    /*! \brief Holds the background colour green component. */         
    double bkg_g;
    
    /*! \brief Holds the background colour blue component. */         
    double bkg_b;
    
    /*! \brief Holds the lamp off colour red component. */         
    double off_r;
    
    /*! \brief Holds the lamp off colour green component. */             
    double off_g;
    
    /*! \brief Holds the lamp off colour blue component. */             
    double off_b;

    /*! \brief Holds the lamp on colour red component. */             
    double on_r;
    
    /*! \brief Holds the lamp on colour green component. */                 
    double on_g;
    
    /*! \brief Holds the lamp on colour blue component. */                 
    double on_b;    
    
    /*! \brief Holds the size in pixels in which the character marking is displayed. */                 
    int char_size;
};

/*! \brief A GUI class that knows how to draw a key from a rotor machine's keyboard into a Cairo drawing context.
 *
 *  A key has a label that is drawn as a marking inside the key. The labelling can consist of one, two or three characters.
 *  A key can be depressed or not. If it is depressed, it is drawn smaller to create the illusion that it has moved
 *  a bit. The size of a key is determined by it radius in pixels (default value is 23 pixels.). A key also has
 *  a keycode, that can be sent through the rotor machine to produce output.
 *
 *  Any key can be configured with three colours.
 *   
 *  1. The colour of the rectangular bounding box of the key (the cover colour, default BACKGROUND_GREY).
 *  2. The colour of the circular background of the key (default DARK_GREY). 
 *  3. The colour of the border of the key (border colour, default WHITE). The border colour is also used to draw the label.   
 */
class key : public element {
public:

    /*! \brief Constructor. The parameters pos_x and pos_y have to specify the position where the key is to be drawn. The
     *         parameter l has to specify the label used for the key and finally key_code has to contain the keycode that
     *         is generated when the key is pressed.
     *         
     *  The position given by pos_x and pos_y determines the center of the key.
     */
    key(int pos_x, int pos_y, unsigned int key_code, gunichar l);
    
    /*! \brief Default constructor. Values are determined by the set_defaults() method.
     */      
    key();

    /*! \brief Draws the key into the drawing context specified by parameter cr.
     */    
    virtual void draw(Cairo::RefPtr<Cairo::Context> cr);
    
    /*! \brief Change the key's radius (specified in pixels) to the new value given in parameter r. When changing
     *         the radius of the key, the size of the labelling is adapted accordingly. 
     */      
    virtual void set_radius(double r) { radius = r; char_size = 2 * radius - 10; }
    
    /*! \brief Returns the radius of this key.
     */      
    virtual double get_radius() { return radius; }
    
    /*! \brief Change the cover colour (see 1.) to the new value specified by the parameters r, g and b.
     */     
    void set_cover_col(double r, double g, double b);

    /*! \brief Change the background colour (see 2.) to the new value specified by the parameters r, g and b.
     */ 
    void set_key_bkg_col(double r, double g, double b);
    
    /*! \brief Change the border colour (see 3.) to the new value specified by the parameters r, g and b.
     */      
    void set_border_col(double r, double g, double b);                      

    /*! \brief An active key wants to handle all clicks that appear within the radius around its position.
     */      
    virtual bool test(int pos_x, int pos_y);
    
    /*! \brief Returns the keyocde which is associated with this key.
     */     
    virtual unsigned int get_key_code() { return key_code; }
    
    /*! \brief Changes the label to the new value given in parameter label, where label is a gtkmm ustring.
     */     
    virtual void set_label(ustring& label) { labeling = label; }

    /*! \brief Changes the label to the new value given in parameter label, where label is a zero terminated C-string.
     */     
    virtual void set_label(const char *label) { labeling = ustring(label); }    
    
    /*! \brief Returns the current label. 
     */         
    virtual ustring get_label() {return labeling; }

    /*! \brief Sets the state (depressed/not depressed) of the key. If the parameter new_val is true, then key
     *         is in the "depressed" state.
     */     
    virtual void set_is_depressed(bool new_val) { is_depressed = new_val; }

    /*! \brief Returns the state (depressed/not depressed) of the key. If True is returned, the key is currently 
     *         "depressed".
     */     
    virtual bool get_is_depressed() { return is_depressed; }

    /*! \brief Destructor. 
     */     
    virtual ~key() { ; }

protected:

    /*! \brief Sets default values for the colours, the labelling, the state, the character size, the radius and
     *         the keycodes. 
     */     
    void set_defaults();
    
    /*! \brief Holds the label. */      
    ustring labeling;
    
    /*! \brief Holds the radius. */      
    double radius;
    
    /*! \brief Holds the the size (in pixels) in which the labelling is drawn. */  
    int char_size; 
    
    /*! \brief Holds the red component of the key's cover colour. */   
    double cover_r;
    
    /*! \brief Holds the green component of the key's cover colour. */       
    double cover_g;
    
    /*! \brief Holds the blue component of the key's cover colour. */       
    double cover_b;
    
    /*! \brief Holds the red component of the key's background colour. */           
    double bkg_r;
    
    /*! \brief Holds the green component of the key's background colour. */               
    double bkg_g;
    
    /*! \brief Holds the blue component of the key's background colour. */               
    double bkg_b;
    
    /*! \brief Holds the red component of the key's border colour. */               
    double border_r;
    
    /*! \brief Holds the red component of the key's border colour. */                   
    double border_g;
    
    /*! \brief Holds the red component of the key's border colour. */                   
    double border_b;
    
    /*! \brief Holds the keycode that is associated with this key. */                   
    unsigned int key_code;

    /*! \brief Holds the state of the key. */                   
    bool is_depressed;
};


/*! \brief A GUI class that knows how to draw a space bar from a rotor machine's keyboard into a Cairo drawing context.
 *
 *  Objects of this class are attached to a "normal" key on the keyboard and share the keycode of this original key. This
 *  models the fact that the rotors of machines who support a space or blank character still have only 26 input characters
 *  and therefore an existing key is rededicated to be interpreted as a blank or space character upon decryption. The Typex
 *  for instance uses the 'X' key for this purpose whereas the KL7 and SIGABA use the 'Z' key. 
 *
 *  The machines mentioned above allow blank or space characters during enryptions but still produce ciphertext that contains
 *  no blanks. That means that during decryptions only the original key ('X' or 'Z' or whatever) must be active. Therefore objects
 *  of this class are given a a callback object (space_bar::enc_state) that allows them to determine if the space bar should
 *  be active at the moment or not.
 */
class space_bar : public key {
public:

    /*! \brief Constructor. The parameters pos_x and pos_y have to specify the position where the space bar is to be drawn and 
     *         wdth and hght determine its width and height. The parameter enc_state_func has to contain a slot object that allows
     *         the space bar to query whether it should be active or not. If the callback returns true the space bar is active
     *         and can be clicked on. The parameter org_key specifies the original key which is rededicated as the space character.
     *         
     *  The position given by pos_x and pos_y determines the upper left corner of the space bar. A space_bar object takes ownership
     *  of the key specified in parameter org_key and therefore space_bar::original key is deleted when a space_bar object is
     *  deleted.
     */
    space_bar(int pos_x, int pos_y, int wdth, int hght, key *org_key, sigc::slot<bool> enc_state_func);    

    /*! \brief Draws the space bar into the drawing context specified by parameter cr.
     */        
    virtual void draw(Cairo::RefPtr<Cairo::Context> cr);

    /*! \brief Sets the radius of the original key.
     */        
    virtual void set_radius(double r);

    /*! \brief Returns the radius of the original key.
     */        
    virtual double get_radius();
    
    /*! \brief Returns true, if the space_bar is active and the position specified by pos_x and pos_y is either inside the original
     *         key or the space bar. If space_bar::enc_state returns false this method returns true only if (pos_x, pos_y) is within
     *         the original key.
     */            
    virtual bool test(int pos_x, int pos_y);

    /*! \brief Returns the keycode of the original key.
     */        
    virtual unsigned int get_key_code();

    /*! \brief Sets the label of the original key.
     */        
    virtual void set_label(ustring& label);

    /*! \brief Sets the label of the original key. The parameter has to be a C-style zero terminated string.
     */        
    virtual void set_label(const char *label); 

    /*! \brief Returns the label of the original key.
     */        
    virtual ustring get_label();

    /*! \brief Sets the position of the original key.
     */            
    virtual void set_elem_pos(int new_x, int new_y) { original_key->set_elem_pos(new_x, new_y); }

    /*! \brief Returns the position of the original key through the reference parameters pos_x and pos_y.
     */            
    virtual void get_elem_pos(int& pos_x, int& pos_y) { original_key->get_elem_pos(pos_x, pos_y); } 
    
    /*! \brief Sets the state (depressed/not depressed) of the original key.
     */                   
    virtual void set_is_depressed(bool new_val);

    /*! \brief Returns the state (depressed/not depressed) of the original key.
     */                   
    virtual bool get_is_depressed();

    /*! \brief Sets the position of the space bar.
     */            
    virtual void set_space_pos(int pos_x, int pos_y) { x = pos_x; y = pos_y; }

    /*! \brief Destructor. Deletes space_bar::original_key.
     */            
    virtual ~space_bar() { delete original_key; }

protected:
    /*! \brief Holds width of the space bar.
     */            
    int width;

    /*! \brief Holds height of the space bar.
     */            
    int height;
    
    /*! \brief Points to the original key.
     */                
    key *original_key;
    
    /*! \brief Callback that allows to test if the space bar should be clickable or not.
     */                    
    sigc::slot<bool> enc_state;
};

/*! \brief A GUI class that knows how to draw a four digit character counter into a Cairo drawing context.
 * 
 *  If the left mouse button is pressed when inside the counter, the counter is reset to 0. 
 */
class counter : public element {
public:

    /*! \brief Constructor. The parameters pos_x and pos_y have to specify the position where the counter is to be drawn and 
     *         wdth and hght determine its width and height.
     *
     *  (pos_x, pos_y) defines the upper left corner of the rectangular counter.
     */
    counter(int pos_x, int pos_y, int wdth, int hght);

    /*! \brief Draws the counter into the drawing context specified by parameter cr.
     */        
    virtual void draw(Cairo::RefPtr<Cairo::Context> cr);

    /*! \brief Checks whether the position specified in pos_x and pos_y is whithin the counter and if yes resets counter::char_counter
     *         to 0 and draws the counter into the Cairo context specified by the parameter cr.
     */        
    virtual void on_mouse_button_down(Cairo::RefPtr<Cairo::Context> cr, int pos_x, int pos_y);

    /*! \brief Returns true if the position given by the parameters pos_x and pos_y is within the box that makes up the counter.
     */        
    virtual bool test(int pos_x, int pos_y);

    /*! \brief Increments the counter. The counter is not automatically redrawn.
     */            
    virtual void increment() { char_counter++; char_counter = char_counter % 10000; }

    /*! \brief Sets the counter to 0. The counter is not automatically redrawn.
     */            
    virtual void reset() { char_counter = 0; }

    /*! \brief Destructor.
     */                
    virtual ~counter() { ; }

protected:
    /*! \brief Holds the counter value. */                
    unsigned int char_counter;
    
    /*! \brief Holds the counter's width. */                    
    int width;

    /*! \brief Holds the counter's height. */                    
    int height;
};

/*! \brief A GUI class that knows how to draw a button into a Cairo drawing context.
 *
 *  A button is a rectangular shape that contains a textual label. When a button is cklicked a signal is emitted.
 */
class button : public element {
public:
    /*! \brief Constructor. The parameters pos_x and pos_y have to specify the position where the button is to be drawn and 
     *         wdth and hght determine its width and height. The parameter text has to give the label that is shown inside
     *         the button.
     *
     *  (pos_x, pos_y) defines the upper left corner of the button.
     */
    button(int pos_x, int pos_y, string text, int wdth, int hght);

    /*! \brief Draws the button into the drawing context specified by parameter cr.
     */        
    virtual void draw(Cairo::RefPtr<Cairo::Context> cr);
    
    /*! \brief Checks whether the position specified in pos_x and pos_y is whithin the button and if yes emits the signal
     *         button::pressed. Additionally the button is drawn into the Cairo context specified by the parameter cr.
     */            
    virtual void on_mouse_button_down(Cairo::RefPtr<Cairo::Context> cr, int pos_x, int pos_y);

    /*! \brief Returns true if the position given by the parameters pos_x and pos_y is within the box that makes up the button.
     */        
    virtual bool test(int pos_x, int pos_y);

    /*! \brief Returns a reference to the signal that is emitted when the button is clicked. This can be used by clients of this
     *         class to register corresponding callbacks.
     */            
    virtual sigc::signal<void>& signal_pressed() { return pressed; }

    /*! \brief Destructor.
     */                        
    virtual ~button() { ; }
    
protected:
    /*! \brief Holds the button's width. */                    
    int width;
    
    /*! \brief Holds the button's height. */                        
    int height;        
    
    /*! \brief Holds the counter's label. */                            
    string label;
    
    /*! \brief Holds the signal to which clients can subscribe. */                                
    sigc::signal<void> pressed;   
};

/*! \brief A struct that captures several parameters that are needed to determine the positions of the keys on a rotor machine's
 *         keyboard or correspondingly of lamps on a lampboard.
 *
 *  A rotor machine keyboard (or a lampboard) has n rows of elements (keys/lamps). The rows are separated vertically by 
 *  row_distance_y pixels and the elements in each horizontal row are elem_distance_x pixels apart. All but the last row are
 *  drawn with an offset in x-direction with respect to the x-positon of the leftmost element in the last (the one with the
 *  biggest y-position) row. Finally width specifies the width of the whole keyboard/lampboard.
 */
struct draw_parameters {
    /*! \brief Constructor. Only the vertical distance between rows (row_dist) and the horizontal distance between keys in the
     *         same row (elem_dist) have to be specified. The remaining members are initalized with the default values ROW_TWO_OFFSET
     *         ROW_ONE_OFFSET and SIZE_X_DEFAULT.
     */
    draw_parameters(int row_dist, int elem_dist);

    /*! \brief Constructor. Values for all members have to be specified.
     */
    draw_parameters(int row_dist, int key_dist, int row1_offset, int row2_offset, int wdth);

    /*! \brief Holds vertical distance in pixels between any two rows. */
    int row_distance_y;
    
    /*! \brief Holds horizontal distance in pixels between any two keys in the dame row. */    
    int elem_distance_x;
    
    /*! \brief Holds the offsets in pixels in x-direction that determines how far further to the left (<0) or the
     *         right (>0) the rows at positions 1-n-1 are drawn with respect to row n.  
     */
    vector<int> offset_row;
    
    /*! \brief Holds the width of the keyboard. Normally this should be equal to the width of the whole Gtk::DrawingArea
     *         in which the keyboard is drawn.
     */    
    int width;
};

/*! \brief Helper function that draws a character at position (x, y) in font_size pixels. */                                
void print_char(Cairo::RefPtr<Cairo::Context> cr, int x, int y, gunichar character, int font_size);

/*! \brief Helper function that draws a string beginning at position (x, y) in font_size pixels. */                                
void print_chars(Cairo::RefPtr<Cairo::Context> cr, int x, int y, ustring& characters, int font_size);

/*! \brief Helper function that draws a an ellpise with center (x, y) and specified width and height. */                                
void draw_ellipse(Cairo::RefPtr<Cairo::Context> cr, int x, int y, int width, int height);

#endif /* __base_elements_h__ */

