// -------------------------------------------------------------- -*- C++ -*-
// File: ttime.cpp
// --------------------------------------------------------------------------
//
// IBM Confidential
// OCO Source Materials
// 5724-Y48 5724-Y49 5724-Y54 5724-Y55 5725-A06 5725-A29
// Copyright IBM Corp. 1990, 2013
// The source code for this program is not published or otherwise
// divested of its trade secrets, irrespective of what has
// been deposited with the U.S. Copyright Office.
//
// ---------------------------------------------------------------------------

// Classe abstraite avec des fonctions virtuelles a definir selon les besoins 
// de la propagation qui permet d'interfacer facilement differents types de 
// represantation de la matrice dependant du temps matrice 3D, 
// matrice de stepFunction, matrice de pwlFunction et les sous-classes 
// de cette classe virtuelle implementant la connection a une representation 
// particuliere.

#ifndef __CP_ttimeH
#include <../team/TDTSP/include/ttime.h>
#endif

#define EPSILON (1e-6)

// #define TIVERIF
// #define FIFOVERIF
// #define INVVERIF
// #define TINYSEGDETECTED
// #define TINYSEGVERIF
// #define NEGTIMEVERIF
// #define PARANOID



void DisplayGnuplot(IloNumToNumSegmentFunction g, ILOSTD(ostream)& str) {
  if (g.getName() != 0)
    str << "#\t" << g.getName() << ILOSTD(endl);
  IloEnv env0;
  for (IloNumToNumSegmentFunctionCursor csor(g); csor.ok(); ++csor) {
    IloNum xmin = csor.getSegmentMin();
    IloNum xmax = csor.getSegmentMax();
    if (xmin > -IloInfinity) {
      str << xmin
          << "\t" << csor.getValueLeft()
          << ILOSTD(endl);
    }
    if (xmax < +IloInfinity) {
      str << xmax
          << "\t" << csor.getValue(xmax)
          << ILOSTD(endl);
    }
  }
  env0.end();
}


void DisplayTikz(IloNumToNumSegmentFunction g, ILOSTD(ostream)& str) {
  IloEnv env0;
  for (IloNumToNumSegmentFunctionCursor csor(g); csor.ok(); ++csor) {
    IloNum xmin = csor.getSegmentMin();
    IloNum xmax = csor.getSegmentMax();
    if (xmin > -IloInfinity) {
      str << "(" << xmin
          << "," << csor.getValueLeft()
          << ")" << ILOSTD(endl);
    }
    if (xmax < +IloInfinity) {
      str << "(" << xmax
          << "," << csor.getValue(xmax)
          << ")" << ILOSTD(endl);
    }
  }
  env0.end();
}

// Get the function value at t-epsilon
IloNum GetValueLeft(IloNumToNumSegmentFunction f, IloNum t) {
  IloNumToNumSegmentFunctionCursor csor(f, t);
  if (csor.getSegmentMin() < t) { 
    return csor.getValue(t);
  } else {
    csor--;
    return csor.getValueRight();
  }
} 

void IloTDSegmentFunctionMatrix::printFIFOTIbench(){
  std::cout << _nbNodes << " " << _nbTimeSteps << " " << _sizeTimeStep << std::endl;
  for(IloInt i=0; i<_nbNodes; i++){
      for(IloInt j=0; j<_nbNodes; j++){
        for(IloInt k=0; k<_nbTimeSteps; k++){
          std::cout << _succEarliestTT[i][j].getValue(k*_sizeTimeStep) << " ";
        }
        std::cout << std::endl;
      }
      std::cout << std::endl;
  }
}

int lookForTinySegments(IloNumToNumSegmentFunction fnc, IloNum start, IloNum end, IloBool print){
  IloNumToNumSegmentFunctionCursor cur= IloNumToNumSegmentFunctionCursor(fnc, start);
  while(cur.getSegmentMin() < end){
    if(cur.getSegmentMin() + EPSILON > cur.getSegmentMax()){
      std::cout << "tiny segment @" << cur.getSegmentMin() << std::endl;
      if(print) std::cout << fnc << std::endl;
      return 1;
    }
    cur++;
  } 
  return 0;
}

int lookForNegativeTimes(IloNumToNumSegmentFunction fnc, IloNum start, IloNum end, IloBool print){
  IloNumToNumSegmentFunctionCursor cur= IloNumToNumSegmentFunctionCursor(fnc, start);
  while(cur.getSegmentMin() < end){
    if(cur.getSegmentMin()>=0 && cur.getValueLeft() < 0){
      std::cout << "negative value @" << cur.getSegmentMin() << std::endl;
      if(print) std::cout << fnc << std::endl;
      return 1;
    }
    if(cur.getSegmentMax()>=0 && cur.getValueRight() < 0){
      std::cout << "negative value @" << cur.getSegmentMax() << std::endl;
      if(print) std::cout << fnc << std::endl;
      return 1;
    }
    cur++;
  } 
  return 0;
}

int verifyTITT(IloNumToNumSegmentFunction fncij, 
         IloNumToNumSegmentFunction fncik, 
         IloNumToNumSegmentFunction fnckj, 
         IloNum start, IloNum end){
#if defined(TINYSEGVERIF)
  if(lookForTinySegments(fncij, start, end, false)) std::cout << "at TITT verification" << std::endl;
  if(lookForTinySegments(fncik, start, end, false)) std::cout << "at TITT verification" << std::endl;
  if(lookForTinySegments(fnckj, start, end, false)) std::cout << "at TITT verification" << std::endl;
#endif    
  IloNum curValue, possValue; 
  for(IloNum t= start; t< end; t+=1){
    curValue= fncij.getValue(t);
    possValue= IloIntervalMax;
    if(fncik.getValue(t)+t < end)
      possValue= fnckj.getValue(fncik.getValue(t)+t) + fncik.getValue(t);
    if(possValue + EPSILON < curValue){
      std::cout << "ERROR AT CALCULATION OF TI FUNCTION @" << t<< " CALCULATED "<< curValue << " CORRECT " << possValue  << std::endl;
      std::cout << "ik "<<  fncik.getValue(t) << std::endl;
      std::cout << fncik << std::endl;
      
      std::cout << "kj "<<  fnckj.getValue(fncik.getValue(t)+t) << std::endl;
      std::cout << fnckj << std::endl;
      
      std::cout << "ij "<<  fncij.getValue(t) << std::endl;
      std::cout << fncij << std::endl;
      return 0;
    }
  }
  return 1;
}

