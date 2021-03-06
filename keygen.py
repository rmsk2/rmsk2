################################################################################
# Copyright 2018 Martin Grap
# 
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
################################################################################

## @package keygen A Python3 GUI program which allows to generate key sheets for all machines provided by rmsk2 and rotorsim.
#           
# \file rmsk2/keygen.py
# \brief This file imlements a keysheet generator with a GUI for all rotor machines provided by rmsk2 and rotorsim.

import multiprocessing
import gi
gi.require_version('Gtk', '3.0')
from gi.repository import Gtk, Gio, GdkPixbuf, GObject
import pyrmsk2
import pyrmsk2.keysheetgen as keysheetgen
import pyrmsk2.keygenicon as keygenicon

## \brief Type-ID for progress messages.
TAG_MESSAGE = 1
## \brief Type-ID for final "all is done" message.
TAG_DONE = 2
## \brief Type-ID for error messages.
TAG_ERROR = 3

## \brief Text of Apache 2.0 license for about dialog.
LICENSE_TEXT = """
Copyright 2020 Martin Grap

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
"""

## \brief XML specification of the structure of the main menu.
MENU_XML="""
<?xml version="1.0" encoding="UTF-8"?>
<interface>
  <menu id="menubar">
    <submenu>
      <attribute name="label" translatable="no">_Help</attribute>
      <section>
      <item>
        <attribute name="action">keygen.help</attribute>
        <attribute name="label" translatable="no">_Help</attribute>
      </item>
      <item>
        <attribute name="action">keygen.about</attribute>
        <attribute name="label" translatable="no">_About</attribute>
      </item>
      </section>
      <section>
      <item>
        <attribute name="action">keygen.quit</attribute>
        <attribute name="label" translatable="no">_Quit</attribute>
      </item>
      </section>
    </submenu>
  </menu>
</interface>
"""

## \brief A class that is used to communicate progress and error messages between the main program and
#         the process ni which the keysheet generator is running.
#        
class ControlValue:
    ## \brief Constructor
    #
    #  \param [tag] Is an integer. It can take the values TAG_MESSAGE, TAG_DONE or TAG_ERROR. Specifies the
    #         type of the message.
    #
    #  \param [message] Is a string. Contains a message intended for the user.
    #        
    def __init__(self, tag, message):
        self.tag = tag
        self.message = message


## \brief A class that records reported events in a list.
#        
class ListReporter(keysheetgen.ReporterBase):
    ## \brief Constructor
    #        
    def __init__(self):
        ## \brief This list of ControlValue objets holds the internal list of events.
        self._list = []
        
    ## \brief This method clears the internally kept list of errors and other messages.
    #
    #  \returns Nothing.
    #            
    def reset(self):
        self._list = []
    
    ## \brief This property returns True if the list of events contains an error.
    #
    #  \returns A boolean. True if the internal list of events contains a message of type TAG_ERROR.
    #
    @property
    def has_error(self):
        return len(self.errors) > 0

    ## \brief This property returns a list of all stored events that are of type TAG_ERROR.
    #
    #  \returns A list of ControlValue objects.
    #
    @property
    def errors(self):
        return list(filter(lambda x: x.tag == TAG_ERROR, self._list))

    ## \brief This property returns a string that combines all the messages of stored events that are of
    #         type TAG_ERROR.
    #
    #  \returns A string. It contains the aggregate of all error messages.
    #
    @property
    def error_messages(self):
        result = ''
        errors = filter(lambda x: x.tag == TAG_ERROR, self._list)
        messages = map(lambda x: x.message, errors)
                
        for m in messages:
            result += m + '\n'
        
        return result
    
    ## \brief This method reports an error to the user.
    #
    #  \param [message] Is a string. Holds the message which is intended to be displayed to the user.
    #
    #  \returns Nothing.
    #
    def report_error(self, message):
        self._list.append(ControlValue(TAG_ERROR, message))

    ## \brief This method reports a progress to the user.
    #
    #  \param [message] Is a string. Holds the message which is intended to be displayed to the user.
    #
    #  \returns Nothing.
    #
    def report_progress(self, message):
        self._list.append(ControlValue(TAG_MESSAGE, message))

    ## \brief This method can be used to signal that processing has been finished.
    #
    #  \returns Nothing.
    #    
    def all_done(self):
        self._list.append(ControlValue(TAG_DONE, ''))


