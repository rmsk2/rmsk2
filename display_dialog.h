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

#ifndef __display_dialog_h__
#define __display_dialog_h__

/*! \file display_dialog.h
 *  \brief Header file for the application classes that are used to implement the log dialog.
 */

#include<set>
#include<vector>
#include<utility>
#include<iostream>

#include<gtkmm.h>
#include<glibmm.h>

using namespace std;

/*! \brief Signifies that no grouping is to be used. */
const unsigned int FORMAT_NONE = 0;

/*! \brief Signifies that a group has to consist of four characters. */
const unsigned int FORMAT_GROUP4 = 4;

/*! \brief Signifies that a group has to consist of five characters. */
const unsigned int FORMAT_GROUP5 = 5;

/*! \brief An abstract application class that provides an interface for a thing that knows how to record single
 *         characters in a log.
 *
 *  Apart from the method that can be used to report a character to be logged there are two additional methods that
 *  allow to query a logger if it is in an error state or to clear that error state.
 */
class logger {
public:
    /*! \brief Constructor.
     */
    logger() { error_flag = false; }

    /*! \brief Method to tell a logger to log the character given in parameter c. Pure virtual method has to be
     *         imlemented by children of this class.
     */
    virtual void report_char(gunichar c) = 0;

    /*! \brief Returns true if the logger is in an error state. False means the logger is not in error and usable.
     */
    virtual bool has_error() { return error_flag; }

    /*! \brief This method can be used to clear the error state. After calling this method the logger should be
     *         fully usable again.
     */
    virtual void reset_error_flag() { error_flag = false; }

    /*! \brief This method can be used to clear the log of reported characters. Has to be implemented by children
     *         of this class.
     */
    virtual void clear() = 0;

    /*! \brief Destructor.
     */
    virtual ~logger() { ; }
    
protected:
    /*! \brief Holds the flag that indicates if this logger is in an error state or not. */
    bool error_flag;
};

/*! \brief An application class that implements the logger interface by storing the reported characters through a referenced
 *         ustring object.
 */
class storage_logger : public logger {
public:
    /*! \brief Constructor. The parameter data has to contain a reference to the string object that is used to record the
     *         the reported characters.
     */
    storage_logger(Glib::ustring& data) : logger(), log_data(data) { ; }

    /*! \brief Simply appends the character specified in parameter c to the string referenced through the member log_data.
     */
    virtual void report_char(gunichar c) { log_data += c; }

    /*! \brief Empties the string referenced through the member log_data.
     */
    virtual void clear() { log_data.clear(); }

    /*! \brief Destructor.
     */
    virtual ~storage_logger() { ;}
    
protected:
    /*! \brief Holds a reference to the string that is used to store the logged characters. */
    Glib::ustring& log_data;
};

/*! \brief An application class that implements the logger interface by displaying the reported characters in a Gtk::Textview object.
 *         
 *  On top of that a textview_logger knows a button (the clear button) that causes the TextView to be cleared when clicked. 
 *  Additionally part of the label of the clear button is the the number of characters that are currently stored in the TextView.
 *  The text in the TextView can be grouped. The group size can be set through the textview_logger::set_format_type() method.
 */
class textview_logger : public logger {
public:

    /*! \brief Constructor. The parameter m has to point to a C-style zero terminated string. This string is used as a prefix when
     *         updating the label on the clear button after a character has been reported.
     */
    textview_logger(const char *m) : logger() { message = Glib::ustring(m); view = NULL; scroll = NULL; button = NULL; format_type = FORMAT_NONE; }
    
    /*! \brief Appends the character specified in parameter c to the member textview_logger::log_contents. textview_logger::log_contents
     *         is then used to fill the TextView. The text in the TextView is grouped according to the value of the member variable 
     *         textview_logger::format_type and the label of the clear button is updated to correctly show the character count.
     */
    virtual void report_char(gunichar c);

    /*! \brief Makes the Gtk::TextView to which the parameter v points the TextView that is used by this textview_logger to display
     *         logged characters.
     */
    virtual void set_view(Gtk::TextView *v, Gtk::ScrolledWindow *s) { view = v; scroll = s; }

    /*! \brief Makes the Gtk::Button to which the parameter b points the clear button that is used by this textview_logger.
     */
    virtual void set_button(Gtk::Button *b) { button = b; }

    /*! \brief Sets the grouping policy to the value that is specified in parameter type and reformat dsiplayed text accordingly.
     *         Possible values are FORMAT_NONE, FORMAT_GROUP4 and FORMAT_GROUP5.
     */
    virtual void set_format_type(unsigned int type) { format_type = type; format_text(); }

    /*! \brief Clears textview_logger::log_contents and the TextView in which this textview_logger displays the
     *         reported characters.
     */
    virtual void clear();

    /*! \brief Destructor.
     */    
    virtual ~textview_logger() { ; }
    
protected:

    /*! \brief Creates the contents of the TextView using the value of textview_logger::log_contents. The text in the TextView is grouped
     *         according to the policy determined by textview_logger::format_type.
     */
    void format_text();

    /*! \brief Holds the grouping policy, i.e. one of the values FORMAT_NONE, FORMAT_GROUP4, FORMAT_GROUP5. */
    unsigned int format_type;
    
    /*! \brief Points to the TextView which forms the basis of this textview_logger instance. */    
    Gtk::TextView *view;
    
    /*! \brief Points to the ScrolledWindow in which textview_logger::view is contained. */        
    Gtk::ScrolledWindow *scroll;
    
    /*! \brief Points to the clear button. */            
    Gtk::Button *button;

    /*! \brief Holds the sequence of reported characters which are used to create the contents of textview_logger::view. */                
    Glib::ustring log_contents;
    
