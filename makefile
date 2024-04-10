all: 
	gcc  mmu.c -o mmu
	gcc  master.c -o master
	gcc  scheduler.c -o scheduler
	gcc  process.c -o process
	./master

clean:
	ipcrm -a
	rm master mmu scheduler process result.txt