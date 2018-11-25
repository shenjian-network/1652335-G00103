CC=g++
Target = server client 
.PHONY: all clean 
HEADFILE= server.h utils.h clientModel.h client.h
INCLUDEFILE= utils.cpp clientModel.cpp
all:$(Target) create
$(Target):%:%.cpp $(HEADFILE) $(INCLUDEFILE)
	$(CC) -o $@ $< $(HEADFILE) $(INCLUDEFILE)
clean:
	-rm -rf $(Target) txt/* txt
create:
	mkdir txt