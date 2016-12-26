all:  tstsm

testfile: testrcdmana.cpp rcdmana/rm.h rcdmana/rm_filehandle.h \
		rcdmana/rm_record.h rcdmana/rm_rid.h rcdmana/rm_filescan.h
	g++ testrcdmana.cpp -o testfile

tstsm: testsm.cpp smmana/sm.h
	g++ testsm.cpp -o  tstsm

clean:
	rm testfile
