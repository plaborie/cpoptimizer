// -------------------------------------------------------------- -*- C++ -*-
// File: ./examples/team/analyze_tdtsp.cpp
// --------------------------------------------------------------------------
//
// IBM Confidential
// OCO Source Materials
// 5724-Y48 5724-Y49 5724-Y54 5724-Y55 5725-A06 5725-A29
// Copyright IBM Corp. 1990, 2014
// The source code for this program is not published or otherwise
// divested of its trade secrets, irrespective of what has
// been deposited with the U.S. Copyright Office.
//
// ---------------------------------------------------------------------------

#include <ilconcert/ilochecker.h>
#include <ilcp/cp.h>

ILOSTLBEGIN
ILCGCCHIDINGON

IloInt const S=5, M=3;
char* algo[10]= {"TDNOOVERRESTART", "NOOVERRESTART", "TW_TDNOOVERRESTART", "TW_NOOVERRESTART","TW__NOOVERRESTART", "TW_TSPMEDIAN", "NEXTDFS", "NEXTRESTART", "TDNOOVERDFS", "TDNEXTRESTART"};
char* matrix[M]= {"", "_10p_ti", "_20p_ti"};
IloInt size[S]= {10, 20, 30, 50, 100};
const char* resultDir = "ResultsTDTSP\\";
const char* resultDirGuerriero = "Results4Guerriero\\";
const char* instancesDir = "InstancesTDTSP\\";




  
IloBool Contains(const char* line, const char* str) {
  return (0 != strstr(line, str)); 
}

void GetFloatsAfterStar(char* line, IloNum& v1, IloNum& v2, IloNum& v3) {
  char* str = strchr(line, '*');
  if ((str==0) || (str && !*(str+1))) // no '*' or '*' at the end of the line
    return;
  str++;
  ILOSTD_ISTREAM post(str);
  post >> v1 >> v2 >> v3;
}

IloInt GetIntAfterSemicolon(char* line) {
  char* str = strchr(line, ';');
  if ((str==0) || (str && !*(str+1))) // no ':' or ':' at the end of the line
    return 0;
  str++;
  ILOSTD_ISTREAM post(str);
  IloInt result = 0;
  post >> result;
  return result;
}

IloNum GetFloatAfterSemicolon(char* line) {
  char* str = strchr(line, ';');
  if ((str==0) || (str && !*(str+1))) // no ':' or ':' at the end of the line
    return 0;
  str++;
  ILOSTD_ISTREAM post(str);
  IloNum result = 0;
  post >> result;
  return result;
}


IloInt GetIntAfterColumn(char* line) {
  char* str = strchr(line, ':');
  if ((str==0) || (str && !*(str+1))) // no ':' or ':' at the end of the line
    return 0;
  str++;
  ILOSTD_ISTREAM post(str);
  IloInt result = 0;
  post >> result;
  return result;
}

IloNum GetFloatAfterColumn(char* line) {
  char* str = strchr(line, ':');
  if ((str==0) || (str && !*(str+1))) // no ':' or ':' at the end of the line
    return 0;
  str++;
  ILOSTD_ISTREAM post(str);
  IloNum result = 0;
  post >> result;
  return result;
}


// Function for display -------------------------------

class PwlFunction {
public:
  PwlFunction(IloEnv env);
  PwlFunction::PwlFunction(IloNumToNumSegmentFunction f);
  ~PwlFunction(){}
  void setSlope(IloNum xl, IloNum xr, IloNum yl, IloNum slope);
  void setValue(IloNum xl, IloNum xr, IloNum y);
  void createTizZ(const char* file, const char* title =0);
  
private:
  IloNumToNumSegmentFunction _func;
};

PwlFunction::PwlFunction(IloEnv env)
  :_func(env)
{}

PwlFunction::PwlFunction(IloNumToNumSegmentFunction f) 
  :_func(f)
{}

void PwlFunction::setSlope(IloNum xl, IloNum xr, IloNum yl, IloNum slope) {
  _func.setSlope(xl,xr,yl,slope);
}

void PwlFunction::setValue(IloNum xl, IloNum xr, IloNum y) {
  _func.setValue(xl,xr,y);
}

