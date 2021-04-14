EXE			= TotalSMS

SRC			= ./src

# Main source file.
SOURCES 	= src/main.c

# core
SOURCES		+= src/sms.c src/cpu.c

CFLAGS 		= -Wall -Wformat $(RELEASE)

OBJS		= $(addsuffix .o, $(basename $(notdir $(SOURCES))))


##---------------------------------------------------------------------
## BUILD RULES
##---------------------------------------------------------------------
%.o:$(SRC)/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

all: $(EXE)
	@echo Build complete for $(EXE)
	# run the exe after build
	./$(EXE)

$(EXE): $(OBJS)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

clean:
	rm -f $(EXE) $(OBJS)

# for some reason sublime cannot see the run function here...
run: all
	./$(EXE)