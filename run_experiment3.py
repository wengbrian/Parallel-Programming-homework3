import os
import array
import random
import time
os.system('make')
# MPI_sync
for i, num_vertex in enumerate([10, 50, 100, 150]): # (2.in,10), (3.in,50), (4.in,100), (5.in.150)
    for N in range(1,5):
        # run experiment
        cmd = "srun -p batch --overcommit -N{0} -n{1} ./APSP_MPI_async testcase/{2}.in out/{2}.out {1} > log".format(N, num_vertex, i+2)
        print('#'+cmd)
        os.system(cmd)

        # check if correct
        cmd2 = "diff testcase/{0}.out out/{0}.out".format(i+2)
        print('#'+cmd2)
        os.system(cmd2)

        def mean(l):
            return sum(l)/num_vertex

        with open('log', 'r') as f:
            time_arr = f.read().split()
            time_arr = [float(v) for v in time_arr]
            time_arr2 = [[],[],[]]
            for j in range(len(time_arr)):
                time_arr2[j%3].append(time_arr[j])
            print('time[{},{}] = [{}, {}, {}]'.format(i,N-1, mean(time_arr2[0]), mean(time_arr2[1]), mean(time_arr2[2])))
