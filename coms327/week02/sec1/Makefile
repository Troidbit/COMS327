#target: dependency list
#	command(s) to build target from dependencies

all: hello times_table hello.class

hello: hello.o
	gcc hello.o -o hello

hello.o: hello.c
	gcc -c hello.c -Wall -Werror

times_table: times_table.c
	gcc times_table.c -o times_table -Wall -Werror

hello.class: hello.java
	javac hello.java

clean:
	rm -f *~ hello hello.class times_table core 
