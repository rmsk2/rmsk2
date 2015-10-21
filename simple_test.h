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

#ifndef __test_h__
#define __test_h__

/*! \file simple_test.h
 *
 *  \brief Header file for classes that provide a primitive unit testing facility.
 *
 */ 

#include<string>
#include<vector>

using namespace std;

/*! \brief A class that abstracts the notion of a unit test. 
 *
 *  A unit test has a name and can be annotated with strings called notes. The notes are intended
 *  to communicate information about the test results to the user. The main method is test(). It
 *  is to be reimplemented by the children of this class and contains the actual test that is to
 *  be performed. If the test() method returns true, the test is assumed to have been executed
 *  successfully.
 *
 *  Why did I implement my own unit testing classes? Honestly, it seemed not appropriate to create
 *  a dependency to a more complex unit testing library just to satisfy the extremely modest needs
 *  of this project.
 */   
class test_case {
public:

    /*! \brief Constructs a test_case. The parameter n is supposed to specifiy the name of the test. 
     */
    test_case(const string& n) { name = n; }

    /*! \brief Constructs a test_case. The parameter n is supposed to specifiy the name of the test
     *         in form of a C-style zero terminated string. 
     */
    test_case(const char *n) { name = string(n); }
    
    /*! \brief The method that actually executes the test. Has to be reimplemented in the children
     *         of this class. The expected semantic is to return true if the test was successfull.
     */    
    virtual bool test() { my_notes.clear(); return true; }

    /*! \brief Appends a note to this test_case.
     */    
    virtual void append_note(const string& new_note) { my_notes.push_back(new_note); }

    /*! \brief Appends a note to this test_case, where the note itself has to be specified by a
     *         C-style zero terminated string.
     */    
    virtual void append_note(const char *new_note) { string temp(new_note); my_notes.push_back(temp); }
    
    /*! \brief Returns all the notes that have been appended to this test_case.
     */        
    virtual vector<string>& get_notes() { return my_notes; }
    
    /*! \brief Returns the name of this test_case.
     */        
    virtual string get_name() { return name; }
    
    /*! \brief Prints all the notes that have been appended to this test_case to stdout.
     */            
    virtual void print_notes();
    
    /*! \brief Destructor.
     */            
    virtual ~test_case() { ; }

protected:

    /*! \brief Holds the name of this test_case.
     */        
    string name;
    
    /*! \brief Holds the notes that have been appended to this test_case.
     */            
    vector<string> my_notes;
    
};

/*! \brief A class that implements a special test_case that is made up of a collection of other 
 *         test cases.
 */   
class composite_test_case : public test_case {
public:
    /*! \brief Constructs a composite_test_case. The parameter name specifies the name of the test. 
     */
    composite_test_case(const string& name) : test_case(name) { ; }

    /*! \brief Constructs a composite_test_case. The parameter name specifies the name of the test
     *         in form of a C-style zero terminated string.
     */
    composite_test_case(const char *name) : test_case(name) { ; }
    
    /*! \brief Adds a pointer to a test_case that should be executed when test() is called.
     */                
    virtual void add(test_case *test);
    
    /*! \brief Executes all the test cases that have been added to this composite_test_case. If one
     *         of these test cases fails (i.e. returns false) then the test cases following the failed
     *         test case are not executed.
     */                    
    virtual bool test();

    /*! \brief Destructor. Does not delete the test_cases that have been added.
     */            
    virtual ~composite_test_case() { ; }

protected:

    /*! \brief Appends all the notes given in the vector notes_to_merge to the notes of this
     *         composite_test_case.
     */        
    void merge_notes(vector<string>& notes_to_merge);

    /*! \brief Holds the collection of sub test cases that are to be executed.
     */        
    vector<test_case *> test_cases;    
};


#endif /* __test_h__ */

