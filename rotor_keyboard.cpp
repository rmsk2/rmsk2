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

/*! \file rotor_keyboard.cpp
 *  \brief Implementation of the rotor_keyboard class and its children.
 */ 

#include<rotor_keyboard.h>
#include<printing_device.h>

ustring rotor_keyboard::symbol_typed_encrypt(gunichar in)
{
    return machine->get_printer()->print_encrypt(get_key_code_encrypt(in, true));
}

ustring rotor_keyboard::symbol_typed_decrypt(gunichar in)
{
    return machine->get_printer()->print_decrypt(get_key_code_decrypt(in));
}

void rotor_keyboard::symbols_typed_encrypt(ustring& in, ustring& out)
{
    ustring::iterator iter;
    
    out.clear();
    
    for(iter = in.begin(); iter != in.end(); ++iter)
    {
        if (is_valid_input_encrypt(*iter))
        {
            out += symbol_typed_encrypt(*iter);
        }
    }    
}

ustring rotor_keyboard::symbols_typed_encrypt(ustring& in)
{
    ustring result;
    ustring::iterator iter;
    
    for(iter = in.begin(); iter != in.end(); ++iter)
    {
        result += symbol_typed_encrypt(*iter);
    }
    
    return result;
}

void rotor_keyboard::symbols_typed_decrypt(ustring& in, ustring& out)
{
    ustring::iterator iter;

    out.clear();
    
    for(iter = in.begin(); iter != in.end(); ++iter)
    {
        if (is_valid_input_decrypt(*iter))
        {
            out += symbol_typed_decrypt(*iter);
        }
    }
}

ustring rotor_keyboard::symbols_typed_decrypt(ustring& in)
{
    ustring result;
    ustring::iterator iter;
    
    for(iter = in.begin(); iter != in.end(); ++iter)
    {
        result += symbol_typed_decrypt(*iter);
    }
    
    return result;
}

/* ----------------------------------------------------------------- */

/*! This constructor allows to define the key codes which are used to switch between lettes
 *  and figures mode. It is a shorthand version of 
 *  the shifting_keyboard(unsigned int, unsigned int, unsigned int, unsigned int)
 *  constructor where the key codes where alphabet switching occurs is independent of whether
 *  the machine is currently in letters mode or in figures mode. This is for instance used in
 *  the Typex simulator.
 *
 *  \param letter_code [in] Defines the code where switching to letters mode occurs.
 *  \param figure_code [in] Defines the code where switching to figures mode occurs.
 */     
shifting_keyboard::shifting_keyboard(unsigned int letter_code, unsigned int figure_code)
     : rotor_keyboard()
 { 
    current_state = FUNC_LETTER_SHIFT;
    
    pair<unsigned int, unsigned int> std_pair(letter_code, figure_code);
    
    // Set key codes which cause state change while in letters or figures mode
    code_contacts[FUNC_LETTER_SHIFT] = std_pair;
    code_contacts[FUNC_FIGURE_SHIFT] = std_pair; 
}

/*! \param ltr_letter_code [in] Defines the code where switching to letters mode occurs while the 
 *                              rotor_keyboard is already in letters mode.
 *  \param ltr_figure_code [in] Defines the code where switching to figures mode occurs while the 
 *                              rotor_keyboard is in letters mode.
 *  \param fig_letter_code [in] Defines the code where switching to letters mode occurs while the 
 *                              rotor_keyboard is in figures mode.
 *  \param fig_figure_code [in] Defines the code where switching to figures mode occurs while the 
 *                              rotor_keyboard is already in figures mode.
 */     
shifting_keyboard::shifting_keyboard(unsigned int ltr_letter_code, unsigned int ltr_figure_code, unsigned int fig_letter_code, unsigned int fig_figure_code)
     : rotor_keyboard()
 { 
    current_state = FUNC_LETTER_SHIFT;
    
    pair<unsigned int, unsigned int> ltr_pair(ltr_letter_code, ltr_figure_code);
    pair<unsigned int, unsigned int> fig_pair(fig_letter_code, fig_figure_code);    

    // Set key codes which cause state change while in letters or figures mode     
    code_contacts[FUNC_LETTER_SHIFT] = ltr_pair;
    code_contacts[FUNC_FIGURE_SHIFT] = fig_pair; 
}

