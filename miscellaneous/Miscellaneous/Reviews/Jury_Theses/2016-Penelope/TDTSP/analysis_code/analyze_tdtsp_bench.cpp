// -------------------------------------------------------------- -*- C++ -*-
// File: analyze.cpp
// --------------------------------------------------------------------------
// Licensed Materials - Property of IBM
//
// 5724-Y48 5724-Y49 5724-Y54 5724-Y55 5725-A06 5725-A29
// Copyright IBM Corporation 1990, 2013. All Rights Reserved.
//
// Note to U.S. Government Users Restricted Rights:
// Use, duplication or disclosure restricted by GSA ADP Schedule
// Contract with IBM Corp.
// --------------------------------------------------------------------------

/* --------------------------------------------------------------------------

This program analyzes a TD-TSP Matrix

-------------------------------------------------------------------------- */

#include <ilconcert/ilomodel.h>
#include <ilconcert/ilosegfunc.h>

#define VERBOSE

ILOSTLBEGIN

IloBool Contains(const char* line, const char* str) {
  return (0 != strstr(line, str)); 
}

IloInt GetIntAfterEqual(char* line) {
  char* str = strchr(line, '=');
  if ((str==0) || (str && !*(str+1))) // no ':' or ':' at the end of the line
    return 0;
  str++;
  ILOSTD_ISTREAM post(str);
  IloInt result = 0;
  post >> result;
  return result;
}

IloInt* GetIntArray(const char* line, IloInt& n) {
  size_t l = strlen(line)+1;
  char* cpy = new char[l];
  strcpy(cpy, line);
  for (int i=0; i<l-1; i++) {
    if (line[i]<'0' || line[i]>'9') {
      cpy[i]=' ';
    } else {
      cpy[i]=line[i];
    }
  }
  n=0;
  IloInt v=0;
  ILOSTD_ISTREAM num0(cpy);
  while (num0 >> v) { n++; }
  IloInt* result = new IloInt[n];
  ILOSTD_ISTREAM num1(cpy);
  n=0;
  while (num1 >> v) { result[n++]=v; }
  delete [] cpy;
  return result;
}

IloBool IsEmpty(const char* line) {
  return (line[0]==0);
}

// --------------------------------------------------------------------------
// Histogram for recording statistics

class Histogram {
public:
  Histogram(IloEnv env) :_values(env), _min(+IloInfinity), _max(-IloInfinity) {}
  ~Histogram() { }
  void addValue(IloNum x);
  IloInt getSize() const { return _values.getSize(); }
  IloNum getMin() const { return _min; }
  IloNum getMax() const { return _max; }
  IloNum getMean() const;
  IloNum getStandardDeviation() const; 
  IloNum getRootMeanSquareError(IloNum ref) const;
  void display(ILOSTD(ostream)& str) const;
  void createTizZ(const char* file, 
                  const char* title  =0,
                  const char* xlabel =0,
                  IloNum xmin        =+IloInfinity,
                  IloNum xmax        =-IloInfinity,
                  IloInt nbBins      =-1);
private:           
  void writeProlog(ILOSTD(ostream)& str,
                   const char* title,
                   const char* xlabel,
                   IloNum xmin, IloNum xmax,
                   IloNum ymin, IloNum ymax) const;
                   
  void writeConclusion(ILOSTD(ostream)& str) const;
private:
  IloNumArray _values;
  IloNum _min;
  IloNum _max;
};

void Histogram::addValue(IloNum x) { 
  _values.add(x); 
  if (x < _min) {
    _min = x;
  } 
  if (x > _max) {
    _max = x;
  } 
}

IloNum Histogram::getMean() const {
  const IloInt n = getSize();
  IloNum sum1 = 0.0;
  for (IloInt i=0; i<n; ++i) 
    sum1 += _values[i];
  return sum1 / IloNum(n);
}

