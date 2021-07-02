IloInt getTDTSPCost(cp, tour){
  IloInt s1=0, s2, counter=0, totalTime=0, startS1, endS1, cost; 
  for(IloIntervalVar a = cp.getFirst(tour); a.getImpl()!=0; a =cp.getNext(tour, a)){ 
    startS1=cp.getStart(a);
    endS1=cp.getEnd(a);
    s1= atoi(a.getName());
    if(s1==_nbVertices) s1=0;
// cout << "total time: " << totalTime << endl;
    if(a.getImpl() != cp.getFirst(tour).getImpl()){
      cost= _cost[s2][s1][totalTime/_lengthTimeWindow];
// cout << "possible cost: " << cost << endl;
      for(IloInt t=totalTime; t < totalTime+cost; t++)
        if((_cost[s2][s1][t/_lengthTimeWindow]+ t - totalTime) < cost){
          cost= t-totalTime + _cost[s2][s1][t/_lengthTimeWindow];
// cout << "cost modif ---> " << cost << " at time " << t << endl;
// cout << "waiting time ---> " << t-totalTime << endl;
        }
      totalTime+= cost;
    }
    counter++;
    if(counter== _nbVertices+1){
      cout <<"# BEST TD SOLUTION: " << totalTime << std::endl;
    }
    else if(REAL_INDEX) cout << _visits[s1] << " " << totalTime << std::endl;
    else cout << s1 << " " << totalTime<< std::endl;
    totalTime+= (endS1- startS1);
    s2=s1;
  }
}