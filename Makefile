# Makefile for cvsfs vfs module
#
# Author: Petric Frank
# Date:   May 24th, 2001

PROJECTS =	cvsfs \
		cvsmnt \
		cvsmount

.PHONY:	$(PROJECTS)

RM = rm -f


all:		$(PROJECTS)


$(PROJECTS):
	cd $@ && $(MAKE)


clean:
	$(foreach dir,$(PROJECTS),cd $(dir) && $(MAKE) clean; cd ..;)


distclean:
	$(foreach dir,$(PROJECTS),cd $(dir) && $(MAKE) distclean; cd ..;)


install:
	$(foreach dir,$(PROJECTS),cd $(dir) && $(MAKE) install; cd ..;)
