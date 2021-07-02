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
// FILL IN

// Declare the earliness/tardiness objective 
// FILL IN

subject to { 
  // The first operation of each job should start after the job's release date  
  // FILL IN
 
  // Precedence constraints between operations of the same job
  // FILL IN
  
  // Operations requiring the same machine do not overlap
  // FILL IN

}

