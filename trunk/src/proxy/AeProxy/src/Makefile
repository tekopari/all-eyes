#==============================================================#
# File:    Makefile                                            #
# Author:  Thomas Pari                                         #
#==============================================================#

#
#---------------------------------------------------------------
# Define the compiler and compiler flags
#---------------------------------------------------------------
#
#JFLAGS = -g:none -d ../build -cp ../build:.

JFLAGS = -g -d ../build -cp ../build:.
JC = javac


#
#---------------------------------------------------------------
# Clear any default targets for building .class files as we are
# providing our own target entry.
#---------------------------------------------------------------
#

.SUFFIXES: .java .class

#
#---------------------------------------------------------------
# The following defines the rule for creating .class files from 
# .java files 
#---------------------------------------------------------------
#

.java.class:
	$(JC) $(JFLAGS) $*.java

#
#---------------------------------------------------------------
# The class files the comprise the secure sandbox tool.
#---------------------------------------------------------------
#

CLASSES = \
	AeMessage.java \
	AeMessageStatistics.java \
	AeConnector.java \
	AeHeartbeat.java \
	AeMessageStore.java \
	AeSSLServer.java \
	AeStatusKeeper.java \
	AeProxy.java \
	InstallCert.java

#
#---------------------------------------------------------------
# The default make target entry. (aka build everything)
#---------------------------------------------------------------
#

default: clean init classes jar install

all: clean init classes jar install

clobber: clean

#
#---------------------------------------------------------------
# The init target creates the directories to store the build.
#---------------------------------------------------------------
#

init: 
	mkdir --mode 750 -p ../bin
	mkdir --mode 750 -p ../build
	sudo mkdir --mode 755 -p /etc/ae/certs

#
#---------------------------------------------------------------
# This target entry uses invoke the the java classes to be 
# compiled
#---------------------------------------------------------------
#

classes: $(CLASSES:.java=.class)


#
#---------------------------------------------------------------
# The clean target removes previously buildt classes and jars
#---------------------------------------------------------------
#

clean:
	$(RM) ../build/org/tbrt/ae/*.class
	$(RM) ../bin/*.jar
	$(RM) ../bin/aeproxy

#
#---------------------------------------------------------------
# The jar target creates an executable jar file that is sealed 
# so that all the classes in the org/tbrt/ae package 
# must come from a single jar file.
#---------------------------------------------------------------
#

jar:
	(cd ../build; echo Main-Class: org.tbrt.ae.AeProxy > ./manifest.txt)
	(cd ../build; echo Name: org/tbrt/ae/ >> ./manifest.txt)
	(cd ../build; echo Sealed: true >> ./manifest.txt)
	(cd ../build; jar -cvfm AeProxy.jar manifest.txt  */*/*/*.class)

#
#---------------------------------------------------------------
# The install target moves the jar file into the bin directory.
#---------------------------------------------------------------
#

install:
	(cd ../build; mv AeProxy.jar ../bin/AeProxy.jar)
	gcc -o ../bin/aeproxy ./aeproxy.c
	sudo chown root:root ../bin/aeproxy
	sudo chmod 6755 ../bin/aeproxy
	sudo cp ./jssecacerts /etc/ae/certs/jssecacerts
	sudo chown root:root /etc/ae/certs/jssecacerts
	sudo chmod 744 /etc/ae/certs/jssecacerts
	sudo cp ./keystore.jks /etc/ae/certs/keystore.jks
	sudo chown root:root /etc/ae/certs/keystore.jks
	sudo chmod 744 /etc/ae/certs/keystore.jks
	sudo cp ../bin/aeproxy /usr/local/bin/aeproxy
	sudo chown root:root /usr/local/bin/aeproxy
	sudo chmod 6755 /usr/local/bin/aeproxy
	sudo cp ../bin/AeProxy.jar /usr/local/bin/AeProxy.jar
	sudo chown root:root /usr/local/bin/AeProxy.jar
	sudo chmod 755 /usr/local/bin/AeProxy.jar
