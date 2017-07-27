/***************************************************************************
 * Copyright 2017 Martin Grap
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

/*! \file rmsk_globals.cpp
 *
 *  \brief Implementations of some general helper functions and classes.
 */     

#include<unistd.h>
#include<rmsk_globals.h>
#include<glade_data.h>
#include<enigma_sim.h>
#include<typex.h>
#include<sigaba.h>
#include<sg39.h>
#include<nema.h>
#include<kl7.h>

const unsigned int IN_LENGTH_NOT_DIVISBLE_BY_THREE = 1;
const unsigned int OUT_LENGTH_INCORRECT = 2;
const unsigned int PAD_LENGTH_LARGER_THAN_TWO = 3;
const unsigned int IN_LENGTH_NOT_DIVISBLE_BY_FOUR = 4;
const unsigned int ILLEGAL_CHAR = 5;
const unsigned int PADDING_BAD_FORMAT = 6;
const unsigned int NO_PADDING_NEEDED = 7;
const unsigned int B64_OK = 0;
const unsigned int DOC_MAX_PATH = 4096;

alphabet<char> ukwd_alpha(UKWD_ALPHA_CIRCUMFENCE, 26);

/*! \brief A class implementing a base64 decoder.
 *
 *  Not very efficient but it does the job. Used to decode the glade file in rmsk::get_glade_data(). A valid base64
 *  string consists of 4 character groups called quantums. Each quantum encodes three payload bytes, where each char
 *  in a quantum encodes six bits of payload data. The last quantum can be padded with up to two '=' characters.
 */     
class base64 {
public:      
    /*! \brief Decode method.
     */    
    unsigned int decode(const string& in, string& out);
    
protected:
    /*! \brief Decode unpadded part of a base64 encoded string.
     */
    unsigned int decode_step(const char *data_in, unsigned int len, string& out);

    /*! \brief Decode padded part of a base64 encoded string.
     */
    unsigned int decode_pad(const char *data_in, string& out);
    
    /*! \brief Test b64char if it is a valid base64 character and return its numerical (translated) value which is 
    *          between 0 and 63.
     */    
    inline unsigned int translate_b64_char(unsigned char b64char, unsigned char& translated);
    
    /*! \brief Piece bits together from first and second byte in base64 quantum to get first payload byte.
     */
    inline unsigned char do_dec_first(unsigned char in_byte_1, unsigned char in_byte_2) {return (in_byte_1 << 2) | (in_byte_2 >> 4);}

    /*! \brief Piece bits together from second and third byte in base64 quantum to get second payload byte.
     */
    inline unsigned char do_dec_second(unsigned char in_byte_1, unsigned char in_byte_2) {return (in_byte_1 << 4) | (in_byte_2 >> 2);}
    
    /*! \brief Piece bits together from third and fourth byte in base64 quantum to get third payload byte.
     */    
    inline unsigned char do_dec_third(unsigned char in_byte_1, unsigned char in_byte_2) {return ((in_byte_1 & 3) << 6) | in_byte_2;}
};

/*! The input string has to contain at least two four character quantums so the payload has to be at least 4 bytes long.
 */
unsigned int base64::decode(const string& in, string& out)
{
    out.clear();
    unsigned int result = B64_OK;
    
    if (((in.length() % 4) != 0) || (in.length() < 8))
    {
        result = IN_LENGTH_NOT_DIVISBLE_BY_FOUR;
    }
    else
    {
        result = decode_step(in.c_str(), (unsigned int)(in.length() - 4), out);
        
        if (result == B64_OK)
        {
            if (in[in.length() -1] == '=')
            {
                result = decode_pad(in.c_str() + in.length() - 4, out);
            }
            else
            {
                result = decode_step(in.c_str() + in.length() - 4, 4, out);       
            }
        }
    }
    
    return result;
}

unsigned int base64::translate_b64_char(unsigned char b64char, unsigned char& translated)
{
    unsigned int result = 0;
    
    if ((b64char >= 'A') && (b64char <= 'Z'))
    {
        translated = b64char - 'A';
    }
    else
    {
        if ((b64char >= 'a') && (b64char <= 'z'))
        {
            translated = 26 + (b64char - 'a');
        }
        else
        {
            if ((b64char >= '0') && (b64char <= '9'))
            {
                translated = 52 + (b64char - '0');
            }
            else
            {
                switch(b64char)
                {
                case '+':
                    translated = 62;
                    break;
                case '/':
                    translated = 63;
                    break;
                default:
                    result = ILLEGAL_CHAR;
                    break;
                }
            }
        }
        
    }
    
    return result;
}

