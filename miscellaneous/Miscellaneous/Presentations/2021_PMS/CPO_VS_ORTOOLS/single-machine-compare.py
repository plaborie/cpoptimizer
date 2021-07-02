import collections
import time

from ortools.sat.python import cp_model
from docplex.cp.model import CpoModel

Workers = 4

def sol_ortools(n):
    itvs = []
    model = cp_model.CpModel()
    for i in range(n):
        h = n**2
        start_var = model.NewIntVar(0, h, name="S{0}".format(i))
        end_var = model.NewIntVar(0, h, name="E{0}".format(i))
        interval_var = model.NewIntervalVar(start_var, i+1, end_var, name="O{0}".format(i))
        itvs.append(interval_var)
    model.AddNoOverlap(itvs)
    solver = cp_model.CpSolver()
    start = time.time()
    global Workers
    solver.parameters.num_search_workers = Workers
    solver.Solve(model)
    end = time.time()
    return end-start
    
def sol_cpo(n):
    model = CpoModel()
    x = [model.interval_var(size=i+1) for i in range(n)]
    model.add(model.no_overlap(x))
    start = time.time()
    global Workers
    model.solve(Workers=Workers, SolutionLimit=1, LogVerbosity="Quiet")
    end = time.time()
    return end-start

PERFS = []
for k in range(1,11):
  ortools = sol_ortools(1000*k)
  cpo = sol_cpo(1000*k)
  PERFS.append((1000*k,ortools,cpo))
  print(1000*k,ortools,cpo)
for k in range(3,20):
    cpo = sol_cpo(5000*k)
    print(5000*k, "X", cpo)