int verifyFIFOTT(IloNumToNumSegmentFunction fnc, 
                 IloNumToNumSegmentFunction fncFIFO, 
                 IloNum start, IloNum end){
#if defined(TINYSEGVERIF)
  if(lookForTinySegments(fncFIFO, start, end, false)) 
	  std::cout << "at FIFOTT verification" << std::endl;
#endif                  
  IloInt curValue, possValue; 
  for(IloNum t= start; t< end; t+=1){
    curValue= ceil(fncFIFO.getValue(t));
    for(IloNum deltat=0; deltat<end-t; deltat+=1){
      possValue= deltat + ceil(fnc.getValue(t+deltat));
      if(possValue < curValue){
        std::cout << "ERROR AT CALCULATION OF COSTS FUNCTION @" << t<< " CALCULATED "<< curValue << " CORRECT " << possValue  << " DELTAT " << deltat << std::endl;
        return -1;
      }
    }
  }
  return 0;
}

int verifyInverseTT(IloNumToNumSegmentFunction fnc, 
                    IloNumToNumSegmentFunction inverse, 
                    IloNum start, IloNum end){
#if defined(TINYSEGVERIF)
  if(lookForTinySegments(inverse, start, end, false)) std::cout << "at InverseTT verification" << std::endl;
#endif 
   
  IloNum curValue, possValue; 
  for(IloNum t=start; t<end; t+=1){
    curValue= inverse.getValue(t);
    for(IloNum deltat=0; deltat<curValue; deltat++){
      possValue=fnc.getValue(t-deltat);
      if(possValue + EPSILON < deltat){
        std::cout << "ERROR AT CALCULATION OF INVERSE COSTS FUNCTION @" << t<< " CALCULATED "<< (float) curValue << " CORRECT " << (float) possValue << " DELTAT " << deltat << std::endl;
        return -1;
      }
    }
  }
  return 0;
}


IloTD3DMatrixI::IloTD3DMatrixI(IloEnv env, IloIntArray3 costs, IloInt sizeTS) 
  :IloTDMatrixI  (env)
  ,_nbNodes      (costs.getSize())
  ,_nbTimeSteps  (0)
  ,_sizeTimeStep (sizeTS) 
  ,_costs        (NULL)
  {
  assert(0 < _nbNodes);
  _nbTimeSteps=costs[0][0].getSize();
#ifndef NDEBUG
  for(IloInt i=0; i<_nbNodes; i++){
    assert(costs[i].getImpl()!=0);
    assert(costs[i].getSize()==_nbNodes);
    for(IloInt j=0; j<_nbNodes; j++){
      assert(costs[i][j].getImpl()!= 0);
      assert(costs[i][j].getSize()== _nbTimeSteps);
    }
  }
#endif
  _costs= new (env) IloInt**[_nbNodes];
  for(IloInt i=0; i<_nbNodes; i++){
    _costs[i]= new (env) IloInt*[_nbNodes];
    for(IloInt j=0; j<_nbNodes; j++){
      _costs[i][j]= new (env) IloInt[_nbTimeSteps];
      for(IloInt k=0; k<_nbTimeSteps; k++){
        _costs[i][j][k]=costs[i][j][k];
      }
    }
  }
}

void IloTDSegmentFunctionMatrix::FIFOTT(IloInt i, IloInt j, 
                                       IloNumToNumSegmentFunction* res, 
                                       const IloNumToNumSegmentFunction costs){
  IloEnv env = getEnv();
  IloNumToNumSegmentFunction FIFOfnc = IloNumToNumSegmentFunction(env);
  FIFOfnc= costs.copy();
  IloNumToNumSegmentFunctionCursor cur= IloNumToNumSegmentFunctionCursor(costs, _start);
  IloNum x0, x1, v0, v1;
  x0= cur.getSegmentMax();
  v0= cur.getValueRight();
  ++cur;
  while(cur.getSegmentMax() < _horizon){
    x1=cur.getSegmentMin();
    v1= cur.getValueLeft();
    if(v1 + EPSILON < v0){
      FIFOfnc.setMin(0, v1+x1, x1,v1);
    }
    else if(v1 < v0){
      FIFOfnc.setSlope(x1, cur.getSegmentMax(),v0,(v1-cur.getValueRight())/(cur.getSegmentMax()-x1));
    }
    if( (v1-cur.getValueRight())/(cur.getSegmentMax()-x1) < -1 ){
      v1=cur.getValueRight();
      x1= cur.getSegmentMax();
      FIFOfnc.setMin(0, v1+x1, x1,v1);
    }
    x0= cur.getSegmentMax();
    v0= cur.getValueRight();
    ++cur;
  }

  *res= FIFOfnc.copy();
  FIFOfnc.end();
}

