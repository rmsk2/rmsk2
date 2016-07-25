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

#include<cmdline_base.h>


istream *cmdline_base::determine_input_stream(string& file_name, ifstream& file_in)
{
    istream *result = NULL;

    if (file_name != "")
    {        
        file_in.open(file_name, ifstream::in | ifstream::binary);
        
        if (file_in.good())
        {
            result = &file_in;
        }
    }
    else
    {
        result = &cin;
    }
    
    return result;
}

ostream *cmdline_base::determine_output_stream(string& file_name, ofstream& file_out)
{
    ostream *result = NULL;
    
    if (file_name != "")
    {
        file_out.open(file_name, ofstream::out | ofstream::binary);
        
        if (file_out.good())
        {
            result = &file_out;
        }
    }            
    else
    {
        result = &cout;
    }
    
    return result;
}

int cmdline_base::save_machine_state(string& file_name, rotor_machine *machine, bool delimiter_first)
{
    Glib::KeyFile ini_file;
    int result = RETVAL_OK;
    Glib::ustring data_temp; 
    string ini_data;       

    if (file_name != "")
    {
        if (machine->save(file_name))
        {
            result = ERR_IO_FAILURE;
        }
    }
    else
    {
        // Write state to stdout using 0xFF as delimiter between the state and the output data
        machine->save_ini(ini_file);
        
        if (delimiter_first)
        {                
            cout << (char)(255);
        }
        
        data_temp = ini_file.to_data();
        ini_data = data_temp;
        cout << ini_data;

        if (!delimiter_first)
        {                
            cout << (char)(255);
        }
    }            
    
    return result;
}

int cmdline_base::read_delimited_stream(istream *in, string& data_read, int delimiter)
{
    int result = RETVAL_OK;
    int char_read = 0;

    data_read.clear();
    
    try
    {
        while (in->good() and (char_read != delimiter))
        {
            // Read input character
            char_read = in->get();
            
            if ((char_read != char_traits<char>::eof()) and (char_read != delimiter))
            {
                data_read += (char)char_read;
            }
        }
        
        // Check if input stream is still ok. If the failbit is set on 
        // the stream then flag an error. If the failbit is set but the
        // stream has also reached EOF, then everything is still OK.
        if (in->fail() and !in->eof())
        {
            result = ERR_IO_FAILURE;
        }        
    }
    catch(...)
    {
        result = ERR_IO_FAILURE;
    }    
    
    return result;
}
