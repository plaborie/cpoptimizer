 using CP;
 int n=9;
 int s=3;
 tuple Cell { int row; int column; int value; }
 {Cell} Cells = ...;
 dvar int value[1..n][1..n] in 1..n;
 constraints {
   forall(x in Cells) // Values in grid
     value[x.row][x.column]==x.value;
   forall(r in 1..n)  // All different in a line
     allDifferent(all(c in 1..n) value[r][c]);
   forall(c in 1..n)  // All different in a column
     allDifferent(all(r in 1..n) value[r][c]);
   forall(sr in 1..s, sc in 1..s) // All different in a square
     allDifferent(all(r in 1+s*(sr-1)..s*sr, c in 1+s*(sc-1)..s*sc) value[r][c]);
 }
 execute { // Display solution
   for(var r=1; r<=n; r++) {
     for(var c=1; c<=n; c++)
       write(value[r][c] + " ");  
     writeln();
   }
 };