unsigned int base64::decode_step(const char *data_in, unsigned int len, string& out)
{
    unsigned int result = 0, count_in, count_out;
    unsigned char quantum[4];
    
    if ((len % 4) != 0)
    {
        return IN_LENGTH_NOT_DIVISBLE_BY_FOUR;
    }
        
    for(count_in = 0, count_out = 0; (count_in < len) && (result == 0); count_in += 4, count_out += 3)
    {
        if (translate_b64_char(*(data_in + count_in), quantum[0]) != 0)
            return ILLEGAL_CHAR;
    
        if (translate_b64_char(*(data_in + count_in + 1), quantum[1]) != 0)
            return ILLEGAL_CHAR;

        if (translate_b64_char(*(data_in + count_in + 2), quantum[2]) != 0)
            return ILLEGAL_CHAR;
        
        if (translate_b64_char(*(data_in + count_in + 3), quantum[3]) != 0)
            return ILLEGAL_CHAR;        
        
        out.push_back(do_dec_first(quantum[0], quantum[1]));
        out.push_back(do_dec_second(quantum[1], quantum[2]));
        out.push_back(do_dec_third(quantum[2], quantum[3]));
    }
        
    return result;
}

unsigned int base64::decode_pad(const char *data_in, string& out)
{
    unsigned int result = 0;
    unsigned char quantum[4];    

    if (translate_b64_char(*(data_in), quantum[0]) != 0)
        return ILLEGAL_CHAR;
    
    if (translate_b64_char(*(data_in + 1), quantum[1]) != 0)
        return ILLEGAL_CHAR;

    
    if((*(data_in + 3)) != '=')
    {
        return PADDING_BAD_FORMAT;
    }
    
    if ((*(data_in + 2)) != '=')
    {
        // 2 bytes in output
        if (translate_b64_char(*(data_in + 2), quantum[2]) != 0)
            return ILLEGAL_CHAR;

        out.push_back(do_dec_first(quantum[0], quantum[1]));
        out.push_back(do_dec_second(quantum[1], quantum[2]));
    }
    else
    {
        // 1 byte in output        
        out.push_back(do_dec_first(quantum[0], quantum[1]));
    }
    
    return result;
}

bool ukw_d_wiring_helper::less_than(const pair<char, char>& l, const pair<char, char>& r)
{
    return l.first < r.first;
}

vector<pair<char, char> > ukw_d_wiring_helper::perm_to_plugs(permutation& perm)
{
    vector<pair<char, char> > result;
    set<pair<unsigned int, unsigned int> > involution;
    set<pair<unsigned int, unsigned int> >::iterator iter;
    pair<unsigned int, unsigned int> fixed_connection(UKWD_FIXED_CONTACT_J, UKWD_FIXED_CONTACT_Y);
    pair<unsigned int, unsigned int> fixed_connection_rev(UKWD_FIXED_CONTACT_Y, UKWD_FIXED_CONTACT_J);
    char f, s;
    vector<pair<char, char> > data_sorted;

    // If any of the tests below fails, return an involution that contains only the fixed connection    
    result.push_back(pair<char, char>('J', 'Y'));
    perm.test_for_involution(involution); // The set involution is cleared if perm is no involution
    
    if ((involution.find(fixed_connection) != involution.end()) || (involution.find(fixed_connection_rev) != involution.end()))
    {
        //  Required connection is included. Good!
        
        // An UKW D requires 13 pairs to be correct.
        if (involution.size() == (rmsk::std_alpha()->get_size() / 2))
        {
            // iterate over the pairs returned by perm.test_for_involution(involution)
            for (iter = involution.begin(); iter != involution.end(); ++iter)
            {
                // transform the unsigned ints from the pair into characterts 
                f = ukwd_alpha.to_val(iter->first);
                s = ukwd_alpha.to_val(iter->second);
                
                // The pairs (f, s) and (s, f) are functionally equivalent. We
                // prefer the form where f < s.
                if (f > s)
                {
                    data_sorted.push_back(pair<char, char>(s, f));
                }
                else
                {
                    data_sorted.push_back(pair<char, char>(f, s));                    
                }
            }
            
            // Normalize sequence of pairs through sorting by the first elements of the pairs
            sort(data_sorted.begin(), data_sorted.end(), less_than);

            result = data_sorted;            
        }
    }
        
    return result;
}