void IloTDSegmentFunctionMatrix::InverseTT(IloInt i, IloInt j, 
                                           IloNumToNumSegmentFunction* res, 
                                           const IloNumToNumSegmentFunction costs){
  IloEnv env = getEnv();
  IloNumToNumSegmentFunction arrivalTime = IloNumToNumSegmentFunction(env);
  IloNumToNumSegmentFunction inverse = IloNumToNumSegmentFunction(env);
  IloNum x0, x1=0, v0, v1=0, prevX1=-1, prevV1=-1;
  arrivalTime.setSlope(0, _horizon, 0, 1);
  arrivalTime+= costs;
  IloNumToNumSegmentFunctionCursor cur = IloNumToNumSegmentFunctionCursor(arrivalTime, _start);
  while(cur.getSegmentMax()<=_horizon){
    x0= cur.getSegmentMin();
    v0= cur.getValueLeft();
    if(x1 !=0 && v1!=0){
#if defined(TINYSEGDETECTED)
  if(x0!=x1){
    std::cout << "x0!=x1" << std::endl;
  }
#endif
      if(v0!=v1 && v0-v1 > 0 && v0-v1 < EPSILON){
#if defined(TINYSEGDETECTED)
  std::cout << "v0-v1 < EPSILON " << v0 << " " << v1 << std::endl;
#endif
        v0=v1;
      }
      if(v0!=v1 && v1-v0 > 0 && v1-v0 < EPSILON){
#if defined(TINYSEGDETECTED)
  std::cout << "v1-v0 < EPSILON " << v0 << " " << v1 << std::endl;
#endif  
        v0=v1;
      }
    }
    x1=cur.getSegmentMax();
    v1= cur.getValueRight();
    // inverse.setMax(v0, x0, v1, x1);
    
    
    IloNumToNumSegmentFunctionCursor curAux1(inverse, v0);
 IloNumToNumSegmentFunctionCursor curAux2(inverse, v1);
 IloNum tMin1=curAux1.getSegmentMin(), tMax1= curAux1.getSegmentMax(), newv0=v0, newv1=v1;
 if(v0!=tMin1 && tMin1 > v0 - EPSILON){
   newv0=tMin1; 
   x0-= EPSILON;
 }
 if(v0!=tMax1 && tMax1 < v0 + EPSILON){
   newv0= tMax1;
 }
 IloNum tMin=curAux2.getSegmentMin(), tMax= curAux2.getSegmentMax();
 if(v1!=tMin && tMin > v1 - EPSILON){
   newv1=tMin; 
 }
 if(v1!=tMax && tMax < v1 + EPSILON){
   newv1= tMax;
   x1+= EPSILON;
 }
 IloNum vnewv0, vnewv1;
 vnewv0=inverse.getValue(newv0);
 vnewv1=inverse.getValue(newv1);
 if(newv1 < IloIntervalMax/10)
  inverse.setMax(newv0,x0,newv1,x1);
 else if (vnewv0 > x0) inverse.setSlope(newv0,newv1,x0, 1);
    
    
    inverse.setMax(v1, _horizon, x1);
#if defined(TINYSEGVERIF)
	if(lookForTinySegments(inverse, 0, _horizon, false)){
    std::cout << "Inside InverseTT, type 1, i,j: "<<i<<" "<<j << std::endl;
  }
#endif     

    ++cur;
  }
  /*
  x0= cur.getSegmentMin();
  v0= cur.getValueLeft();
   if(x1 !=0 && v1!=0){
#if defined(TINYSEGDETECTED)
  if(x0!=x1){
    std::cout << "x0!=x1" << std::endl;
  }
#endif
      if(v0!=v1 && v0-v1 > 0 && v0-v1 < EPSILON){
#if defined(TINYSEGDETECTED)
  std::cout << "v0-v1 < EPSILON " << v0 << " " << v1 << std::endl;
#endif
        v0=v1;
      }
      if(v0!=v1 && v1-v0 > 0 && v1-v0 < EPSILON){
#if defined(TINYSEGDETECTED)
  std::cout << "v1-v0 < EPSILON " << v0 << " " << v1 << std::endl;
#endif      
       v0=v1;
      }
    }
  x1=cur.getSegmentMax();
  v1= cur.getValueRight();
  // inverse.setMax(v0,x0, v1, x1);
  
 IloNumToNumSegmentFunctionCursor curAux1(inverse, v0);
 IloNumToNumSegmentFunctionCursor curAux2(inverse, v1);
 IloNum tMin1=curAux1.getSegmentMin(), tMax1= curAux1.getSegmentMax(), newv0=v0, newv1=v1;
 if(v0!=tMin1 && tMin1 > v0 - EPSILON){
   newv0=tMin1; 
   x0-= EPSILON;
 }
 if(v0!=tMax1 && tMax1 < v0 + EPSILON){
   newv0= tMax1;
 }
 IloNum tMin=curAux2.getSegmentMin(), tMax= curAux2.getSegmentMax();
 if(v1!=tMin && tMin > v1 - EPSILON){
   newv1=tMin; 
 }
 if(v1!=tMax && tMax < v1 + EPSILON){
   newv1= tMax;
   x1+= EPSILON;
 }
 IloNum vnewv0, vnewv1;
 vnewv0=inverse.getValue(newv0);
 vnewv1=inverse.getValue(newv1);
 if(newv1 < IloIntervalMax/10)
  inverse.setMax(newv0,x0,newv1,x1);
 else if (vnewv0 > x0) inverse.setSlope(newv0,newv1,x0, 1);
  
  
  
  
  inverse.setMax(v1, _horizon, x1);
#if defined(TINYSEGVERIF)
	if(lookForTinySegments(inverse, 0, _horizon, false)){
    std::cout << "Inside InverseTT, type 2, i,j: "<<i<<" "<<j << std::endl;
  }
#endif  */
   
  arrivalTime.setSlope(0, _horizon, 0, 1);
  arrivalTime-=inverse;
  // arrivalTime.setValue(_horizon, IloIntervalMax, arrivalTime.getValue(_horizon));
  *res= arrivalTime.copy();
  arrivalTime.end();
  inverse.end();
}


