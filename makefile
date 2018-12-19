default: secondary.o utilities.o primary.o introduceerror.o
	gcc -o sender sender.o primary.o utilities.o ccitt16.o introduceerror.o
	gcc -o receiver receiver.o secondary.o utilities.o ccitt16.o
