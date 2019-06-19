include ../Makefile.inc

GEN_EXE = t_cp  mmap_boundary nonlinear_mapping svshm_xfr_writer svshm_xfr_reader
EXE = ${GEN_EXE} 

all : ${EXE}

allgen : ${GEN_EXE}


clean : 
	${RM} ${EXE} *.o

showall :
	@ echo ${EXE}

${EXE} : ${TLPI_LIB}		# True as a rough approximation