string ukw_d_wiring_helper::GAF_to_BP_wiring(string& gaf_perm)
{
    string result;
    alphabet<char> bp_alpha("azyxwvutsorqpnmlkjihgfedbc", 26);
    
    for (char a : gaf_perm)
    {
        result += bp_alpha.to_val(rmsk::std_alpha()->from_val(a));;
    }
    
    return result;
}
    
string ukw_d_wiring_helper::BP_to_GAF_wiring(string& bp_perm)
{
    string result;
    
    for (char a : bp_perm)
    {
        result += ukwd_alpha.to_val(rmsk::std_alpha()->from_val(a));;
    }
    
    return result;
}

permutation ukw_d_wiring_helper::plugs_to_perm(vector<pair<char, char> >& plugs)
{
    permutation result = ukwd_alpha.make_involution(plugs);
    
    return result;
}

vector<pair<char, char> > ukw_d_wiring_helper::string_to_plugs(string& perm_spec)
{
    vector<pair<char, char> > result;
    unsigned int num_pairs = perm_spec.length() / 2;
    
    for (unsigned int count = 0; count < num_pairs; count++)
    {
        result.push_back(pair<char, char>(perm_spec[2 * count], perm_spec[2 * count + 1]));
    }    
    
    return result;
}


/*! \brief Holds singelton object returned by std_alpha().
 */ 
alphabet<char> *rmsk::classical_alpha = NULL;

/*! \brief Holds singelton object returned by std_uni_alpha().
 */ 
alphabet<gunichar> *rmsk::classical_uni_alpha = NULL;

alphabet<char> *rmsk::std_alpha()
{
    if (classical_alpha == NULL)
    {
        classical_alpha = new alphabet<char>("abcdefghijklmnopqrstuvwxyz", 26);
    }
    
    return classical_alpha;
}

alphabet<gunichar> *rmsk::std_uni_alpha()
{
    if (classical_uni_alpha == NULL)
    {
        vector<gunichar> help;
        Glib::ustring std_uni_chars("abcdefghijklmnopqrstuvwxyz");            

        for (unsigned int count = 0; count < std_uni_chars.length(); count++)
        {
            help.push_back(std_uni_chars[count]);
        }
        
        classical_uni_alpha = new alphabet<gunichar>(help);
    }
    
    return classical_uni_alpha;
}

void rmsk::str_to_bool_vec(const char *str, unsigned int len, vector<unsigned int>& bool_vec)
{
    bool_vec.clear();
    
    for (unsigned int count = 0; count < len; count++)
    {
        if (str[count] == '0')
        {
            bool_vec.push_back(0);
        }
        else
        {
            bool_vec.push_back(1);
        }
    }
}

vector<unsigned int> rmsk::uint_arr_to_vec(unsigned int *arr, unsigned int len)
{
    vector<unsigned int> result;
    
    for(unsigned int count = 0; count < len; count++)
    {
        result.push_back(arr[count]);
    }
    
    return result;
}

/*! The path returned normally is the the path of the directory in which the program binary resides to which /doc is appended.
 *  In case that can not be determined the current directory appended with /doc is returned.
 */ 
string rmsk::get_doc_path()
{
#ifndef RMSK_DOC_PATH
    char file_name[DOC_MAX_PATH + 1];
    string result = Glib::get_current_dir() + "/doc", temp;
    ssize_t chars_read;
    size_t final_slash_pos;
    
    chars_read = readlink("/proc/self/exe", file_name, DOC_MAX_PATH);
    
    if (chars_read > 0)
    {
        file_name[chars_read] = 0;
        temp = string(file_name);
        final_slash_pos = temp.rfind('/');
        
        if (final_slash_pos != string::npos)
        {
            result = temp.substr(0, final_slash_pos) + "/doc";
        }
    }
    
    return result;   
#else
    string doc_path(RMSK_DOC_PATH);
    
    return doc_path;
#endif 
}

string rmsk::get_glade_data()
{
    string result;
    base64 b64;
    
    if (b64.decode(gladedata, result) != B64_OK)
    {
        result.clear();
    }
    
    return result;
}

void rmsk::simple_assert(bool condition, const char *exception_message)
{
    if (condition)
    {
        throw runtime_error(exception_message);
    }
}

