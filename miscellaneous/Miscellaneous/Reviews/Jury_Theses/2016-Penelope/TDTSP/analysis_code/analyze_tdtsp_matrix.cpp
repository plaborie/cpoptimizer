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
        << ","  << std::endl;
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
  str << " height=4cm, width=8cm, grid=major," << endl;
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

class TDTSPMatrix {
  public:
    TDTSPMatrix(IloInt benchSize, IloInt nbTimeWindows, IloInt lengthTimeWindows);
    TDTSPMatrix(const char* file);
    ~TDTSPMatrix();
    void setValue(IloInt i, IloInt j, IloInt k, IloInt v) { matrix[i][j][k]=v; }
    void dump(const char* file);
    void generateFunction(const char* file, IloInt location1, IloInt location2);
    IloInt benchSize;
    IloInt nbTimeWindows; 
    IloInt lengthTimeWindow; 
    IloInt*** matrix;
};

TDTSPMatrix::TDTSPMatrix(IloInt s, IloInt n, IloInt l)
 :benchSize       (s)
 ,nbTimeWindows   (n)
 ,lengthTimeWindow(l)
 ,matrix          (0)
 {
   matrix = new IloInt**[benchSize];
   for(IloInt i=0; i<benchSize; i++){
      matrix[i] = new IloInt*[benchSize];
      for(IloInt j=0; j<benchSize; j++) {
        matrix[i][j] = new IloInt[nbTimeWindows];
        for (IloInt k=0; k<nbTimeWindows; ++k) {
          matrix[i][j][k] = 0;
        }
      }
    }
 }
    
TDTSPMatrix::TDTSPMatrix(const char* file) {
  ifstream input_bench(file);
  if (!input_bench.is_open()) {
    std::cout << "Can't open file '" << file << "'!" << endl;
  }
  benchSize        =0;
  nbTimeWindows    =0;
  lengthTimeWindow =0;
  input_bench >> benchSize;
  input_bench >> nbTimeWindows;
  input_bench >> lengthTimeWindow;
  matrix = new IloInt**[benchSize];
  for(IloInt i=0; i<benchSize; i++){
    matrix[i] = new IloInt*[benchSize];
    for(IloInt j=0; j<benchSize; j++) {
      matrix[i][j] = new IloInt[nbTimeWindows];
      for (IloInt k=0; k<nbTimeWindows; ++k) {
        input_bench >> matrix[i][j][k];
      }
    }
  }
  input_bench.close();
}

TDTSPMatrix::~TDTSPMatrix() {
  for(IloInt i=0; i<benchSize; i++){
    for(IloInt j=0; j<benchSize; j++) {
      delete[] matrix[i][j];
    }
    delete [] matrix[i];
  }
  delete [] matrix;
}

void TDTSPMatrix::dump(const char* file) {
  ofstream output_bench(file);
  if (!output_bench.is_open()) {
    std::cout << "Can't open file '" << file << "'!" << endl;
  }
  output_bench << benchSize << " ";
  output_bench << nbTimeWindows << " ";
  output_bench << lengthTimeWindow << std::endl;
  for(IloInt i=0; i<benchSize; i++){
      for(IloInt j=0; j<benchSize; j++) {
        for (IloInt k=0; k<nbTimeWindows; ++k) {
          output_bench << matrix[i][j][k] << " ";
        }
      }
    }
}
                    
void TDTSPMatrix::generateFunction(const char* file, IloInt location1, IloInt location2) {
  IloEnv env;
  PwlFunction f(env);
  for (IloInt j=0; j<nbTimeWindows; j++) {
    //if (j+1<nbTimeWindows) {
    //  f.setSlope(lengthTimeWindow*j, lengthTimeWindow*(j+1), 
    //             matrix[location1][location2][j], 
    //             IloNum(matrix[location1][location2][j+1]-matrix[location1][location2][j])/IloNum(lengthTimeWindow));
    //} else {
      f.setValue(lengthTimeWindow*j, lengthTimeWindow*(j+1), matrix[location1][location2][j]);
    //}
  }
  char title[128];
  sprintf(title, "Travel time function between location %ld and location %ld", location1, location2);
  f.createTizZ(file, title);
  env.end();
}

// --------------------------------------------------------------------------
// 

class Triple {
public:
  Triple(IloInt i, IloInt j, IloNum v) :_i(i),_j(j),_v(v){}
  ~Triple(){}
  IloInt _i;
  IloInt _j;
  IloNum _v;
};


int Compare (const void * b, const void * a){
  IloNum val=(*((Triple**)a))->_v - (*((Triple**)b))->_v;
  if (val < 0) {
    return -1;
  } else if (val > 0) {
    return +1;
  }
	return 0;
}

