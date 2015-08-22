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

/*! \file printing_device.cpp
 *  \brief Implementation of the printing_device class and its children.
 */ 
#include<printing_device.h>

ustring printing_device::print_encrypt(vector<unsigned int>& in)
{
    ustring result;
    vector<unsigned int>::iterator iter;
    
    for (iter = in.begin(); iter != in.end(); ++iter)
    {
        result += print_encrypt(*iter);
    }
    
    return result;
}

ustring printing_device::print_decrypt(vector<unsigned int>& in)
{
    ustring result;
    vector<unsigned int>::iterator iter;
    
    for (iter = in.begin(); iter != in.end(); ++iter)
    {
        result += print_decrypt(*iter);
    }
    
    return result;    
}


void printing_device::ustr_to_vec(ustring& str, vector<gunichar>& vec)
{
    vec.clear();
    ustring::iterator iter;
    
    for (iter = str.begin(); iter != str.end(); ++iter)
    {
        vec.push_back(*iter);
    }
}

/* ----------------------------------------------------------------- */

shifting_printing_device::shifting_printing_device(unsigned int letter_char, unsigned int figure_char)
     : printing_device()
 { 
    current_state = FUNC_LETTER_SHIFT; 
    pair<unsigned int, unsigned int> std_pair(letter_char, figure_char);
     
    code_contacts[FUNC_LETTER_SHIFT] = std_pair;
    code_contacts[FUNC_FIGURE_SHIFT] = std_pair;
    
    init_special_outputs();
}


/*! \param ltr_letter_char [in] Defines the code where switching to letters mode occurs while the 
 *                              shifting_printing_device is already in letters mode.
 *  \param ltr_figure_char [in] Defines the code where switching to figures mode occurs while the 
 *                              shifting_printing_device is in letters mode.
 *  \param fig_letter_char [in] Defines the code where switching to letters mode occurs while the 
 *                              shifting_printing_device is in figures mode.
 *  \param fig_figure_char [in] Defines the code where switching to figures mode occurs while the 
 *                              shifting_printing_device is already in figures mode.
 */     
shifting_printing_device::shifting_printing_device(unsigned int ltr_letter_char, unsigned int ltr_figure_char, unsigned int fig_letter_char, unsigned int fig_figure_char)
     : printing_device()
{ 
    current_state = FUNC_LETTER_SHIFT;
    
    pair<unsigned int, unsigned int> ltr_pair(ltr_letter_char, ltr_figure_char);
    pair<unsigned int, unsigned int> fig_pair(fig_letter_char, fig_figure_char);    
     
    code_contacts[FUNC_LETTER_SHIFT] = ltr_pair;
    code_contacts[FUNC_FIGURE_SHIFT] = fig_pair; 
    
    init_special_outputs();
} 

void shifting_printing_device::init_special_outputs()
{
    pair<ustring, ustring> no_output(ustring(""), ustring(""));

    special_outputs.clear();
    
    special_outputs[FUNC_LETTER_SHIFT] = no_output;
    special_outputs[FUNC_FIGURE_SHIFT] = no_output;    
}

/*! \param out_ltr_state [in] Defines the output value when the "switch to letters" code occurs during 
 *                            decryption while this shifting_printing_device is in letters mode
 *  \param out_fig_state [in] Defines the output value when the "switch to letters" code occurs during 
 *                            decryption while this shifting_printing_device is in figures mode
 */     
void shifting_printing_device::set_ltr_key_output(ustring out_ltr_state, ustring out_fig_state)
{
    special_outputs[FUNC_LETTER_SHIFT].first = out_ltr_state;
    special_outputs[FUNC_FIGURE_SHIFT].first = out_fig_state;
}

/*! \param out_ltr_state [in] Defines the output value when the "switch to figures" code occurs during 
 *                            decryption while this shifting_printing_device is in letters mode
 *  \param out_fig_state [in] Defines the output value when the "switch to figures" code occurs during 
 *                            decryption while this shifting_printing_device is in figures mode
 */     
void shifting_printing_device::set_fig_key_output(ustring out_ltr_state, ustring out_fig_state)
{
    special_outputs[FUNC_LETTER_SHIFT].second = out_ltr_state;
    special_outputs[FUNC_FIGURE_SHIFT].second = out_fig_state;
}

ustring shifting_printing_device::print_encrypt(unsigned int in)
{
    ustring result;
    
    result += out_alpha->to_val((last_cipher_result = machine->encrypt(in)));
    
    return result;
}


ustring shifting_printing_device::print_decrypt(unsigned int in)
{
    ustring result;
    
    in = machine->decrypt(in);
    last_cipher_result = in;

    // Switch to letters mode if necessary
    if (in == code_contacts[current_state].first)
    {
        result = special_outputs[current_state].first;
        current_state = FUNC_LETTER_SHIFT;
    }
    else
    {
        // Switch to figures mode if necessary
        if (in == code_contacts[current_state].second)
        {
            result = special_outputs[current_state].second;
            current_state = FUNC_FIGURE_SHIFT;
        }
        else
        {
            // No switching code, apply normal alphabetes as defined by current_state
            if (current_state == FUNC_LETTER_SHIFT)
            {
                result += letters_alpha->to_val(in);
            }
            else
            {
                result += figures_alpha->to_val(in);
            }        
        }
    }
    
    return result;
}

/* ----------------------------------------------------------------- */

ustring asymmetric_printing_device::print_encrypt(unsigned int in)
{
    ustring result;

    result += enc_alpha->to_val((last_cipher_result = machine->encrypt(in)));

    return result;
}

ustring asymmetric_printing_device::print_decrypt(unsigned int in)
{
    ustring result;

    result += dec_alpha->to_val((last_cipher_result = machine->decrypt(in)));

    return result;
}

/* ----------------------------------------------------------------- */

symmetric_printing_device::symmetric_printing_device(ustring alpha_chars)
{
    vector<gunichar> help;
    
    ustr_to_vec(alpha_chars, help);
    boost::shared_ptr<alphabet<gunichar> > alpha(new alphabet<gunichar>(help));
    set_alphabet(alpha);
}

ustring symmetric_printing_device::print_encrypt(unsigned int in) 
{ 
    ustring result; 
    result += sym_alpha->to_val((last_cipher_result = machine->encrypt(in))); 
    return result; 
}

ustring symmetric_printing_device::print_decrypt(unsigned int in)
{ 
    ustring result; 
    result += sym_alpha->to_val((last_cipher_result = machine->decrypt(in))); 
    return result;  
}

