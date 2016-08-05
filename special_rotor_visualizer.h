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

#ifndef __special_rotor_visualizer_h__
#define __special_rotor_visualizer_h__

/*! \file special_rotor_visualizer.h
 *  \brief Header file for the GUI classes that know how to draw the simulated rotor stacks of the SIGABA and the SG39.
 */ 

#include<rotor_visualizer.h>
#include<sigaba.h>
#include<sg39.h>

/*! \brief A GUI class that knows how to combine several thin_rotor_visualizer objects into a single rotor_visualizer.
 *         
 *  The rotor_visualizer objects that are managed by instances of this class are called sub visualizers. The sub
 *  visualizers are stored in the member composite_rotor_visualizer::sub_visualizers This class is needed to implement
 *  graphical simulators for the SIGABA and the SG39.
 */
class composite_rotor_visualizer : public rotor_visualizer {
public:
    /*! \brief Constructor. The parameter y_pos specifies the y-position, where the sub visualizers are to be drawn.
     *         The parameter r_most_pos determines the x-position where the rightmost sub visualizer is to be drawn.
     */
    composite_rotor_visualizer(int r_most_pos = RIGHTMOST_ROTOR_POS, int y_pos = DEFAULT_ROTOR_Y);

    /*! \brief Draws the sub visualizers into the drawing context specified by parameter cr.
     */    
    virtual void draw(Cairo::RefPtr<Cairo::Context> cr);
    
    /*! \brief This method syncs the rotor positions displayed in the rotor windows to the rotor positions as they
     *         currently are in the underlying rotor_machine object.
     */    
    virtual void update_all_rotor_windows();    

    /*! \brief Syncs the rotor positons of all sub visualizers and subsequently redraws them into the drawing
     *         context speified by the parameter cr.
     */    
    virtual void update_rotors(Cairo::RefPtr<Cairo::Context> cr);

    /*! \brief Asks all sub visualizers if they want to handle the click that happened at position (pos_x, pos_y). If one
     *         is found that is willing to handle the click the on_mouse_button method of that sub visualizer is called.
     */    
    virtual void on_mouse_button_down(Cairo::RefPtr<Cairo::Context> cr, int pos_x, int pos_y); 

    /*! \brief Asks all sub visualizers if they want to handle the click that happened at position (pos_x, pos_y). Returns
     *         True if one the sub visualizers declared interest to handle the click.
     */    
    virtual bool test(int pos_x, int pos_y);

    /*! \brief Destructor. Deletes the sub visualizers.
     */    
    virtual ~composite_rotor_visualizer();

protected:

    /*! \brief Holds the sub visualizers.
     */    
    vector<thin_rotor_visualizer *> sub_visualizers;
};

/* ------------------------------------------------------------------ */

/*! \brief A GUI class that knows how to draw a simulated SIGABA rotor stack into a Cairo drawing context.
 *
 *  As the SIGABA is in essence a nested rotor machine, this class combines the rotor_visualizer objects for the index,
 *  driver and ciphering rotor stacks.
 */
class sigaba_rotor_visualizer : public composite_rotor_visualizer {
public:
    /*! \brief Constructor. 
     *
     *  \param r_names [in] Specifies the symbolic names of the 15 rotor slots that are in use in a SIGABA. The first five elements have to 
     *                      specify the slot names for the cipher rotors, the next five the slot names of the driver rotors and the last five
     *                      the slot names of the index rotors.
     *  \param r_most_pos [in] Determines the x-position where the cipher rotor sub visualizer is to be drawn. 
     *  \param y_pos [in] Specifies the y-position, where the sub visualizers are to be drawn
     */
    sigaba_rotor_visualizer(vector<string>& r_names, int r_most_pos = RIGHTMOST_ROTOR_POS, int y_pos = DEFAULT_ROTOR_Y);

    /*! \brief This method allows to link the underlying ::sigaba object (given in parameter machine) with each of the sub visualizers.
     */    
    virtual void set_machine(rotor_machine *machine);    

    /*! \brief This method allows to set the x-position, specified by parameter new_pos, where the rightmost sub visualizer is to be drawn. 
     *
     *  This method subsequently recalculates the x-positions of the rightmost rotor_window_base objects in each of the sub visualizers.
     */    
    virtual void set_rightmost_rotor_pos(unsigned int new_pos);

    /*! \brief Destructor. Deletes the sub visualizers.
     */    
    virtual ~sigaba_rotor_visualizer() { ; }
    
protected:    
    /*! \brief This method recalculates the x-positions of the rightmost rotor_window_base objects in each of the sub visualizers based
     *         on the x-position contained in rightmost_rotor_pos.
     */    
    void set_sub_vis_positions();

    /*! \brief This method is used as the callback for the thin_action_rotor objects that make up the sub visualizer for the driver machine.
     *
     *  It performs a setup stepping (see sigaba_stepper::setup_step) of the rotor named by parameter arg and then uses the drawing context cr
     *  to sync and subsequently redraw the rotor windows of all sub visualizers. 
     */    
    void action_container(string& arg, Cairo::RefPtr<Cairo::Context> cr);
    
