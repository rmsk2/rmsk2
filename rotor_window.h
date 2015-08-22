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

#ifndef __rotor_window_h__
#define __rotor_window_h__

/*! \file rotor_window.h
 *  \brief Header file for the GUI classes that know how to draw rotor windows for the Enigma, KL7 and several other machines.
 */

#include<base_elements.h>
#include<rotor_machine.h>

/*! \brief Default width of a thin_rotor. */
const int THIN_WIDTH_DEFAULT = 30;
/*! \brief Default height of a thin_rotor. */
const int THIN_HEIGHT_DEFAULT = 150;

/*! \brief A GUI class that serves as the base class for all classes that know how to draw a graphical representation of a
 *         rotor's position into a Cairo drawing context.
 *        
 *  This graphical representation is also called a rotor window. A rotor_window_base object knows a rotor_machine 
 *  (member rotor_window_base::the_machine) and the name of a rotor slot in that machine (member rotor_window_base::rotor_identifier).
 *  This data is used to query the current position of the named rotor within the said machine.
 *
 *  Most typically objects of this type draw the character that can be seen on the rotor's circumfence on its current position. In machines
 *  that have movable rings this means that the ring position has to be taken into account when determining this character.
 */
class rotor_window_base : public element {
public:
    /*! \brief Constructor. The parameters pos_x and pos_y have to specifiy the coordinates of the point where the rotor window is to 
     *         be drawn.
     *
     *  To be more precise (pos_y, pos_y) has to specify the middle of the graphical representation of the rotor window.
     */
    rotor_window_base(int pos_x, int pos_y) : element(pos_x, pos_y) { ; }

    /*! \brief This method is used to query the rotor position of the rotor which is visualized by this rotor_window_base object. After
     *         calling this method the member rotor_window_base::wheel_pos has been updated to reflect the current rotor position but
     *         the rotor window is not redrawn to reflect a possible change.
     */    
    virtual void update();

    /*! \brief Does the same as rotor_window_base::update but the rotor window is redrawn in the context specified by the parameter cr
     *         in order to reflect a possible change.
     */    
    virtual void update(Cairo::RefPtr<Cairo::Context> cr);    

    /*! \brief Allows to step the rotor represented by this rotor_window_base object one step ahead or back by clicking the rotor window.
     *         The parameters pos_x and pos_y have to specify where the click occurred and the parameter cr has to conatain a drawing
     *         context into which the rotor window can draw itself in order to reflect the updated rotor position.
     *
     *  A rotor in a rotor machine can be moved to a predetermined postion by hand. Moving a rotor is simulated by clicking on
     *  the rotor window in the simulator's GUI.
     *  
     *  Normally clicks that occur below the y-position specified in the constructor are interpreted as a command to advance the rotor
     *  one position and clicks that occur above the y-position cause the rotor to step back one position.
     */    
    virtual void on_mouse_button_down(Cairo::RefPtr<Cairo::Context> cr, int x_pos, int y_pos);

    /*! \brief This method allows to change the rotor which is visualized by this rotor window. This is done by specifying 
     *         the rotor machine and the symbolic name of a rotor slot in that machine through the paramters m and identifier.
     */    
    virtual void set_machine(rotor_machine *m, string &identifier);

    /*! \brief This method returns the character currently shown in the rotor window.
     */        
    virtual gunichar get_wheel_pos() { return wheel_pos; }

    /*! \brief This method sets the character that is currently shown in the rotor window. This method performs no redraw!
     */        
    virtual void set_wheel_pos(gunichar new_wheel_pos) { wheel_pos = new_wheel_pos; }
    
    /*! \brief Destructor.
     */        
    virtual ~rotor_window_base() { ; }

    
protected:
    /*! \brief Holds the character currently displayed in the rotor window.
     */            
    gunichar wheel_pos;

    /*! \brief This method queries the actual rotor in the slot named by rotor_window_base::rotor_identifier in the machine to which
     *         rotor_window_base::the_machine points for the character that can be seen on its circumfence at its current position.
     */        
    virtual gunichar get_ring_pos();

    /*! \brief Holds the symbolic name of the rotor which is visualized by this rotor window. */        
    string rotor_identifier;
    
    /*! \brief Holds a pointer to the machine in which the rotor, that is visualized by this rotor window, resides.  */            
    rotor_machine *the_machine; 
};

