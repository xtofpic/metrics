

all: tests


tests:	tests.cpp
		clang++ -std=c++17 tests.cpp -o tests

clean:
	rm -f tests
	

	