## \brief A class that records reported events in a multiprocessing.Queue object.
#        
class QueueReporter(keysheetgen.ReporterBase):
    ## \brief Constructor
    #
    #  \param [q] Is an object of type multiprocessing.Queue. It references the queue which is used to record
    #         reported events.
    #        
    def __init__(self, q):
        self._q = q
    
    ## \brief This method reports an error to the user.
    #
    #  \param [message] Is a string. Holds the message which is intended to be displayed to the user.
    #
    #  \returns Nothing.
    #
    def report_error(self, message):
        self._q.put(ControlValue(TAG_ERROR, message))

    ## \brief This method reports a progress to the user.
    #
    #  \param [message] Is a string. Holds the message which is intended to be displayed to the user.
    #
    #  \returns Nothing.
    #
    def report_progress(self, message):
        self._q.put(ControlValue(TAG_MESSAGE, message))

    ## \brief This method can be used to signal that processing has been finished.
    #
    #  \returns Nothing.
    #    
    def all_done(self):
        self._q.put(ControlValue(TAG_DONE, ''))


## \brief A class that binds together the parameters which are needed to generate a key sheet.
#        
class SheetGenArgs:
    ## \brief Constructor
    #
    #  \param [machine] Is a string. It has to contain the name of the machine for which a key sheet is to be 
    #         generated.
    #        
    #  \param [year] Is an integer. It has to contain the year for which a key sheet is to be generated.
    #        
    #  \param [month] Is an integer or None. It has to contain the number of the month (1..12)  for which a sheet
    #         is to be generated.  
    #        
    #  \param [classification] Is a string. It has to contain the classification level that is to appear on the sheet.
    #        
    #  \param [net] Is a string. It has to contain the name of the key or crypto net which is to appear on the key sheet.
    #            
    #  \param [rotor_set_file_name] Is a string. It has to contain the name of the file that contains a custom rotor set.
    #         If this value is '' no custom rotor set is loaded.
    #            
    #  \param [msg_proc_type] Is a string. It has to specify the type of the message procdure the keysheet is intended.
    #            
    def __init__(self, machine, year, month, classification, net, rotor_set_file_name, msg_proc_type):
        self.type = machine
        self.year = year
        self.month = month
        self.classification = classification
        self.net = net
        self.save_states = False
        self.out = None
        self.html = False
        self.tlv_server = keysheetgen.rotorsim.tlvobject.get_tlv_server_path()
        self.msg_proc_type = msg_proc_type
        self.load_set = rotor_set_file_name


## \brief A class that abstracts the background process that is started when key sheets for a whole year
#         are generated.
#        
class Backgrounder:
    ## \brief Constructor
    #
    #  \param [args] An object of type SheetGenArgs that holds the desired sheet parameters.
    #        
    def __init__(self, args):
        self.args = args

    ## \brief This method performs the key sheet generation.
    #
    #  \param [queue] Is an object of type multiprocessing.Queue. This queue is used to for communication between the main
    #         process and the process in which the sheet generation is actually performed.
    #
    #  \returns Nothing.
    #        
    def do_work(self, queue):
        reporter = QueueReporter(queue)
        keysheetgen.KeysheetGeneratorMain.generate_sheets(self.args, reporter)
        queue.close()


