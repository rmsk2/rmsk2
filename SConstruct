def quote_path(path):
    return "'\"" + path + "\"'"

if ARGUMENTS.get('debug', 0):
    rmsk = Environment(CCFLAGS = '-Wall -g -std=c++11 -pedantic -I./')
else:
    rmsk = Environment(CCFLAGS = '-Wall -O2 -std=c++11 -pedantic -I./')

rmsk.ParseConfig('pkg-config gtkmm-3.0 --cflags --libs')

# Custom Builders to create autogenerated code
# Generate base64 encoded glade file from rotor_dialog_2.ui
Gladeheader = Builder(action = 'python3 make_glade_header.py $SOURCE $TARGET')
rmsk['BUILDERS']['Gladeheader'] = Gladeheader

# Generate enigma_rotor_set.h and enigma_rotor_set.cpp by running enigrotorset.py
Enigmarotorset = Builder(action = 'python3 enigrotorset.py')
rmsk['BUILDERS']['Enigmarotorset'] = Enigmarotorset

# ---- Build configuration variables ----

rmsk_defines = {}

# Uncomment and modify the following statement if you want to put the help files at a fixed path. 
# Default behaviour is to use the same directory in which the program binary resides.
#rmsk_defines['RMSK_DOC_PATH'] = quote_path('/usr/share/doc/rmsk2')

# Build SG39 with an asymmetric keyboard/printing device using 'Q' as space
rmsk_defines['SG39_ASYMMETRIC'] = 'asym'

# ---- Build configuration variables ----

rmsk_lib_path =  ['./'] 
rmsk_libs = rmsk['LIBS'] + ['specmachines', 'application_base', 'rmsk_base', 'base_visual', 'specmachines', 'boost_system', 'boost_program_options', 'libgdkmm-3.0.so', 'libpangomm-1.4.so', 'libcairomm-1.0.so', 'libglib-2.0.so', 'libglibmm-2.4.so', 'libatkmm-1.6.so', 'libsigc-2.0.so', 'libgiomm-2.4.so']
enigma_includes = rmsk['CPPPATH']

base_files = ['permutation.cpp', 'rand_gen.cpp', 'rotor.cpp', 'simple_mod_int.cpp', 'stepping.cpp']
base_files += ['rotor_machine.cpp', 'enigma_sim.cpp', 'rmsk_globals.cpp', 'rotor_ring.cpp', 'typex.cpp']
base_files += ['nema.cpp', 'rotor_set.cpp', 'enigma_uhr.cpp']
base_files += ['printing_device.cpp', 'rotor_keyboard.cpp', 'kl7.cpp', 'machine_config.cpp']

base_visual_files = ['display_dialog.cpp', 'base_elements.cpp', 'rotor_window.cpp', 'keyboard_vis.cpp', 'selection_dialog.cpp']
base_visual_files += ['rotor_draw.cpp', 'output_device.cpp', 'rotor_visualizer.cpp', 'special_rotor_visualizer.cpp']

rmsk_files = ['test.cpp', 'simple_test.cpp', 'rotor_test.cpp', 'machine_test.cpp', 'enigma_test.cpp', 'typex_test.cpp']
rmsk_files += ['sigaba_test.cpp', 'nema_test.cpp', 'stepping_test.cpp', 'sg39_test.cpp', 'decipherment_test.cpp', 'kl7_test.cpp']

specmachines_files = ['sigaba.cpp', 'sg39.cpp', 'configurator.cpp'];

application_base_files = ['simple_state.cpp', 'app_helpers.cpp']

enigma_files = ['enigma_app_window.cpp', 'enigma_app.cpp', 'rotor_dialog.cpp', 'plugboard_dialog.cpp']
enigma_files += ['ukwd_wiring_dialog.cpp']

rotor_vis_files = ['rotorvis.cpp', 'configurator_dialog.cpp']

perm_files = ['rand_perm.cpp']

rotor_sim_files = ['rotor_sim.cpp']

tlv_object_files = ['tlv_object.cpp', 'tlv_stream.cpp', 'tlv_server.cpp', 'object_registry.cpp', 'arith_test.cpp']

hdr = rmsk.Gladeheader('glade_data.h', 'rotor_dialog_2.ui')
rotor_set = rmsk.Enigmarotorset(['enigma_rotor_set.cpp', 'enigma_rotor_set.h'], 'enigrotorset.py')
base_lib = rmsk.Library('rmsk_base', base_files, CPPDEFINES = rmsk_defines)
spec_lib = rmsk.Library('specmachines', specmachines_files, CPPDEFINES = rmsk_defines)
visual_lib = rmsk.Library('base_visual', base_visual_files, CPPPATH = enigma_includes, CPPDEFINES = rmsk_defines)
app_lib = rmsk.Library('application_base', application_base_files, CPPPATH = enigma_includes, CPPDEFINES = rmsk_defines)
rmsk_prog = rmsk.Program('rmsk', rmsk_files, LIBS = rmsk_libs, LIBPATH = rmsk_lib_path, CPPDEFINES = rmsk_defines)
enigma_prog = rmsk.Program('enigma', enigma_files, LIBS = rmsk_libs, LIBPATH = rmsk_lib_path, CPPPATH = enigma_includes, CPPDEFINES = rmsk_defines)
rand_perm_prog = rmsk.Program('rand_perm', perm_files, LIBS = rmsk_libs, LIBPATH = rmsk_lib_path, CPPDEFINES = rmsk_defines)
rotorvis_prog = rmsk.Program('rotorvis', rotor_vis_files, LIBS = rmsk_libs, LIBPATH = rmsk_lib_path, CPPPATH = enigma_includes, CPPDEFINES = rmsk_defines)
rotor_sim_prog = rmsk.Program('rotorsim', rotor_sim_files, LIBS = rmsk_libs, LIBPATH = rmsk_lib_path, CPPDEFINES = rmsk_defines)
tlv_object_server = rmsk.Program('tlv_rotorsim', tlv_object_files, LIBS = rmsk_libs, LIBPATH = rmsk_lib_path, CPPDEFINES = rmsk_defines)

rmsk.Install('dist', rotorvis_prog)
rmsk.Install('dist', enigma_prog)
rmsk.Install('dist', rotor_sim_prog)
rmsk.Install('dist', tlv_object_server)
rmsk.Install('dist/doc/enigma', Glob('doc/enigma/*.page'))
rmsk.Install('dist/doc/enigma', Glob('doc/enigma/*.png'))
rmsk.Install('dist/doc/rotorvis', Glob('doc/rotorvis/*.page'))
rmsk.Install('dist/doc/rotorvis', Glob('doc/rotorvis/*.png'))
rmsk.Install('dist', Glob('rotorsim.py'))
rmsk.Install('dist', Glob('tlvobject.py'))
rmsk.Install('dist', Glob('enigrotorset.py'))
rmsk.Install('dist', Glob('rotorsetdata.py'))
rmsk.Install('dist', Glob('rotorrandom.py'))
Alias('install', 'dist')