/*! \brief A GUI class that knows how to draw a graphical representation of an Enigma rotor window into a Cairo drawing context.
 *         
 *  An Enigma rotor window consists of an elliptical background (filled with the background colour) that has a black border. Into this
 *  background two screw tops are drawn. In the center of the ellipse is a square filled with the rotor background colour and a black
 *  border. In the center of that square the rotor position is drawn in black (or red). The rotor position can either be a single character
 *  or a number betwen 01 and 26. To the right a narrow rectangle is drawn that is intended to symbolize the handle or rotor rim by which
 *  the rotor can be moved manually. The rim has a background colour (the dash background colour). The "notches" on the rim are drawn in
 *  the dash colour.
 *
 *  If enigma_rotor_window::is_greek is True the rotor position is drawn in red.
 *  If enigma_rotor_window::is_numeric is True the rotor position is drawn as a number.
 *  If enigma_rotor_window::has_ellipse is False, the background ellipse and its border are not drawn.
 */
class enigma_rotor_window : public rotor_window_base {
public:
    /*! \brief Constructor. The parameters pos_x and pos_y have to specifiy the coordinates of the point where the rotor window is to 
     *         be drawn. The parameter orient flags whether the slot of the upper ::screw should be drawn horizontally (True) or in 
     *         a 45 degree angle (False). The slot of the lower screw is then drawn with the opposite orientation.
     *
     *  To be more precise (pos_y, pos_y) has to specify the middle of the graphical representation of the rotor window.
     */
    enigma_rotor_window(int pos_x, int pos_y, bool orient);

    /*! \brief Returns True, if (pos_x, pos_y) is within the rectangle that bounds the simulated rotor rim/handle.
     */                
    virtual bool test(int pos_x, int pos_y);

    /*! \brief Draws the rotor window into the drawing context given in parameter cr.
     */                
    virtual void draw(Cairo::RefPtr<Cairo::Context> cr);

    /*! \brief The parameter new_size has to specify the new height and width of the central square into which the rotor
     *         position is drawn.
     *
     *  From this value several other drawing parameters are derived.
     */                
    virtual void set_window_size(int new_size) { window_size = new_size; calc_limits(); }

    /*! \brief This method can be used to set the position of this enigma_rotor_window. The parameters new_x and new_y
     *         specify the coordinates of the new position.
     */                
    virtual void set_elem_pos(int new_x, int new_y);
    
    /*! \brief Can be used to set the RGB components of the background colour.
     */
    void set_bkg_col(double r, double g, double b);

    /*! \brief Can be used to set the RGB components of the dash colour.
     */
    void set_dash_col(double r, double g, double b);

    /*! \brief Can be used to set the RGB components of the dash background colour.
     */
    void set_dash_bkg_col(double r, double g, double b);    

    /*! \brief Can be used to set the RGB components of the rotor background colour.
     */
    void set_rotor_bkg_col(double r, double g, double b); 
    
    /*! \brief Destructor.
     */                
    virtual ~enigma_rotor_window() { delete upper; delete lower; }

    /*! \brief True if the rotor position is to be drawn in red. */     
    bool is_greek;
    
    /*! \brief True if the rotor position is to be drawn in numeric form. */         
    bool is_numeric;

    /*! \brief True if the background ellipse should be drawn. */         
    bool has_ellipse;


protected:
    /*! \brief Draws the rotor position (the central square and the label/marking) into the drawing 
     *         context given in parameter cr.
     */                
    virtual void draw_wheel_pos(Cairo::RefPtr<Cairo::Context> cr, gunichar new_pos);

    /*! \brief Sets defaults for the colours and drawing parameters.
     */                
    void set_defaults();

    /*! \brief Derives drawing parameters from the value of enigma_rotor_window::window_size.
     */                
    void calc_limits();

    /*! \brief Points to the ::screw object that is used to draw the upper screw. */    
    screw *upper;
    
    /*! \brief Points to the ::screw object that is used to draw the lower screw. */        
    screw *lower;
    
    /*! \brief Holds size (width and height) of the central square of the rotor window. */            
    int window_size;
    
    /*! \brief True if the slot of enigma_rotor_window::upper is to be drawn horizontally. If False slot is drawn in 45 degree angle. */
    bool screw_orientation;
    
    /*! \brief Holds width in pixels of the background ellipse (derived from window_size). */    
    double ellipse_width;
    
