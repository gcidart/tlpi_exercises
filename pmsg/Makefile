include ../Makefile.inc

GEN_EXE =  mq_notify_sigwaitinfo  pmsg_receive  \
      mq_seqnum_client mq_seqnum_server mq_file_server mq_file_client t_talk

LINUX_EXE =

EXE = ${GEN_EXE} ${LINUX_EXE}

all : ${EXE}

allgen : ${GEN_EXE}

LDLIBS = ${IMPL_LDLIBS} ${LINUX_LIBRT}
	# All of the programs in this directory need the 
	# realtime library, librt.

clean : 
	${RM} ${EXE} *.o

showall :
	@ echo ${EXE}

${EXE} : ${TLPI_LIB}		# True as a rough approximation

