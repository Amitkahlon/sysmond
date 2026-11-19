CC = gcc
CFLAGS = -Wall -g
HEADERS = src/config.h src/sysmon_ipc.h

all: bin/sysmond bin/mem_sensor bin/cpu_sensor bin/proc_sensor

bin:
	mkdir -p bin

bin/sysmond: src/sysmond.c $(HEADERS) | bin
	$(CC) $(CFLAGS) -o bin/sysmond src/sysmond.c

bin/mem_sensor: src/mem_sensor.c $(HEADERS) | bin
	$(CC) $(CFLAGS) -o bin/mem_sensor src/mem_sensor.c

bin/cpu_sensor: src/cpu_sensor.c $(HEADERS) | bin
	$(CC) $(CFLAGS) -o bin/cpu_sensor src/cpu_sensor.c

bin/proc_sensor: src/proc_sensor.c $(HEADERS) | bin
	$(CC) $(CFLAGS) -o bin/proc_sensor src/proc_sensor.c

clean:
	rm -rf bin/*

.PHONY: all clean
