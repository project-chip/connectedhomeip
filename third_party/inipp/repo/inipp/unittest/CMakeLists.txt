add_executable(headertest headertest.cpp)
add_executable(unittest unittest.cpp)
add_test(test0 headertest)
add_test(test1 unittest)