rotor_machine *rmsk::make_default_machine(string& machine_name)
{
    rotor_machine *result = NULL;
    
    // Used for Services and M3 Enigma
    if ((machine_name == "Services") || (machine_name == "M3"))
    {
        result = new enigma_I(UKW_C, WALZE_I, WALZE_II, WALZE_III, machine_name == "M3");
    }

    // Used for M4 Enigma
    if ((machine_name == MNAME_M4_ENIGMA) || (machine_name == "M4"))
    {
        result = new enigma_M4(UKW_C_DN, WALZE_BETA, WALZE_I, WALZE_II, WALZE_VII);
    }

    // Used for Railway Enigma
    if ((machine_name == MNAME_RAILWAY_ENIGMA) || (machine_name == "Railway"))
    {
        result = new railway_enigma(WALZE_RB_I, WALZE_RB_II, WALZE_RB_III);
    }

    // Used for Tirpitz Enigma
    if ((machine_name == MNAME_TIRPITZ_ENIGMA) || (machine_name == "Tirpitz"))
    {
        result = new tirpitz_enigma(WALZE_T_I, WALZE_T_II, WALZE_T_III);
    }

    // Used for Abwehr Enigma
    if ((machine_name == MNAME_ABWEHR_ENIGMA) || (machine_name == "Abwehr"))
    {
        result = new abwehr_enigma(WALZE_ABW_I, WALZE_ABW_II, WALZE_ABW_III);
    }

    // Used for KD Enigma
    if ((machine_name == MNAME_KD_ENIGMA) || (machine_name == "KD"))
    {
        result = new kd_enigma(WALZE_KD_I, WALZE_KD_II, WALZE_KD_III);
    }

    // Used for Tpex
    if (machine_name == MNAME_TYPEX)
    {
        result = new typex(TYPEX_SP_02390_UKW, TYPEX_SP_02390_A, TYPEX_SP_02390_B, TYPEX_SP_02390_C, TYPEX_SP_02390_D, TYPEX_SP_02390_E);
    }

    // Used for SIGABA
    if (machine_name == MNAME_SIGABA)
    {
        rotor_id r5(SIGABA_ROTOR_5), r6(SIGABA_ROTOR_6), r7(SIGABA_ROTOR_7, true), r8(SIGABA_ROTOR_8), r9(SIGABA_ROTOR_9);
        rotor_id r0(SIGABA_ROTOR_0), r1(SIGABA_ROTOR_1), r2(SIGABA_ROTOR_2, true), r3(SIGABA_ROTOR_3), r4(SIGABA_ROTOR_4);
        rotor_id i0(SIGABA_INDEX_0), i1(SIGABA_INDEX_1), i2(SIGABA_INDEX_2, true), i3(SIGABA_INDEX_3), i4(SIGABA_INDEX_4);
        vector<rotor_id> all_ids;
        
        // Pack together rotor ids for simple test machine
        all_ids.push_back(r0);
        all_ids.push_back(r1);
        all_ids.push_back(r2);
        all_ids.push_back(r3);
        all_ids.push_back(r4);
        all_ids.push_back(r5);
        all_ids.push_back(r6);
        all_ids.push_back(r7);
        all_ids.push_back(r8);
        all_ids.push_back(r9);
        all_ids.push_back(i0);
        all_ids.push_back(i1);
        all_ids.push_back(i2);
        all_ids.push_back(i3);
        all_ids.push_back(i4);
        
        result = new sigaba(all_ids, false);                    
    }

    // Used for Schluesselgeraet39
    if (machine_name == MNAME_SG39)
    {
        result = new schluesselgeraet39(SG39_ROTOR_0, SG39_ROTOR_1, SG39_ROTOR_2, SG39_ROTOR_3);
    }
    
    // Used for KL7
    if (machine_name == MNAME_KL7)
    {
        rotor_id id_1(KL7_ROTOR_A, KL7_RING_1);
        rotor_id id_2(KL7_ROTOR_B, KL7_RING_2);
        rotor_id id_3(KL7_ROTOR_C, KL7_RING_3);
        rotor_id id_4(KL7_ROTOR_L, KL7_RING_WIDE);    
        rotor_id id_5(KL7_ROTOR_D, KL7_RING_4);
        rotor_id id_6(KL7_ROTOR_E, KL7_RING_5);
        rotor_id id_7(KL7_ROTOR_F, KL7_RING_6);
        rotor_id id_8(KL7_ROTOR_G, KL7_RING_7);        
        
        vector<rotor_id> rotor_spec;
        rotor_spec.push_back(id_1);
        rotor_spec.push_back(id_2);
        rotor_spec.push_back(id_3);        
        rotor_spec.push_back(id_4);
        rotor_spec.push_back(id_5);
        rotor_spec.push_back(id_6);    
        rotor_spec.push_back(id_7);    
        rotor_spec.push_back(id_8); 
        
        result = new kl7(rotor_spec);               
    }
    
    // Used for Nema
    if (machine_name == MNAME_NEMA)
    {
        vector<rotor_assembly> settings_l;        
        
        settings_l.push_back(rotor_assembly(NEMA_DRIVE_WHEEL_13, NEMA_ROTOR_D));
        settings_l.push_back(rotor_assembly(NEMA_DRIVE_WHEEL_12, NEMA_ROTOR_C));
        settings_l.push_back(rotor_assembly(NEMA_DRIVE_WHEEL_15, NEMA_ROTOR_B));
        settings_l.push_back(rotor_assembly(NEMA_DRIVE_WHEEL_14, NEMA_ROTOR_A));    
        
        result = new nema(settings_l, NEMA_DRIVE_WHEEL_22, NEMA_DRIVE_WHEEL_1);
    }    
    
    return result;
}

