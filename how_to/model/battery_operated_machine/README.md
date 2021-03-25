Suppose you have a machine available for performing activities one at a time. 
Processing an activity `i` consumes some energy on the machine `RD*D[i]` that is proportional to the activity duration `D[i]`. 
The energy is stored on a battery of limited capacity `C`. In between activities, the machine batterry can be recharged at a constant rate `RR`.

Let's suppose for simplification that `RD=RR=1`. For instance, if the machine battery is initially empty and the first activity (say `A`) last 10 time units, then activity `A` will not be able to start earlier than date 10 in order to leave enough time for the battery to charge at level 10. If the next activity (say `B`) lasts 20 units, it will have to wait at least 20 units after the end date of `A` (20) before to start. So a feasible solution is (`t` denotes the start or end time of the activity, `l` the battery level at this time):

`[t=10,l=10]--A->[t=20,l=0] ... [t=40,l=20]--B->[t=60,l=0]`

Another solution could be :

`[t=20,l=20]--A->[t=30,l=10] ... [t=40,l=20]--B->[t=60,l=0]`

The level of the battery is capped by the battery capacity so if in this example the battery capacity is `C=30`, another feasible solution is for instance:

`[t=20,l=20]--A->[t=30,l=10] ... [t=100,l=30]--B->[t=120,l=10]`

Here is a formulation of the problem on a single battery operated machine, assuming the battery consumption/production rate is 1 (the model can trivially be adapted to the case of different rates). Activities are optional, have specific time-windows and the objective is to maximize the number of executed activities. It uses expressions `typeOfPrev` to track the level of the battery at the end of each operation.



The corresponding formulation in OPL is here : [single-machine.mod](./opl/single-machine.mod)

A second OPL model ([jobshop-batteries.mod](./opl/jobshop-batteries.mod)) integrates the battery operated machines in a classical jobshop scheduling problem. 
