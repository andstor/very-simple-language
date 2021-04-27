TARGETS=$(shell ls *.vsl | sed s/\.vsl//g)
all: ${TARGETS}

# Currently the binaries won't compile directly
# to do so remove the "\.s" from the TARGET expression
# i.e. it is set to 
# TARGETS=$(shell ls *.vsl | sed s/\.vsl/\.s/g)
# after the change it should be
# TARGETS=$(shell ls *.vsl | sed s/\.vsl//g)
# If you leave as is and run make
# assembly files will be generated. 
# individually compile the files by
# running the following command in the shell
#  cc -o easy easy.s -no-pie
# ^ above is for the example easy.vsl
#

#CFLAGS = -g

%: %.s %.tree %.txt
	$(CC) -o $@ $< -no-pie

.PRECIOUS: %.s %.tree %.txt

%.s: %.vsl
	../src/vslc <$*.vsl > $*.s

%.tree: %.vsl
	../src/vslc -t <$*.vsl > $*.tree

%.txt: %.vsl
	../src/vslc -s <$*.vsl > $*.txt

clean:
	-rm -f *.s *.txt *.tree

purge: clean
	-rm -f ${TARGETS}