void IloTDSegmentFunctionMatrix::minTT(IloInt k, IloInt i, IloInt j){

  IloEnv env = getEnv();
  IloNumToNumSegmentFunction identity= IloNumToNumSegmentFunction(env, _start, _horizon, 0);
  identity.setSlope(_start, _horizon, _start, 1);
  IloNumToNumSegmentFunction arrivalAtJFromI= IloNumToNumSegmentFunction(env, _start, _horizon, 0);
  arrivalAtJFromI.setSlope(_start, _horizon, _start, 1);
  arrivalAtJFromI+= _succEarliestTT[i][j];
  IloNumToNumSegmentFunction arrivalAtKFromI= IloNumToNumSegmentFunction(env, _start, _horizon, 0);
  IloNumToNumSegmentFunction arrivalAtJFromK= IloNumToNumSegmentFunction(env, _start, _horizon, 0);
  IloNumToNumSegmentFunction arrivalAtJFromIThroughK= IloNumToNumSegmentFunction(env);
  arrivalAtKFromI.setSlope(_start, _horizon, _start, 1);
  arrivalAtJFromK.setSlope(_start, _horizon, _start, 1);
  arrivalAtJFromIThroughK.setSlope(_start, _horizon, _start, 1);
  arrivalAtKFromI+= _succEarliestTT[i][k];
  arrivalAtJFromK+= _succEarliestTT[k][j];
  IloNumToNumSegmentFunctionCursor curK= IloNumToNumSegmentFunctionCursor(arrivalAtKFromI, _start);  
  IloNum tStart, tEnd, tKStart, tKEnd, curKSlope, previousEnd=_start, previousEndValue;
  IloInt nbSegs=0;
  while(curK.getSegmentMin() < _horizon){
    tStart= curK.getSegmentMin();
    tEnd= curK.getSegmentMax();
    tKStart= curK.getValueLeft();
    tKEnd= curK.getValueRight();

    if(tKStart+ EPSILON < tKEnd && tKStart < _horizon){
      curKSlope= (tKEnd - tKStart)/(tEnd - tStart);
      IloNumToNumSegmentFunctionCursor curJ = IloNumToNumSegmentFunctionCursor(arrivalAtJFromK, tKStart);  
      IloNum tJStart, tJEnd, tKStartIt, tKEndIt, x0, x1, previousX0=-1, previousX1=-1, curJSlope;
      if(curJ.getSegmentMax() < tKStart + EPSILON) curJ++;
      while(curJ.getSegmentMin() + EPSILON < tKEnd){
        tKStartIt= curJ.getSegmentMin();
        if(tKStartIt < tKStart){
          tKStartIt= tKStart;
          tJStart= curJ.getValue(tKStartIt);
        } else tJStart= curJ.getValueLeft();
        
        tKEndIt= curJ.getSegmentMax();
        if(tKEndIt > tKEnd){
          tKEndIt= tKEnd;
          tJEnd= curJ.getValue(tKEndIt);
        } else tJEnd= curJ.getValueRight();
        curJSlope= (tJEnd-tJStart)/(tKEndIt- tKStartIt);
        x0= (tKStartIt-tKStart)/curKSlope + tStart;
        if(previousX1!=-1 && x0 != previousX1) x0= previousX1;
        if( (tStart < x0 && tStart + EPSILON > x0) || (tStart > x0 && tStart - EPSILON < x0)) x0= tStart; 
        x1= (tKEndIt-tKStart)/curKSlope + tStart;
        if( (tEnd < x1 && tEnd + EPSILON > x1) || (tEnd > x1 && tEnd - EPSILON < x1)) x1= tEnd; 
        IloNum currentValueAtX0=arrivalAtJFromIThroughK.getValue(x0);
        if((currentValueAtX0 > tJStart && currentValueAtX0 < tJStart + EPSILON) || (currentValueAtX0 < tJStart && currentValueAtX0 > tJStart - EPSILON )) tJStart= currentValueAtX0;
        if(x1 - x0 < EPSILON){
#if defined(TINYSEGDETECTED)
  if(lookForTinySegments(arrivalAtJFromI, 0, _horizon, false)){
	 std::cout << "Avoiding to introduce tiny segment inside MinTT of type bizarre1" << std::endl;
  }
#endif  
        }
        else if(curJSlope*curKSlope > 1 && curJSlope*curKSlope < 1+EPSILON){
// std::cout << arrivalAtJFromIThroughK << std::endl;
          arrivalAtJFromIThroughK.setSlope(x0,x1, tJStart, 1);
#if defined(TINYSEGDETECTED)
  if(lookForTinySegments(arrivalAtJFromIThroughK, 0, _horizon, true)){
	 std::cout << "else if bizarre 1" << std::endl;
  }
#endif            
        }
        else{
// std::cout << arrivalAtJFromIThroughK << std::endl;          
          arrivalAtJFromIThroughK.setSlope(x0,x1, tJStart, curJSlope*curKSlope);

#if defined(TINYSEGDETECTED)
  if(lookForTinySegments(arrivalAtJFromIThroughK, 0, _horizon, true)){
	 std::cout << "else bizarre 1" << std::endl;
  }
#endif            
        }
        ++curJ;
        nbSegs++;
      }
      if(curJ.getSegmentMin() < tKEnd){
        arrivalAtJFromIThroughK.setSlope(x0,tEnd, tJStart, curJSlope*curKSlope);
        previousX1= tEnd;
      }
    } 
    else if(tKStart < _horizon){
      
      if(tEnd - tStart < EPSILON){
#if defined(TINYSEGDETECTED)
      if(lookForTinySegments(arrivalAtJFromI, 0, _horizon, false)){
       std::cout << "Avoiding to introduce tiny segment inside MinTT of type bizarre2" << std::endl;
      }
#endif  
      } 
      else arrivalAtJFromIThroughK.setValue(tStart , tEnd, arrivalAtJFromK.getValue(tKStart));
    }
    previousEnd= tEnd;
    previousEndValue= tKEnd;
    ++curK;
  }
  if(GetValueLeft(arrivalAtJFromIThroughK, previousEnd)==0){
    arrivalAtJFromIThroughK+= _succEarliestTT[i][k] + _succEarliestTT[k][j];
  } 
  else {
    arrivalAtJFromIThroughK.setSlope(previousEnd, _horizon
    , GetValueLeft(arrivalAtJFromIThroughK, previousEnd), 1);
    // , arrivalAtJFromK.getValue(previousEndValue), 1);
  }

  // if(i==4 && j==21 && k==3){
    // std::cout <<  "Inside MINTT: arrivalAtJFromIThroughK" << std::endl;
    // std::cout <<  arrivalAtJFromIThroughK << std::endl;
	// std::cout <<  "arrivalAtJFromI" << std::endl;
    // std::cout <<  arrivalAtJFromI << std::endl;
  // }

  IloNumToNumSegmentFunctionCursor cur0(arrivalAtJFromIThroughK, _start);
  IloNum xs, xe, vs, ve, slope;
  while(cur0.getSegmentMax() < _horizon){
   xs= cur0.getSegmentMin();
   xe= cur0.getSegmentMax();
   vs= cur0.getValueLeft();
   ve= cur0.getValueRight();
   slope= (ve-vs)/(xe-xs);
   IloNumToNumSegmentFunctionCursor curAux1(arrivalAtJFromI, xs);
   IloNumToNumSegmentFunctionCursor curAux2(arrivalAtJFromI, xe);
   IloNum tMin1=curAux1.getSegmentMin(), tMax1= curAux1.getSegmentMax(), newxs=xs, newxe=xe;
   if(xs!=tMin1 && tMin1 > xs - EPSILON){
     newxs=tMin1; 
	 vs-= EPSILON;
   }
   if(xs!=tMax1 && tMax1 < xs + EPSILON){
     newxs= tMax1;
   }
   IloNum tMin=curAux2.getSegmentMin(), tMax= curAux2.getSegmentMax();
   if(xe!=tMin && tMin > xe - EPSILON){
     newxe=tMin; 
   }
   if(xe!=tMax && tMax < xe + EPSILON){
     newxe= tMax;
     ve+= EPSILON;
   }
   IloNum vnewxs, vnewxe;
   vnewxs=arrivalAtJFromI.getValue(newxs);
   vnewxe=arrivalAtJFromI.getValue(newxe);
   if(vnewxs < vs && vnewxe > ve){
     
   }
   if(newxe < IloIntervalMax/10)
    arrivalAtJFromI.setMin(newxs,vs,newxe,ve);
   else if (vnewxs > vs) arrivalAtJFromI.setSlope(newxs,newxe,vs, 1);
   
#if defined(TINYSEGDETECTED)
 if(lookForTinySegments(arrivalAtJFromI, 0, _horizon, true)){
   std::cout << "MinTT " <<  i << " " << k << " " << j << " nbSegs " << nbSegs << std::endl;
	 std::cout << "tiny segment introduced inside MinTT, in function " << arrivalAtJFromI << std::endl;
  }
#endif   
   // std::cout << "SetMin " <<  newxs << " " << newxe << " " << vs << " " << ve << std::endl;
   // std::cout << "Function became " << arrivalAtJFromI << std::endl;
   cur0++;
  }  
  
  /*if(i==3 && j==9 && k==2){
    std::cout <<  "arrivalAtJFromI after setMin" << std::endl;
    std::cout <<  arrivalAtJFromI << std::endl;
  }*/
  arrivalAtJFromI-= identity;
  arrivalAtJFromIThroughK.end();
  arrivalAtJFromK.end();
  arrivalAtKFromI.end();
  identity.end();
  
  _succEarliestTT[i][j].end();
  _succEarliestTT[i][j]= arrivalAtJFromI.copy();
  
#if defined(TINYSEGDETECTED)
 if(lookForTinySegments(arrivalAtJFromI, 0, _horizon, true)){
   std::cout << "MinTT " <<  i << " " << k << " " << j << " nbSegs " << nbSegs << std::endl;
	 std::cout << "tiny segment introduced inside MinTT, in function " << arrivalAtJFromI << std::endl;
  }
#endif    

#if defined(NEGTIMEVERIF)
 if(lookForNegativeTimes(arrivalAtJFromI, 0, _horizon, false)){
	 std::cout << "negative introduced inside MinTT, in function " << arrivalAtJFromI;
	 std::cout<< std::endl;
  }
#endif    
 
  arrivalAtJFromI.end();
 
}

