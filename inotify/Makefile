include ../Makefile.inc

GEN_EXE =  mon_dir

EXE = ${GEN_EXE} 

all : ${EXE}

allgen : ${GEN_EXE}


clean : 
	${RM} ${EXE} *.o

showall :
	@ echo ${EXE}

${EXE} : ${TLPI_LIB}		# True as a rough approximation

