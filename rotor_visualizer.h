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

#ifndef __rotor_visualizer_h__
#define __rotor_visualizer_h__

/*! \file rotor_visualizer.h
 *  \brief Header file for the GUI classes that implement the simulated rotor stack.
 */ 

#include<base_elements.h>
#include<rotor_machine.h>
#include<rotor_window.h>

/*! \brief Default value for the number of pixels between two rotor_window objects in a rotor_visalizer. */
const int ROTOR_STEP = 80;

/*! \brief Default value for the number of pixels between two thin_rotor objects. */
const int THIN_WHEEL_SPACE = 5;

/*! \brief Default value for the number of pixels between two rotor banks in a sigaba_rotor_visualizer. */
const int THIN_WHEEL_BANK_SPACE = 10;

/*! \brief Default value for the height in pixels of a simulated SIGABA index rotor. */
const int INDEX_ROTOR_HEIGHT = 100;

/*! \brief Default value for the x-position of the rightmost rotor_window in a rotor_visalizer. */
const int RIGHTMOST_ROTOR_POS = 340;

/*! \brief Default value for the y-position of all the rotor_window objects in a rotor_visalizer. */
const int DEFAULT_ROTOR_Y = 110;

/*! \brief A GUI class that serves as the base class for all classes that know how to draw a simulated
 *         rotor stack into a Cairo drawing context.
 *  
 *  Objects of this class and its children manage a set of ::rotor_window_base objects which in turn are responsible
 *  for visualizing a single rotor. The main purpose of this class is to hide the details of how to draw and manage
 *  a set of rotor windows from the ::rotor_draw class and deal with the simulated rotor stack as whole instead of
 *  single rotor windows.
 */
class rotor_visualizer : public element {
public:
    /*! \brief Constructor. The parameter pos_y specifies the y-position, where the rotor_windows are to be drawn.
     *         The parameter pos_x is needed by the parent class, but is not used by rotor_visualizer. Finally 
     *         r_most_pos determines the x-position where the rightmost rotor_window is to be drawn.
     *  
     *  The rotor windows are drawn from right to left with rotor_visualizer::rotor_step pixels between them.
     */
    rotor_visualizer(int pos_x, int pos_y, int r_most_pos = RIGHTMOST_ROTOR_POS);

    /*! \brief Draws the simulated rotor stack into the drawing context specified by cr.
     */
    virtual void draw(Cairo::RefPtr<Cairo::Context> cr);
    
    /*! \brief Causes the wheel position of each rotor_window, that is managed by this rotor_visualizer, to be synced
     *         with the underlying rotor machine and then redrawn utilizing the drawing context given in parameter cr.
     */    
    virtual void update_rotors(Cairo::RefPtr<Cairo::Context> cr);

    /*! \brief This method can be used to tell this rotor_visualizer instance which rotor_machine to use. The
     *         parameter machine has to point to the corresponding rotor_machine object.
     *         
     *  The rotor_visualizer does not take ownership of the object to which machine points. I.e. this object
     *  is not deleted when the destructor is called.
     */    
    virtual void set_machine(rotor_machine *machine);

    /*! \brief This method syncs the rotor positions displayed in the rotor windows to the rotor positions as they
     *         currently are in the underlying rotor_machine object.
     */    
    virtual void update_all_rotor_windows();

    /*! \brief This method asks each of the rotor_window objects that are managed by this instance of rotor_visualizer
     *         whether it wants to handle the click that appeared at (pos_x, pos_y). If it finds one it forwards the
     *         call to that rotor_window.
     */
    virtual void on_mouse_button_down(Cairo::RefPtr<Cairo::Context> cr, int pos_x, int pos_y); 

    /*! \brief This method asks each of the rotor_window objects that are managed by this instance of rotor_visualizer
     *         whether it wants to handle the click that appeared at (pos_x, pos_y). If it finds one True is returned.
     */
    virtual bool test(int pos_x, int pos_y);

    /*! \brief Returns the position at which the rightmost rotor_window is to be drawn.
     */    
    virtual unsigned int get_rightmost_rotor_pos() { return rightmost_rotor_pos; }

    /*! \brief Sets the position at which the rightmost rotor_window is to be drawn to the value of the parameter new_pos.
     */    
    virtual void set_rightmost_rotor_pos(unsigned int new_pos);

    /*! \brief Returns the width (in pixels) of the gap that is drawn between two rotor windows.
     */    
    virtual unsigned int get_rotor_step() { return rotor_step; }

    /*! \brief Sets the width (in pixels) of the gap that is drawn between two rotor windows to the value specified in
     *         parameter new_step.
     */    
    virtual void set_rotor_step(int new_step) { rotor_step = new_step; } 

    /*! \brief Destructor. Also deletes the rotor_window objects that are managed by this rotor_visualizer.
     */        
    virtual ~rotor_visualizer();
    
protected:
    /*! \brief Holds the x-position of the rightmost rotor window. */        
    unsigned int rightmost_rotor_pos;

    /*! \brief Holds the width (in pixels) of the gap that is drawn between two rotor windows. */        
    int rotor_step;

    /*! \brief Sets the positions of the rotor_window objects that are managed by this instance of rotor_visualizer.
     *
     *  In rotor_visualizer this method simply calls set_positions_base().
     */        
    virtual void set_positions();

    /*! \brief Sets the positions of the rotor_window objects that are managed by this instance of rotor_visualizer.
     * 
     *  This method actually does the work. It is non virtual so that it can be called safely in the constructor.
     */        
    void set_positions_base();