IloTDSegmentFunctionMatrix::IloTDSegmentFunctionMatrix(IloEnv env, IloIntArray3 costs, IloInt sizeTS) 
  :IloTDMatrixI    (env)
  ,_start          (0)
  ,_horizon        (0)
  ,_nbNodes        (costs.getSize())
  ,_sizeTimeStep   (sizeTS)
  ,_costMax        (0)
  ,_min            (NULL)
  ,_max            (NULL)
  ,_costs          (NULL)
  ,_nextEarliestTT (NULL)
  ,_nextLatestTT   (NULL)
  ,_succEarliestTT (NULL)
  ,_succLatestTT   (NULL)
  {
  assert(0 < _nbNodes);
  _nbTimeSteps= costs[0][0].getSize();
  _horizon = (IloNum)_nbTimeSteps*_sizeTimeStep;
#ifndef NDEBUG
  for(IloInt i=0; i<_nbNodes; i++){
    assert(costs[i].getImpl()!=0);
    assert(costs[i].getSize()==_nbNodes);
    for(IloInt j=0; j<_nbNodes; j++){
      assert(costs[i][j].getImpl()!= 0);
      assert(costs[i][j].getSize()== _nbTimeSteps);
    }
  }
#endif
  _nextEarliestTT = new (env) IloNumToNumSegmentFunction*[_nbNodes];
  _nextLatestTT   = new (env) IloNumToNumSegmentFunction*[_nbNodes];
  _succEarliestTT = new (env) IloNumToNumSegmentFunction*[_nbNodes];
  _succLatestTT   = new (env) IloNumToNumSegmentFunction*[_nbNodes];
  _costs          = new (env) IloNumToNumSegmentFunction*[_nbNodes];
  _max             = new (env) IloInt*[_nbNodes];
  _min             = new (env) IloInt*[_nbNodes];
  for(IloInt i=0; i<2; i++){
    _costsMinTo[i]   = new (env) IloInt[_nbNodes];
    _costsMinFrom[i] = new (env) IloInt[_nbNodes];
    for(IloInt j=0; j<_nbNodes; j++){
      _costsMinTo[i][j]   = IloIntMax;
      _costsMinFrom[i][j] = IloIntMax;
    }
  }
  for(IloInt i=0; i<_nbNodes; i++){
    _max[i] = new (env) IloInt[_nbNodes];
    _min[i] = new (env) IloInt[_nbNodes];
    for(IloInt j=0; j<_nbNodes; j++){
      _min[i][j]           = IloIntMax;
      _max[i][j]           = 0;
    }
  }
  IloNum v0, v1, x0, x1;
  for(IloInt i=0; i<_nbNodes; i++){
    _costs[i]= new (env) IloNumToNumSegmentFunction[_nbNodes];
    for(IloInt j=0; j<_nbNodes; j++){   
      _costs[i][j]= IloNumToNumSegmentFunction(env);
      // Filling _costs
      v0= (IloNum) costs[i][j][0];
      x0= (IloNum) _start; 
      for(IloInt k=1; k<_nbTimeSteps; k++){
        v1= (IloNum) costs[i][j][k];
        if (v1 > _costMax) _costMax= IloInt(v1);
        if(v1 > _max[i][j]) _max[i][j]= IloInt(v1); 
        if(v1 < _min[i][j]) _min[i][j]= IloInt(v1);
        x1= (IloNum) k*_sizeTimeStep;
        _costs[i][j].setValue(x0, x1, v0);
        v0=v1;
        x0=x1;
      } 
      _costs[i][j].setValue(x0, _horizon, v0);
    }
  }
  
  for(IloInt i=0; i<_nbNodes; i++){
    _nextEarliestTT [i] = new (env) IloNumToNumSegmentFunction[_nbNodes];
    _nextLatestTT   [i] = new (env) IloNumToNumSegmentFunction[_nbNodes];
    _succEarliestTT [i] = new (env) IloNumToNumSegmentFunction[_nbNodes];
    _succLatestTT   [i] = new (env) IloNumToNumSegmentFunction[_nbNodes];
    for(IloInt j=0; j<_nbNodes; j++){   
      _nextEarliestTT [i][j] = IloNumToNumSegmentFunction(env);
      _nextLatestTT   [i][j] = IloNumToNumSegmentFunction(env);
      _succEarliestTT [i][j] = IloNumToNumSegmentFunction(env);
      _succLatestTT   [i][j] = IloNumToNumSegmentFunction(env);
    }
  }
  
  IloInt a=1, b=2;
// printing function  
  // std::cout << "Printing original function" << std::endl;
  // DisplayTikz(_costs[a][b], std::cout);
  
  // std::cout << "Verifying FIFOTT with horizon " << _horizon << std::endl;
  // std::cout << "Verifying InverseTT" << std::endl;
  for(IloInt i=0; i<_nbNodes; i++){
    for(IloInt j=0; j<_nbNodes; j++){   
      FIFOTT(i,j, &_nextEarliestTT[i][j], _costs[i][j]);
       // verifyFIFOTT(_costs[i][j], _nextEarliestTT[i][j], 0, _horizon);
      _succEarliestTT[i][j]= _nextEarliestTT[i][j].copy();
      InverseTT(i,j, &_nextLatestTT[i][j], _nextEarliestTT[i][j]);
      verifyInverseTT(_nextEarliestTT[i][j], _nextLatestTT[i][j], 0, _horizon);
   
    }
  }
//printing function
  // std::cout << "Printing fifo function" << std::endl;
  // DisplayTikz(_nextEarliestTT[a][b], std::cout);
  
  
  // IloInt flag=0;
  // Verifying TI property
  for(IloInt k=0; k<_nbNodes; k++){  
    for(IloInt i=0; i<_nbNodes; i++){
      for(IloInt j=0; j<_nbNodes; j++){
        if(k!= i && k!=j && i!=j && (_min[i][k] + _min[k][j] <= _max[i][j])){
          // flag=1; add flag at if
          // DisplayTikz(_nextEarliestTT[i][j], std::cout);
          minTT(k, i, j);
          // std::cout << "Printing ti function" << std::endl;
          // DisplayTikz(_succEarliestTT[i][j], std::cout);
        } 
        // if(k!= i && k!=j && i!=j) minTT(k, i, j);
      }
    }   
  }
  
 /*/ DEGUGGING TI TT -------------------- 
 std::cout << "Verifying TITT" << std::endl;
 IloBool flag= true;
  for(IloInt i=0; i<_nbNodes; i++){
    for(IloInt j=0; j<_nbNodes; j++){
     for(IloInt k=0; k<_nbNodes; k++){  
      if(k!= i && k!=j && i!=j && flag){ 
        if(!verifyTITT(_succEarliestTT[i][j], 
                   _succEarliestTT[i][k], 
                   _succEarliestTT[k][j], 
                   0, _horizon)){
         // flag=false;
         std::cout << i << " " << k <<" " << j << std::endl;
         // DisplayGnuplot(_nextEarliestTT[i][j], std::cout);
         // DisplayGnuplot(_nextEarliestTT[i][k], std::cout);
         // DisplayGnuplot(_nextEarliestTT[k][j], std::cout);
         // DisplayTikz(_nextEarliestTT[i][j], std::cout);
         // std::cout << "Printing ti function" << std::endl;
         // DisplayGnuplot(_succEarliestTT[i][j], std::cout);
        }
      }
     }
    }   
  }
  //*/
  
  //printing function
  // std::cout << "Printing ti function" << std::endl;
  // DisplayTikz(_nextLatestTT[a][b], std::cout);
  
  
  
  
  
  for(IloInt i=0; i<_nbNodes; i++){
    for(IloInt j=0; j<_nbNodes; j++){   
      InverseTT(i,j, &_succLatestTT[i][j], _succEarliestTT[i][j]);
    }
  }

  for(IloInt i=0; i<_nbNodes; i++){
    for(IloInt j=0; j<_nbNodes; j++){   
      v1 = (IloNum) _succEarliestTT[i][j].getMin(0, _horizon);
      if(i!=j) {
        if(v1 < _costsMinFrom[0][i]) _costsMinFrom[0][i] = IloInt(v1);
        if(v1 < _costsMinTo[0][j])   _costsMinTo  [0][j] = IloInt(v1);
        if(v1 < _costsMinFrom[1][i]) _costsMinFrom[1][i] = IloInt(v1);
        if(v1 < _costsMinTo[1][j])   _costsMinTo  [1][j] = IloInt(v1);
      } else {
        if(v1 < _costsMinFrom[1][i]) _costsMinFrom[1][i] = IloInt(v1);
        if(v1 < _costsMinTo[1][j])   _costsMinTo  [1][j] = IloInt(v1);
      }
    }
  }
  
  // DEBUG  
  // compareSuccNext(); 
  
  /*print FIFO/TI bench 
  std::cout << _nbNodes << " " << _nbTimeSteps << " " << _sizeTimeStep << std::endl;
  for(IloInt i=0; i<_nbNodes; i++){
      for(IloInt j=0; j<_nbNodes; j++){
        for(IloInt k=0; k<_nbTimeSteps; k++){
          std::cout << _succEarliestTT[i][j].getValue(k*_sizeTimeStep) << " ";
        }
        std::cout << std::endl;
      }
      std::cout << std::endl;
  } 
  // */
  
}