IloNum Histogram::getStandardDeviation() const {
  // Compensated variant algorithm
  IloInt n = getSize();
  IloNum sum1 = 0.0;
  IloNum sum2 = 0.0;
  IloNum sum3 = 0.0;
  for (IloInt i=0; i<n; ++i) {
    sum1 = sum1 + _values[i];
  }
  IloNum mean = sum1/IloNum(n);
  for (IloInt i=0; i<n; ++i) {
    const IloNum x = _values[i];
    sum2 = sum2 + pow((x - mean),2);
    sum3 = sum3 + (x - mean);
  }
  return sqrt((sum2 - pow(sum3,2)/IloNum(n))/IloNum(n - 1)); 
}
 
IloNum Histogram::getRootMeanSquareError(IloNum ref) const {
  IloInt n = getSize();
  IloNum sum2 = 0.0;
  for (IloInt i=0; i<n; ++i) {
    const IloNum x = _values[i];
    sum2 += pow(x-ref, 2);
  }
  return sqrt(sum2/(IloNum)n);
}

void Histogram::display(ILOSTD(ostream)& str) const {
  str.precision(10);
  str << "Mean  = " << getMean() << std::endl;
  str << "Min   = " << getMin() << std::endl;
  str << "Max   = " << getMax() << std::endl;
  str << "StDev = " <<  getStandardDeviation() << std::endl;
}

void Histogram::writeProlog(ILOSTD(ostream)& str,
                            const char* title,
                            const char* xlabel,
                            IloNum xmin, IloNum xmax,
                            IloNum ymin, IloNum ymax) const {
  str << "\\begin{tikzpicture}" << std::endl;
  str << "\\begin{axis}[" << std::endl;
  str << "    %axis on top," << std::endl;
  str << "    align=left," << std::endl;
  if (title != 0) {
    str << "    title={" << title 
        << "\\\\ {\\em Mean = " << getMean() << "}" 
        << ", {\\em StDev = " << getStandardDeviation() << "}" 
        << "},"  << std::endl;
  } else {
    str << "    {\\em Mean = " << getMean() << "}" 
        << ", {\\em StDev = " << getStandardDeviation() << "}" 
        << "},"  << std::endl;
  }
  if (xlabel != 0) {
    str << "		xlabel={" << xlabel << "}," << std::endl;
  }
  str << "		ylabel={Probability}," << std::endl;
  str << "    scale only axis," << std::endl;
  str << "    ybar interval," << std::endl;
  str << "    x tick label as interval=false," << std::endl;
  str << "    xtick={}," << std::endl;
  str << "    yticklabel={}," << std::endl;
  str << "    xmin=" << xmin << ", xmax=" << xmax << "," << std::endl;
  str << "    ymin=" << ymin << ", ymax=" << ymax << "," << std::endl;
  str << "    ymajorgrids," << std::endl;
  str << "]" << std::endl;
  str << "\\addplot [blue, fill=blue!20] table [x=Lower, y=Count] {" << std::endl;
  str << "Lower Upper Count"  << std::endl;
}

void Histogram::writeConclusion(ILOSTD(ostream)& str) const {
  //str << "0.7475" << std::endl;
  str << "};" << std::endl;
  str << "\\end{axis}" << std::endl;
  str << "\\end{tikzpicture}" << std::endl;
}

void Histogram::createTizZ(const char* file, 
                           const char* title,
                           const char* xlabel,
                           IloNum xmin,
                           IloNum xmax,
                           IloInt nbBins) {
  IloInt n = getSize();
  if (nbBins < 0) {
    // Automatic computation of number of bins
    // Some people like n^(1/2), others n^(1/3), let's try n^(1/2.5)
    nbBins = IloInt(ceil(pow(IloNum(getSize()), 1.0/3)));
  }
  ILOSTD(ofstream) str(file);
#if defined(VERBOSE)
  std::cout << "Generating TikZ file " << file << " for histogram with " << nbBins << " bins." << std::endl;
#endif
  //str << "\\subsection{" << title << "}" << endl;
  IloInt* nbVals = new IloInt[nbBins];
  IloInt j;
  for (j=0; j<nbBins; ++j) {
    nbVals[j]=0;
  }
  for (IloInt i=0; i<n; ++i) {
    IloNum x = _values[i];
    j = IloInt(floor(IloNum(nbBins) * (x-_min) / (_max-_min)));
    if (j==nbBins)
     j--;
    assert(j<nbBins);
    nbVals[j]++;
  }
  IloNum ymin = +IloInfinity;
  IloNum ymax = -IloInfinity;
  for (j=0; j<nbBins; ++j) {
    IloNum y = IloNum(nbVals[j])/IloNum(n);
    if (y < ymin) {
      ymin = y;
    } 
    if (y > ymax) {
      ymax = y;
    }     
  }
  if (xmin == +IloInfinity) {
    xmin = _min;
  }
  if (xmax == -IloInfinity) {
    xmax = _max;
  }
  writeProlog(str, title, xlabel, xmin, xmax, ymin, ymax);
  for (j=0; j<nbBins; ++j) {
    str << _min+j*(_max-_min)/IloNum(nbBins)     << " " 
        << _min+(j+1)*(_max-_min)/IloNum(nbBins) << " " 
        << IloNum(nbVals[j])/IloNum(n) << std::endl;
  }
  str << _max << std::endl;
  writeConclusion(str);
  str << std::endl;
}

