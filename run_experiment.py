import os
import array
import random
import time
os.system('make')
# Pthread
for i, num_threads in enumerate([1, 4, 8, 12]): # (2.in,10), (3.in,50), (4.in,100), (5.in.150)

    # run experiment
    cmd = "./APSP_Pthread testcase/5.in out/5.out {} > log".format(num_threads)
    print('#'+cmd)
    os.system(cmd)

    # check if correct
    cmd2 = "diff testcase/5.out out/5.out"
    print('#'+cmd2)
    os.system(cmd2)

    with open('log', 'r') as f:
        time_arr = f.read().split()
        time_arr = [float(v) for v in time_arr]
        print('time[{}] = [{}, {}, {}]'.format(i, time_arr[0], time_arr[1], time_arr[2]))
