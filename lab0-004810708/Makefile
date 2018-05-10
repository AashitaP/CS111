.SILENT:

default: 
	gcc -g -Wall -Wextra lab0.c -o lab0

dist: lab0-004810708.tar.gz

lab0-004810708.tar.gz: lab0.c
	tar -czf $@ lab0.c Makefile README backtrace.png breakpoint.png

check: default wrongOptionTest catchTest unreadableInputTest unwritableOutputTest sucessfulTest copyTest

wrongOptionTest:
	echo | ./lab0 --something &> /dev/null; \
	if [[ $$? -ne 1 ]]; then \
		echo "test failed - wrong argument not detected correctly"; \
	else \
		echo "test passed - wrong argument detected correctly"; \
	fi 

catchTest: 
	echo | ./lab0 --catch --segfault &> /dev/null; \
	if [[ $$? -ne 4 ]]; then \
	echo "test failed - catching segfault not detected correctly"; \
	else \
	echo "test passed - catch detected correctly"; \
	fi

unreadableInputTest: 
	echo "hello" > input.txt; \
	chmod -r input.txt; \
	./lab0 --input=input.txt &> /dev/null; \
	if [[ $$? -ne 2 ]]; then \
	echo "test failed - unreadable input not detected correctly"; \
	else \
	echo "test passed - unreadable input detected correctly";\
	fi 
	rm input.txt

unwritableOutputTest: 
	echo "hello" > input.txt; \
	touch output.txt; \
	chmod -w output.txt; \
	./lab0 --input=input.txt --output=output.txt &> /dev/null; \
	if [[ $$? -ne 3 ]]; then \
	echo "test failed - unwritable output not detected correctly"; \
	else \
	echo "test passed - writable output detected correctly"; \
	fi 
	chmod +w output.txt;\
	rm output.txt input.txt

sucessfulTest:
	echo "hello" > input.txt; \
	touch output.txt; \
	./lab0 --input=input.txt --output=output.txt &> /dev/null; \
	if [[ $$? -ne 0 ]]; then \
	echo "test failed -  sucessful program not detected correctly";\
	else \
	echo "test passed - succesful program detected correctly";\
	fi 
	rm input.txt output.txt


copyTest: 
	echo "hello" > input.txt; \
	touch output.txt; \
	./lab0 --input=input.txt --output=output.txt &> /dev/null; \
	cmp input.txt output.txt; \
	if [[ $$? -ne 0 ]]; then \
	echo "test failed - input not successfully redirected to output";\
	else \
	echo "test passed - input successfully redirected to output";\
	fi 
	rm input.txt output.txt


clean:
	rm -f *.o *.txt *.tar.gz lab0 