# Makefile for Lab2 - barrier synchronization

#ENV = local
ENV = jinx

ifeq ($(ENV),local)
	GCC = gcc-5
	MPICC = mpicc
else
	GCC = gcc
	MPICC = /opt/openmpi-1.4.3-gcc44/bin/mpicc
endif

# OpenMp
OPMPFLAGS = -fopenmp

# OpenMPI
OPMPIFLAGS = 

# shared flags
OTHERFLAGS = -lm -Wno-implicit -Wno-main-return-type 

SRC = centralized_barrier_mp.c dissemination_barrier.c \
centralized_barrier_mpi.c mcs_barrier.c mcs_cen_combine.c

OBJ = centralized_barrier_mp dissemination_barrier \
centralized_barrier_mpi mcs_barrier mcs_cen_combine

.PHONY : clean centralized_barrier_mp dissemination_barrier \
centralized_barrier_mpi mcs_barrier mcs_cen_combine

all: $(SRC) $(OBJ)

centralized_barrier_mp: centralized_barrier_mp.c
	@$(GCC) $(OPMPFLAGS) -o $@ $^ $(OTHERFLAGS)
dissemination_barrier: dissemination_barrier.c
	@$(GCC) $(OPMPFLAGS) -o $@ $^ $(OTHERFLAGS)
centralized_barrier_mpi: centralized_barrier_mpi.c
	$(MPICC) $(OPMPIFLAGS) -o $@ $^ $(OTHERFLAGS)
mcs_barrier: mcs_barrier.c
	@$(MPICC) $(OPMPIFLAGS) -o $@ $^ $(OTHERFLAGS)
mcs_cen_combine: mcs_cen_combine.c
	@$(MPICC) $(OPMPIFLAGS) $(OPMPFLAGS) -o $@ $^ $(OTHERFLAGS)

clean:
	@rm -rf centralized_barrier_mp dissemination_barrier \
	centralized_barrier_mpi mcs_barrier mcs_cen_combine