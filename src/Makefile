CC      = gcc
PROF    = 
NOCRYPT =
C_FLAGS = -O0 -g -Wall -Dunix -arch x86_64 $(PROF) $(NOCRYPT)
L_FLAGS = -O0 $(PROF)

O_FILES = act_comm.o act_info.o act_move.o act_obj.o act_wiz.o comm.o const.o \
          db.o db2.o fight.o handler.o healer.o interp.o magic.o save.o \
          skills.o special.o update.o quest.o board.o

blood: $(O_FILES)
	rm -f blood
	$(CC) $(L_FLAGS) -o blood $(O_FILES)

.c.o: merc.h
	$(CC) -c -g $(C_FLAGS) $<
