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
accurate simulators for several Enigma variants and other rotor machines are included. The
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

For longer messages or for experiments that require a level of automation or scriptability generic command line tools (rotorsim for message processing and rotorstate for generating and saving rotor machine states) for all machines implemented by rotorvis and enigma are part of the package.

If you want to run your own encrypted communications network with your friends rmsk2 also contains a key sheet generator for all simulated machines which allows you to generate text or HTML files that specify random machine settings.

A Python3 interface to the C++ implementation of all the rotor machines is also provided. This Python3 interface has been used to implement the key sheet generator.

Finally help files in mallard format (http://projectmallard.org/) are included with the simulators that describe how to operate them. Documentation for the command line simulators and the key sheet generator is provided through this Wiki.

This software was written between 2008 and 2016 between by Martin Grap (rmsk2@gmx.de) and is licensed under the Apache 2.0 license (http://www.apache.org/licenses/LICENSE-2.0).

You find more information and some documentation on the GitHub Wiki of this project: https://github.com/rmsk2/rmsk2/wiki

\section idependencies_sec Dependencies

On a clean Debian, Raspbian or Ubuntu installation you have to additionally install the following packages in order to be able to build rmsk2 and the simulators.

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
       +---doc                  Directory where documentation is created and 
       |    |                   maintained
       |    +---enigma          Mallard help files for enigma simulator
       |    +---rotorvis        Mallard help files for rotorvis       
       +---doxygen              Directory for documentation extracted by Doxygen
       |    +---html            Source code documentation in HTML format as  
       |                        extracted by Doxygen 
       +---pyrmsk2              Directory where the modules making up the Python3
       |                        interface are created and maintained          
       +---dist                 Directory which contains a usable binary  
            |                   distribution of enigma and rotorvis
            +---pyrmsk2         Modules making up the Python3 interface
            +---doc             Documentation intended for distribution 
                 +---enigma     Mallard help files for enigma simulator
                 +---rotorvis   Mallard help files for rotorvis       


\section buid_extrac_sec Building and extracting documentation

\subsection building_sec Building Enigma and rotorvis

Rmsk2 uses scons (http://www.scons.org/) as its build tool. After installing the dependencies as mentioned above, you have to change into the source code directory of rmsk and simply type

    scons

in order to build the programs enigma, rotorvis (and others). This is equivalent to executing make all if GNU make would have been used as a build tool. The analogue of make clean is

    scons -c

There is currently no analogue for make install and it may not be needed as rotorvis and enigma can be run from the build or dist directories.

The build information is contained in the file called SConstruct. It describes the targets and their dependencies. Additionally it allows to configure the build process. Currently there are several features that can be configured.

The first is where the simulators look for their help files. In SConstruct you find the following section enclosed in the comment lines # -— Build configuration variables -—

    # Uncomment and modify the following statement if you want to 
    # put the help files at a fixed path. Default behaviour is to 
    # use the same directory in which the program binary resides.
    #rmsk_defines['RMSK_DOC_PATH'] = quote_path('/usr/share/doc/rmsk2')

If you uncomment and modify the line

    rmsk_defines['RMSK_DOC_PATH'] = quote_path('/usr/share/doc/rmsk2') 

then you can specify a fixed path which is used by the simulators to determine the location of the help files. This is intended if you prefer to install enigma and rotorvis globally for all users as is the standard in the UNIX-world. The default behaviour is to look for a doc/enigma or doc/rotorvis directory in the same directory where the program binary resides. This allows to use the simulators in a fashion that has come to be known as a 'portable application' that requires no installation whatsoever.

The second feature deals with the simulated properties of the Schlüsselgerät 39 (SG39) machine. Pictures of the SG39 show that its keyboard has a space key but it remains unknown whether this was used during normal operation or only in combination with a teleprinter.

    # Build SG39 with an asymmetric keyboard/printing device using 'Q' as space
    #rmsk_defines['SG39_ASYMMETRIC'] = 'asym'

Comment out the previous line in SConstruct in order to simulate a functional space key when building the SG39 simulator. In this case it is assumed that the letter Q was used as a stand in for the blank character when encrypting a message. Additional build configuration variables:

    # Include Enigma variants in rotorvis
    #rmsk_defines['INCLUDE_ENIGMA'] = 'enigma'

Uncomment the previous line in order to include three Enigma variants (M3, M4 and Services) in rotorvis.

    # Build Typex with a dual printer
    #rmsk_defines['TYPEX_DUAL_PRINTER'] = 'typexdual'

Uncomment the previous line to build the graphical Typex simulator with a dual printer.

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

*/

#endif /* __enigma_draw_h__ */ 

