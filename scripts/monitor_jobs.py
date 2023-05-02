
import subprocess
import re
import pandas as pd

#job_monitor = subprocess.Popen(['squeue','-o','"i%.18i %.9P %.50j %.8u %.2t %.10M %.6D %R"'], 
#															stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
#print(job_monitor.stdout.read())

#out, err = job_monitor.stdout.decode('utf-8')
#job_data_raw, err = job_monitor.communicate()
#print(err)
#print(type(job_data_raw))
#job_data_raw = job_data_raw.replace(' ', ',')
#job_data_raw = re.sub('"\n"', '\n', job_data_raw)
#job_data_raw = re.sub('\s+', ',', job_data_raw)
#print(job_data_raw)

job_monitor = subprocess.run(['squeue','-o','%.10i %.10P %.30j %.8u %.2t %.5M %.5D %R'], 
														stdout=subprocess.PIPE)
job_data_raw = job_monitor.stdout.decode('utf-8')

#job_data_raw = re.sub('\s+', ',', job_data_raw)

#print(job_data_raw)

for line in job_data_raw.splitlines():
	print(line)
job_data_raw = job_data_raw.splitlines()

#print(job_data_raw[2])

#df = pd.DataFrame({'JobID': job_data_raw[::2], 'Value': job_data_raw[1::2]})


#print(df)
#for line in re.findall( cache + "\s" + op + ".*", data):
