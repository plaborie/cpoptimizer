 using CP;
 int n = ...;
 int d[1..n] = ...;       // Due-date
 int c[1..n][1..2] = ...; // Mode cost
 dvar interval task[i in 1..n] optional;
 dvar interval mode[i in 1..n][m in 1..2] optional; // 2 modes for each task
 dexpr float etc[i in 1..n] = abs(endOf(task[i],d[i])-d[i]); // Early/Tardy
 dexpr float dur[i in 1..n] = lengthOf(task[i],0);           // Duration
 dexpr float upf[i in 1..n] = 1-presenceOf(task[i]);         // Unperformed
 dexpr float mdc[i in 1..n] = sum(m in 1..2) presenceOf(mode[i][m])*c[i][m];        
 minimize 1.231 * sum(i in 1..n) etc[i] +
          0.277 * sum(i in 1..n) dur[i] +
          1000  * sum(i in 1..n) upf[i] +
          1.000 * sum(i in 1..n) mdc[i];
 subject to {
   // ...
 }
