                    VERY HARD OPEN-SHOP INSTANCES


                Christian PRINS - Christelle GUERET
                     prins@emn.fr, gueret@emn.fr

                          2 January 1999

1. DESCRIPTION

This table contains 80 "square" (m=n) and very hard open-shop instances, with 
10 instances for each size 3x3 to 10x10. For each size n, the file names have the format GPn-k, where k is a file number from 1 to 10. Both n and k are 
edited with two characters to obtain 8-character file names. For instance, 
the 5x5 files have names GP05-01 to GP05-10. Theses instances were generated 
with the random generator described in [GP97], also presented at MAPSP'97 in 
Cambridge and to appear in Annals of OR. All line sums (rows and columns) and 
so the classical lower bound LB are all equal to 1000. The results of the following algorithms are given :

a) LPT, the Longest Processing Time simple list scheduling heuristic.
b) BBL, the black box of list scheduling heuristics described in [GP98c]. 
   This black box includes LPT and H1R, among others.
c) BBM, the black box of matching heuristics also presented in [GP98c]. 
   Contrary to uniform instances or Taillard's benchmarks, BBL outperforms 
   this black-box on average when processing times have a high dispersion, 
   which is the case here.
d) GA, the best of the genetic algorithms proposed in [P99]. It works on a 
   population of 30 schedules with distinct makespans, initialized with BBL,
   BBM, and 28 random schedules. The chromosomes (lists of operations) undergo 
   a LOX crossover. They are evaluated with a FIFO active schedule builder, 
   and reordered in increasing order of starting times. The perturbation 
   heuristic H1R of [GP98a] with 50 iterations is applied as mutation operator 
   with rate 0.1. H1R works in non-delay mode, but reordering its result makes 
   it transparent for the FIFO engine.
e) BB, an improved branch-and-bound algorithm. The basic skeleton comes from 
   Bruckers's branch-and-bound [BHJW97]. H1R is used to get a first schedule 
   at the root. The intelligent backtracking techniques of [GJP98] and the 
   forbidden intervals of [GP98b] are then applied to reduce the search tree.


The final selection is given in the following tables. An asterisk indicates a proven optimal makespan obtained by branch and bound.

2. REFERENCES

[BHJW97] P. Brucker, J. Hurink, B. Jurisch, B. Wöstmann, A branch-and-bound 
         algorithm for the open-shop problem, Discrete Applied Mathematics, 
         76, 43-49, 1997.
[GP97]   C. Guéret and C. Prins, A new lower bound for the open-shop problem, 
         Research Report 97/1/AUTO, Ecole des Mines de Nantes, 1997. To appear 
         in Annals of Operations Research.
[GJP98]  C. Guéret, N. Jussien and C. Prins, Using intelligent backtracking to 
         improve branch-and-bound methods : an application to open-shop 
         problems, Research Report 98/8/AUTO, Ecole des Mines de Nantes, 1998. 
         Presented at EURO-WG PMS 6, Turkey, July 1998. To appear in EJOR.
[GP98a]  C. Guéret and C. Prins, Classical and new heuristics for the open-shop 
         problem : a computational evaluation, EJOR, 107(2), 306-314, 1998.
[GP98b]  C. Guéret and C. Prins, Forbidden intervals for open-shop problems, 
         Research Report 98/10/AUTO, Ecole des Mines de Nantes, 1998.
[GP98c]  C. Guéret and C. Prins, Efficient heuristic black boxes for the open-
         shop: comparison with other methods on three benchmarks, Research 
         Report 98/11/AUTO, Ecole des Mines de Nantes, 1998.
[P99]	   C. Prins, Competitive genetic algorithms for the open-shop scheduling 
         problem, Research Report 99/1/AUTO, Ecole des Mines de Nantes, 1999.


3. RESULTS OF ALGORITHMS ON THE THE HARD INSTANCES

