# 1. READING THE DATA

import json
with open("data-ft10.json") as file:
    data = json.load(file)
J = data["jobs"]

n = len(J)
N = range(n)
m = len(J[0])
M = range(m)

# 2. MODELING THE PROBLEM WITH CP-OPTIMIZER

from docplex.cp.model import *
model = CpoModel()

# Decision variables: operations
x = [ [interval_var(size=d) for d in J[i]] for i in N ]
s = [ sequence_var([x[i][k] for i in N]) for k in M]

# Objective: minimize makespan
model.add(minimize(max([end_of(x[i][m-1]) for i in N])))

# Constraints: operations do not overlap on machines
model.add([ no_overlap(s[k])  for k in M])

# Constraints: same order of operations on all machines
model.add([ same_sequence(s[0],s[k]) for k in range(1,m)])

# Constraints: precedence between consecutive operations of a job
model.add([ end_before_start(x[i][j-1], x[i][j])  for i in N for j in range(1,m)])

# 3. SOLVING THE PROBLEM

sol = model.solve(TimeLimit=30, LogPeriod=1000000)

# 4. DISPLAY THE SOLUTION

for i in N:
    print("Job"+str(i)+":")
    for j in M:
        op = sol.get_var_solution(x[i][j])
        print("  "+str(op.start)+"->"+str(op.end))