void MergeMatrices(TDTSPMatrix* m1, TDTSPMatrix* m2, TDTSPMatrix*& m12) {
  IloInt size = m1->benchSize;
  IloInt n = size*size;
  Triple** avg1_34_38 = new Triple*[n];
  Triple** avg2_9_13 = new Triple*[n];
  IloNum avg1 = 0.0;
  IloNum avg2 = 0.0;
  IloEnv env;
  Histogram havg1(env);
  Histogram havg2(env);
  for (IloInt i=0; i<size; ++i) {
    for (IloInt j=0; j<size; ++j) {
      IloNum v1 = 0.0;
      IloNum v2 = 0.0;
      for (IloInt k=1; k<=13; ++k) {
        v1 += k*m1->matrix[i][j][k+25]; // v1[26]*1 + v1[27]*2 + ... + v1[38]*13
        v2 += k*m2->matrix[i][j][k];    // v2[01]*1 + v2[02]*2 + ... + v2[13]*13
      }
      avg1_34_38[i*size+j] = new Triple(i,j,v1);
      avg1 += v1;
      avg2_9_13[i*size+j] = new Triple(i,j,v2);
      avg2 += v2;
      havg1.addValue(v1);
      havg2.addValue(v2);
    }
  }
  avg1 = avg1 / IloNum(n);
  avg2 = avg2 / IloNum(n);

  havg1.createTizZ("havg1.tikz");
  havg2.createTizZ("havg2.tikz");
  std::cout << "AVG1=" << avg1 << " AVG2=" << avg2 << std::endl;
  
  qsort(avg1_34_38, n, sizeof(Triple*), Compare);
  qsort(avg2_9_13, n, sizeof(Triple*), Compare);
  
  for (IloInt i=0; i<n; i+=100) {
     std::cout << "avg1_34_38[" << i << "]=" << avg1_34_38[i]->_v  << "\t avg2_9_13[" << i << "]=" << avg2_9_13[i]->_v  << std::endl;
  }
  IloNum ratio = avg1 / avg2;
  
  m12 = new TDTSPMatrix(size, 65, m1->lengthTimeWindow);
  for (IloInt i=0; i<size; ++i) {
    for (IloInt j=0; j<size; ++j) {
      Triple* t1 = avg1_34_38[i*size+j];
      Triple* t2 = avg2_9_13[i*size+j];
      IloInt i1 = t1->_i;
      IloInt j1 = t1->_j;
      IloInt i2 = t2->_i;
      IloInt j2 = t2->_j;
      for (IloInt k=0; k<39; k++) {
        m12->matrix[i1][j1][k] = m1->matrix[i1][j1][k];    
      }
      for (IloInt k=38; k<65; k++) {
        m12->matrix[i1][j1][k] = IloInt(ratio*m2->matrix[i2][j2][k-25]);    
      }
    }
  }
}

void MergeMatrices() {
  TDTSPMatrix m1("tdtsp_bench_255_6h00.txt");
  TDTSPMatrix m2("tdtsp_bench_255_8h30.txt");
  TDTSPMatrix* m12;
  
  MergeMatrices(&m1, &m2, m12);
  m12->dump("tdtsp_bench_255_6h00_12h30.txt");
  IloEnv env;
  IloRandom rand(env);
  char fname[128];
  for (IloInt i=0; i<100; i++) {
    IloInt l1 = rand.getInt(255);
    IloInt l2 = rand.getInt(255);
    while (l2 == l1) {
      l2 = rand.getInt(255);
    }
    sprintf(fname, "ttime_%ld_%ld.tikz", l1, l2);
    m12->generateFunction(fname, l1, l2);
  }
   sprintf(fname, "ttime_%ld_%ld.tikz", 109, 253);
   m12->generateFunction(fname, 109, 253);
   sprintf(fname, "ttime_%ld_%ld.tikz", 253, 109);
   m12->generateFunction(fname, 253, 109);
  env.end();
}

int main(int argc, const char* argv[]) {
  IloEnv env;
  const char* file;
  if (argc>1) {
    file = argv[1];
  }
  // Matrice merging
   MergeMatrices();
   return 0;
  
  TDTSPMatrix matrix(file);
  IloRandom rand(env);
  char fname[128];
  for (IloInt i=0; i<100; i++) {
    IloInt l1 = rand.getInt(255);
    IloInt l2 = rand.getInt(255);
    while (l2 == l1) {
      l2 = rand.getInt(255);
    }
    sprintf(fname, "ttime_%ld_%ld.tikz", l1, l2);
    matrix.generateFunction(fname, l1, l2);
  }
    
  return 0;
}