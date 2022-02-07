
all: main

#bl_command.o
bl_commands.o: bl_commands.c bl_commands.h
	gcc -c -o bl_commands.o bl_commands.c 

#uart.o
uart.o: uart.c uart.h
	gcc -c -o uart.o uart.c 

#main.o
main.o: main.c 
	gcc -c -o main.o main.c  

ARCH:= $(shell arch) # invokes terminal to execute arch cmd and assign to ARCH
OS	= $(shell uname)
#main executable
main: main.o uart.o bl_commands.o 
	gcc -o $@ $^ 
	@echo ' '
	@echo 'Finished Build on '
	@echo '  ARCH : $(ARCH) '
	@echo '    OS : $(OS) '
	
.PHONY: clean
clean:
	rm -rf main.out main.o uart.o bl_commands.o 
	@echo ' '
	@echo 'Clean Finished'