IloTDSegmentFunctionMatrix::IloTDSegmentFunctionMatrix(IloEnv env, IloNumToNumSegmentFunction** costs, IloInt nbVertices, IloNum Horizon) 
  :IloTDMatrixI    (env)
  ,_start          (0)
  ,_horizon        (Horizon)
  ,_nbNodes        (nbVertices)
  ,_sizeTimeStep   (0)
  ,_costMax        (0)
  ,_min            (NULL)
  ,_max            (NULL)
  ,_costs          (costs)
  ,_nextEarliestTT (NULL)
  ,_nextLatestTT   (NULL)
  ,_succEarliestTT (NULL)
  ,_succLatestTT   (NULL)
  {
  assert(0 < _nbNodes);
#ifndef NDEBUG
  for(IloInt i=0; i<_nbNodes; i++){
    assert(costs[i]!=NULL);
    for(IloInt j=0; j<_nbNodes; j++){
      assert(costs[i][j].getImpl()!= 0);
    }
  }
#endif
  
  _nextEarliestTT = new (env) IloNumToNumSegmentFunction*[_nbNodes];
  _nextLatestTT   = new (env) IloNumToNumSegmentFunction*[_nbNodes];
  _succEarliestTT = new (env) IloNumToNumSegmentFunction*[_nbNodes];
  _succLatestTT   = new (env) IloNumToNumSegmentFunction*[_nbNodes];
  _max             = new (env) IloInt*[_nbNodes];
  _min             = new (env) IloInt*[_nbNodes];
  for(IloInt i=0; i<2; i++){
    _costsMinTo[i]   = new (env) IloInt[_nbNodes];
    _costsMinFrom[i] = new (env) IloInt[_nbNodes];
    for(IloInt j=0; j<_nbNodes; j++){
      _costsMinTo[i][j]   = IloIntMax;
      _costsMinFrom[i][j] = IloIntMax;
    }
  }
  for(IloInt i=0; i<_nbNodes; i++){
    _max[i] = new (env) IloInt[_nbNodes];
    _min[i] = new (env) IloInt[_nbNodes];
    for(IloInt j=0; j<_nbNodes; j++){
      _min[i][j]           = _costs[i][j].getMin(0, _horizon);
      _max[i][j]           = _costs[i][j].getMax(0, _horizon);
      if(_max[i][j] > _costMax) _costMax= _max[i][j];
#if defined(TINYSEGDETECTED)
      if(lookForTinySegments(_costs[i][j], 0, _horizon, true)) 
        std::cout << "Tinyseg in _costs" << i << "," << j << std::endl;
#endif          
    }
  }
  
  
  for(IloInt i=0; i<_nbNodes; i++){
    _nextEarliestTT [i] = new (env) IloNumToNumSegmentFunction[_nbNodes];
    _nextLatestTT   [i] = new (env) IloNumToNumSegmentFunction[_nbNodes];
    _succEarliestTT [i] = new (env) IloNumToNumSegmentFunction[_nbNodes];
    _succLatestTT   [i] = new (env) IloNumToNumSegmentFunction[_nbNodes];
    for(IloInt j=0; j<_nbNodes; j++){   
      _nextEarliestTT [i][j] = IloNumToNumSegmentFunction(env, 0, _horizon, 0);
      _nextLatestTT   [i][j] = IloNumToNumSegmentFunction(env, 0, _horizon, 0);
      _succEarliestTT [i][j] = IloNumToNumSegmentFunction(env, 0, _horizon, 0);
      _succLatestTT   [i][j] = IloNumToNumSegmentFunction(env, 0, _horizon, 0);
    }
  }
  
#if defined(FIFOVERIF)
  std::cout << "Verifying FIFOTT" << std::endl;
#endif  
#if defined(INVVERIF)
  std::cout << "Verifying InverseTT" << std::endl;
#endif 
   for(IloInt i=0; i<_nbNodes; i++){
    for(IloInt j=0; j<_nbNodes; j++){  
      FIFOTT(i,j, &_nextEarliestTT[i][j], _costs[i][j]);
#if defined(FIFOVERIF)
      verifyFIFOTT(_costs[i][j], _nextEarliestTT[i][j], 0, _horizon);
#endif  
      _succEarliestTT[i][j]= _nextEarliestTT[i][j].copy();
      // InverseTT(i,j, &_nextLatestTT[i][j], _nextEarliestTT[i][j]);
#if defined(INVVERIF)
      // if(verifyInverseTT(_nextEarliestTT[i][j], _nextLatestTT[i][j], 0, _horizon)) std::cout << "From " <<i << " to " << j << std::endl;
#endif 
    
    }
  }
  
  // std::cout << "Min TITT" << std::endl;
  for(IloInt k=0; k<_nbNodes; k++){  
    for(IloInt i=0; i<_nbNodes; i++){
      for(IloInt j=0; j<_nbNodes; j++){
        if(k!= i && k!=j && i!=j && (_min[i][k] + _min[k][j] <= _max[i][j])){
          minTT(k, i, j);
        } 
      }
    }   
  }

#if defined(TIVERIF)
 std::cout << "Verifying TITT with horizon " << _horizon << std::endl;
  for(IloInt i=0; i<_nbNodes; i++){
    for(IloInt j=0; j<_nbNodes; j++){
     for(IloInt k=0; k<_nbNodes; k++){  
      if(k!= i && k!=j && i!=j){ 
        if(!verifyTITT(_succEarliestTT[i][j], 
                   _succEarliestTT[i][k], 
                   _succEarliestTT[k][j], 
                   0, _horizon)){
          std::cout << i << " " << j << " " << k << std::endl;
      
        }
      }
     }
    }   
  }
#endif  
  
  for(IloInt i=0; i<_nbNodes; i++){
    for(IloInt j=0; j<_nbNodes; j++){   
      InverseTT(i,j, &_succLatestTT[i][j], _succEarliestTT[i][j]);
#if defined(INVVERIF)
      if(verifyInverseTT(_succEarliestTT[i][j], _succLatestTT[i][j], 0, _horizon)) std::cout << "From " <<i << " to " << j << std::endl;
#endif 
    }
  }

  IloNum v1;
  for(IloInt i=0; i<_nbNodes; i++){
    for(IloInt j=0; j<_nbNodes; j++){   
      v1 = _succEarliestTT[i][j].getMin(0, _horizon);
      if(i!=j) {
        if(v1 < _costsMinFrom[0][i]) _costsMinFrom[0][i] = IloInt(v1);
        if(v1 < _costsMinTo[0][j])   _costsMinTo  [0][j] = IloInt(v1);
        if(v1 < _costsMinFrom[1][i]) _costsMinFrom[1][i] = IloInt(v1);
        if(v1 < _costsMinTo[1][j])   _costsMinTo  [1][j] = IloInt(v1);
      } else {
        if(v1 < _costsMinFrom[1][i]) _costsMinFrom[1][i] = IloInt(v1);
        if(v1 < _costsMinTo[1][j])   _costsMinTo  [1][j] = IloInt(v1);
      }
    }
  }
  
#if defined(PARANOID)
  compareSuccNext(); 
#endif
  
  // printFIFOTIbench();
}


