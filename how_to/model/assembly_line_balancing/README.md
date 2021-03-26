# Simple Assembly Line Balancing Problem 


## Problem description

The Simple Assembly Line Balancing Problem ([SALBP](https://assembly-line-balancing.de/salbp/)) is the basic optimization problem in assembly line balancing research. Given is a set of operations each of which has a deterministic duration. The operations are partially ordered by precedence relations defining a precedence graph. The paced assembly line consists of a sequence of (work) stations. In each station a subset of the operations is performed by a single operator. The resulting station time (sum of the respective operation times) must not exceed the cycle time. Concerning the precedence relations, no task is allowed to be executed in an earlier station than any of its predecessors. We consider the version where the cycle time is given and we want to minimize the number of stations (SALBP-1).

More information about the formulation of this problem in CP Optimizer and the performance of the search is available in this [post](https://ibm.biz/CPO_SALBP).

