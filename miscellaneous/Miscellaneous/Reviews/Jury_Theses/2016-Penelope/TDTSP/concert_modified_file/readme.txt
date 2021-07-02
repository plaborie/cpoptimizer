following modifs at lines 18, 1141, 1194:

@18 #define EPSILON (1e-6)

- inside IloSegmentedFunctionI::doSetMax

@1141 if (p->maxInterval(x1, x2, v1, s, ax1, ax2) && ax1 + EPSILON < ax2) { // added && ax1 + EPSILON < ax2

- inside IloSegmentedFunctionI::doSetMin

@1194 if (p->minInterval(x1, x2, v1, s, ax1, ax2) && ax1 + EPSILON < ax2) { // added && ax1 + EPSILON < ax2 