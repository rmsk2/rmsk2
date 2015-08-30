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

#ifndef __enigma_draw_h__
#define __enigma_draw_h__

/*! \file enigma_draw.h
 *  \brief Sort of helper file. Only includes some other includes.
 */ 

#include<map>
#include<base_elements.h>
#include<rotor_window.h>
#include<keyboard_vis.h>
#include<rotor_draw.h>
#include<rotor_visualizer.h>
#include<output_device.h>

/*!

\mainpage The Rotor Machine Simulation Kit

\section abut_sec About

This software aims to provide a framework to implement rotor machine simulators. Cryptographically
accurate sample simulators for several Enigma variants and other rotor machines are included. The
following machines are implemented by the program *enigma*:

    Enigma M4
    Enigma M4 with Schreibmax printer
    Enigma M3
    Wehrmacht or Services Enigma
    Enigma KD
    Abwehr Enigma
    Railway Enigma
    Tirpitz Enigma

And simulators for these machines are implemented by the application *rotorvis*:

    SIGABA (CSP889 and CSP2900 variants)
    Nema
    Typex
    Schlüsselgerät 39
    KL7

Additionally a test program (called *rmsk*) is provided that performs unit tests to verify the accuracy
of the provided simulators. 

For your own experiments a small tool called *randperm* has been implemented which prints a set of (pseudo)random
permutations of different types (permutations, involutions, fixed point free permutations) of the standard
latin alphabet ABCDEFGHIJKLMNOPQRSTUVWXYZ. 

On top of that help files in mallard format (http://projectmallard.org/) are included with the simulators that
desribe how to operate them.

This software was written between 2008 and 2015 between by Martin Grap (rmsk@gmx.de) and is licensed under the
Apache 2.0 license (http://www.apache.org/licenses/LICENSE-2.0).

\section idependencies_sec Dependencies

On a Debian, Raspbian or Ubuntu systems you have to install the following packages in order to be able
to build rmsk and the sample simulators.

    apt-get install scons    
    apt-get install libgtkmm-3.0-dev    
    apt-get install libboost-dev    
    apt-get install libboost-system-dev    
    apt-get install libboost-program-options-dev        
    apt-get install doxygen    
    apt-get install yelp (Raspbian only)

\section layout_sec Directory layout

Rmsk's source code is contained in a directory which is structured as follows:

    ---./                       Source code 
       +---doc                  Directory where documentation is created and maintained
       |    +---enigma          Mallard help files for enigma simulator
       |    +---rotorvis        Mallard help files for rotorvis
       +---doxygen              Directory for documentation extracted by Doxygen
       |    +---html            Source code documentation in HTML format as extracted by Doxygen           
       +---dist                 Directory which contains a usable binary distribution of enigma and rotorvis
            +---doc             Documentation intended for distribution 
                 +---enigma     Mallard help files for enigma simulator
                 +---rotorvis   Mallard help files for rotorvis       

\section buid_extrac_sec Building and extracting documentation

\subsection building_sec Building Enigma and rotorvis

Rmsk uses scons (http://www.scons.org/) as its build tool. After installing the dependencies as mentioned above,
you have to change into the source code directory of rmsk and simply type

    scons
    
in order to build the programs enigma, rotorvis, rmsk and randperm. This is equivalent to executing 'make all'
if I would have used GNU make as a build tool. The analog of 'make clean' is 

    scons -c
    
There is currently no analog for 'make install' and it may not be needed as rotorvis and enigma can be run from
the build directories.

The build information is contained in the file called SConstruct. It describes the targets and their dependencies.
Additionally it allows to configure the build process. Currently there is only one feature that can be configured
and that is where the simulators look for their help files. In SConstruct you find the following section enclosed in
the comment lines '# ---- Build configuration variables ----'

    # Uncomment and modify the following statement if you want to put the help files at a fixed path. 
    # Default behaviour is to use the same directory in which the program binary resides.
    #rmsk_defines['RMSK_DOC_PATH'] = quote_path('/usr/share/doc/rmsk2')

If you uncomment and modify the line 

    rmsk_defines['RMSK_DOC_PATH'] = quote_path('/usr/share/doc/rmsk2') 
    
then you can specify a fixed path which is used by the simulators to determine the loacation of the help files. This is
intended if you prefer to install enigma and rotorvis globally for all users as is the standard in the UNIX-world. 
The default behaviour is to look for a doc/enigma or doc/rotorvis directory in the same directory where the program
binary resides. This allows to use the simulators in a fashion that has come to be known as a 'portable application'
that requires no installation whatsoever.

\subsection extracting_doc Extracting documentation from source code

Doxygen can be used to extract documentation from rsmk's source code. Simply issue the command

    doxygen
    
and point your browser to the file doxygen/html/index.html.

\subsection debug_sec Building a debug version

In order to build a debug version of rmsk use the following command:

    scons debug=1
    
If your previous build did not contain debug information you also should clear all the created binaries by executing

    scons -c
    
before rebuilding the project. The same is true when you switch from a debug build to a release build.

\subsection dist_sec Distributable files

While building the project scons automatically copies the specified output files into the dist subdirectory from where 
they can be tar'ed and gzip'ed in order to create a binary distributable. Use

    scons install
    
to initiate that step manually. Executing scons -c also clears the dist directory.

\subsection architecture_sec The big picture

The source of rmsk is organized in three subsystems: crypto, GUI and application. The crypto subsystem contains the "naked"
simulators and cryptographic primitives that are needed to implement them. Here all of the cryptography is happening.

On top of that sits the GUI subsystem. This system implements the visualization of the rotor machines. I.e. it deals
mostly with drawing all the elements like the keyboard, the lamp board, the rotor windows and so on.

Finally whe have the application subsystem that implements all the functionality that is needed to embed the GUI simulator
in a full fledged gtkmm (http://www.gtkmm.org) application. It provides the user interface that allows the user to
configure the underlying rotor machines, saving and restoring the state of the simlator and so on.

In the source documentation that can be exctracted by doxygen (http://www.stack.nl/~dimitri/doxygen/) classes belonging
to the GUI and application subsystems are marked accordingly "... a GUI/application class ...". Classes that are not
specially marked belong to the crypto subsystem.

\section missing_sec Stuff that remains to be done

Currently the simulators only provide a user interface in English therefore some future version has to be properly
internationalized.
*/

#endif /* __enigma_draw_h__ */ 

