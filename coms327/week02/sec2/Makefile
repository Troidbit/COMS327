#target: list of dependencies
#	command to build target from dependencies

all: hello times_table hello.class

hello: hello.c
	gcc hello.c -o hello -Wall -Werror

times_table: times_table.c
	gcc times_table.c -o times_table -Wall -Werror

hello.class: hello.java
	javac hello.java

clean:
	rm -f hello hello.class times_table *~ *.o core
