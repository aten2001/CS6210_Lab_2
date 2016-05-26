from subprocess import call
import sys
#mpirun = "mpirun"
mpirun = "/opt/openmpi-1.4.3-gcc44/bin/mpirun"
mpiaddi = "--hostfile $PBS_NODEFILE"

def centralized_barrier_mp(thread_num, barrier_num):
    print(sys.argv[1])
    call(["make", "centralized_barrier_mp"])
    for i in range(int(thread_num), 10, 2):
		call(["./centralized_barrier_mp", str(i), str(barrier_num)])
def dissemination_barrier(thread_num, barrier_num):
    print(sys.argv[1])
    call(["make", "dissemination_barrier"])
    for i in range(int(thread_num), 10, 2):
		call(["./dissemination_barrier", str(i), str(barrier_num)])	

def centralized_barrier_mpi(proc_num, barrier_num):
    print(sys.argv[1])
    call(["make", "centralized_barrier_mpi"])
    for i in range(int(proc_num), 14, 2):
    	call(mpirun+" "+mpiaddi+" -np "+str(i)+" centralized_barrier_mpi "+str(barrier_num), shell=True)
def mcs_barrier(proc_num, barrier_num):
    print(sys.argv[1])
    call(["make", "mcs_barrier"])
    for i in range(int(proc_num), 14, 2):
    	call(mpirun+" "+mpiaddi+" -np "+str(i)+" mcs_barrier "+str(barrier_num), shell=True)
def mcs_cen_combine(proc_num, thread_num, barrier_num):
    print(sys.argv[1])
    call(["make", "mcs_cen_combine"])
    for i in range(int(proc_num), 10, 2):
    	call(mpirun+" "+mpiaddi+" -np "+str(i)+" mcs_cen_combine "+str(thread_num)+" "+str(barrier_num), shell=True)

def mcs_barrier_npernode(proc_num, barrier_num):
    print(sys.argv[1])
    call(["make", "mcs_barrier"])
    for i in range(int(proc_num), 14, 2):
    	call(mpirun+" "+mpiaddi+" -npernode "+str(i)+" mcs_barrier "+str(barrier_num), shell=True)




if len(sys.argv) < 2:
        print("usage: python test.py <OBJ_NAME> <...>")
elif sys.argv[1] == "centralized_barrier_mp":
	if len(sys.argv) < 4:
		print("usage: python test.py centralized_barrier_mp <thread_num> <barrier_num>")
	else:
		centralized_barrier_mp(sys.argv[2], sys.argv[3])

elif sys.argv[1] == "dissemination_barrier":
	if len(sys.argv) < 4:
		print("usage: python test.py dissemination_barrier <thread_num> <barrier_num>")
	else:
		dissemination_barrier(sys.argv[2], sys.argv[3])		

elif sys.argv[1] == "centralized_barrier_mpi":
	if len(sys.argv) < 4:
		print("usage: python test.py centralized_barrier_mpi <proc_num> <barrier_num>")
	else:
		centralized_barrier_mpi(sys.argv[2], sys.argv[3])		

elif sys.argv[1] == "mcs_barrier":
	if len(sys.argv) < 4:
		print("usage: python test.py mcs_barrier <proc_num> <barrier_num>")
	else:
		mcs_barrier(sys.argv[2], sys.argv[3])		

elif sys.argv[1] == "mcs_cen_combine":
	if len(sys.argv) < 5:
		print("usage: python test.py mcs_cen_combine <proc_num> <thread_num> <barrier_num>")
	else:
		mcs_cen_combine(sys.argv[2], sys.argv[3], sys.argv[4])

elif sys.argv[1] == "mcs_barrier_npernode":
	if len(sys.argv) < 4:
		print("usage: python test.py mcs_barrier_npernode <proc_num> <barrier_num>")
	else:
		mcs_barrier_npernode(sys.argv[2], sys.argv[3])

else:
	print("usage: python test.py <OBJ_NAME> <...>")
