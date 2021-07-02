using CP;

// The number of jobs
int   n = ...;
// The number of operations in a job
int   m = ...;
// The job's release dates
int   rd[1..n] = ...;
// The job's due dates
int   dd[1..n] = ...;
// The job's priorities
float w [1..n] = ...;
// Operation processing times
int   pt[1..n][1..m] = ...;

// Declare decision variables of type interval
dvar interval op[i in 1..n][j in 1..m] size pt[i][j];

// Declare the earliness/tardiness objective 
minimize sum(i in 1..n) w[i]*abs(endOf(op[i][m])-dd[i]);

subject to { 
  forall(i in 1..n) { 
    // The first operation of each job should start after the job's release date  
    rd[i] <= startOf(op[i][1]);
    // Precedence constraints between operations of the same job
    forall(j in 1..m-1)
      endBeforeStart(op[i][j],op[i][j+1]);
  } 
  // Operations requiring the same machine do not overlap
  forall(j in 1..m)
    noOverlap(all(i in 1..n) op[i][j]);
}
