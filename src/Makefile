BINDIR = ../bin
BIN = $(BINDIR)/base64
TESTBIN = $(BINDIR)/test-base64
MAIN_SOURCES = main.cpp Base64.cpp
TEST_SOURCES = Base64Test.cpp Base64FileTest.cpp Base64.cpp

all: main test

main:
	$(CXX) $(MAIN_SOURCES) -Wall -o $(BIN)
	
test:
	$(CXX) $(TEST_SOURCES) -Wall -o $(TESTBIN)
	
clean:
	$(RM) $(BIN) $(TESTBIN)