void PwlFunction::createTizZ(const char* file, const char* title) {
  IloNum xmin = _func.getDefinitionIntervalMin();
  IloNum xmax = _func.getDefinitionIntervalMax();
  IloNum ymin = +IloInfinity;
  IloNum ymax = -IloInfinity;
  IloNumToNumSegmentFunctionCursor csor0(_func, xmin);
  if (csor0.getValueLeft()==0 && csor0.getValueRight()==0) {
    xmin = csor0.getSegmentMax();
    ++csor0;
  }
  for (; csor0.ok(); ++csor0) {
    if (csor0.getValueLeft()!=0 || csor0.getValueRight()!=0 || csor0.getSegmentMax()!=xmax) {
      std::cout << "(" << csor0.getSegmentMin() << "," << csor0.getValueLeft() << ") ";
      std::cout << "(" << csor0.getSegmentMax() << "," << csor0.getValueRight() << ") ";
      std::cout << std::endl;
      if (csor0.getValueLeft() < ymin) {
        ymin = csor0.getValueLeft();
      }
      if (csor0.getValueRight() < ymin) {
        ymin = csor0.getValueRight();
      }
      if (csor0.getValueLeft() > ymax) {
        ymax = csor0.getValueLeft();
      }
      if (csor0.getValueRight() > ymax) {
        ymax = csor0.getValueRight();
      }
    } else {
      xmax = csor0.getSegmentMin();
    }
  }
  
  IloNum ystep = 10;
  IloNum xstep = 360;
  
  ILOSTD(ofstream) str(file);
#if defined(VERBOSE)
  std::cout << "Generating function TikZ file " << file  << std::endl;
#endif
 
  str << "\\begin{tikzpicture}" << endl;

  str << "\\begin{axis}[ " << endl;
  if (title!=0) {
    str << " title={" << title << "}," << endl;
  }
  str << " enlarge x limits=false," << endl;
	str << " scaled x ticks = false," << endl;
	str << " xmode= log," << endl;
	str << " tick label style={/pgf/number format/1000 sep={}}," << endl;
	str << " xlabel={CPU time (log)}," << endl;
	str << " ylabel={Objective gap}," << endl;
  str << " height=8cm, width=12cm, grid=major," << endl;
	str << "]" << endl;
  str << "\\addplot[color=blue,mark=*,mark size=1pt] coordinates {" << endl;
  
  IloNumToNumSegmentFunctionCursor csor(_func, xmin);
  if (csor.getValueLeft()==0 && csor.getValueRight()==0) {
    xmin = csor.getSegmentMax();
    ++csor;
  }
  for (; csor.ok(); ++csor) {
    if (csor.getValueLeft()!=0 || csor.getValueRight()!=0 || csor.getSegmentMin()!=xmax) {
      str << "(" << csor.getSegmentMin() << "," << csor.getValueLeft() << ") ";
      str << "(" << csor.getSegmentMax() << "," << csor.getValueRight() << ") ";
      str << std::endl;
    } 
  }
  str << "};" << endl;
  str << "\\end{axis}" << endl;
  str << "\\end{tikzpicture}" << endl;
  str.close();
}

// ----------------------------------------------------------

void PrintResults(IloInt size, IloInt inst, const char* matrix, IloBool isWithTW) {
  char file[128];
  IloInt posInFile=0;
  IloInt res;
  
  if(isWithTW) sprintf(file, "%sresults_TW_%d.txt", resultDir, size);
  else sprintf(file, "%sresults_%d.txt", resultDir, size);
  ifstream input(file);
  if (!input.is_open()) {
    std::cout << "Can't open file '" << file << "'!" << endl;
  }
  if(strcmp(matrix, "") ==0) posInFile= (inst-1)*3 + 1; 
  if(strcmp(matrix, "_10p_ti") ==0) posInFile= (inst-1)*3 + 2; 
  if(strcmp(matrix, "_20p_ti") ==0) posInFile= (inst-1)*3 + 3; 
  
  const IloInt lineLimit = 131072;
  char line[lineLimit];
  for(IloInt counter=1; counter < posInFile; counter++){
    input.getline(line, lineLimit);
  }
  input.getline(line, lineLimit, ';');
  input >> res;
  // cout << GetIntAfterSemicolon(line);
  cout << res;
  input.getline(line, lineLimit);
  // cout << line;
  if(line[1]=='*') cout << "*";
  // cout << " ";
  // cout << GetFloatAfterSemicolon(line);
}


IloInt GetReferenceValue(IloInt size, IloInt inst, const char* matrix, IloBool isWithTW) {
  char file[128];
  IloInt posInFile=0;
  
  if(isWithTW) sprintf(file, "%sresults_TW_%d.txt", resultDir, size);
  else sprintf(file, "%sresults_%d.txt", resultDir, size);
  ifstream input(file);
  if (!input.is_open()) {
    std::cout << "Can't open file '" << file << "'!" << endl;
  }
  
  if(strcmp(matrix, "") ==0) posInFile= (inst-1)*3 + 1; 
  if(strcmp(matrix, "_10p_ti") ==0) posInFile= (inst-1)*3 + 2; 
  if(strcmp(matrix, "_20p_ti") ==0) posInFile= (inst-1)*3 + 3; 
  
  const IloInt lineLimit = 131072;
  char line[lineLimit];
  for(IloInt counter=1; counter <= posInFile; counter++){
    input.getline(line, lineLimit);
  }
  return GetIntAfterSemicolon(line);
}


class OptimizationLog {
public:
  OptimizationLog(IloInt size, IloInt inst, IloInt indexAlgo, IloInt indexMatrix, IloBool isWithTW);
  OptimizationLog(const char* logName);
  void printStatsTSP2TDTSP();
  IloNum getGap() const {return gap;}
  IloBool getProof() const {if(proof) return true; return false;}
  IloInt getBestObj() const {return bestObj;}
  IloInt getBestObjUntil(IloInt timeT);
  IloBool getInfea() const {return isInfea;}
  IloNumToNumSegmentFunction getFunction() const {return convergenceCurve;}
  IloNum getStdDev(IloInt index) const { return stdDevPoints[index];}
  IloNum getValueConv(IloNum val) const { return convergenceCurve.getValue(val);}
  IloNum getEndTime() const { return totalTime;}
  IloInt getNbErrors() const { return nbErrors;}
 
private:
  IloEnv env;
  IloInt size;
  IloInt inst; 
  IloInt indexAlgo;
  IloInt indexMatrix;
  IloInt bestObj;
  IloInt bestTdObj;
  IloNum totalTime;
  IloInt nbBranches;
  IloInt proof; // 1 if proof 0 otherwise
  IloNum* times;
  IloInt* objs;
  IloInt* tdObjs;
  IloInt nbSols;
  IloInt nbErrors;
  IloInt nbInfeas;
  IloInt refValue;
  IloNum gap;
  IloBool isWithTW;
  IloBool isInfea;
  IloNumToNumSegmentFunction convergenceCurve;
  IloNum* stdDevPoints;

};

