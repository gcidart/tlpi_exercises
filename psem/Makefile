include ../Makefile.inc

GEN_EXE = sem_shm psem_timedwait 
LINUX_EXE =

EXE = ${GEN_EXE} ${LINUX_EXE}

all : ${EXE}

allgen : ${GEN_EXE}

CFLAGS = ${IMPL_CFLAGS} ${IMPL_THREAD_FLAGS}
LDLIBS = ${IMPL_LDLIBS} ${IMPL_THREAD_FLAGS} ${LINUX_LIBRT}
	# POSIX semaphores need the NPTL thread library on Linux
    # psem_timedwait uses clock_gettime() which is in librt

clean : 
	${RM} ${EXE} *.o

showall :
	@ echo ${EXE}

${EXE} : ${TLPI_LIB}		# True as a rough approximation
