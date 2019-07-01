include ../Makefile.inc

GEN_EXE =   ss_seqnum_client ss_seqnum_server is_seqnum_sv is_seqnum_cl is_keyval_sv is_keyval_cl us_xfr_sv us_xfr_cl \
            is_echo_sv_lim listen_wo_bind t_sendfile is_shell_sv is_shell_cl t_pipe oo_sv oo_cl

EXE = ${GEN_EXE} 

all : ${EXE}

allgen : ${GEN_EXE}


clean : 
	${RM} ${EXE} *.o

showall :
	@ echo ${EXE}

${EXE} : ${TLPI_LIB}		# True as a rough approximation