OptimizationLog::OptimizationLog(const char* logName)
  :env(IloEnv())
  ,size(0)
  ,inst(0)
  ,indexAlgo(0)
  ,indexMatrix(0)
  ,bestObj(0)
  ,bestTdObj(0)
  ,totalTime(0)
  ,nbBranches(0)
  ,proof(0)
  ,times(0)
  ,objs(0)
  ,tdObjs(0)
  ,nbSols(0)
  ,nbErrors(0)
  ,nbInfeas(0)
  ,refValue(0)
  ,gap(IloIntMax)
  ,isWithTW(false)
  ,isInfea(false)
  ,convergenceCurve(IloNumToNumSegmentFunction(env))
  ,stdDevPoints(0)
  {
  
  ifstream input(logName);
  if (!input.is_open()) {
    std::cout << "Can't open file '" << logName << "'!" << endl;
  }
  const IloInt lineLimit = 131072;
  char line[lineLimit];
  times = new IloNum[200];
  objs = new IloInt[200];
  tdObjs = new IloInt[200];
  IloInt min= IloIntMax;
  IloNum tlimit = 600;
 
  while (input.getline(line, lineLimit)) {
    if (Contains(line, "BEST SOLUTION")) {
      bestObj = GetIntAfterColumn(line);
    } else if (Contains(line, "NUMBER OF BRANCHES")) {
      nbBranches = GetIntAfterColumn(line);
    } else if (Contains(line, "END TIME")) {
      totalTime = GetFloatAfterColumn(line);
    } else if (Contains(line, "OPTIMALITY NOT PROVED")) {
      proof = 0;
    } else if (Contains(line, "OPTIMALITY PROVED")) {
      proof = 1;
    } else if (Contains(line, "BEST TD SOLUTION")) {
      bestTdObj = GetIntAfterColumn(line);
    } else if (Contains(line, "ERROR")) {
      nbErrors++;
    }
  }
}


OptimizationLog::OptimizationLog(IloInt size, IloInt inst, IloInt indexAlgo, IloInt indexMatrix, IloBool isWithTW)
  :env(IloEnv())
  ,size(size)
  ,inst(inst)
  ,indexAlgo(indexAlgo)
  ,indexMatrix(indexMatrix)
  ,bestObj(0)
  ,bestTdObj(0)
  ,totalTime(0)
  ,nbBranches(0)
  ,proof(0)
  ,times(0)
  ,objs(0)
  ,tdObjs(0)
  ,nbSols(0)
  ,nbErrors(0)
  ,nbInfeas(0)
  ,refValue(0)
  ,gap(IloIntMax)
  ,isWithTW(isWithTW)
  ,isInfea(false)
  ,convergenceCurve(IloNumToNumSegmentFunction(env))
  ,stdDevPoints(0)
  {
  char file[128];
  if(isWithTW) sprintf(file, "%sinst_%ld_%ld_TW_%s%s.log", resultDir, size, inst, algo[indexAlgo], matrix[indexMatrix]);
  else sprintf(file, "%sinst_%ld_%ld_%s%s.log", resultDir, size, inst, algo[indexAlgo], matrix[indexMatrix]);
  ifstream input(file);
  if (!input.is_open()) {
    std::cout << "Can't open file '" << file << "'!" << endl;
  }
  const IloInt lineLimit = 131072;
  char line[lineLimit];
  times = new IloNum[200];
  objs = new IloInt[200];
  tdObjs = new IloInt[200];
  IloInt min= IloIntMax;
  IloNum tlimit = (size<50?3600:7200);
  // refValue= GetReferenceValue(size, inst, matrix[indexMatrix], isWithTW);
  
  while (input.getline(line, lineLimit)) {
    if (Contains(line, "BEST SOLUTION")) {
      bestObj = GetIntAfterColumn(line);
    } else if (Contains(line, "NUMBER OF BRANCHES")) {
      nbBranches = GetIntAfterColumn(line);
    } else if (Contains(line, "END TIME")) {
      totalTime = GetFloatAfterColumn(line);
    } else if (Contains(line, "OPTIMALITY NOT PROVED")) {
      proof = 0;
    } else if (Contains(line, "OPTIMALITY PROVED")) {
      proof = 1;
    } else if (Contains(line, "BEST TD SOLUTION")) {
      bestTdObj = GetIntAfterColumn(line);
    }/* else if (Contains(line, "*")) {
      // tdObjs[nbSols]= -1;
      IloNum v1, v2, v3=-2;
      GetFloatsAfterStar(line, v1, v2, v3);
      if(v3==-1 || v3 > 46800) nbInfeas++;
      times[nbSols]= v1;
      objs[nbSols]= v2;      
      tdObjs[nbSols]= v3;
      
      if(v3!= -2) v2= v3;
      if(v2!= -1 && v2 < min) min =v2;
      else v2= min;
      if (nbSols==0 && v2!= -1) {
        convergenceCurve.setValue(0, tlimit, (v2/refValue) - 1);
      }
      else if(nbSols!=0 && v2!= -1){
        convergenceCurve.setValue(v1, tlimit, (v2/refValue) - 1);
      }
      nbSols++;
    }*/
  }
  
  stdDevPoints= new IloNum[5];
  stdDevPoints[0]= convergenceCurve.getValue(0.1);
  stdDevPoints[1]= convergenceCurve.getValue(1);
  stdDevPoints[2]= convergenceCurve.getValue(10);
  stdDevPoints[3]= convergenceCurve.getValue(100);
  stdDevPoints[4]= convergenceCurve.getValue(1000);
  
  if(bestTdObj!= -1 && bestTdObj > 0 && bestTdObj <= 46800 && refValue!=0) gap= (((IloNum)bestTdObj/refValue) - 1)*100;
  else if(bestTdObj==0) gap= (((IloNum)bestObj/refValue) - 1)*100;
// cout << inst << " " << inst << " " << refValue << " " << bestTdObj << endl;
  if(nbInfeas == nbSols) isInfea= true;
}

