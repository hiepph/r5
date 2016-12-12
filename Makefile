CC = g++
CFLAGS = -Wall -g
LIBS = `pkg-config --cflags --libs opencv`
SCRS = center.cpp
MAIN = center

.PHONY: depend clean

all: $(MAIN)

$(MAIN): $(OBJS)
	$(CC) $(CFLAGS) $(LIBS) -o $(MAIN) $(SCRS)

clean:
	$(RM) *.o *~ $(MAIN)