    /*! \brief Holds the underlying ::sigaba machine. */
    sigaba *the_sigaba_machine;            
};

/* ------------------------------------------------------------------ */

/*! \brief A GUI class that knows how to draw a simulated rotor stack of a ::schluesselgeraet39 into a Cairo drawing context.
 *
 *  A Schlüsselgerät 39 technically only has one stack of wired rotors but this class models the three drive wheels as a separate
 *  rotor_visualizer (see ::sg39_drive_wheel_visualizer). This class combines the drive wheel visualizer with the thin_rotor_visualizer
 *  that simulates the stack of wired rotors.
 */
class sg39_rotor_visualizer : public composite_rotor_visualizer {
public:
    /*! \brief Constructor. 
     *
     *  \param r_names [in] Specifies the symbolic names of the four rotor slots that are in use in an SG39. 
     *  \param r_most_pos [in] Determines the x-position where the drive wheel sub visualizer is to be drawn. 
     *  \param y_pos [in] Specifies the y-position, where the sub visualizers are to be drawn
     */
    sg39_rotor_visualizer(vector<string>& r_names, int r_most_pos = RIGHTMOST_ROTOR_POS, int y_pos = DEFAULT_ROTOR_Y);

    /*! \brief This method allows to link the underlying ::schluesselgeraet39 object (given in parameter machine) with each of the
     *         sub visualizers.
     */    
    virtual void set_machine(rotor_machine *machine);    
    
    /*! \brief This method allows to set the x-position, specified by parameter new_pos, where the rightmost sub visualizer is to be drawn. 
     *
     *  This method subsequently recalculates the x-positions of the rightmost rotor_window_base objects in each of the sub visualizers.
     */    
    virtual void set_rightmost_rotor_pos(unsigned int new_pos);

    /*! \brief Destructor. Deletes the sub visualizers.
     */        
    virtual ~sg39_rotor_visualizer() { ; }  

protected:
    /*! \brief This method recalculates the x-positions of the rightmost rotor_window_base objects in each of the sub visualizers based
     *         on the x-position contained in rightmost_rotor_pos.
     */    
    void set_sub_vis_positions();  
};

/*! \brief A GUI class that knows how to draw a simulated rotor window of a ::schluesselgeraet39 drive wheel into a Cairo drawing context.
 *
 *  A drive wheel rotor window is drawn in a pinion like shape, whereas a basic thin_rotor is drawn as a simple rectangle. The fact that
 *  the SG39 drive wheels are implemented as attributes of the ::rotor_descriptor objects of the wired rotors made it necessary to reimplement
 *  on_mouse_button_down from the thin_rotor base class.
 */
class sg39_drive_wheel : public thin_rotor {
public:

    /*! \brief Constructor. The parameters pos_x and pos_y have to specifiy the coordinates of the point in the middle of the rectangle
     *         that is drawn as the drive wheel's graphical representation. The parameters tr_width and tr_height then specify the width
     *         and height of that rectangle.
     */
    sg39_drive_wheel(int pos_x, int pos_y, int tr_width = THIN_WIDTH_DEFAULT, int tr_height = THIN_HEIGHT_DEFAULT);

    /*! \brief Draws a graphical representation of the drive wheel into the drawing context specified by parameter cr.
     */    
    virtual void draw(Cairo::RefPtr<Cairo::Context> cr);
    virtual void on_mouse_button_down(Cairo::RefPtr<Cairo::Context> cr, int x_pos, int y_pos);

    /*! \brief Allows to link this sg39_drive_wheel with the underlying schluesselgeraet39 object, that has to be given in the parameter
     *         m. Additionally the parameter identifier has to specify the symbolic name to refer to that drive wheel in the machine m for
     *         which this sg39_drive_wheel is a graphical representation.
     */    
    virtual void set_machine(rotor_machine *m, string &identifier);

    /*! \brief Destructor.
     */    
    virtual ~sg39_drive_wheel() { ; }
        
protected:
    /*! \brief Returns a character that represents the current position of the drive wheel.
     */    
    virtual gunichar get_ring_pos();    
};

/*! \brief A GUI class that knows how to draw a simulated stack of a ::schluesselgeraet39 drive wheels into a Cairo drawing context.
 */
class sg39_drive_wheel_visualizer : public thin_rotor_visualizer {
public:
    /*! \brief Constructor. 
     *
     *  \param r_names [in] Specifies the symbolic names of the three rotor slots that have a drive wheel. 
     *  \param r_most_pos [in] Determines the x-position where the rightmost drive wheel is to be drawn. 
     *  \param y_pos [in] Specifies the y-position, where the drive wheels are to be drawn
     */
    sg39_drive_wheel_visualizer(vector<string>& r_names, int r_most_pos = RIGHTMOST_ROTOR_POS, int y_pos = DEFAULT_ROTOR_Y);

    /*! \brief Destructor.
     */    
    virtual ~sg39_drive_wheel_visualizer() { ; }
};


#endif /* __special_rotor_visualizer_h__ */