string rmsk::get_config_name(rotor_machine *machine)
{
    string result;
    enigma_base *enigma_machine = dynamic_cast<enigma_base *>(machine);
    
    if (enigma_machine != NULL)
    {
        // We have an Enigma variant here. Use get_machine_type() to determine model.
        result = enigma_machine->get_machine_type();
    }
    else
    {   
        // Not an Enigma, use get_name()  
        result = machine->get_name();
    }
    
    return result;    
}

rotor_machine *rmsk::restore_from_ini(Glib::KeyFile& machine_state)
{
    rotor_machine *result = NULL;    
    string machine_name = "";
        
    if (machine_state.has_key(MACHINE_SECTION, KEY_MACHINE_NAME))
    {
        // load machine name from ini file
        machine_name = machine_state.get_string(MACHINE_SECTION, KEY_MACHINE_NAME);
        
        if (machine_name == MNAME_ENIGMA_I)
        {
            if (machine_state.has_key(MACHINE_SECTION, KEY_MACHINE_TYPE))
            {
                machine_name = machine_state.get_string(MACHINE_SECTION, KEY_MACHINE_TYPE);
            }
            else
            {
                machine_name = "";
            }
        }
        
        // construct a dummy machine which is then used to load the settings file designated
        // by the parameter file_name        
        result = make_default_machine(machine_name);
        
        if (result != NULL)
        {
            // Use dummy rotor machine object to restore the state given in the ini file
            if (result->load_ini(machine_state))
            {
                // Loading failed. Clean up.
                delete result;
                result = NULL;
            }
        }
    }
    
    return result;
}

rotor_machine *rmsk::restore_from_data(string& machine_state)
{
    rotor_machine *result = NULL;    
    bool ini_load_successful = true;
    Glib::KeyFile ini_file;    
    
    // First construct KeyFile object from data
    try
    {
        ini_load_successful = ini_file.load_from_data(machine_state);        
    }
    catch(...)
    {
        ini_load_successful = false; 
    }    
    
    if (ini_load_successful)
    {
        result = restore_from_ini(ini_file);
    }
    
    return result;
}

rotor_machine *rmsk::restore_from_file(string& file_name)
{
    rotor_machine *result = NULL;    
    bool ini_load_successful = true;
    Glib::KeyFile ini_file;    
    
    // First load ini file
    try
    {
        ini_load_successful = ini_file.load_from_file(file_name);        
    }
    catch(...)
    {
        ini_load_successful = false; 
    }    
    
    if (ini_load_successful)
    {
        result = restore_from_ini(ini_file);
    }
    
    return result;
}

bool rmsk::settings_file_load(string& file_name, sigc::slot<bool, Glib::KeyFile&> processor)
{
    bool result = false;
    Glib::KeyFile ini_file;    

    try
    {
        result = !ini_file.load_from_file(file_name);
        
        if (!result)
        {
            result = processor(ini_file);
        }           
    }
    catch(...)
    {
        result = true; 
    }    
    
    return result;
}

bool rmsk::settings_file_save(string& file_name, sigc::slot<void, Glib::KeyFile&> generator)
{
    bool result = false;
    Glib::KeyFile ini_file;
    Glib::ustring ini_data;

    generator(ini_file);    
    
    ini_data = ini_file.to_data();
    
    try
    {
        Glib::RefPtr<Glib::IOChannel> out_file = Glib::IOChannel::create_from_file(file_name, "w");    
        result = out_file->write(ini_data) != Glib::IO_STATUS_NORMAL;
        out_file->close();
    }
    catch(...)
    {
        result = true;
    }    
    
    return result;    
}

void rmsk::clean_up()
{
    delete classical_alpha;
    delete classical_uni_alpha;
}