## \brief A class that implements the main window for the key sheet generator.
#        
class KeyGenWindow(Gtk.Window):
    ## \brief Constructor
    #
    def __init__(self):
        Gtk.Window.__init__(self, title="Keysheet Generator")
        self.set_border_width(10)
        self._ainfo = Gio.app_info_get_default_for_uri_scheme('ghelp')

        vbox = Gtk.Box(orientation=Gtk.Orientation.VERTICAL, spacing=6)
        self.add(vbox)
        
        self.setup_menus(vbox)

        self._progressbar = Gtk.ProgressBar()
        vbox.pack_start(self._progressbar, True, True, 0)
        self._progressbar.set_text('Done')
        self._progressbar.set_show_text(True)
        
        grid = Gtk.Grid()
        vbox.pack_start(grid, True, True, 0)

        # Machine row        
        machine_label = Gtk.Label('Machine:')
        grid.attach(machine_label, 0, 0, 1, 1)
        grid.set_row_spacing(6)
        
        self._machine_combo = Gtk.ComboBoxText()
        self._machine_combo.set_hexpand(True)
        self._machine_combo.set_entry_text_column(0)

        for machine_name in keysheetgen.MACHINE_NAMES:
            self._machine_combo.append_text(machine_name)
            
        self._machine_combo.set_active(0)
        
        grid.attach(self._machine_combo, 1, 0, 1, 1)
                
        # Month row        
        month_label = Gtk.Label('Month:')
        grid.attach(month_label, 0, 1, 1, 1)
        
        self._month_combo = Gtk.ComboBoxText()
        self._month_combo.set_hexpand(True)
        self._month_combo.set_entry_text_column(0)

        for month in ['Whole year', 'January', 'February', 'March', 'April', 'May', 'June', 'July', 'August', 'September', 'October', 'November', 'December']:
            self._month_combo.append_text(month)
            
        self._month_combo.set_active(0)
        
        grid.attach(self._month_combo, 1, 1, 1, 1)

        # Year row
        year_label = Gtk.Label('Year:')
        grid.attach(year_label, 0, 2, 1, 1)
        
        year_adjustment = Gtk.Adjustment(1942, 1900, 2100, 1, 10, 0)
        self._year_entry = Gtk.SpinButton()
        self._year_entry.set_adjustment(year_adjustment)
        self._year_entry.set_numeric(True)
        self._year_entry.set_value(1942)
        self._year_entry.set_hexpand(True)
        grid.attach(self._year_entry, 1, 2, 1, 1)

        # Classification row
        classification_label = Gtk.Label('Classification:')
        grid.attach(classification_label, 0, 3, 1, 1)
                                
        self._classifciation_entry = Gtk.Entry()
        self._classifciation_entry.set_text('STRENG GEHEIM')
        self._classifciation_entry.set_hexpand(True)
        grid.attach(self._classifciation_entry, 1, 3, 1, 1)

        # Crypto net/key name row
        net_name_label = Gtk.Label('Crypto net/key name:')
        grid.attach(net_name_label, 0, 4, 1, 1)
                                
        self._net_name_entry = Gtk.Entry()
        self._net_name_entry.set_text('Maschinenschlüssel Nr. 476')
        (width, height) = self._net_name_entry.get_size_request()
        size_request = self._net_name_entry.set_size_request(width + 300, height)        
        self._net_name_entry.set_hexpand(True)
        grid.attach(self._net_name_entry, 1, 4, 1, 1)
        
        # Message procedure row        
        proc_label = Gtk.Label('Message Procedure:')
        grid.attach(proc_label, 0, 5, 1, 1)
        
        self._proc_combo = Gtk.ComboBoxText()
        self._proc_combo.set_hexpand(True)
        self._proc_combo.set_entry_text_column(0)

        for proc_type in keysheetgen.PROC_TYPES:
            self._proc_combo.append_text(proc_type)
            
        self._proc_combo.set_active(0)
        
        grid.attach(self._proc_combo, 1, 5, 1, 1)        

        # Output format row
        html_label = Gtk.Label('Output format:')
        grid.attach(html_label, 0, 6, 1, 1)
                                
        self._html_button = Gtk.RadioButton('HTML')
        self._html_button.set_hexpand(True)
        self._text_button = Gtk.RadioButton(label='TXT', group=self._html_button)
        self._text_button.set_hexpand(True)
        radio_hbox = Gtk.Box(orientation=Gtk.Orientation.HORIZONTAL, spacing=6)
        radio_hbox.pack_start(self._html_button, True, True, 0)
        radio_hbox.pack_start(self._text_button, True, True, 0)        

        grid.attach(radio_hbox, 1, 6, 1, 1)

        # Save state files row
        save_state_label = Gtk.Label('Save state files:')
        grid.attach(save_state_label, 0, 7, 1, 1)
                                
        self._save_state_button = Gtk.CheckButton()
        self._save_state_button.set_active(False)
        self._save_state_button.set_hexpand(True)
        grid.attach(self._save_state_button, 1, 7, 1, 1)

        # Output directory row
        outdir_label = Gtk.Label('Output directory:')
        grid.attach(outdir_label, 0, 8, 1, 1)
        
        hbox = Gtk.Box(orientation=Gtk.Orientation.HORIZONTAL, spacing=6)
                                
        self._outdir_entry = Gtk.Entry()
        self._outdir_entry.set_text('')
        self._outdir_entry.set_hexpand(True)                
        hbox.pack_start(self._outdir_entry, True, True, 0)
        
        self._file_button = Gtk.Button("...")
        self._file_button.connect("clicked", self.select_directory)
        hbox.pack_start(self._file_button, False, True, 0)
        
        grid.attach(hbox, 1, 8, 1, 1)

        # Rotor set to load row
        load_set_label = Gtk.Label('Rotor set to load:')
        grid.attach(load_set_label, 0, 9, 1, 1)
        
        hbox = Gtk.Box(orientation=Gtk.Orientation.HORIZONTAL, spacing=6)
                                
        self._load_set_entry = Gtk.Entry()
        self._load_set_entry.set_text('')
        self._load_set_entry.set_hexpand(True)                
        hbox.pack_start(self._load_set_entry, True, True, 0)
        
        data_getter = lambda: self._load_set_entry.get_text()
        data_setter = lambda x: self._load_set_entry.set_text(x)
        load_set_callback = lambda x: self.select_input_file(x, data_getter, data_setter, "Select rotor set file")
        
        self._load_set_button = Gtk.Button("...")
        self._load_set_button.connect("clicked", load_set_callback)
        hbox.pack_start(self._load_set_button, False, True, 0)
        
        grid.attach(hbox, 1, 9, 1, 1)
        
        # Generate button        
        self._main_button = Gtk.Button("Generate")
        self._main_button.connect("clicked", self.generate_sheet)
        vbox.pack_start(self._main_button, True, True, 0)        
        
        self._b = None
        self._error_list = ListReporter()
                
        xpm_data = keygenicon.get_xpm_data('keygenicon')
        self._logo = GdkPixbuf.Pixbuf().new_from_xpm_data(xpm_data)
        
        self.set_icon(self._logo)
        
        self.show_all()

    ## \brief This method creates the menubar of the application.
    #
    #  \param [vbox] Is a Gtk.Box object. After creation the new menubar is inserted into this box.
    #
    #  \returns Nothing.
    #            
    def setup_menus(self, vbox):
        self._action_group = Gio.SimpleActionGroup()
        
        self._about_action = Gio.SimpleAction.new('about', None)
        self._about_action.connect('activate', self.on_about)
        
        self._help_action = Gio.SimpleAction.new('help', None)  
        self._help_action.connect('activate', self.on_help)              

        self._quit_action = Gio.SimpleAction.new('quit', None)  
        self._quit_action.connect('activate', self.on_quit)              
        
        self._action_group.add_action(self._about_action)
        self._action_group.add_action(self._help_action)
        self._action_group.add_action(self._quit_action)        
        
        self.insert_action_group('keygen', self._action_group)
        
        builder = Gtk.Builder()
        builder.add_from_string(MENU_XML)
        menubar = builder.get_object('menubar')
        vbox.pack_start(Gtk.MenuBar.new_from_model(menubar), False, True, 0)        

    ## \brief This method allows to show an error message to the user.
    #
    #  \param [message] Is a string. It specifies the message to be shown to the user.
    #
    #  \returns Nothing.
    #        
    def show_error_message(self, message):
        dialog = Gtk.MessageDialog(self, 0, Gtk.MessageType.ERROR, Gtk.ButtonsType.OK, message)        
        dialog.run()       
        dialog.destroy()    

    ## \brief This method allows to show an informational message to the user.
    #
    #  \param [message] Is a string. It specifies the message to be shown to the user.
    #
    #  \returns Nothing.
    #        
    def show_message(self, message):
        dialog = Gtk.MessageDialog(self, 0, Gtk.MessageType.INFO, Gtk.ButtonsType.OK, message)        
        dialog.run()       
        dialog.destroy()

    ## \brief This method serves as a callback for the "..." button next of the "Output directory" which is intended to
    #         select an output directory for the keysheets and (optionally) state files.
    #
    #  \param [widget] Is an object of type Gtk.Widget. This is a required part of a callback's signature.
    #
    #  \returns Nothing.
    #        
    def select_directory(self, widget):
        dialog = Gtk.FileChooserDialog("Choose output directory", self, Gtk.FileChooserAction.SELECT_FOLDER, (Gtk.STOCK_CANCEL, Gtk.ResponseType.CANCEL, "Select", Gtk.ResponseType.OK))
        dialog.set_create_folders(False)

        # Set folder initially displayed by the dialog
        if self._outdir_entry.get_text() != '':
            dialog.set_filename(self._outdir_entry.get_text())

        response = dialog.run()
        
        # Use data if user clicked on OK 
        if response == Gtk.ResponseType.OK:
            self._outdir_entry.set_text(dialog.get_filename())

        dialog.destroy()

    ## \brief This method serves as a callback for for selecting an input file.
    #
    #  \param [widget] Is an object of type Gtk.Widget. This is a required part of a callback's signature.
    #
    #  \param [var_getter] Is a callable object that takes no argument and returns a string. It is used to retrieve the
    #         the default value for file to select.
    #
    #  \param [var_setter] Is a callable object that takes a string and returns nothing. It is used to set a variable
    #         to the file name selected by the user.    
    #
    #  \param [user_info_text] Is a string. Contains the information text displayed to the user in order to explain what type
    #         of file is to be selected.
    #    
    #  \returns Nothing.
    #        
    def select_input_file(self, widget, var_getter, var_setter, user_info_text):
        dialog = Gtk.FileChooserDialog(user_info_text, self, Gtk.FileChooserAction.OPEN, (Gtk.STOCK_CANCEL, Gtk.ResponseType.CANCEL, "Select", Gtk.ResponseType.OK))
        dialog.set_create_folders(False)

        # Set file_name initially displayed by the dialog
        if var_getter() != '':
            dialog.set_filename(var_getter())

        response = dialog.run()
        
        # Use data if user clicked on OK 
        if response == Gtk.ResponseType.OK:
            var_setter(dialog.get_filename())

        dialog.destroy()

    ## \brief This method serves as a callback for the "Generate" button.
    #
    #  \param [button] Is an object of type Gtk.Button.
    #
    #  \returns Nothing.
    #        
    def generate_sheet(self, button):
        if self._outdir_entry.get_text() != '':
            # Get sheet generation parameters from GUI  
            args = SheetGenArgs(keysheetgen.MACHINE_NAMES[self._machine_combo.get_active()], int(self._year_entry.get_value()), self._month_combo.get_active(),\
                                self._classifciation_entry.get_text(), self._net_name_entry.get_text(), self._load_set_entry.get_text(), \
                                keysheetgen.PROC_TYPES[self._proc_combo.get_active()])
            
            args.out = self._outdir_entry.get_text()
            args.html = self._html_button.get_active()
            args.save_states = self._save_state_button.get_active()
            
            # "Whole year" is entry with index 0 in the combo box
            if self._month_combo.get_active() == 0:
                # Generate sheets for a whole year
                args.month = None
                self.generate_year(args)
            else:
                # Generate sheets for a single month.
                self.generate_month(args)
        else:
            self.show_error_message('Please select an output directory')

    ## \brief This method generates a key sheet for a given month.
    #
    #  \param [args] Is an object of type SheetGenArgs. It contains the arguments used for key sheet generation.
    #
    #  \returns Nothing.
    #        
    def generate_month(self, args):
        self._error_list.reset()

        keysheetgen.KeysheetGeneratorMain.generate_sheets(args, self._error_list)            
        
        if self._error_list.has_error:
            self.show_error_message(self._error_list.error_messages)
        else:
            self.show_message('Keysheet generated successfully')

    ## \brief This method generates a key sheet for a whole year.
    #
    #  It spawns a background key sheet generation process. The GUI is updated each 750ms via a timer. This may not
    #  be the fastest, most sensible or even the most stable way to do this, but I like the fact that this facilitates
    #  using the exact same code as the command line version of the key sheet generator.
    #
    #  \param [args] Is an object of type SheetGenArgs. It contains the arguments used for key sheet generation.
    #
    #  \returns Nothing.
    #        
    def generate_year(self, args):        
        # Preparations
        self._error_list.reset()
        # "Grey out" the "Generate" button. Only one sheet generation should run at any given point in time with any
        # instance of this program.
        self._main_button.set_sensitive(False)
        self._progressbar.set_text('Generating keysheets')        
        
        # Start background worker
        self._q = multiprocessing.Queue()
        self._b = Backgrounder(args)
        self._t = multiprocessing.Process(target=self._b.do_work, args=(self._q,))
        self._t.start()
        
        # Start timer
        self.timeout_id = GObject.timeout_add(750, self.on_timeout, None)      


    ## \brief Callback for menu entry "Help"
    #
    #  \param [action] Is a Gtk.Action object. Not used by this method.
    #
    #  \param [value] Is an object of generic type. It is not used by this method.
    #
    #  \returns Nothing.
    #            
    def on_help(self, action, value):
        self._ainfo.launch_uris(['ghelp://' + pyrmsk2.get_doc_path(__file__) + '/keygen/index.page'])

    ## \brief Callback for menu entry "Quit"
    #
    #  \param [action] Is a Gtk.Action object. Not used by this method.
    #
    #  \param [value] Is an object of generic type. It is not used by this method.
    #
    #  \returns Nothing.
    #            
    def on_quit(self, action, value):
        Gtk.main_quit()

    ## \brief Callback for menu entry "About"
    #
    #  \param [action] Is a Gtk.Action object. Not used by this method.
    #
    #  \param [value] Is an object of generic type. It is not used by this method.
    #
    #  \returns Nothing.
    #            
    def on_about(self, action, value):
        about_dialog = Gtk.AboutDialog(self) 
        about_dialog.set_transient_for(self)
        about_dialog.set_program_name('Key Sheet Generator for rmsk2')
        about_dialog.set_copyright('Copyright 2020 Martin Grap')
        about_dialog.set_version(pyrmsk2.get_version_string()) 
        about_dialog.set_website("https://github.com/rmsk2/rmsk2/wiki/Key-sheet-generator") 
        about_dialog.set_website_label("GitHub Wiki for rmsk2") 
        about_dialog.set_authors(["Martin Grap"]) 
        about_dialog.set_license(LICENSE_TEXT)
        
        about_dialog.set_logo(self._logo)
        
        about_dialog.run() 
        about_dialog.destroy()

    ## \brief This method is the callback for the timer that is used to update the GUI with respect to the sheet
    #         generation progress.
    #
    #  \param [user_data] Is an object/value of undefined type. This is required by the python3 GTK bindings.
    #
    #  \returns Nothing.
    #            
    def on_timeout(self, user_data):        
        cv = self._q.get()
        stop = (cv.tag != TAG_DONE)        

        if not stop:
            # All sheets have been generated. Show possible error messages and clean up things.
            self._progressbar.set_fraction(0.0)
            self._progressbar.set_text('Done')
            self._q.close()
            self._q.join_thread()            
            self._t.join()
            
            if self._error_list.has_error:
                self.show_error_message(self._error_list.error_messages)
            
            # Reenable "Generate" button.
            self._main_button.set_sensitive(True)
        else:
            if cv.tag == TAG_MESSAGE:
                # Update progress bar
                new_value = self._progressbar.get_fraction() + (1/12)

                if new_value > 1:
                    new_value = 0

                self._progressbar.set_fraction(new_value)
                self._progressbar.set_text(cv.message)
            else:
                # Collect error messages
                self._progressbar.set_text('Error')
                self._error_list.report_error(cv.message)

        return stop

win = KeyGenWindow()
win.connect("delete-event", Gtk.main_quit)
win.show_all()
Gtk.main()

