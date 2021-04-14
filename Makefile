EXE			= TotalSMS

ROM_PATH	= "roms/Sonic The Hedgehog (USA, Europe).sms"
# ROM_PATH	= "roms/Altered Beast (USA, Europe).sms"

SRC			= ./src

# Main source file.
SOURCES 	= src/main.c

# core
SOURCES		+= src/sms.c src/cpu.c

CFLAGS 		= -Wall -Wextra $(RELEASE) -DSMS_DEBUG

OBJS		= $(addsuffix .o, $(basename $(notdir $(SOURCES))))

LIBS		= -lz

##---------------------------------------------------------------------
## BUILD RULES
##---------------------------------------------------------------------
%.o:$(SRC)/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

all: $(EXE)
	./$(EXE) $(ROM_PATH)

$(EXE): $(OBJS)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

clean:
	rm -f $(EXE) $(OBJS)

# for some reason sublime cannot see the run function here...
run: all
	./$(EXE)