    /*! \brief Holds height in pixels of the background ellipse (derived from window_size). */    
    double ellipse_height;
    
    /*! \brief Holds radius in pixels of the two screws (derived from window_size). */    
    double screw_size; 
    
    /*! \brief Holds the width in pixels of the rotor rim/handle (derived from window_size). */        
    double rotor_rim_width;

    /*! \brief Holds the distance in pixels in x direction between the center of the background ellipse and the left border of
     *         the rotor rim/handle (derived from window_size). 
     */            
    double padded_size;
    
    /*! \brief Holds the size in pixels of the font which is used when the rotor position is drawn as a character. 
     *         (derived from window_size).
     */    
    int font_size_char;
    
    /*! \brief Holds the size in pixels of the font which is used when the rotor position is drawn in numeric form. 
     *         (derived from window_size).
     */ 
    int font_size_numeric; 
    
    /*! \brief Holds the width in pixels of each character when the rotor position is drawn in numeric form. */     
    int char_width_numeric; 
    
    /*! \brief Holds red component of the background colour. */
    double bkg_r;
    
    /*! \brief Holds green component of the background colour. */    
    double bkg_g;
    
    /*! \brief Holds blue component of the background colour. */    
    double bkg_b;
    
    /*! \brief Holds red component of the dash colour. */    
    double dash_r;
    
    /*! \brief Holds green component of the dash colour. */        
    double dash_g;
    
    /*! \brief Holds blue component of the dash colour. */        
    double dash_b;
    
    /*! \brief Holds red component of the dash background colour. */        
    double dash_bkg_r;
    
    /*! \brief Holds green component of the dash background colour. */            
    double dash_bkg_g;
    
    /*! \brief Holds blue component of the dash background colour. */            
    double dash_bkg_b;    
    
    /*! \brief Holds red component of the rotor background colour. */            
    double rotor_r;
    
    /*! \brief Holds green component of the rotor background colour. */    
    double rotor_g;
    
    /*! \brief Holds blue component of the rotor background colour. */    
    double rotor_b;
};

/*! \brief A GUI class that knows how to draw an abstract graphical representation of a rotor position into a Cairo drawing context.
 *         Objects of this type are used to visualize the rotor positions for all machines apart from Enigma variants (including Typex).
 *
 *  A thin_rotor draws a filled rectangle into a drawing context using the so called background colour. In the center of this rectangle
 *  the rotor position is printed.
 */
class thin_rotor : public rotor_window_base {
public:
    /*! \brief Constructor. 
     *
     *  \param pos_x [in] Specifies the x-coordinate of the center point of the rectangle.
     *  \param pos_y [in] Specifies the y-coordinate of the center point of the rectangle.
     *  \param tr_width [in] Specifies the width in pixels of the retangle.
     *  \param tr_height [in] Specifies the height in pixels of the retangle.     
     */
    thin_rotor(int pos_x, int pos_y, int tr_width = THIN_WIDTH_DEFAULT, int tr_height = THIN_HEIGHT_DEFAULT);

    /*! \brief Draws the rotor window into the drawing context given in parameter cr.
     */
    virtual void draw(Cairo::RefPtr<Cairo::Context> cr);
    
    /*! \brief Returns True, if (pos_x, pos_y) is within the rectangle that is drawn to represent the rotor position.
     */                    
    virtual bool test(int pos_x, int pos_y);

    /*! \brief Returns the current width of the rectangle.
     */                        
    virtual int get_width() { return width; }

    /*! \brief Returns the current height of the rectangle.
     */                        
    virtual int get_height() { return height; }    

    /*! \brief Sets the current height of the rectangle to the new value specified in parameter new_height.
     */                            
    virtual void set_height(int new_height) { height = new_height; }

    /*! \brief Sets the current width of the rectangle to the new value specified in parameter new_width.
     */                            
    virtual void set_width(int new_width) { width = new_width; } 

    /*! \brief Sets the background colour to the new value that is specified by the RGB components given in the parameters
     *         r, g and b.
     */                                
    virtual void set_bkg_col(double r, double g, double b) { set_col_base(r, g, b); }

    /*! \brief Destructor.
     */    
    virtual ~thin_rotor() { ; }
    
protected:

    /*! \brief Sets the background colour to the new value that is specified by the RGB components given in the parameters
     *         r, g and b. Non virtual so it can be called sfaely from constructor.
     */    
    void set_col_base(double r, double g, double b);