// --------------------------------------------------------------------------
// Function for display 

class PwlFunction {
public:
  PwlFunction(IloEnv env);
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
	str << " tick label style={/pgf/number format/1000 sep={}}," << endl;
	str << " xlabel={Start time}," << endl;
	str << " ylabel={Travel time}," << endl;
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

// --------------------------------------------------------------------------
// 

IloInt lengthTimeWindow = 0;
IloInt nbVertices = 0;
IloInt nbTimeWindows = 0;
  
// --------------------------------------------------------------------------
// 

void zoom(IloEnv env, IloInt*** ttime, IloInt location1, IloInt location2, IloInt smoothness) {
  cout << "TRAVEL TIMES BETWEEN LOCATION " << location1 
       << " AND LOCATION " << location2 << ":" << endl;
  IloInt ok = IloTrue;
  if (nbVertices <= location1) {
    ok = IloFalse;
    cout << "INVALID LOCATION " << location1 
         << " (NUMBER OF LOCATIONS IS " << nbVertices << ")" << endl;
  }
  if (nbVertices <= location2) {
    ok = IloFalse;
    cout << "INVALID LOCATION " << location2 
         << " (NUMBER OF LOCATIONS IS " << nbVertices << ")" << endl;
  }
  if (!ok) {
    return;
  }
  IloNum min = IloIntMax;
  IloNum max = IloIntMin;
  IloNum sum = 0;
  IloNum nlmins = 0;
  for (IloInt j=0; j<nbTimeWindows; j++) {
    IloNum tt = IloNum(ttime[location1][location2][j]);
    if (tt<min) min=tt;
    if (tt>max) max=tt;
    sum+=tt;
    if ((0<j) && (j<nbTimeWindows+1)) {
      if ((tt < ttime[location1][location2][j-1]) && 
          (tt <= ttime[location1][location2][j+1])) {
        nlmins++;  
      }
    }
    cout << IloInt(tt) << " ";
  }
  cout << endl;
  
  IloNum avg = sum/IloNum(nbTimeWindows);
  cout.precision(4);
  cout << "AMPLITUDE: " << 100*(max-min)/avg << "%  [" << -100*(avg-min)/avg << "% | +" << 100*(max-avg)/avg << "%]" << endl;
  cout << "NUMBER OF LOCAL MINIMAS: " << nlmins << endl;
  PwlFunction f(env);
  for (IloInt j=0; j<nbTimeWindows; j++) {
    if (j+1<nbTimeWindows) {
      f.setSlope(lengthTimeWindow*j, lengthTimeWindow*(j+1), ttime[location1][location2][j], IloNum(ttime[location1][location2][j+1]-ttime[location1][location2][j])/IloNum(lengthTimeWindow));
    } else {
      f.setValue(lengthTimeWindow*j, lengthTimeWindow*(j+1), ttime[location1][location2][j]);
    }
  }
  char file[128];
  sprintf(file, "tt_%ld_%ld_%ld.tikz", location1, location2, smoothness);
  char title[128];
  sprintf(title, "Travel time function between location %ld and location %ld", location1, location2);
  f.createTizZ(file, title);
}
  
void generateStatistics(IloInt*** ttime, const char* localMinima, const char* relativeAmplitude) {
  IloEnv env;
  Histogram nbLocalMinimas(env);
  Histogram relMinAmplitude(env);
  Histogram relMaxAmplitude(env);
  Histogram relAmplitude(env);
  IloInt i,j;
  for (i=0; i<nbVertices; i++){
    for (j=0; j<nbVertices; j++){
      if (i!=j) {
        IloNum min = IloIntMax;
        IloNum max = IloIntMin;
        IloNum sum = 0;
        IloNum nlmins = 0;
        for (IloInt k=0; k<nbTimeWindows; k++) {
          IloNum tt = IloNum(ttime[i][j][k]);
          if (tt<min) min=tt;
          if (tt>max) max=tt;
          sum+=tt;
          if (((k==0)||(tt < ttime[i][j][k-1])) && 
              ((k==nbTimeWindows-1)||(tt <= ttime[i][j][k+1]))) {
            nlmins++;  
          }
        }
        if (0<sum) {
          IloNum avg = sum/IloNum(nbTimeWindows);
          /*
          cout.precision(4);
          cout << "AMPLITUDE: " << 100*(max-min)/avg << "%  [" << 100*(avg-min)/avg << "% | +" << 100*(max-avg)/avg << "%]" << endl;
          cout << "NUMBER OF LOCAL MINIMAS: " << nlmins << endl;
          */
          relMinAmplitude.addValue(100*(avg-min)/avg);
          relMaxAmplitude.addValue(100*(max-avg)/avg);
          relAmplitude.addValue(100*(max-min)/min);
          nbLocalMinimas.addValue(nlmins);
        }
      }
    }
  }
  IloInt minLocalMinimas = nbLocalMinimas.getMin();
  IloInt maxLocalMinimas = nbLocalMinimas.getMax();
  nbLocalMinimas.createTizZ(localMinima, "Distribution of number of local minimas", "Number of local minimas", minLocalMinimas, maxLocalMinimas, maxLocalMinimas-minLocalMinimas);
  relAmplitude.createTizZ(relativeAmplitude, "Distribution of relative amplitude", "Relative amplitude (\\%)");
  env.end();
}

// --------------------------------------------------------------------------
// 

void generateStatisticsTT(IloInt*** ttime, const char* triangIneqViol, const char* transT, const char* deltaTransT, const char* asym) {
  IloEnv env;
  Histogram triangIneqViolations(env);
  Histogram transTimes(env);
  Histogram deltaTransTimes(env);
  Histogram asymmetries(env);
  IloInt i,j,k,t, counter=0, total=0, asymmetry;

  IloInt horizon = nbTimeWindows*lengthTimeWindow;
  for (t=0; t<nbTimeWindows; t++) {
    IloInt ti=t*lengthTimeWindow;
    for(i=0; i<nbVertices; i++){
      for(j=0; j<nbVertices; j++) {
        transTimes.addValue(ttime[i][j][t]);
        total++;
        if(t!=0) deltaTransTimes.addValue(ttime[i][j][t] - ttime[i][j][t-1]);
        if (i!=j) {
          asymmetry= ttime[i][j][t] - ttime[j][i][t];
          if(asymmetry > 0)
            asymmetries.addValue(asymmetry);
          if(asymmetry < 0)
            asymmetries.addValue(asymmetry*(-1));
          IloInt tij=ti+ttime[i][j][t];
          if (tij<horizon) {
            for(k=0; k<nbVertices; k++) {
              if ((i!=k)&&(j!=k)) {
                IloInt tijk = tij+ttime[j][k][tij/lengthTimeWindow];
                IloInt tik  = ti+ttime[i][k][t];
                if ((tijk<horizon) && (tik<horizon)){
                  if (tijk < tik) {
                    triangIneqViolations.addValue(tik- tijk);
                    counter++;
                  }
                }
              }
            }
          }
        }
      }
    }
  }
  triangIneqViolations.createTizZ(triangIneqViol, "Distribution of violations of the triangular inequality", "Violations of the triangular inequality");
  transTimes.createTizZ(transT, "Distribution of transition times", "Transition times");
  deltaTransTimes.createTizZ(deltaTransT, "Distribution of delta", "Delta= difference between consecutive transition times");
  asymmetries.createTizZ(asym, "Distribution of relative asymmetries", "Relative asymmetries");
  std::cout << "Percentage of violating pairs: " << (double)counter/(double) total << std::endl;
  env.end();
}

IloBool checkTriangularInequality(IloEnv env, IloInt*** ttime) {
  IloBool ok = IloTrue;
  IloInt i,j,k,t;
  IloInt horizon = nbTimeWindows*lengthTimeWindow;
  for (t=0; t<nbTimeWindows; t++) {
    IloInt ti=t*lengthTimeWindow;
    for(i=0; i<nbVertices; i++){
      for(j=0; j<nbVertices; j++) {
        if (i!=j) {
          IloInt tij=ti+ttime[i][j][t];
          if (tij<horizon) {
            for(k=0; k<nbVertices; k++) {
              if ((i!=k)&&(j!=k)) {
                IloInt tijk = tij+ttime[j][k][tij/lengthTimeWindow];
                IloInt tik  = ti+ttime[i][k][t];
                if ((tijk<horizon) && (tik<horizon)){
                  if (tijk+2*lengthTimeWindow<tik) {
                    ok = IloFalse;
                    std::cout << "Triangular inequality not satisfied for"
                              << " i=" << i 
                              << " j=" << j
                              << " k=" << k
                              << " t=" << t 
                              << ": ti=" << ti 
                              << " tij=" << tij
                              << " tijk=" << tijk
                              << " tik=" << tik
                              << std::endl;
                  } else {
                    
                  }
                }
              }
            }
          }
        }
      }
    }
  }
  return ok;
}

void checkFIFO(IloEnv env, IloInt*** ttime){
  std::cout << "Checking for intrinsic non-FIFOness" << std::endl;
  IloInt i,j,k,t;
  for(i=0; i<nbVertices; i++){
    for(j=0; j<nbVertices; j++) {
      for(k=1; k<nbTimeWindows; k++) {
        if( (((ttime[i][j][k-1]- ttime[i][j][k])/(2*lengthTimeWindow))  > 1) || (((ttime[i][j][k-1]- ttime[i][j][k])/(2*lengthTimeWindow)) < -1)){
          std::cerr << i << " " << j << " " << k << " " << (ttime[i][j][k-1]- ttime[i][j][k])/(2*lengthTimeWindow) << std::endl;
        }
      }
    }
  }
}
// --------------------------------------------------------------------------
// 

int main(int argc, const char* argv[]){
  IloEnv env;
  const char* file;
  if (argc>1) {
    file = argv[1];
  }
  IloInt location1 = 0;
  IloInt location2 = 0;
  if (argc>3) {
    location1 = atoi(argv[2]);
    location2 = atoi(argv[3]);
  }
  
  ifstream input(file);
  if (!input.is_open()) {
    std::cout << "Can't open file '" << file << "'!" << endl;
  }
  const IloInt lineLimit = 131072;
  char line[lineLimit];
  IloInt r = 0;
  
  // while (r<3 && input.getline(line, lineLimit)) {
    // if (Contains(line, "sizeOfTimeStep")) {
      // lengthTimeWindow = GetIntAfterEqual(line);
      // r++;
    // } else if (Contains(line, "n")) {
       // nbVertices = GetIntAfterEqual(line);
       // r++;
    // } else if (Contains(line, "m")) {
      // nbTimeWindows = GetIntAfterEqual(line);
      // r++;
    // } 
  // }
  input >> nbVertices;
  input >> nbTimeWindows;
  input >> lengthTimeWindow;
  
  IloInt*** ttime = new IloInt**[nbVertices];
  IloInt i,j,k;
  for(i=0; i<nbVertices; i++){
    ttime   [i] = new IloInt*[nbVertices];
    for(j=0; j<nbVertices; j++) {
      ttime[i][j] = new IloInt[nbTimeWindows];
    }
  }
  i=0; j=0; k=-1;
  IloInt n=0;
  while (input.getline(line, lineLimit)) {
    IloInt* vals = GetIntArray(line, n);
    for (IloInt l=0; l<n; ++l) {
      IloInt v = vals[l];
      k++;
      if (k==nbTimeWindows) { k=0; j++;  }
      if (j==nbVertices)    { j=0; i++; }
      ttime[i][j][k] = v;
    }
  }
  input.close();
  cout << "NUMBER OF LOCATIONS   : " << nbVertices << endl;
  cout << "NUMBER OF TIME-WINDOWS: " << nbTimeWindows << endl;
  cout << "SIZE OF TIME-WINDOWS  : " << lengthTimeWindow << endl;
  
  generateStatisticsTT(ttime,"triangIneqViolations.tikz","transTimes.tikz","deltaTransTimes.tikz", "asym.tikz"); 
  // generateStatistics(ttime, "nbLocalMinimas0.tikz", "relativeAmplitude0.tikz");
  // zoom(env, ttime, location1, location2, 0);
  // checkTriangularInequality(env, ttime);
  
  // Smoothing
  // IloInt*** ttime1 = new IloInt**[nbVertices];
  // for(i=0; i<nbVertices; i++){
    // ttime1[i] = new IloInt*[nbVertices];
    // for(j=0; j<nbVertices; j++) {
      // ttime1[i][j] = new IloInt[nbTimeWindows];
      // for (k=0; k<nbTimeWindows; k++) {
        // IloInt s = ttime[i][j][k];
        // IloInt n = 1;
        // if (0<k) {
          // s += ttime[i][j][k-1];
          // n++;
        // }
        // if (k<nbTimeWindows-1) {
          // s += ttime[i][j][k+1];
          // n++;
        // }
        // ttime1[i][j][k] = s / n;
      // }
    // }
  // }
  // generateStatistics(ttime1, "nbLocalMinimas1.tikz", "relativeAmplitude1.tikz");
  // zoom(env, ttime1, location1, location2, 1);
  
  // IloInt*** ttime2 = new IloInt**[nbVertices];
  // for(i=0; i<nbVertices; i++){
    // ttime2[i] = new IloInt*[nbVertices];
    // for(j=0; j<nbVertices; j++) {
      // ttime2[i][j] = new IloInt[nbTimeWindows];
      // for (k=0; k<nbTimeWindows; k++) {
        // IloInt s = ttime1[i][j][k];
        // IloInt n = 1;
        // if (0<k) {
          // s += ttime1[i][j][k-1];
          // n++;
        // }
        // if (k<nbTimeWindows-1) {
          // s += ttime1[i][j][k+1];
          // n++;
        // }
        // ttime2[i][j][k] = s / n;
      // }
    // }
  // }
  // generateStatistics(ttime2, "nbLocalMinimas2.tikz", "relativeAmplitude2.tikz");
  // zoom(env, ttime2, location1, location2, 2);
  
  // IloInt*** ttime3 = new IloInt**[nbVertices];
  // for(i=0; i<nbVertices; i++){
    // ttime3[i] = new IloInt*[nbVertices];
    // for(j=0; j<nbVertices; j++) {
      // ttime3[i][j] = new IloInt[nbTimeWindows];
      // for (k=0; k<nbTimeWindows; k++) {
        // IloInt s = ttime2[i][j][k];
        // IloInt n = 1;
        // if (0<k) {
          // s += ttime2[i][j][k-1];
          // n++;
        // }
        // if (k<nbTimeWindows-1) {
          // s += ttime2[i][j][k+1];
          // n++;
        // }
        // ttime3[i][j][k] = s / n;
      // }
    // }
  // }
  // generateStatistics(ttime3, "nbLocalMinimas3.tikz", "relativeAmplitude3.tikz");
  
  // zoom(env, ttime3, location1, location2, 3);
  
  // analyze.exe ../tdtsp_bench_255_8h30.in  22 34
  // analyze.exe ../tdtsp_bench_255_8h30.in  18 29
  // checkFIFO(env, ttime);
  
  env.end();
  return 0;
}