void IloTDSegmentFunctionMatrix::compareSuccNext(){
  IloNum nextv, succv;
  for(IloInt i=0; i<_nbNodes; i++){
    for(IloInt j=0; j<_nbNodes; j++){   
      for(IloInt t=0; t < _horizon; t++){
        nextv = _nextEarliestTT[i][j].getValue(IloNum(t));
        succv = _succEarliestTT[i][j].getValue(IloNum(t));
        if(_costsMinFrom[1][i] > succv){
          std::cout << "Wrong minFrom " << _costsMinFrom[0][i] <<"  calculated for: " << i << std::endl;
          std::cout << "Succv " << succv << " t " << t << std::endl;
        }
        if(_costsMinTo[1][j] > succv){
          std::cout << "Wrong minTo calculated for: " << j << std::endl;
        }
        if(i!=j){
          if(_costsMinFrom[0][i] > succv){
            std::cout << "Wrong minFrom1 calculated for: " << i << std::endl;
          }
          if(_costsMinTo[0][j] > succv){
            std::cout << "Wrong minTo1 calculated for: " << j << std::endl;
          }
        }
        if( nextv + 0.02 < succv)
          std::cout << "NEXT < SUCC " << i << "->" << j << " @time " << t << " ( " << nextv << "," << succv <<" )" << std::endl;
      }
    }
  }
}



