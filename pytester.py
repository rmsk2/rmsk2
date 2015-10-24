import tlvtest
import rotorsimtest
import cmdlinetest
import simpletest

def perform_all_tests():
    all_tests = simpletest.CompositeTest("All Tests")
    all_tests.add(tlvtest.get_module_test())
    all_tests.add(rotorsimtest.get_module_test())    
    all_tests.add(cmdlinetest.get_module_test())    
    all_tests.execute()


perform_all_tests()


