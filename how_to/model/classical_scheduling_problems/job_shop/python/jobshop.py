# 1. READING THE DATA

import json
with open("data-ft10.json") as file:
    data = json.load(file)
J = data["jobs"]

n = len(J)
N = range(n)
O = [len(J[i]) for i in N]
m = max([o[0] for o in J[i] for i in N])
M = range(m+1)

# 2. MODELING THE PROBLEM WITH CP-OPTIMIZER

from docplex.cp.model import *
model = CpoModel()

# Decision variables: operations
x = [ [interval_var(size=o[1]) for o in J[i]] for i in N ]

model.add(
 # Objective: minimize makespan
 [ minimize(max([end_of(x[i][O[i]-1]) for i in N])) ] +
 # Constraints: operations do not overlap on machines
 [ no_overlap([x[i][j] for i in N for j in range(O[i]) if J[i][j][0]==k])  for k in M ] +
 # Constraints: precedence between consecutive operations of a job
 [ end_before_start(x[i][j-1], x[i][j])  for i in N for j in range(1,O[i]) ] 
)

# 3. SOLVING THE PROBLEM

sol = model.solve(TimeLimit=30, LogPeriod=1000000)

# 4. DISPLAY THE SOLUTION

for i in N:
    print("Job"+str(i)+":")
    for j in range(O[i]):
        op = sol.get_var_solution(x[i][j])
        print("  "+str(op.start)+"->"+str(op.end))