IloInt IloTDSegmentFunctionMatrix::getTransitionTimeNext(IloInt i, IloInt j, IloInt departureTime){
  return (IloInt) ceil(_nextEarliestTT[i][j].getValue((IloNum)departureTime)- EPSILON);
}
IloInt IloTDSegmentFunctionMatrix::getTransitionTimeSucc(IloInt i, IloInt j, IloInt departureTime){
  return (IloInt) ceil(_succEarliestTT[i][j].getValue((IloNum)departureTime)- EPSILON);
}
IloInt IloTDSegmentFunctionMatrix::getInverseTransitionTimeNext(IloInt i, IloInt j, IloInt arrivalTime){
  return (IloInt) ceil(_nextLatestTT[i][j].getValue((IloNum)arrivalTime) - EPSILON);
}
IloInt IloTDSegmentFunctionMatrix::getInverseTransitionTimeSucc(IloInt i, IloInt j, IloInt arrivalTime){
  return (IloInt) ceil(_succLatestTT[i][j].getValue((IloNum)arrivalTime)- EPSILON);
}
IloInt IloTD3DMatrixI::getTransitionTimeNext(IloInt i, IloInt j, IloInt departureTime){
  return _costs[i][j][departureTime/_sizeTimeStep];
}
IloInt IloTD3DMatrixI::getTransitionTimeSucc(IloInt i, IloInt j, IloInt departureTime){
  return _costs[i][j][departureTime/_sizeTimeStep];
}
IloInt IloTD3DMatrixI::getInverseTransitionTimeNext(IloInt i, IloInt j, IloInt arrivalTime){
  return _costs[i][j][arrivalTime/_sizeTimeStep];
}
IloInt IloTD3DMatrixI::getInverseTransitionTimeSucc(IloInt i, IloInt j, IloInt arrivalTime){
  return _costs[i][j][arrivalTime/_sizeTimeStep];
}