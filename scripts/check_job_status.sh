
token=$1

job_status=$(squeue -o "%.18i %.9P %.50j %.8u %.2t %.10M %.6D %R" | grep ${token} | tr -s ' ' | cut -d ' ' -f 6 )
job_time=$(squeue -o "%.18i %.9P %.50j %.8u %.2t %.10M %.6D %R" | grep ${token} | tr -s ' ' | cut -d ' ' -f  7 )


if [ "${job_status}" = "R" ]; then
	echo "Job $1 has beend running for ${job_time}"
elif [ "${job_status}" = "PD" ] ; then
	echo "Job $1 is pending..."
elif [ "${job_status}" = "CG" ]; then
	echo "Job $1 is exiting..."
else 
	echo "Job not found!"
fi
