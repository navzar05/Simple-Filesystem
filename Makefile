main: src/library/main.o lib/libdisk.a
	g++ -static src/library/main.o -o bin/main -L./lib -ldisk

src/library/disk_driver.o: src/library/disk_driver.cpp
	g++ -Wall -c -I/includes -o src/library/disk_driver.o src/library/disk_driver.cpp

lib/libdisk.a: src/library/disk_driver.o
	ar rcs lib/libdisk.a src/library/disk_driver.o

main.o: src/library/main.cpp
	g++ -Wall -c -I/includes -o src/library/main.o src/library/main.cpp

.PHONY: clean

clean:
	rm src/library/*.o lib/* bin/*