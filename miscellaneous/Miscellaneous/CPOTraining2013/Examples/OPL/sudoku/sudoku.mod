using CP;

int n=9;
int s=3;

tuple Cell {
  int row;
  int column;
  int value;
}
{Cell} Cells = ...;

// Display grid
int Grid[r in 1..n][c in 1..n]=0; 
execute {
  for(var x in Cells)
    Grid[x.row][x.column]=x.value;
  writeln("// grid");
  for(var r=1; r<=n; r++) {
    for(var c=1; c<=n; c++) {
      if (0<Grid[r][c]) {
        write(Grid[r][c] + " ");
      } else {
        write("X ");
      }
    }
    writeln();
  }
  writeln();
}

dvar int value[1..n][1..n] in 1..n;

constraints {
  // Values in grid
  forall(x in Cells)
    value[x.row][x.column]==x.value;
  // All different in a line
  forall(r in 1..n)
    allDifferent(all(c in 1..n) value[r][c]);
  // All different in a column
  forall(c in 1..n)
    allDifferent(all(r in 1..n) value[r][c]);
  // All different in a square
  forall(sr in 1..s, sc in 1..s) 
    allDifferent(all(r in 1+s*(sr-1)..s*sr, 
                     c in 1+s*(sc-1)..s*sc) value[r][c]);
}

// Display solution
execute {
  for(var r=1; r<=n; r++) {
    for(var c=1; c<=n; c++)
      write(value[r][c] + " ");  
    writeln();
  }
};
