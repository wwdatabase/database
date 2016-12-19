all: testfile

testfile: testrcdmana.cpp rcdmana/rm.h rcdmana/rm_filehandle.h \
		rcdmana/rm_record.h rcdmana/rm_rid.h rcdmana/rm_filescan.h
	g++ testrcdmana.cpp -o testfile

clean:
	rm testfile