    /*! \brief Holds the prefix which is used in the construction of the label of the clear button. */    
    Glib::ustring message;
};


/*! \brief An application class that implements the logic of a log dialog that can be used to record and display the 
 *         input and output characters while processing a message with a simulated rotor machine.
 *
 *  The dialog provides a GUI which displays an input and an output log. Each of the logs is managed by a textview_logger.
 *  Each log has a TextView to display logged text, a clear button and some RadioButtons which can be used to influence
 *  grouping of the text in the TextView.
 *
 *  Input and output characters which are to be logged are expected to be emitted by signals. When the method
 *  display_dialog::connect_input() and/or display_dialog::connect_input() is called this display_dialog connects to
 *  the provided signal to receive its input and/or output characters.
 *
 *  This very general mechanism makes sure that the display_dialog does not depend on any other classes or interfaces
 *  within this project. For the same reason the signal display_dialog::become_invisible was introduced.
 */
class display_dialog {
public:
    /*! \brief Constructor.
     *
     *  \param p [in] Has to point to the parent window, i.e. the main window of the application.
     *  \param w [in] has to point to the window in which the the log dialog and its controls reside. Has to be
     *                created by the same Gtk::Builder to which parameter r points.
     *  \param r [in] Has to point to a Gtk::Builder object which can be used to get access to the controls
     *                inside the window from parameter w.
     */
    display_dialog(Gtk::Window *p, Gtk::Window *w, Glib::RefPtr<Gtk::Builder> r);

    /*! \brief Can be used to set the grouping used in the input log. The parameter id has to specifiy 
     *         FORMAT_NONE, FORMAT_GROUP4 or FORMAT_GROUP5 depending on which grouping is desired.
     */       
    virtual void set_grouping_state_in(unsigned int id);

    /*! \brief Can be used to set the grouping used in the output log. The parameter id has to specifiy 
     *         FORMAT_NONE, FORMAT_GROUP4 or FORMAT_GROUP5 depending on which grouping is desired.
     */       
    virtual void set_grouping_state_out(unsigned int id);      

    /*! \brief Callback that is called when display_dialog::display_window is closed.
     */      
    virtual bool on_delete_log(GdkEventAny* event);
    
    /*! \brief The log dialog has to receive input characters. It is expected that this data is delivered through
     *         an appropriate signal. The parameter in_signal has to reference such a signal. When calling this
     *         method the display_dialog connects to in_signal. The resulting connection object is handled by
     *         the caller.
     */          
    virtual sigc::connection connect_input(sigc::signal<void, gunichar>& in_signal);

    /*! \brief The log dialog has to receive output characters. It is expected that this data is delivered through
     *         an appropriate signal. The parameter out_signal has to reference such a signal. When calling this
     *         method the display_dialog connects to out_signal. The resulting connection object is handled by
     *         the caller.
     */          
    virtual sigc::connection connect_output(sigc::signal<void, gunichar>& out_signal); 
    
    /*! \brief Returns a reference to a signal that is emitted when display_dialog::display_window is closed. 
     */       
    virtual sigc::signal<void>& signal_become_invisible() { return become_invisible; }     

    /*! \brief This method returns the Gtk::Window that is underlying this log dialog.
     */      
    virtual Gtk::Window *get_window() { return display_window; }

    /*! \brief Destructor.
     */      
    virtual ~display_dialog() { ; }
    
protected:
    /*! \brief Holds the Gtk::Builder object which can be used to create pointers to controls inside  
     *         display_dialog::display_window.
     */    
    Glib::RefPtr<Gtk::Builder> ref_xml;

    /*! \brief Holds a pointer to the Gtk::Window in which the actual log dialog controls reside. */    
    Gtk::Window *display_window;
    
    /*! \brief Holds a pointer to the main window of the application. */    
    Gtk::Window *parent;
    
    /*! \brief Holds a pointer to the "Group by 4" RadioButton in the output log. */    
    Gtk::RadioButton *group_4;
    
    /*! \brief Holds a pointer to the "Group by 5" RadioButton in the output log. */        
    Gtk::RadioButton *group_5;
    
    /*! \brief Holds a pointer to the "No grouping" RadioButton in the output log. */            
    Gtk::RadioButton *group_none;
    
    /*! \brief Holds a pointer to the "Group by 4" RadioButton in the input log. */    
    Gtk::RadioButton *group_4_in;
    
    /*! \brief Holds a pointer to the "Group by 5" RadioButton in the input log. */    
    Gtk::RadioButton *group_5_in;
    
    /*! \brief Holds a pointer to the "No grouping" RadioButton in the output log. */    
    Gtk::RadioButton *group_none_in;
    
    /*! \brief Holds a pointer to the TextView of in the input log. */    
    Gtk::TextView *in_view;
    
    /*! \brief Holds a pointer to the TextView of in the output log. */    
    Gtk::TextView *out_view;
    
    /*! \brief Holds the textview_logger for the input log. */    
    textview_logger in;
    
    /*! \brief Holds the textview_logger for the input log. */    
    textview_logger out;
    
    /*! \brief Points to the clear button for the input log. */    
    Gtk::Button *clear_input;
    
    /*! \brief Points to the clear button for the output log. */    
    Gtk::Button *clear_output;
    
    /*! \brief Points to the ScrolledWindow that contains the TextView of the input log. */    
    Gtk::ScrolledWindow *scroll_in;
    
    /*! \brief Points to the ScrolledWindow that contains the TextView of the output log. */    
    Gtk::ScrolledWindow *scroll_out;   
    
    /*! \brief Signal that is emitted when display_dialog::display_window is closed. */    
    sigc::signal<void> become_invisible;                 
};

#endif /* __display_dialog_h__ */
