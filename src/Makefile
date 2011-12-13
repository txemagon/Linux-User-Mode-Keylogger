keylog: main.o keylog.o mapper.o logger.o kb_device.o 
	g++ $(CFLAGS) -o keylog logger.o mapper.o kb_device.o keylog.o main.o

keylog.o: keylog.cpp
	gcc $(CFLAGS) -c keylog.cpp

logger.o: logger.cpp logger.h
	gcc $(CFLAGS) -c logger.cpp

mapper.o: mapper.cpp mapper.h
	gcc $(CFLAGS) -c mapper.cpp

kb_device.o: kb_device.cpp kb_device.h
	gcc $(CFLAGS) -c kb_device.cpp

main.o: main.cpp
	gcc $(CFLAGS) -c main.cpp


clean:
	rm -f *.o keylog
