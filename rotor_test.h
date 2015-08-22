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

#ifndef __test_rotor_h__
#define __test_rotor_h__

/*! \file rotor_test.h
 *  \brief This file contains the header for a class that implements unit tests for the
 *         rotor class.
 */ 

#include<simple_test.h>

using namespace std;


/*! \brief Namespace for simple tests of the ::rotor class
 */   
namespace test_rotor {

/*! \brief Function that registers the tests contained in this namespace with a parent
 *         composite_test_case.
 */   
void register_tests(composite_test_case *container);

}

#endif /* __test_rotor_h__ */

