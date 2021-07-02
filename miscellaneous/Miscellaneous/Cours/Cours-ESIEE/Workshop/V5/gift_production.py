# Import CP Optimizer modelization functions
from docplex.cp.model import *
# Import JSON
import json
# Time handling
import datetime as dt

# This is a useful function for translating from date to our time unit (minute)
# mn('10/01') -> 10 days -> 14400 mn
def mn(ddmm):
  (d,m) = ddmm.split('/')
  t = dt.datetime(2019, int(m), int(d)) - dt.datetime(2019, 1, 1)
  return int(t.total_seconds() / 60)

T = 30 # Time limit

# Reading data file for assembly and letter durations
print("Reading data file for assembly and letter durations ...")
with open("../Data/data.json") as data_file:
    data = json.load(data_file)
A = data["assembly"]
D = data["letters"]

# Reading production orders
print("Reading production orders ...")
with open("../Data/orders.json") as data_file:
    data = json.load(data_file)
O = data["orders"]
n = len(O) # Number of orders
L = [ [l for l in O[i][0]] for i in range(n) ]

# Displaying orders
print("Displaying orders ...")
print("{:<20} {:<10}".format('NAME','BIRTHDAY'))
print("-----------------------------")
for o in O:
    print("{:<20} {:<10}".format(o[0],o[1]))

# Reading workers information
print("Reading workers information ...")
with open("../Data/workers.json") as data_file:
    data = json.load(data_file)
W = data["workers"]
m = len(W) # Number of workers

# Maximal number of (same) letters that can be produced together
BatchCapacity = 4

# Create model object

print("Creating CP Optimizer model ...")
model = CpoModel()

# Creating calendar functions
C = [ CpoStepFunction() for k in W ]
for k in range(len(W)):
        C[k].set_value(0, mn('31/12'), 100)
        for w in range(1,52): # Weekends
            C[k].set_value(7*w*1440, (7*w+2)*1440, 0)
        for v in W[k][2]: # Vacations
            C[k].set_value(mn(v[0]),mn(v[1]), 0)

# Creating an integer index I[l] for each letter l
I = {} 
i = 0
for l in D:
   I[l] = i
   i += 1


####################################################################
##
## DEFINE THE CP-OPTIMIZER MODEL HERE.
##
## YOU MAY ALSO HAVE TO ADAPT THE DISPLAY CODE BELOW.
##
####################################################################


# Solve the model
print("Solving CP Optimizer model ...")
sol = model.solve(LogPeriod=1000000, TimeLimit=T,trace_log=True)

# Display late gifts
print("Displaying late gifts ...")
for i in range(n):
    tardiness = (max(0,sol.get_var_solution(assemble[i]).get_end())-mn(O[i][1])) // (24*60) 
    if 0<tardiness:
        print("Gift for " + O[i][0] + " will be late by " + str(tardiness) + " days")

# Display solution in a Gantt chart
print("Displaying solution in a Gantt chart ...")
import docplex.cp.utils_visu as visu
if sol and visu.is_visu_enabled():
    visu.timeline(origin=0)
    visu.panel()
    for i in range(n):
        visu.sequence(O[i][0])
        for j in range(len(L[i])):
            visu.interval(sol.get_var_solution(letter[i][j]), i, L[i][j])
        visu.interval(sol.get_var_solution(assemble[i]), i, '+')
    visu.panel("Workers")
    for k in range(m):
        visu.sequence(W[k][0])
        for i in range(n):
            for j in range(len(L[i])):
                itv = sol.get_var_solution(letterW[k][i][j])
                if itv.is_present():
                    visu.interval(itv, i, L[i][j])
            itv = sol.get_var_solution(assembleW[k][i])
            if itv.is_present():
                visu.interval(itv, i, '+')
    visu.show()
