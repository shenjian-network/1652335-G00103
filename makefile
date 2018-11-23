CC=g++
Target = client
.PHONY: all clean 
HEADFILE= parser.h
INCLUDEFILE=parser.cpp
all:$(Target)
$(Target):%:%.cpp $(HEADFILE) $(INCLUDEFILE)
	$(CC) -o $@ $< $(HEADFILE) $(INCLUDEFILE)
clean:
	-rm -rf $(Target) *.txt