IloInt OptimizationLog::getBestObjUntil(IloInt t){
  IloInt bestObjUntil=IloIntMax;
  for(IloInt i=0; i< nbSols; i++){
    if(times[i] < t) bestObjUntil= objs[i];
  }
  return bestObjUntil;
}

void OptimizationLog::printStatsTSP2TDTSP(){
  if(nbInfeas == 0) {
    cout << size << "_" << inst << matrix[indexMatrix] << " " << gap << endl;
  }
  else if(nbInfeas == nbSols) {
    cout << size << "_" << inst << matrix[indexMatrix] << " - " << nbInfeas << " " << nbSols << endl;
  }
  else  cout << size << "_" << inst << matrix[indexMatrix] << " " << gap << " " << nbInfeas << " " << nbSols << endl;
  
}

void printQualityComp(IloInt size, IloInt algo, IloInt algo2, IloBool isWithTW, IloInt timeT){

  cout << "\\addplot+[only marks, color=blue,mark=o,mark size=1pt] coordinates {" << endl;
  for(IloInt inst=1; inst <21; inst++){
    OptimizationLog log(size, inst, algo, 0, isWithTW);
    OptimizationLog log2(size, inst, algo2, 0, isWithTW);
    std::cout << "(" << log.getBestObjUntil(timeT) << ", ";
    std::cout << log2.getBestObjUntil(timeT)  << ")";
    std::cout << std::endl;
  }
  cout << "};" << endl;
  cout << "\\addplot+[only marks, color=red,mark=o,mark size=1pt] coordinates {" << endl;
  for(IloInt inst=1; inst <21; inst++){
    OptimizationLog log(size, inst, algo, 1, isWithTW);
    OptimizationLog log2(size, inst, algo2, 1, isWithTW);
    std::cout << "(" << log.getBestObjUntil(timeT)  << ", ";
    std::cout << log2.getBestObjUntil(timeT)  << ")";
    std::cout << std::endl;
  }
  cout << "};" << endl;
  cout << "\\addplot+[only marks, color=green,mark=o,mark size=1pt] coordinates {" << endl;
  for(IloInt inst=1; inst <21; inst++){
    OptimizationLog log(size, inst, algo, 2, isWithTW);
    OptimizationLog log2(size, inst, algo2, 2, isWithTW);
    std::cout << "(" << log.getBestObjUntil(timeT)  << ", ";
    std::cout << log2.getBestObjUntil(timeT)  << ")";
    std::cout << std::endl;
  }
  cout << "};" << endl;
  
}

void printAverageConvergenceCurve(IloInt size, IloInt matrixIndex, IloInt algoIndex, IloBool isWithTW, IloNum eps) {
  IloEnv env;
  IloNumToNumSegmentFunction averageFunction(env);
  IloNum* stdDevAverage= new IloNum[5];
  IloNum* times= new IloNum[5];
  times[0]= 0.1*(1 + eps); 
  times[1]= 1*(1 + eps); 
  times[2]= 10*(1 + eps); 
  times[3]= 100*(1 + eps); 
  times[4]= 1000*(1 + eps); 
  for (IloInt inst = 1; inst<=20; inst++) {
    OptimizationLog log(size, inst, algoIndex, matrixIndex, isWithTW);
    averageFunction  += log.getFunction();
  }
  averageFunction  *= 5;
  for (IloInt inst = 1; inst<=20; inst++) {
    OptimizationLog log(size, inst, algoIndex, matrixIndex, isWithTW);
    stdDevAverage[0] += (log.getValueConv(times[0])*100 - averageFunction.getValue(times[0]))*(log.getValueConv(times[0])*100 - averageFunction.getValue(times[0]));
    stdDevAverage[1] += (log.getValueConv(times[1])*100 - averageFunction.getValue(times[1]))*(log.getValueConv(times[1])*100 - averageFunction.getValue(times[1]));
    stdDevAverage[2] += (log.getValueConv(times[2])*100 - averageFunction.getValue(times[2]))*(log.getValueConv(times[2])*100 - averageFunction.getValue(times[2]));
    stdDevAverage[3] += (log.getValueConv(times[3])*100 - averageFunction.getValue(times[3]))*(log.getValueConv(times[3])*100 - averageFunction.getValue(times[3]));
    stdDevAverage[4] += (log.getValueConv(times[4])*100 - averageFunction.getValue(times[4]))*(log.getValueConv(times[4])*100 - averageFunction.getValue(times[4]));
  }
  
  
  cout << "\\addplot [color=red, mark=-] coordinates { ("<< times[0] << "," << averageFunction.getValue(times[0]) - sqrt(stdDevAverage[0]/20) << ")("<< times[0] << ","  << averageFunction.getValue(times[0]) + sqrt(stdDevAverage[0]/20) <<  ")};" << endl;
  cout << "\\addplot [color=red, mark=-] coordinates { ("<< times[1] << ","  << averageFunction.getValue(times[1]) - sqrt(stdDevAverage[1]/20) << ")("<< times[1] << "," << averageFunction.getValue(times[1]) + sqrt(stdDevAverage[1]/20) <<  ")};" << endl;
  cout << "\\addplot [color=red, mark=-] coordinates {("<< times[2] << "," << averageFunction.getValue(times[2]) - sqrt(stdDevAverage[2]/20) << ")("<< times[2] << ","  << averageFunction.getValue(times[2]) + sqrt(stdDevAverage[2]/20) <<  ")};" << endl;
  cout << "\\addplot [color=red, mark=-] coordinates {("<< times[3] << "," << averageFunction.getValue(times[3]) - sqrt(stdDevAverage[3]/20) << ")("<< times[3] << "," << averageFunction.getValue(times[3]) + sqrt(stdDevAverage[3]/20) <<  ")};" << endl;
  cout << "\\addplot [color=red, mark=-] coordinates {("<< times[4] << "," << averageFunction.getValue(times[4]) - sqrt(stdDevAverage[4]/20) << ")("<< times[4] << ","  << averageFunction.getValue(times[4]) + sqrt(stdDevAverage[4]/20) <<  ")};" << endl;
  
  // cout << averageFunction.getValue(1+ eps) - sqrt(stdDevAverage[1]/20)<< endl;
  // cout << averageFunction.getValue(10+ eps) -sqrt(stdDevAverage[2]/20)<<endl;
  // cout << averageFunction.getValue(100+ eps) -sqrt(stdDevAverage[3]/20)<< endl;
  // cout << averageFunction.getValue(1000+ eps) -sqrt(stdDevAverage[4]/20)<< endl;
  
  PwlFunction PwlAverageFunction(averageFunction);
  
  char file[1000], title[1000];
  sprintf(file, "convergence%s_%ld%s.tikz", algo[algoIndex], size, matrix[matrixIndex]);
  sprintf(title, "Convergence %s %ld", algo[algoIndex], size);
  // PwlAverageFunction.createTizZ(file, title);
  
  env.end();
}
  
