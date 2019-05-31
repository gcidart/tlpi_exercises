include ../Makefile.inc

GEN_EXE =  sig_receiver sig_receiver_ign sig_receiver_nodefer_resethand t_siginterrupt t_abort sigcont_demo sig_speed_sigwaitinfo system_v_sigapi

EXE = ${GEN_EXE} 

all : ${EXE}

allgen : ${GEN_EXE}


clean : 
	${RM} ${EXE} *.o

showall :
	@ echo ${EXE}

${EXE} : ${TLPI_LIB}		# True as a rough approximation