    /*! \brief Holds the mapping of the symbolic names of the rotor_window objects to the objects themselves. */        
    std::map<std::string, rotor_window_base*> rotors;

    /*! \brief Holds the symbolic names of the rotor_window objects that are managed by this rotor_visualizer. */        
    vector<string> rotor_names; 
};

/*! \brief A GUI class that knows how to draw a simulated Enigma rotor stack into a Cairo drawing context.
 */
class enigma_visualizer : public rotor_visualizer {
public:
    /*! \brief Constructor. 
     *
     *  \param r_names [in] Specifies the symbolic names of the rotor slots that are in use in the type of machine for which this rotor_draw object
     *                      is constructed. These names have to match the symbolic names that are in use in the underlying rotor_machine object.
     *  \param numeric_wheels [in] If True the rotor positions are labeled with numbers not characters. This flag is set to True when visualizing
     *                             the rotors of the Services Enigma.
     *  \param r_most_pos [in] Determines the x-position where the rightmost rotor_window is to be drawn. 
     *  \param y_pos [in] Specifies the y-position, where the rotor_windows are to be drawn
     */
    enigma_visualizer(vector<string>& r_names, bool numeric_wheels, int r_most_pos = RIGHTMOST_ROTOR_POS, int y_pos = DEFAULT_ROTOR_Y);

    /*! \brief Destructor.
     */                
    virtual ~enigma_visualizer() { ; }      
};

/*! \brief A GUI class that knows how to draw a simulated rotor stack, which manages only thin_rotor objects, into a Cairo drawing context.
 */
class thin_rotor_visualizer : public rotor_visualizer {
public:

    /*! \brief Constructor. 
     *
     *  \param r_names [in] Specifies the symbolic names of the rotor slots that are in use in the type of machine for which this rotor_draw object
     *                      is constructed. These names have to match the symbolic names that are in use in the underlying rotor_machine object.
     *  \param is_nema [in] Has to be true if the simulated rotor stack is part of the Nema machine. In that case the background of the rightmost
     *                      thin_rotor is drawn in red.
     *  \param r_most_pos [in] Determines the x-position where the rightmost rotor_window is to be drawn. 
     *  \param y_pos [in] Specifies the y-position, where the rotor_windows are to be drawn
     */
    thin_rotor_visualizer(vector<string>& r_names, bool is_nema, int r_most_pos = RIGHTMOST_ROTOR_POS, int y_pos = DEFAULT_ROTOR_Y);

    /*! \brief Sets the width of each of the managed thin_rotor objects to the value specified in parameter new_width.
     */            
    virtual void set_width(int new_width);

    /*! \brief Sets the height of each of the managed thin_rotor objects to the value specified in parameter new_height.
     */            
    virtual void set_height(int new_height);    

    /*! \brief Returns the width of the managed thin_rotor objects.
     */            
    virtual int get_width();

    /*! \brief Returns the height of the managed thin_rotor objects.
     */            
    virtual int get_height();

    /*! \brief Destructor.
     */        
    virtual ~thin_rotor_visualizer() { ; }      

protected:
    /*! \brief True if the simulated machine is a Nema. */                
    bool is_nema_machine;
};

/*! \brief A GUI class that knows how to draw a simulated KL7 rotor stack into a Cairo drawing context.
 */
class thin_kl7_rotor_visualizer : public thin_rotor_visualizer {
public:
    /*! \brief Constructor. 
     *
     *  \param r_names [in] Specifies the symbolic names of the rotor slots that are in use in the type of machine for which this rotor_draw object
     *                      is constructed. These names have to match the symbolic names that are in use in the underlying rotor_machine object.
     *  \param r_most_pos [in] Determines the x-position where the rightmost rotor_window is to be drawn. 
     *  \param y_pos [in] Specifies the y-position, where the rotor_windows are to be drawn
     */
    thin_kl7_rotor_visualizer(vector<string>& r_names, int r_most_pos = RIGHTMOST_ROTOR_POS, int y_pos = DEFAULT_ROTOR_Y);

    /*! \brief Destructor.
     */    
    virtual ~thin_kl7_rotor_visualizer() { ; }

};

/*! \brief A GUI class that knows how to draw a simulated rotor stack, which manages only thin_action_rotor objects, into a Cairo drawing context.
 *
 *  This is used when implmenting the setup feature of the SIGABA driver machine.
 */
class thin_action_rotor_visualizer : public thin_rotor_visualizer {
public:
    /*! \brief Constructor. 
     *
     *  \param r_names [in] Specifies the symbolic names of the rotor slots that are in use in the type of machine for which this rotor_draw object
     *                      is constructed. These names have to match the symbolic names that are in use in the underlying rotor_machine object.
     *  \param action_func [in] Has to specifiy the callback that is executed when a thin_action_rotor is clicked in the right spot.
     *  \param r_most_pos [in] Determines the x-position where the rightmost rotor_window is to be drawn. 
     *  \param y_pos [in] Specifies the y-position, where the rotor_windows are to be drawn
     */
    thin_action_rotor_visualizer(vector<string>& r_names, sigc::slot<void, string&, Cairo::RefPtr<Cairo::Context> > action_func, int r_most_pos = RIGHTMOST_ROTOR_POS, int y_pos = DEFAULT_ROTOR_Y);

    /*! \brief Destructor.
     */        
    virtual ~thin_action_rotor_visualizer() { ; }
};

#endif /* __rotor_visualizer_h__ */

