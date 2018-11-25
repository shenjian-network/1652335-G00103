CC=g++
Target = client
.PHONY: all clean 
HEADFILE= utils.h
INCLUDEFILE= utils.cpp
all:$(Target)
$(Target):%:%.cpp $(HEADFILE) $(INCLUDEFILE)
	$(CC) -o $@ $< $(HEADFILE) $(INCLUDEFILE)
clean:
	-rm -rf $(Target) *.txt