    /*! \brief Holds the width of the rectangle. */
    int width;
    
    /*! \brief Holds the height of the rectangle. */    
    int height;
    
    /*! \brief Holds red component of the background colour. */                
    double red;
    
    /*! \brief Holds green component of the background colour. */    
    double green;

    /*! \brief Holds blue component of the background colour. */    
    double blue;
};

/*! \brief A GUI class that knows how to draw an abstract graphical representation of a KL7 rotor position into a Cairo drawing context.
 *
 *  A thin_kl7_rotor is nearly identical to a normal thin_rotor. The only difference is in the thin_kl7_rotor::get_ring_pos() method because the
 *  ring position that is retrieved by calling kl7::visualize_rotor_pos() has to be post processed in order to be able to correctly visualize
 *  the empty positions on the circumfence of a KL7 rotor.
 */
class thin_kl7_rotor : public thin_rotor {
public:
    /*! \brief Constructor. 
     *
     *  \param pos_x [in] Specifies the x-coordinate of the center point of the rectangle.
     *  \param pos_y [in] Specifies the y-coordinate of the center point of the rectangle.
     *  \param tr_width [in] Specifies the width in pixels of the retangle.
     *  \param tr_height [in] Specifies the height in pixels of the retangle.     
     */
    thin_kl7_rotor(int pos_x, int pos_y, int tr_width = THIN_WIDTH_DEFAULT, int tr_height = THIN_HEIGHT_DEFAULT) : thin_rotor (pos_x, pos_y, tr_width, tr_height) { ; }

    /*! \brief Destructor.
     */        
    virtual ~thin_kl7_rotor() { ; }

protected:

    /*! \brief Replaces numeric characters returned by kl7::visualize_rotor_pos() with space characters.
     */        
    virtual gunichar get_ring_pos();    
};

/*! \brief A GUI class that not only knows how to draw an abstract graphical representation of a rotor position into a Cairo drawing context but also
 *         allows to associate that representation with a callback that is executed when the representaion is clicked in the "right spot". 
 *
 *  The right spot is an additional rectangle (action rectangle) in DARK_GREY which is drawn below the representation of the rotor position. Rotor
 *  windows of this type are used to visualize SIGABA control/driver rotors in order to implement the setup stepping feature.
 */
class thin_action_rotor : public thin_rotor {
public:
    /*! \brief Constructor. 
     *
     *  \param pos_x [in] Specifies the x-coordinate of the center point of the rotor window rectangle.
     *  \param pos_y [in] Specifies the y-coordinate of the center point of the rotor window rectangle.
     *  \param action_func [in] Callback that is executed when the action rectangle is clicked.
     *  \param tr_width [in] Specifies the width in pixels of the retangle.
     *  \param tr_height [in] Specifies the height in pixels of the retangle.     
     */
    thin_action_rotor(int pos_x, int pos_y, sigc::slot<void, string&, Cairo::RefPtr<Cairo::Context> > action_func, int tr_width = THIN_WIDTH_DEFAULT, int tr_height = THIN_HEIGHT_DEFAULT);

    /*! \brief Draws the rotor window into the drawing context given in parameter cr as is done in thin_rotor but additionally
     *         draws a DARK_GREY rectangle below the rotor window rectangle.
     */
    virtual void draw(Cairo::RefPtr<Cairo::Context> cr);
    
    /*! \brief Returns True, if (pos_x, pos_y) is within the rectangle that is drawn to represent the rotor position or the additional
     *         action rectangle.
     */                        
    virtual bool test(int pos_x, int pos_y);

    /*! \brief When clicked at a position (pos_x, pos_y) that is within the rotor window rectangle then thin_rotor::on_mouse_button_down() is called.
     *         A click at a position (pos_x, pos_y) that is within the action rectangle causes the callback stored in thin_action_rotor::action to be
     *         executed. The drawing context given in parameter cr is used for drawing operations.
     */                        
    virtual void on_mouse_button_down(Cairo::RefPtr<Cairo::Context> cr, int x_pos, int y_pos);    

    /*! \brief Destructor.
     */
    virtual ~thin_action_rotor() { ; }
    
protected:
    /*! \brief Holds the callback that was specified when constructing this object. */
    sigc::slot<void, string&, Cairo::RefPtr<Cairo::Context> > action;
};

#endif /* __rotor_window_h__ */