FILE NAME   LPT     BBL     BBM      GA      BB
GP03-01   *1168   *1168    1318   *1168   *1168
GP03-02    1171   *1170    1326   *1170   *1170
GP03-03   *1168   *1168    1326   *1168   *1168
GP03-04    1171   *1166    1329   *1166   *1166
GP03-05    1178   *1170    1318   *1170   *1170
GP03-06    1173   *1169    1325   *1169   *1169
GP03-07    1177   *1165    1325   *1165   *1165
GP03-08    1172   *1167    1328   *1167   *1167
GP03-09    1175   *1162    1329   *1162   *1162
GP03-10    1169   *1165    1325   *1165   *1165
GP04-01    1598    1282    1289   *1281   *1281
GP04-02    1553    1271    1281   *1270   *1270
GP04-03    1606   *1288    1306   *1288   *1288
GP04-04    1566    1265   *1261   *1261   *1261
GP04-05    1622   *1289    1329   *1289   *1289
GP04-06    1582   *1269    1306   *1269   *1269
GP04-07    1570    1279    1279   *1267   *1267
GP04-08    1596   *1259    1288   *1259   *1259
GP04-09    1602    1285    1291   *1280   *1280
GP04-10    1556   *1263    1293   *1263   *1263
GP05-01    1569    1256    1286   *1245   *1245
GP05-02    1610   *1247    1253   *1247   *1247
GP05-03    1578    1267    1274   *1265   *1265
GP05-04    1567    1274    1272    1263   *1258
GP05-05    1597    1281    1314   *1280   *1280
GP05-06    1589    1270    1279   *1269   *1269
GP05-07    1620    1270    1270   *1269   *1269
GP05-08    1609    1288    1299   *1287   *1287
GP05-09    1595    1289   *1262   *1262   *1262
GP05-10    1585    1283    1275    1255   *1254
GP06-01    1600    1266    1282    1265   *1264
GP06-02    1604    1289    1298    1289    1285
GP06-03    1601    1257    1262    1257   *1255
GP06-04    1614    1278    1307   *1275   *1275
GP06-05    1641    1300    1304    1300   *1299
GP06-06    1596    1286    1302    1285   *1284
GP06-07    1612    1295    1298    1290    1290
GP06-08    1624    1266    1266    1266   *1265
GP06-09    1607    1263    1285    1246    1264
GP06-10    1592    1265    1288    1262   *1254
GP07-01    1545    1186    1161    1159    1160
GP07-02    1488    1189    1238    1185    1191
GP07-03    1596    1242    1248    1237    1242
GP07-04    1512    1169    1212    1168    1167
GP07-05    1552    1191    1247    1158    1191
GP07-06    1504    1200    1248    1195    1200
GP07-07    1525    1186    1202    1185    1201
GP07-08    1293    1193    1196    1181    1183
GP07-09    1538    1222    1232    1220    1220
GP07-10    1589    1272    1273   *1270   *1270
GP08-01    1436    1179    1190    1160    1195
GP08-02    1494    1158    1208    1137    1197
GP08-03    1461    1139    1171    1115    1154
GP08-04    1474    1207    1208    1170    1168
GP08-05    1473    1220    1236   *1218   *1218
GP08-06    1479    1179    1249    1147    1171
GP08-07    1556    1185    1271    1135    1127
GP08-08    1428    1178    1208    1148    1191
GP08-09    1350    1136    1154    1122    1142
GP08-10    1405    1178    1184    1162    1161
GP09-01    1426    1165    1218    1149    1150
GP09-02    1400    1139    1201    1126    1226
GP09-03    1457    1149    1214    1118    1150
GP09-04    1498    1179    1222    1151    1181
GP09-05    1524    1184    1207    1181   *1180 
GP09-06    1200    1153    1134    1122    1136
GP09-07    1447    1132    1168    1124    1173
GP09-08    1440    1165    1205    1111    1193
GP09-09    1513    1182    1182    1138    1218
GP09-10    1471    1146    1192    1131    1166
GP10-01    1151    1145    1161    1113    1151
GP10-02    1477    1144    1208    1122    1178
GP10-03    1512    1160    1185    1102    1162
GP10-04    1244    1144    1142    1100    1165
GP10-05    1356    1125    1175    1103    1125
GP10-06    1448    1144    1231    1115    1173
GP10-07    1487    1138    1220    1103    1172
GP10-08    1313    1131    1185    1105    1181
GP10-09    1403    1148    1203    1130    1184
GP10-10    1237    1169    1247    1127    1172
