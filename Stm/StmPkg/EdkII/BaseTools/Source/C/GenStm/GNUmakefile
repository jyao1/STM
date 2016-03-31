ARCH ?= IA32
MAKEROOT ?= ..

STM_HOME=$(WORKSPACE)/StmPkg

# BUGBUG need add "-I $(INC)" to $(INCLUDE)
INC = -I $(STM_HOME)/Include -I $(WORKSPACE)/MdePkg/Include

APPNAME = GenStm

LIBS = -lCommon

OBJECTS = GenStm.o

include $(MAKEROOT)/Makefiles/app.makefile
