#!/bin/sh
#SBATCH -A k01
#SBATCH -J test_wrf_cyclone
#SBATCH -N 300
#SBATCH -t 40
#SBATCH -o wrf_out_%j
#SBATCH -e wrf_err_%j

#DW jobdw type=scratch access_mode=striped capacity=1500GiB
#DW stage_in type=directory source=/scratch/markomg/wrf_cyclone/wrf-3.7.1/run_big/forburst  destination=$DW_JOB_STRIPED
#DW stage_out type=directory destination=/scratch/markomg/tmp/wrf-3.7.1/run_big/burst_buffer_output source=$DW_JOB_STRIPED/


export MPICH_ENV_DISPLAY=1
export MPICH_VERSION_DISPLAY=1
export MPICH_MPIIO_HINTS_DISPLAY=1
export MPICH_MPIIO_STATS=1
export MPICH_MPIIO_AGGREGATOR_PLACEMENT_DISPLAY=1

export MPICH_MPIIO_TIMERS=1
export OMP_NUM_THREADS=8
#export PMI_MMAP_SYNC_WAIT_TIME=600 

cd $DW_JOB_STRIPED
mkdir run_$SLURM_JOBID
mv * run_$SLURM_JOBID
cd run_$SLURM_JOBID
chmod +x wrf.exe
#export MPICH_MPIIO_HINTS="wrfrst*:cb_nodes=8:striping_unit=52428,wrfout*:cb_nodes=8:striping_unit=524288,wrfi*:cb_nodes=8:striping_unit=524288"
#export MPICH_MPIIO_HINTS="wrfrst*:cb_nodes=32:striping_unit=262144,wrfout*:cb_nodes=32:striping_unit=262144,wrfi*:cb_nodes=32:striping_unit=1048576"
export MPICH_MPIIO_HINTS="wrfrst*:cb_nodes=40:,wrfout*:cb_nodes=40:striping_unit=2097152,wrfi*:cb_nodes=40:striping_unit=1048576"

#export MPICH_MPIIO_HINTS="wrfrst*:cb_nodes=64:striping_unit=524288:ind_wr_buffer_size=4194304,wrfout*:cb_nodes=20:striping_unit=524288:ind_wr_buffer_size=4194304,wrfi*:cb_nodes=32:striping_unit=524288:ind_rd_buffer_size=4194304"


#export MPICH_MPIIO_HINTS="wrfrst*:cb_nodes=20,wrfout*:cb_nodes=20,wrfi*:cb_nodes=20"

#srun -n 640 ./wrf.exe 
#srun -n 640 --hint=nomultithread ./wrf.exe
time srun --ntasks=1200 --ntasks-per-node=4 --cpus-per-task=8 --threads-per-core=1 --hint=nomultithread ./wrf.exe

