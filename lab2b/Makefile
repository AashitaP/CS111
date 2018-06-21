#NAME: Aashita Patwari
#EMAIL: harshupatwari@gmail.com
 
default: build

build: 
	gcc -Wall -Wextra SortedList.c lab2_list.c -o lab2_list -lpthread

profile: clean build 
	rm -f ./raw.gperf 
	LD_PRELOAD=/usr/lib64/libprofiler.so CPUPROFILE=./raw.gperf ./lab2_list --threads=12 --iterations=1000 --sync=s
	pprof --text ./lab2_list ./raw.gperf > profile.out
	pprof --list=insertAndDelete ./lab2_list ./raw.gperf >> profile.out
	rm -f ./raw.gperf 


graphs: tests
	gnuplot lab2b_list.gp

tests: clean build

	./lab2_list --threads=1 --iterations=1000 --sync=m >> lab2b_list.csv
	./lab2_list --threads=2 --iterations=1000 --sync=m >> lab2b_list.csv
	./lab2_list --threads=4 --iterations=1000 --sync=m >> lab2b_list.csv
	./lab2_list --threads=8 --iterations=1000 --sync=m >> lab2b_list.csv	
	./lab2_list --threads=12 --iterations=1000 --sync=m >> lab2b_list.csv
	./lab2_list --threads=16 --iterations=1000 --sync=m >> lab2b_list.csv
	./lab2_list --threads=24 --iterations=1000 --sync=m >> lab2b_list.csv
	./lab2_list --threads=1 --iterations=1000 --sync=s >> lab2b_list.csv
	./lab2_list --threads=2 --iterations=1000 --sync=s >> lab2b_list.csv
	./lab2_list --threads=4 --iterations=1000 --sync=s >> lab2b_list.csv
	./lab2_list --threads=8 --iterations=1000 --sync=s >> lab2b_list.csv	
	./lab2_list --threads=12 --iterations=1000 --sync=s >> lab2b_list.csv
	./lab2_list --threads=16 --iterations=1000 --sync=s >> lab2b_list.csv
	./lab2_list --threads=24 --iterations=1000 --sync=s >> lab2b_list.csv


	-./lab2_list --threads=1 --iterations=1 --yield=id --lists=4 >> lab2b_list.csv
	-./lab2_list --threads=2 --iterations=1 --yield=id --lists=4 >> lab2b_list.csv
	-./lab2_list --threads=4 --iterations=1 --yield=id --lists=4 >> lab2b_list.csv
	-./lab2_list --threads=8 --iterations=1 --yield=id  --lists=4 >> lab2b_list.csv
	-./lab2_list --threads=12 --iterations=1 --yield=id  --lists=4 >> lab2b_list.csv
	-./lab2_list --threads=16 --iterations=1 --yield=id --lists=4 >> lab2b_list.csv

	-./lab2_list --threads=1 --iterations=2 --yield=id --lists=4 >> lab2b_list.csv
	-./lab2_list --threads=2 --iterations=2 --yield=id --lists=4 >> lab2b_list.csv
	-./lab2_list --threads=4 --iterations=2 --yield=id --lists=4 >> lab2b_list.csv
	-./lab2_list --threads=8 --iterations=2 --yield=id  --lists=4 >> lab2b_list.csv
	-./lab2_list --threads=12 --iterations=2 --yield=id  --lists=4 >> lab2b_list.csv
	-./lab2_list --threads=16 --iterations=2 --yield=id --lists=4 >> lab2b_list.csv

	-./lab2_list --threads=1 --iterations=4 --yield=id --lists=4 >> lab2b_list.csv
	-./lab2_list --threads=2 --iterations=4 --yield=id --lists=4 >> lab2b_list.csv
	-./lab2_list --threads=4 --iterations=4 --yield=id --lists=4 >> lab2b_list.csv
	-./lab2_list --threads=8 --iterations=4 --yield=id  --lists=4 >> lab2b_list.csv
	-./lab2_list --threads=12 --iterations=4 --yield=id  --lists=4 >> lab2b_list.csv
	-./lab2_list --threads=16 --iterations=4 --yield=id --lists=4 >> lab2b_list.csv

	-./lab2_list --threads=1 --iterations=8 --yield=id --lists=4 >> lab2b_list.csv
	-./lab2_list --threads=2 --iterations=8 --yield=id --lists=4 >> lab2b_list.csv
	-./lab2_list --threads=4 --iterations=8 --yield=id --lists=4 >> lab2b_list.csv
	-./lab2_list --threads=8 --iterations=8 --yield=id  --lists=4 >> lab2b_list.csv
	-./lab2_list --threads=12 --iterations=8 --yield=id  --lists=4 >> lab2b_list.csv
	-./lab2_list --threads=16 --iterations=8 --yield=id --lists=4 >> lab2b_list.csv

	-./lab2_list --threads=1 --iterations=16 --yield=id --lists=4 >> lab2b_list.csv
	-./lab2_list --threads=2 --iterations=16 --yield=id --lists=4 >> lab2b_list.csv
	-./lab2_list --threads=4 --iterations=16 --yield=id --lists=4 >> lab2b_list.csv
	-./lab2_list --threads=8 --iterations=16 --yield=id  --lists=4 >> lab2b_list.csv
	-./lab2_list --threads=12 --iterations=16 --yield=id  --lists=4 >> lab2b_list.csv
	-./lab2_list --threads=16 --iterations=16 --yield=id --lists=4 >> lab2b_list.csv

	./lab2_list --threads=1 --iterations=10 --yield=id --lists=4 --sync=s >> lab2b_list.csv
	./lab2_list --threads=2 --iterations=10 --yield=id --lists=4 --sync=s >> lab2b_list.csv
	./lab2_list --threads=4 --iterations=10 --yield=id --lists=4 --sync=s >> lab2b_list.csv
	./lab2_list --threads=8 --iterations=10 --yield=id  --lists=4 --sync=s >> lab2b_list.csv
	./lab2_list --threads=12 --iterations=10 --yield=id  --lists=4 --sync=s >> lab2b_list.csv
	./lab2_list --threads=16 --iterations=10 --yield=id --lists=4 --sync=s >> lab2b_list.csv

	./lab2_list --threads=1 --iterations=20 --yield=id --lists=4 --sync=s >> lab2b_list.csv
	./lab2_list --threads=2 --iterations=20 --yield=id --lists=4 --sync=s >> lab2b_list.csv
	./lab2_list --threads=4 --iterations=20 --yield=id --lists=4 --sync=s >> lab2b_list.csv
	./lab2_list --threads=8 --iterations=20 --yield=id  --lists=4 --sync=s >> lab2b_list.csv
	./lab2_list --threads=12 --iterations=20 --yield=id  --lists=4 --sync=s >> lab2b_list.csv
	./lab2_list --threads=16 --iterations=20 --yield=id --lists=4 --sync=s >> lab2b_list.csv

	./lab2_list --threads=1 --iterations=40 --yield=id --lists=4 --sync=s >> lab2b_list.csv
	./lab2_list --threads=2 --iterations=40 --yield=id --lists=4 --sync=s >> lab2b_list.csv
	./lab2_list --threads=4 --iterations=40 --yield=id --lists=4 --sync=s >> lab2b_list.csv
	./lab2_list --threads=8 --iterations=40 --yield=id  --lists=4 --sync=s >> lab2b_list.csv
	./lab2_list --threads=12 --iterations=40 --yield=id  --lists=4 --sync=s >> lab2b_list.csv
	./lab2_list --threads=16 --iterations=40 --yield=id --lists=4 --sync=s >> lab2b_list.csv

	./lab2_list --threads=1 --iterations=80 --yield=id --lists=4 --sync=s >> lab2b_list.csv
	./lab2_list --threads=2 --iterations=80 --yield=id --lists=4 --sync=s >> lab2b_list.csv
	./lab2_list --threads=4 --iterations=80 --yield=id --lists=4 --sync=s >> lab2b_list.csv
	./lab2_list --threads=8 --iterations=80 --yield=id  --lists=4 --sync=s >> lab2b_list.csv
	./lab2_list --threads=12 --iterations=80 --yield=id  --lists=4 --sync=s >> lab2b_list.csv
	./lab2_list --threads=16 --iterations=80 --yield=id --lists=4 --sync=s >> lab2b_list.csv


	./lab2_list --threads=1 --iterations=10 --yield=id --lists=4 --sync=m >> lab2b_list.csv
	./lab2_list --threads=2 --iterations=10 --yield=id --lists=4 --sync=m >> lab2b_list.csv
	./lab2_list --threads=4 --iterations=10 --yield=id --lists=4 --sync=m >> lab2b_list.csv
	./lab2_list --threads=8 --iterations=10 --yield=id  --lists=4 --sync=m >> lab2b_list.csv
	./lab2_list --threads=12 --iterations=10 --yield=id  --lists=4 --sync=m >> lab2b_list.csv
	./lab2_list --threads=16 --iterations=10 --yield=id --lists=4 --sync=m >> lab2b_list.csv

	./lab2_list --threads=1 --iterations=20 --yield=id --lists=4 --sync=m >> lab2b_list.csv
	./lab2_list --threads=2 --iterations=20 --yield=id --lists=4 --sync=m >> lab2b_list.csv
	./lab2_list --threads=4 --iterations=20 --yield=id --lists=4 --sync=m >> lab2b_list.csv
	./lab2_list --threads=8 --iterations=20 --yield=id  --lists=4 --sync=m >> lab2b_list.csv
	./lab2_list --threads=12 --iterations=20 --yield=id  --lists=4 --sync=m >> lab2b_list.csv
	./lab2_list --threads=16 --iterations=20 --yield=id --lists=4 --sync=m >> lab2b_list.csv

	./lab2_list --threads=1 --iterations=40 --yield=id --lists=4 --sync=m >> lab2b_list.csv
	./lab2_list --threads=2 --iterations=40 --yield=id --lists=4 --sync=m >> lab2b_list.csv
	./lab2_list --threads=4 --iterations=40 --yield=id --lists=4 --sync=m >> lab2b_list.csv
	./lab2_list --threads=8 --iterations=40 --yield=id  --lists=4 --sync=m >> lab2b_list.csv
	./lab2_list --threads=12 --iterations=40 --yield=id  --lists=4 --sync=m >> lab2b_list.csv
	./lab2_list --threads=16 --iterations=40 --yield=id --lists=4 --sync=m >> lab2b_list.csv

	./lab2_list --threads=1 --iterations=80 --yield=id --lists=4 --sync=m >> lab2b_list.csv
	./lab2_list --threads=2 --iterations=80 --yield=id --lists=4 --sync=m >> lab2b_list.csv
	./lab2_list --threads=4 --iterations=80 --yield=id --lists=4 --sync=m >> lab2b_list.csv
	./lab2_list --threads=8 --iterations=80 --yield=id  --lists=4 --sync=m >> lab2b_list.csv
	./lab2_list --threads=12 --iterations=80 --yield=id  --lists=4 --sync=m >> lab2b_list.csv
	./lab2_list --threads=16 --iterations=80 --yield=id --lists=4 --sync=m >> lab2b_list.csv



	./lab2_list --threads=1 --iterations=1000 --sync=m --lists=1 >> lab2b_list.csv
	./lab2_list --threads=2 --iterations=1000 --sync=m --lists=1 >> lab2b_list.csv
	./lab2_list --threads=4 --iterations=1000 --sync=m --lists=1 >> lab2b_list.csv
	./lab2_list --threads=8 --iterations=1000 --sync=m --lists=1 >> lab2b_list.csv	
	./lab2_list --threads=12 --iterations=1000 --sync=m --lists=1 >> lab2b_list.csv

	./lab2_list --threads=1 --iterations=1000 --sync=m --lists=4 >> lab2b_list.csv
	./lab2_list --threads=2 --iterations=1000 --sync=m --lists=4 >> lab2b_list.csv
	./lab2_list --threads=4 --iterations=1000 --sync=m --lists=4 >> lab2b_list.csv
	./lab2_list --threads=8 --iterations=1000 --sync=m --lists=4 >> lab2b_list.csv	
	./lab2_list --threads=12 --iterations=1000 --sync=m --lists=4 >> lab2b_list.csv

	./lab2_list --threads=1 --iterations=1000 --sync=m --lists=8 >> lab2b_list.csv
	./lab2_list --threads=2 --iterations=1000 --sync=m --lists=8 >> lab2b_list.csv
	./lab2_list --threads=4 --iterations=1000 --sync=m --lists=8 >> lab2b_list.csv
	./lab2_list --threads=8 --iterations=1000 --sync=m --lists=8 >> lab2b_list.csv	
	./lab2_list --threads=12 --iterations=1000 --sync=m --lists=8 >> lab2b_list.csv

	./lab2_list --threads=1 --iterations=1000 --sync=m --lists=16 >> lab2b_list.csv
	./lab2_list --threads=2 --iterations=1000 --sync=m --lists=16 >> lab2b_list.csv
	./lab2_list --threads=4 --iterations=1000 --sync=m --lists=16 >> lab2b_list.csv
	./lab2_list --threads=8 --iterations=1000 --sync=m --lists=16 >> lab2b_list.csv	
	./lab2_list --threads=12 --iterations=1000 --sync=m --lists=16 >> lab2b_list.csv

	./lab2_list --threads=1 --iterations=1000 --sync=s --lists=1 >> lab2b_list.csv
	./lab2_list --threads=2 --iterations=1000 --sync=s --lists=1 >> lab2b_list.csv
	./lab2_list --threads=4 --iterations=1000 --sync=s --lists=1 >> lab2b_list.csv
	./lab2_list --threads=8 --iterations=1000 --sync=s --lists=1 >> lab2b_list.csv	
	./lab2_list --threads=12 --iterations=1000 --sync=s --lists=1 >> lab2b_list.csv

	./lab2_list --threads=1 --iterations=1000 --sync=s --lists=4 >> lab2b_list.csv
	./lab2_list --threads=2 --iterations=1000 --sync=s --lists=4 >> lab2b_list.csv
	./lab2_list --threads=4 --iterations=1000 --sync=s --lists=4 >> lab2b_list.csv
	./lab2_list --threads=8 --iterations=1000 --sync=s --lists=4 >> lab2b_list.csv	
	./lab2_list --threads=12 --iterations=1000 --sync=s --lists=4 >> lab2b_list.csv

	./lab2_list --threads=1 --iterations=1000 --sync=s --lists=8 >> lab2b_list.csv
	./lab2_list --threads=2 --iterations=1000 --sync=s --lists=8 >> lab2b_list.csv
	./lab2_list --threads=4 --iterations=1000 --sync=s --lists=8 >> lab2b_list.csv
	./lab2_list --threads=8 --iterations=1000 --sync=s --lists=8 >> lab2b_list.csv	
	./lab2_list --threads=12 --iterations=1000 --sync=s --lists=8 >> lab2b_list.csv

	./lab2_list --threads=1 --iterations=1000 --sync=s --lists=16 >> lab2b_list.csv
	./lab2_list --threads=2 --iterations=1000 --sync=s --lists=16 >> lab2b_list.csv
	./lab2_list --threads=4 --iterations=1000 --sync=s --lists=16 >> lab2b_list.csv
	./lab2_list --threads=8 --iterations=1000 --sync=s --lists=16 >> lab2b_list.csv	
	./lab2_list --threads=12 --iterations=1000 --sync=s --lists=16 >> lab2b_list.csv

dist: lab2b-004810708.tar.gz

lab2b-004810708.tar.gz: graphs profile
	tar -czf $@ lab2_list.c SortedList.h SortedList.c lab2b_list.csv lab2b_1.png lab2b_2.png lab2b_3.png lab2b_4.png lab2b_5.png profile.out Makefile README lab2b_list.gp

clean:
	rm -f *.tar.gz lab2_list