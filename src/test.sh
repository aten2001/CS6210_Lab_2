#PBS -q cs6210
#PBS -l nodes=2:sixcore 
#PBS -l walltime=00:20:00 
#PBS -N test_script_mpi_combine
OMPI_MCA_mpi_yield_when_idle=0

cd Lab2/
#python test.py centralized_barrier_mp 2 1000000
#python test.py dissemination_barrier 2 1000000
#python test.py tree_barrier 1000000
#python test.py centralized_barrier_mpi 2 1000000
#python test.py mcs_barrier 2 1000000
#python test.py mcs_cen_combine 2 2 1000000

#python test.py mcs_barrier_npernode 2 1000000