// -----------------------------------------------------------------

int main(int argc, char* argv[]) {
// OptimizationLog(IloInt size, IloInt inst, IloInt indexAlgo, IloInt indexMatrix, IloBool isWithTW);
  // printQualityComp(30, 9, 0, true, 300);
  // printQualityComp(30, 9, 0, false, 300);

/*/  optimality + average gap - tdnext x tdnoover 
 IloInt opt, algIndex=0; //Next=9, TDnoOver=0;
 // IloBool TW=false;
 IloBool TW=true;
  IloNum averageGap, temp;
  if(algIndex==9) cout << "Using TDNEXTRESTART"; 
  else cout << "Using TDNOOVERRESTART";
  if(TW) cout << "_TW";
  cout << endl;
  cout << "InstClass OptProof AvgGap"<< endl; 
  for(IloInt j=1; j< 3; j++){
    for(IloInt i=0; i< 3; i++){
      averageGap=0; opt=0;
      for(IloInt k=1; k<21; k++){
        OptimizationLog inst(size[j], k, algIndex, i, TW); 
        if(inst.getProof()) opt++;
        else averageGap+=inst.getGap();
      }
      cout << size[j] << "_" << i << " " << opt << " " << averageGap/20 << endl;
    }
  } 
 // */ 
/*/   Print best results 
   IloInt algIndex=0; //Next=9, TDnoOver=0;
 IloBool TW=false;
 // IloBool TW=true;
  for(IloInt j=0; j< 1; j++){
   for(IloInt k=1; k<61; k++){
     for(IloInt i=0; i< 3; i++){
      OptimizationLog inst(size[j], k, algIndex, i, TW); 
        cout << k << matrix[i] << ";" << inst.getBestObj();
        if(inst.getProof()) cout << " *";
        cout << ";" << inst.getEndTime() << endl;
      }
    }
  } 
  // */
  
  /*/   Print all results 
  for(IloInt j=0; j< 5; j++){
   for(IloInt i=0; i< 3; i++){
    IloInt kmax=21;
    // if(j<3) kmax=61;
    for(IloInt k=1; k< kmax; k++){
      cout << "inst_" << size[j] << "_" << k << ".txt matrix" << i << "0.txt " ;
      PrintResults(size[j], k, matrix[i], false);
      cout << endl;
    }
      cout << endl << endl;
   }
  }
  for(IloInt j=0; j< 5; j++){
   for(IloInt i=0; i< 3; i++){
    for(IloInt k=1; k< 21; k++){
      cout << "inst_" << size[j] << "_" << k << "_TW.txt matrix" << i << "0.txt "; 
      PrintResults(size[j], k, matrix[i], true);
      cout << endl;
    }
    cout << endl << endl;
   }
  }
  // */
  
  
//   Print all results GUERRIERO
  char logName[500];
  for(IloInt size=20; size<= 40; size+=5){
    for(char type='A'; type < 'D'; type++){
     for(IloInt inst=1; inst< 11; inst++){
      for(IloInt k=70; k< 101; k+=5){
        for(char pat='A'; pat < 'C'; pat++){
          if(k==100) k=98;
          sprintf(logName, "%s%ld%cNodi_%ld_Pattern%c%ld.log", resultDirGuerriero, size, type, inst,pat, k);
          OptimizationLog logGuerriero(logName);
          // if(logGuerriero.getBestObj()==0){
            cout << logName << " ";
            // cout << logGuerriero.getBestObj() << " " << logGuerriero.getEndTime() << endl;
            cout << logGuerriero.getBestObj() << " " << logGuerriero.getEndTime() << " " << logGuerriero.getProof() << endl;
          // }
          if(logGuerriero.getNbErrors()!=0){
          cout << logGuerriero.getNbErrors() << endl;}
        }
      }
      cout << endl << endl;
     }
   }
  }
  // */
  
  
  
  
/*/prints convergence curves (IloInt size, IloInt matrix, IloInt algo, IloBool isWithTW)
  for(IloInt j=1; j<=1; j++){
    for(IloInt i=2; i< 3; i++){
      // cout << "\\addplot+[only marks, color=blue,mark=o,mark size=1pt] coordinates {" << endl;
      printAverageConvergenceCurve(size[j], i, 0, false, 0.1);
      // printAverageConvergenceCurve(size[j], i, 9, false);
      // cout << "};" << endl;
    }
  }
  for(IloInt j=1; j<=1; j++){
    for(IloInt i=2; i< 3; i++){
      // cout << "\\addplot+[only marks, color=blue,mark=o,mark size=1pt] coordinates {" << endl;
      // printAverageConvergenceCurve(size[j], i, 0, false);
      printAverageConvergenceCurve(size[j], i, 1, false, 0.3);
      // cout << "};" << endl;
    }
  }
 // */

/*/ prints stats about tsp solutions in TD context (with TW)--
  IloInt infeaCount, nameAlg;
  IloNum totalGap, temp;
  cout<< "INST BESTGAP NBINFEASIBLE NBSOLS"<< endl;
  for(IloInt j=1; j< 5; j++){
    for(IloInt i=0; i< 3; i++){
      totalGap=0; infeaCount=0;
      for(IloInt k=1; k<21; k++){
        // if(matrix[i][0]!='_') nameAlg=3;
        nameAlg=1;
        OptimizationLog inst(size[j], k, nameAlg, i, false);
        inst.printStatsTSP2TDTSP();
        if(inst.getInfea()) infeaCount++;
        else totalGap+=inst.getGap();
      }
      // cout << "Solved Instances: " << 20 - infeaCount << endl;
      if(infeaCount < 20)
        cout << "Solved Instances Average Best Gap: " << totalGap/(20-infeaCount) << endl;
    }
  }
  
 // */ 
/*/ extracts first solutions of tdtsp and tsp with TW ----------
  cout<< "TW_TDNOOVERRESTART; TW_NOOVERRESTART"<< endl; 
  for(IloInt j=1; j< S; j++){
    cout << "Instance size: " << size[j] << endl;
    cout << "Matrix: T00" << endl;
    for(IloInt k=1; k<21; k++)
      cout << extractFirstSolution(size[j],k, matrix[0], algo[2])<< "; " << extractFirstSolution(size[j],k, matrix[0], algo[3]) << endl;
    cout << "Matrix: T10" << endl;
    for(IloInt k=1; k<21; k++)
      cout << extractFirstSolution(size[j],k, matrix[1], algo[2])<< "; " << extractFirstSolution(size[j],k, matrix[1], algo[3]) << endl;
    cout << "Matrix: T20" << endl;
    for(IloInt k=1; k<21; k++)
      cout << extractFirstSolution(size[j],k, matrix[2], algo[2])<< "; " << extractFirstSolution(size[j],k, matrix[2], algo[3]) << endl;
  }
*/  

/*/  print branches or times for algorithms comparison size 10 instances
  // printBranchesOrTimes(algo[8], true, 0); //TDNOOVERDFS branches
  // printBranchesOrTimes(algo[6], true, 0); //NEXTDFS branches
  // printBranchesOrTimes(algo[8], false, 0); //TDNOOVERDFS times
  // printBranchesOrTimes(algo[6], false, 0); //NEXTDFS times
// /
*/
  
/*/ extracts indicators (exec time, best sol, branches etc.)
  for(IloInt size= 10; size <= 10; size+=50){
    for (IloInt inst = 1; inst<=20; inst++) {
      // analyzeIndicators(size, inst, algo[0]);
      // analyzeIndicators(size, inst, algo[6]);
      // analyzeIndicators(size, inst, algo[7]);
      analyzeIndicators(size, inst, algo[8]);
    }
  }
// */
    
}



