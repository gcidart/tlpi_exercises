include ../Makefile.inc

GEN_EXE = t_tee t_cp append_test atomic_append dup scatter_gather_io

EXE = ${GEN_EXE} 

all : ${EXE}

allgen : ${GEN_EXE}


clean : 
	${RM} ${EXE} *.o

showall :
	@ echo ${EXE}

${EXE} : ${TLPI_LIB}		# True as a rough approximation

