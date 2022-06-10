RC=rum

build:	main.rum bridge.c
	$(RC) main.rum -o main

test:	main
	./main

.PHONY: clean
clean:
	rm -f *.o main
	rm -f bridge.rum
