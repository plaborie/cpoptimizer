using CP;
int n = ...;
int m = ...;
int rd[1..n] = ...;
int dd[1..n] = ...;
float w[1..n] = ...;
int pt[1..n][1..m] = ...;
float W = sum(i in 1..n) (w[i] * sum(j in 1..m) pt[i][j]);
dvar interval op[i in 1..n][j in 1..m] size pt[i][j];
dexpr int C[i in 1..n] = endOf(op[i][m]);
minimize sum(i in 1..n) w[i]*abs(C[i]-dd[i])/W;
subject to { 
  forall(i in 1..n) { 
    rd[i] <= startOf(op[i][1]);
    forall(j in 1..m-1)
    endBeforeStart(op[i][j],op[i][j+1]);
  } 
  forall(j in 1..m)
    noOverlap(all(i in 1..n) op[i][j]);
}