/*/ PREVIOUS CODE IN THIS FILE FOR REFERENCE


IloInt extractStatsTSP2TDTSP(IloInt size, IloInt inst, const char* matrix, const char* algo) {
  char file[128];
  // sprintf(file, "%sNewTWinstances\\inst_%ld_%ld_%s%s.log", resultDir, size, inst, algo, matrix);
  sprintf(file, "%sinst_%ld_%ld_%s%s.log", resultDir, size, inst, algo, matrix);
  ifstream input;
  input.open(file);
  if (!input.is_open()) {
    std::cout << "Can't open file '" << file << "'!" << endl;
  }
  IloNum ref = GetReferenceValue(size, inst, matrix, true);  
  const IloInt lineLimit = 131072;
  char line[lineLimit];
  IloInt nbInfea=0, nbSols=0;
  IloNum t, min=100000; 
  IloNum v, previousV, tdValue;
     
  while (input.getline(line, lineLimit)) {
    if (Contains(line, "*")) {
      tdValue= -2;
      nbSols++;
      GetFloatsAfterStar(line, t, v, tdValue);
      if(tdValue==-2 && v < min){ 
        min=v;
        if( v > 46800) nbInfea++;
      }
      if(tdValue!= -2 && tdValue!= -1 && tdValue < min) min= tdValue;
      if(tdValue== -1 || tdValue > 46800) nbInfea++;
    }
  }
  if(nbInfea == nbSols) {
    cout << size << "_" << inst << matrix << " - " << nbInfea << " " << nbSols << endl;
    return 1;
  }
  else cout << size << "_" << inst << matrix << " " << ((min/ref) - 1)*100 << " " << nbInfea << " " << nbSols << endl;
  return 0;
}

IloNum extractGapTSP2TDTSP(IloInt size, IloInt inst, const char* matrix, const char* algo) {
  char file[128];
  // sprintf(file, "%sNewTWinstances\\inst_%ld_%ld_%s%s.log", resultDir, size, inst, algo, matrix);
  sprintf(file, "%sinst_%ld_%ld_%s%s.log", resultDir, size, inst, algo, matrix);
  ifstream input;
  input.open(file);
  if (!input.is_open()) {
    std::cout << "Can't open file '" << file << "'!" << endl;
  }
  IloNum ref = GetReferenceValue(size, inst, matrix, true);  
  const IloInt lineLimit = 131072;
  char line[lineLimit];
  IloInt nbInfea=0, nbSols=0;
  IloNum t, min=100000; 
  IloNum v, previousV, tdValue;
     
  while (input.getline(line, lineLimit)) {
    if (Contains(line, "*")) {
      tdValue= -2;
      nbSols++;
      GetFloatsAfterStar(line, t, v, tdValue);
      if(tdValue==-2 && v < min){ 
        min=v;
        if( v > 46800) nbInfea++;
      }
      if(tdValue!= -2 && tdValue!= -1 && tdValue < min) min= tdValue;
      if(tdValue== -1 || tdValue > 46800) nbInfea++;
    }
  }
  if(nbInfea == nbSols) {
    return -1;
  }
  return ((min/ref) - 1)*100;
}


void extractConvergenceCurve(IloInt size, IloInt inst, const char* matrix, const char* algo, IloNumToNumSegmentFunction& f) {
  char file[128];
  IloNum tlimit = (size<50?3600:7200);
  IloNum times[20]={0.1, 0.2, 0.4, 0.8, 1, 2, 4, 8, 10, 20, 40, 80, 100, 200, 400, 800, 1000, 2000, 4000, 8000};
  sprintf(file, "%sTW_NOOVERLAP\\inst_%ld_%ld_%s%s.log", resultDir, size, inst, algo, matrix);
  // sprintf(file, "%sinst_%ld_%ld_%s%s.log", resultDir, size, inst, algo, matrix);
  ifstream input;
  input.open(file);
  if (!input.is_open()) {
    std::cout << "Can't open file '" << file << "'!" << endl;
  }
  IloNum ref = GetReferenceValue(size, inst, matrix, true);  
  const IloInt lineLimit = 131072;
  char line[lineLimit];
  IloInt r = 0;
  IloNum t, min=100000; 
  IloNum v, previousV;
     
  while (input.getline(line, lineLimit)) {
    if (Contains(line, "*")) {
      IloNum tdValue= -2;
      GetFloatsAfterStar(line, t, v, tdValue);
      if(tdValue!= -2) v= tdValue;
      if(v != -1 && v < min) min =v;
      else v= min;
      if (r==0 && v!= -1) {
        f.setValue(0, tlimit, (v/ref) - 1);
        // cout << "Function: 0 " << ((v/ref) - 1)*100 << endl;
        r++;
      }
      else if(r!=0 && v!= -1){
        f.setValue(t, tlimit, (v/ref) - 1);
        r++;
      }
      // else if(t >= times[r]){
        // f.setValue(times[r], tlimit, (previousV/ref) - 1);
        // //cout << "Function: " << times[r] << " " << ((previousV/ref) - 1)*100 << endl;
        // r++;
      // }
      // cout << t << " " << ((v/ref) - 1)*100 << endl;
      previousV= v;
    }
  }
  // cout << "last value= " << (v/ref) - 1 << " last time= " << times[r] << endl; 
  // f.setValue(times[r], tlimit, (v/ref) - 1);
}

void extractAverageConvergenceCurve(IloInt size, const char* matrix, const char* algo) {
  IloEnv env;
  IloNumToNumSegmentFunction TDNoOv(env);
  
  for (IloInt inst = 1; inst<=20; inst++) {
    IloNumToNumSegmentFunction fTDNoOv(env);
    extractConvergenceCurve(size, inst, matrix, algo, fTDNoOv);
    TDNoOv += fTDNoOv;
  }
  TDNoOv *= 5;

  PwlFunction TDNoOvF(TDNoOv);
  char fileTDNoOv[128], title[128];
  sprintf(fileTDNoOv, "convergence%s_%ld%s.tikz", algo, size, matrix);
  sprintf(title, "Convergence %s %ld", algo, size);
  TDNoOvF.createTizZ(fileTDNoOv, title);

  env.end();
}

IloNum extractFirstSolution(IloInt size, IloInt inst, const char* matrix, const char* algo) {
  char file[128];
  sprintf(file, "%sinst_%ld_%ld_%s%s.log", resultDir, size, inst, algo, matrix);
  ifstream input;
  input.open(file);
  if (!input.is_open()) {
    std::cout << "Can't open file '" << file << "'!" << endl;
  }
  const IloInt lineLimit = 131072;
  char line[lineLimit];
  IloNum t, v;
     
  while (input.getline(line, lineLimit)) {
    if (Contains(line, "*")) {
      IloNum tdValue= -1;
      GetFloatsAfterStar(line, t, v, tdValue);
      if(tdValue!= -1) v= tdValue;
      return v;
    }
  }
  return 0;
}


void extractIndicators(IloInt size, IloInt inst, 
                       const char* algo, IloInt& obj, IloInt& nbbranches, IloInt& proof, IloNum& time, const char* matrix) {
  char file[128];
  // sprintf(file, "%sNewTWinstances\\inst_%ld_%ld_TW_%s%s.log", resultDir, size, inst, algo, matrix);
  // sprintf(file, "%sinst_%ld_%ld_TW_%s%s.log", resultDir, size, inst, algo, matrix);
  sprintf(file, "%sinst_%ld_%ld_%s%s.log", resultDir, size, inst, algo, matrix);
  ifstream input(file);
  if (!input.is_open()) {
    std::cout << "Can't open file '" << file << "'!" << endl;
  }
  const IloInt lineLimit = 131072;
  char line[lineLimit];
  IloInt r = 0;
  obj=-1;
  while (r<4 && input.getline(line, lineLimit)) {
    if/* (Contains(line, "BEST SOLUTION")) {
      obj = GetIntAfterColumn(line);
      r++;
    } else if/ (Contains(line, "NUMBER OF BRANCHES")) {
       nbbranches = GetIntAfterColumn(line);
       r++;
    } else if (Contains(line, "END TIME")) {
      time = GetFloatAfterColumn(line);
      r++;
    } else if (Contains(line, "OPTIMALITY NOT PROVED")) {
      proof = 0;
      r++;
    } else if (Contains(line, "OPTIMALITY PROVED")) {
      proof = 1;
      r++;
    }
     else if (Contains(line, "BEST TD SOLUTION")) {
      obj = GetIntAfterColumn(line);
      r++;
    }
  }
}

void analyzeIndicators(IloInt size, IloInt inst, const char* algo) {
  if (inst < 0) {
    std::cout << "ERROR: UNSPECIFIED INSTANCE" << std::endl;
    exit(0);
  }
  //Possible algo names: "TSPMEDIAN", "NEXTDFS", "NEXTRESTART", "SCHEDDFS", "TDNOOVERDFS", "TDNOOVERRESTART"
  
  IloInt objNext, nbBranchesNext, proofNext;
  IloNum timeNext;
  // extractIndicators(size, inst, "NEXTDFS", objNext, nbBranchesNext, proofNext, timeNext);
  IloInt objNoOv, nbBranchesNoOv, proofNoOv;
  IloNum timeNoOv;
  extractIndicators(size, inst, algo, objNoOv, nbBranchesNoOv, proofNoOv, timeNoOv, "");
  // extractIndicators(size, inst, "TDNOOVERDFS", objNoOv, nbBranchesNoOv, proofNoOv, timeNoOv, "");
  std::cout << inst << ";";
  if(proofNoOv)  std::cout << objNoOv << " *;"; 
  else  std::cout << objNoOv << ";";
  std::cout << nbBranchesNoOv << ";";
  std::cout << timeNoOv<< ";";
  std::cout << std::endl;
 
  extractIndicators(size, inst, algo, objNoOv, nbBranchesNoOv, proofNoOv, timeNoOv, "_10p_ti");
  // extractIndicators(size, inst, "TDNOOVERDFS", objNoOv, nbBranchesNoOv, proofNoOv, timeNoOv, "_10p_ti");
  std::cout << inst << "_10p_ti;";
  if(proofNoOv)  std::cout << objNoOv << " *;"; 
  else  std::cout << objNoOv << ";"; 
  std::cout << nbBranchesNoOv << ";";
  std::cout << timeNoOv<< ";";
  std::cout << std::endl;
  
  extractIndicators(size, inst, algo, objNoOv, nbBranchesNoOv, proofNoOv, timeNoOv, "_20p_ti");
  // extractIndicators(size, inst, "TDNOOVERDFS", objNoOv, nbBranchesNoOv, proofNoOv, timeNoOv, "_20p_ti");
  std::cout << inst << "_20p_ti;";
  if(proofNoOv)  std::cout << objNoOv << " *;"; 
  else  std::cout << objNoOv << ";";
  std::cout << nbBranchesNoOv << ";";
  std::cout << timeNoOv << ";";
  // std::cout << inst << ";";
  // std::cout << objNext << ";"; 
  // std::cout << objNoOv << ";"; 
  // std::cout << nbBranchesNext << ";"; 
  // std::cout << nbBranchesNoOv << ";"; 
  // std::cout << timeNext << ";"; 
  // std::cout << timeNoOv; // << ";"; 
  // std::cout << proofNext << ";"; 
  // std::cout << proofNoOv;
  std::cout << std::endl;          
}

void printBranchesOrTimes(const char* algo, bool branches, IloInt instZero) {
 
  IloInt size=10, inst;
  IloInt objNext, nbBranchesNext, proofNext;
  IloNum timeNext;
  IloInt objNoOv, nbBranchesNoOv, proofNoOv;
  IloNum timeNoOv;
 
  for(inst=1; inst <21; inst++){
    extractIndicators(size, inst, algo, objNoOv, nbBranchesNoOv, proofNoOv, timeNoOv, "");
    std::cout << "(" << instZero+inst << ", ";
    if(branches) std::cout << nbBranchesNoOv << ")";
    else std::cout << timeNoOv<< ")";
    std::cout << std::endl;
  }
  
  for(inst=1; inst <21; inst++){
    extractIndicators(size, inst, algo, objNoOv, nbBranchesNoOv, proofNoOv, timeNoOv, "_10p_ti");
    std::cout << "(" << instZero+20+ inst << ", ";
    if(branches) std::cout << nbBranchesNoOv << ")";
    else std::cout << timeNoOv<< ")";
    std::cout << std::endl;
  }
 
  for(inst=1; inst <21; inst++){
    extractIndicators(size, inst, algo, objNoOv, nbBranchesNoOv, proofNoOv, timeNoOv, "_20p_ti");
    std::cout << "(" << instZero+40+ inst << ", ";
    if(branches) std::cout << nbBranchesNoOv << ")";
    else std::cout << timeNoOv<< ")";
    std::cout << std::endl;
  }
}



void printQualityComp(IloInt size, const char* algo, const char* algo2, const char* matrix) {
 
  IloInt objNext, nbBranchesNext, proofNext;
  IloNum timeNext;
  IloInt objNoOv, nbBranchesNoOv, proofNoOv;
  IloNum timeNoOv;
  for(IloInt inst=1; inst <21; inst++){
      extractIndicators(size, inst, algo, objNoOv, nbBranchesNoOv, proofNoOv, timeNoOv, matrix);
      extractIndicators(size, inst, algo2, objNext, nbBranchesNext, proofNext, timeNext, matrix);
      std::cout << "(" << objNoOv << ", ";
       std::cout << objNext << ")";
      std::cout << std::endl;
  }
}

*/