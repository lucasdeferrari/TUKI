.DEFAULT_GOAL := all

SUBPROJECTS := ../Memoria/Default ../CPU/Default ../FileSystem/Default ../Kernel/Default ../Consola/Default

all: Memoria CPU FileSystem Kernel Consola

Memoria: 
	 $(MAKE) -C ../Memoria/Default all
CPU: 
	$(MAKE) -C ../CPU/Default all
FileSystem: 
	$(MAKE) -C ../FileSystem/Default all
Kernel: 
	$(MAKE) -C ../Kernel/Default all
Consola:
	$(MAKE) -C ../Consola/Default all

clean:
	for dir in $(SUBPROJECTS); do \
		$(MAKE) -C $$dir clean; \
	done