include ../Makefile.inc

GEN_EXE = bidir_pipe t_popen fifo_seqnum_server_file_startup fifo_seqnum_server_signal fifo_seqnum_server_nonblocking fifo_nonblock  
EXE = ${GEN_EXE} 

all : ${EXE}

allgen : ${GEN_EXE}


clean : 
	${RM} ${EXE} *.o

showall :
	@ echo ${EXE}

${EXE} : ${TLPI_LIB}		# True as a rough approximation