void shifting_keyboard::add_to_state(unsigned int key_code)
{
    if (key_code == code_contacts[current_state].first)
    {
        current_state = FUNC_LETTER_SHIFT;
    }
    else
    {
        if (key_code == code_contacts[current_state].second)
        {
            current_state = FUNC_FIGURE_SHIFT;
        }
    }
}

void shifting_keyboard::add_char_to_state(gunichar symbol)
{
    if (symbol == '<')
    {
        current_state = FUNC_LETTER_SHIFT;
    }
    else
    {
        if (symbol == '>')
        {
            current_state = FUNC_FIGURE_SHIFT;
        }
    }
}

bool shifting_keyboard::is_valid_input_encrypt(gunichar to_test)
{
    bool result = false;
    
    if (current_state == FUNC_LETTER_SHIFT)
    {
        result = letters_alpha->contains_symbol(to_test);
    }
    else
    {
        result = figures_alpha->contains_symbol(to_test);  
    }                    
    
    return result;
}
 
bool shifting_keyboard::is_valid_input_decrypt(gunichar to_test)
{
    return out_alpha->contains_symbol(to_test);
}


gunichar shifting_keyboard::map_key_code_encrypt(unsigned int key_code, bool f_add_to_state)
{
    gunichar result;
    
    if (current_state == FUNC_LETTER_SHIFT)
    {
        result = letters_alpha->to_val(key_code);
    }
    else
    {
        result = figures_alpha->to_val(key_code);    
    }                    
    
    if (f_add_to_state)
    {
        add_to_state(key_code);
    }
    
    return result;
}

gunichar shifting_keyboard::map_key_code_decrypt(unsigned int key_code)
{
    return out_alpha->to_val(key_code);
}

unsigned int shifting_keyboard::get_key_code_encrypt(gunichar in, bool f_add_to_state)
{
    unsigned int result = 0;

    if (current_state == FUNC_LETTER_SHIFT)
    {
        result = letters_alpha->from_val(in);
    }
    else
    {
        result = figures_alpha->from_val(in);    
    }                
    
    if (f_add_to_state)
    {
        add_char_to_state(in);
    }
    
    return result;
}

unsigned int shifting_keyboard::get_key_code_decrypt(gunichar in)
{   
    return out_alpha->from_val(in);
}

/* ----------------------------------------------------------------- */

symmetric_keyboard::symmetric_keyboard(ustring alpha_chars)
{
    vector<gunichar> temp;
    
    printing_device::ustr_to_vec(alpha_chars, temp);
    sym_alpha = boost::shared_ptr<alphabet<gunichar> >(new alphabet<gunichar>(temp));
}

gunichar symmetric_keyboard::map_key_code_encrypt(unsigned int key_code, bool add_to_state)
{    
    return sym_alpha->to_val(key_code);
}

gunichar symmetric_keyboard::map_key_code_decrypt(unsigned int key_code)
{
    return sym_alpha->to_val(key_code);
}

unsigned int symmetric_keyboard::get_key_code_encrypt(gunichar in, bool add_to_state)
{
    return sym_alpha->from_val(in);
}

unsigned int symmetric_keyboard::get_key_code_decrypt(gunichar in)
{
    return sym_alpha->from_val(in);
}

bool symmetric_keyboard::is_valid_input_decrypt(gunichar to_test)
{
    return sym_alpha->contains_symbol(to_test);
}

bool symmetric_keyboard::is_valid_input_encrypt(gunichar to_test)
{
    return sym_alpha->contains_symbol(to_test);
}


/* ----------------------------------------------------------------- */

gunichar asymmetric_keyboard::map_key_code_encrypt(unsigned int key_code, bool add_to_state)
{
    return dec_alpha->to_val(key_code);
}

gunichar asymmetric_keyboard::map_key_code_decrypt(unsigned int key_code)
{
    return enc_alpha->to_val(key_code);
}

unsigned int asymmetric_keyboard::get_key_code_encrypt(gunichar in, bool add_to_state)
{
    return dec_alpha->from_val(in);
}

unsigned int asymmetric_keyboard::get_key_code_decrypt(gunichar in)
{
    return enc_alpha->from_val(in);
}

bool asymmetric_keyboard::is_valid_input_encrypt(gunichar to_test)
{
    return dec_alpha->contains_symbol(to_test);
}

bool asymmetric_keyboard::is_valid_input_decrypt(gunichar to_test)
{
    return enc_alpha->contains_symbol(to_test);
}


