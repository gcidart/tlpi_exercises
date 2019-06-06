include ../Makefile.inc

GEN_EXE =  fork_sig_sync parent_term grandchild child_status make_zombie t_execlp parent_sigchld fork_vfork
EXE = ${GEN_EXE} 

all : ${EXE}

allgen : ${GEN_EXE}


clean : 
	${RM} ${EXE} *.o

showall :
	@ echo ${EXE}

${EXE} : ${TLPI_LIB}		# True as a rough approximation

