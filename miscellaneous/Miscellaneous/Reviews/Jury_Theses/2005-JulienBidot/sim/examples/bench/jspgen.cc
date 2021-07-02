#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <limits.h>
#include <time.h>

#include <vector>
#include <utility>
#include <map>
#include <string>
#include <algorithm>
//#include <bits/stl_numeric.h>

#include <iomanip>
#include <iostream>

using namespace std;

static map<string,string> GBL_OptionalParameterValueMap;

void storeOptionalParameters(char **parms,int numParms)
{
  for(int i=0;i<numParms;i++)
    {
      char *option=parms[i];
      char *equalPos=strchr(option,'=');
      if(equalPos==0)
	{
	  cout << "Invalid option: " << option << " (ignored)" << endl;
	}
      else
	{
	  // form the keyword/value components from the option string.
	  (*equalPos)='\0';
	  string key(option+1);     // skip the dash
	  string value(equalPos+1); 
	  GBL_OptionalParameterValueMap[key]=value;
	}
    }
}

bool optionPresent(const string &option)
{
  map<string,string>::iterator iter=GBL_OptionalParameterValueMap.find(option);
  return iter!=GBL_OptionalParameterValueMap.end();
}

int optionAsInteger(const string &option)
{
  assert(optionPresent(option));

  const char *stringData=GBL_OptionalParameterValueMap[option].c_str();

  return atoi(stringData);
}

double optionAsDouble(const string &option)
{
  assert(optionPresent(option));

  const char *stringData=GBL_OptionalParameterValueMap[option].c_str();

  return atof(stringData);
}

string optionAsString(const string &option)
{
  assert(optionPresent(option));

  return GBL_OptionalParameterValueMap[option];
}

////////////////////////////////////////////////////////////////////////////////////
// RANDOM NUMBER GENERATORS DERIVED FROM TAILLARD'S ORIGINAL PAPER
// AND NUMERICAL RECIPIES IN 'C'
////////////////////////////////////////////////////////////////////////////////////

static long GBL_RandNumSeed=0;

void setSeed(long newSeed)
{
  assert(newSeed>0);
  GBL_RandNumSeed=newSeed;
}

// generate a random number from 0.0 (inclusive) to 1.0 (exclusive)
double unifZeroOne(void)
{
  static long m=2147483647,a=16807,b=127773,c=2836;
  double value_0_1;              

  long k=GBL_RandNumSeed/b;
  GBL_RandNumSeed=a*(GBL_RandNumSeed%b)-k*c;
  if(GBL_RandNumSeed<0) 
    {
      GBL_RandNumSeed=GBL_RandNumSeed+m;
    }
  value_0_1=double(GBL_RandNumSeed)/(double)m;

  assert((value_0_1>=0.0)&&(value_0_1<1.0));

  return value_0_1;
}

// generate a number from low to high, inclusive
int unifSupplied(int low, int high)
{
  assert(low<=high);

  double value_0_1=unifZeroOne();

  int result=low+(int)(floor(value_0_1 * double(high - low + 1))); // floor returns the largest integer less than or equal to the argument number 

  assert((result>=low)&&(result<=high));

  return result;
}

// generate a random number from a normal distribution
double normal(double mean, double stdDev)
{
  // straight from 'Numerical Recipies in C'.
  // (the Box-Muller method)
  static int iset=0;
  double fac,r,v1,v2;
  static double gset;
  
  if(iset==0)
    {
      do
	{
	  v1=2.0*unifZeroOne()-1.0;
	  v2=2.0*unifZeroOne()-1.0;
	  r=v1*v1+v2*v2;
	}
      while(r>=1.0);
      fac=sqrt(-2.0*log(r)/r);
      gset=v1*fac;
      iset=1;
      return (v2*fac)*stdDev+mean;
    }
  else
    {
      iset=0;
      return gset*stdDev+mean;
    }

  // UNREACHABLE
}

////////////////////////////////////////////////////////////////////////////////////
// GLOBALS REPRESENTING THE GENERATED INSTANCE
////////////////////////////////////////////////////////////////////////////////////

// indexed by machine first, job second (both indicies are 0-based).
const int MAX_JOBS=500;
const int MAX_MACHINES=100;
int JSPDurations[MAX_MACHINES][MAX_JOBS]; // operation durations
int JSPMachines[MAX_MACHINES][MAX_JOBS];  // machines are 0-based indicies

///////////////////////////////////////////////////////////////////////////// 
// STUFF FOR DETERMINING THE LOWER/UPPER BOUNDS ON THE OPERATION DURATIONS //
///////////////////////////////////////////////////////////////////////////// 

const string DurationLBKeyword="durationLB";
const string DurationUBKeyword="durationUB";

static int GBL_DurationLB=1;
static int GBL_DurationUB=99;

bool loadDurationIntervalBounds(void)
{
  if(optionPresent(DurationLBKeyword))
    {
      GBL_DurationLB=optionAsInteger(DurationLBKeyword);
    }
  if(optionPresent(DurationUBKeyword))
    {
      GBL_DurationUB=optionAsInteger(DurationUBKeyword);
    }

  if(GBL_DurationLB>GBL_DurationUB)
    {
      cerr << "***Illegal bounds on the operation durations specified: " << endl;
      cerr << "***Duration lower bound: " << GBL_DurationLB << endl;
      cerr << "***Duration upper bound: " << GBL_DurationUB << endl;
      return false;
    }

  return true;
}

/////////////////////////////////////////////////////////////////
// ROUTINE FOR GENERATING STANDARD TAILLARD-LIKE JSP INSTANCES //
/////////////////////////////////////////////////////////////////

bool generateTaillardJobShop(int numJobs, int numMachines)
{
  for(int i=0;i<numMachines;i++)
    {
      for(int j=0;j<numJobs;j++)
	{
	  JSPDurations[i][j]=unifSupplied(GBL_DurationLB,
					  GBL_DurationUB);
	}
    }

  return true;
}

////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////
// ROUTINE FOR GENERATING RANDOM GAUSSIAN JSP INSTANCES //
//////////////////////////////////////////////////////////

bool generateGaussianJobShop(int numJobs, int numMachines)
{
  // compute the mean/variance of a Gaussian which 'covers' the duration interval 
  // width, using the fact that +-3 standard deviations covers 99+% of the distribution.
  int gaussianMean=((GBL_DurationUB-GBL_DurationLB)/2)+GBL_DurationLB;
  int gaussianSigma=(GBL_DurationUB-GBL_DurationLB)/6;

  for(int i=0;i<numMachines;i++)
    {
      for(int j=0;j<numJobs;j++)
	{
	  JSPDurations[i][j]=int(rint(normal(gaussianMean,gaussianSigma)));
	}
    }

  return true;
}

////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////
// ROUTINE FOR GENERATING JOB-CORRELATED JSP INSTANCES //
/////////////////////////////////////////////////////////

bool generateJobCorrelatedJobShop(int numJobs,int numMachines)
{
  const string DistributionHWLBKeyword="distHalfWidthLB";
  const string DistributionHWUBKeyword="distHalfWidthUB";
  const string AlphaKeyword="alpha";

  int distHWLB=1;
  int distHWUB=5;
  double alpha=0.5;        

  // process any command-line over-rides of the default parameters.
  if(optionPresent(DistributionHWLBKeyword))
    {
      distHWLB=optionAsInteger(DistributionHWLBKeyword);
    }
  if(optionPresent(DistributionHWUBKeyword))
    {
      distHWUB=optionAsInteger(DistributionHWUBKeyword);
    }

  if((distHWLB>distHWUB)||
     (distHWLB<=0)||
     (distHWUB<=0))
    {
      cerr << "***Illegal lower and/or upper bounds on the distribution half-widths were specified" << endl;
      cerr << "***Lower bound: " << distHWLB << endl;
      cerr << "***Upper bound: " << distHWUB << endl;
      return false;
    }

  if(optionPresent(AlphaKeyword))
    {
      alpha=optionAsDouble(AlphaKeyword);
    }

  if(alpha<0.0)
    {
      cerr << "***An illegal value of alpha was specified: " << alpha << endl;
      return false;
    }

  vector<int> distributionMeans(numJobs,0);
  vector<int> distributionHalfWidths(numJobs,0);

  int actualIntervalWidth=GBL_DurationUB-GBL_DurationLB;
  int effectiveIntervalWidth=int(rint(alpha*double(actualIntervalWidth)));
  int startPoint=unifSupplied(GBL_DurationLB,GBL_DurationUB-effectiveIntervalWidth);

  for(int i=0;i<numJobs;i++)
    {
      distributionMeans[i]=unifSupplied(startPoint,startPoint+effectiveIntervalWidth);
    }

  for(int i=0;i<numJobs;i++)
    {
      distributionHalfWidths[i]=unifSupplied(distHWLB,distHWUB);
    }

  for(int i=0;i<numMachines;i++)
    {
      for(int j=0;j<numJobs;j++)
	{
	  JSPDurations[i][j]=unifSupplied(distributionMeans[j]-distributionHalfWidths[j],
					  distributionMeans[j]+distributionHalfWidths[j]);
	}
    }

  return true;
}

/////////////////////////////////////////////////////////////
// ROUTINE FOR GENERATING MACHINE-CORRELATED JSP INSTANCES //
/////////////////////////////////////////////////////////////

bool generateMachineCorrelatedJobShop(int numJobs,int numMachines)
{
  const string DistributionHWLBKeyword="distHalfWidthLB";
  const string DistributionHWUBKeyword="distHalfWidthUB";
  const string AlphaKeyword="alpha";

  int distHWLB=1;
  int distHWUB=5;
  double alpha=0.5;        

  // process any command-line over-rides of the default parameters.
  if(optionPresent(DistributionHWLBKeyword))
    {
      distHWLB=optionAsInteger(DistributionHWLBKeyword);
    }
  if(optionPresent(DistributionHWUBKeyword))
    {
      distHWUB=optionAsInteger(DistributionHWUBKeyword);
    }

  if((distHWLB>distHWUB)||
     (distHWLB<=0)||
     (distHWUB<=0))
    {
      cerr << "***Illegal lower and/or upper bounds on the distribution half-widths were specified" << endl;
      cerr << "***Lower bound: " << distHWLB << endl;
      cerr << "***Upper bound: " << distHWUB << endl;
      return false;
    }

  if(optionPresent(AlphaKeyword))
    {
      alpha=optionAsDouble(AlphaKeyword);
    }

  if(alpha<0.0)
    {
      cerr << "***An illegal value of alpha was specified: " << alpha << endl;
      return false;
    }

  vector<int> distributionMeans(numMachines,0);
  vector<int> distributionHalfWidths(numMachines,0);

  int actualIntervalWidth=GBL_DurationUB-GBL_DurationLB;
  int effectiveIntervalWidth=int(rint(alpha*double(actualIntervalWidth)));
  int startPoint=unifSupplied(GBL_DurationLB,GBL_DurationUB-effectiveIntervalWidth);

  for(int i=0;i<numMachines;i++)
    {
      distributionMeans[i]=unifSupplied(startPoint,startPoint+effectiveIntervalWidth);
    }

  for(int i=0;i<numMachines;i++)
    {
      distributionHalfWidths[i]=unifSupplied(distHWLB,distHWUB);
    }

  for(int i=0;i<numMachines;i++)
    {
      for(int j=0;j<numJobs;j++)
	{
	  JSPDurations[i][j]=unifSupplied(distributionMeans[i]-distributionHalfWidths[i],
					  distributionMeans[i]+distributionHalfWidths[i]);
	}
    }

  return true;
}

////////////////////////////////////////////////////////////
// ROUTINE FOR GENERATING MIXED-CORRELATION JSP INSTANCES //
////////////////////////////////////////////////////////////

bool generateMixedCorrelatedJobShop(int numJobs,int numMachines)
{
  const string DistributionHWLBKeyword="distHalfWidthLB";
  const string DistributionHWUBKeyword="distHalfWidthUB";
  const string AlphaKeyword="alpha";
  const string DurationNoiseKeyword="durationNoise";

  int distHWLB=1;
  int distHWUB=5;
  double alpha=0.5;        
  int durationNoise=0;

  // process any command-line over-rides of the default parameters.
  if(optionPresent(DistributionHWLBKeyword))
    {
      distHWLB=optionAsInteger(DistributionHWLBKeyword);
    }
  if(optionPresent(DistributionHWUBKeyword))
    {
      distHWUB=optionAsInteger(DistributionHWUBKeyword);
    }

  if((distHWLB>distHWUB)||
     (distHWLB<=0)||
     (distHWUB<=0))
    {
      cerr << "***Illegal lower and/or upper bounds on the distribution half-widths were specified" << endl;
      cerr << "***Lower bound: " << distHWLB << endl;
      cerr << "***Upper bound: " << distHWUB << endl;
      return false;
    }

  if(optionPresent(AlphaKeyword))
    {
      alpha=optionAsDouble(AlphaKeyword);
    }

  if(alpha<0.0)
    {
      cerr << "***An illegal value of alpha was specified: " << alpha << endl;
      return false;
    }

  if(optionPresent(DurationNoiseKeyword))
    {
      durationNoise=optionAsInteger(DurationNoiseKeyword);
    }

  if(durationNoise<0)
    {
      cerr << "***An illegal duration noise value was specified: " << durationNoise << endl;
      return false;
    }

  vector<int> distributionMeans(numMachines,0);
  vector<int> distributionHalfWidths(numMachines,0);

  int actualIntervalWidth=GBL_DurationUB-GBL_DurationLB;
  int effectiveIntervalWidth=int(rint(alpha*double(actualIntervalWidth)));
  int startPoint=unifSupplied(GBL_DurationLB,GBL_DurationUB-effectiveIntervalWidth);

  for(int i=0;i<numMachines;i++)
    {
      distributionMeans[i]=unifSupplied(startPoint,startPoint+effectiveIntervalWidth);
    }

  for(int i=0;i<numMachines;i++)
    {
      distributionHalfWidths[i]=unifSupplied(distHWLB,distHWUB);
    }

  // determine the relative orderings of the jobs within each distribution:
  // 0.0  = lowest value in the distribution
  // 0.5  = distribution mean
  // ~1.0 = highest value in the distribution
  vector<double> relativeJobOrder(numJobs,0.0);
  for(int i=0;i<numJobs;i++)
    {
      relativeJobOrder[i]=unifZeroOne();
    }

  // and select the operation durations.
  for(int i=0;i<numMachines;i++)
    {
      for(int j=0;j<numJobs;j++)
	{
	  int distLowerBound=distributionMeans[i]-distributionHalfWidths[i];
	  int distUpperBound=distributionMeans[i]+distributionHalfWidths[i];
	  int distWidth=distUpperBound-distLowerBound;
	  int thisJobMean=int(rint(relativeJobOrder[j]*double(distWidth)))+distLowerBound;
	  int sampleError=unifSupplied(-durationNoise,durationNoise);
	  int thisDuration=thisJobMean+sampleError;

	  JSPDurations[i][j]=thisDuration;
	}
    }

  return true;
}

////////////////////////////////////////////////////////////////////////////////////

// assign machines to each of the job operations - implements 
// Taillard's method, with work-flow partitioning among the machines.
void computeOperationMachineAssignments(int numJobs,
					int numMachines,
					int numPartitions)
{
  // default the operation machine to the operation # (e.g., a flow-shop).
  for(int i=0;i<numJobs;i++)
    {
      for(int j=0;j<numMachines;j++)
	{
	  JSPMachines[j][i]=j;
	}
    }

  // randomly swap orderings to create the actual assignments.
  assert((numMachines%numPartitions)==0);
  int partitionSize=numMachines/numPartitions;

  for(int i=0;i<numJobs;i++)
    {
      for(int j=0;j<numPartitions;j++)
	{
	  for(int k=0;k<partitionSize;k++)
	    {
	      int thisMachineIdx=((j)*partitionSize)+k;
	      int lastMachineIdx=((j+1)*partitionSize)-1;
	      int swapIndex=unifSupplied(thisMachineIdx,lastMachineIdx);
	      int temp=JSPMachines[swapIndex][i];
	      JSPMachines[swapIndex][i]=JSPMachines[thisMachineIdx][i];
	      JSPMachines[thisMachineIdx][i]=temp;
	    }
	}
    }
}

//////////////////////////////////////////
// ROUTINES FOR GENERATING LOWER BOUNDS //
//////////////////////////////////////////

// computes the index of the operation of the input job
// that is to be processed on the input machine - just
// computes a 1-d scan of a row of JSPMachines.
int operationIndex(int job,int machine,int numMachines)
{
  for(int i=0;i<numMachines;i++)
    {
      if(JSPMachines[i][job]==machine)
	{
	  return i;
	}
    }

  // to force failure - you should never get this far.
  assert(true);
  return 0;
}

// straight from Taillard's paper 'Benchmarks for basic scheduling problems'.
// currently does not include consideration of setup times.
long taillardLowerBound(int numJobs,int numMachines)
{
  vector<long> b(numMachines,LONG_MAX); // Bi's
  vector<long> a(numMachines,LONG_MAX); // Ai's
  vector<long> t(numMachines,0);        // Ti's

  for(int i=0;i<numMachines;i++)
    {
      // compute Bi's.
      long minSoFar=LONG_MAX;
      for(int j=0;j<numJobs;j++)
	{
	  long thisMin=0;
	  for(int k=0;k<operationIndex(j,i,numMachines);k++)
	    {
	      thisMin+=JSPDurations[k][j];
	    }
	  minSoFar=min(minSoFar,thisMin);
	}
      b[i]=minSoFar;

      // compute Ai's.
      minSoFar=LONG_MAX;
      for(int j=0;j<numJobs;j++)
	{
	  long thisMin=0;
	  for(int k=operationIndex(j,i,numMachines)+1;k<numMachines;k++)
	    {
	      thisMin+=JSPDurations[k][j];
	    }
	  minSoFar=min(minSoFar,thisMin);
	}
      a[i]=minSoFar;

      // compute Ti's.
      for(int j=0;j<numJobs;j++)
	{
	  t[i]+=JSPDurations[operationIndex(j,i,numMachines)][j];
	}
    }

  // compute the longest-duration job.
  vector<long> d(numJobs,0); // d_{j}'s
  long maxDur=0;
  for(int j=0;j<numJobs;j++)
    {
      long sum=0;
      for(int k=0;k<numMachines;k++)
	{
	  sum+=JSPDurations[k][j];
	}
      maxDur=max(maxDur,sum);
    }

  // compute max 'compressed' machine duration
  long maxCompTime=0;
  for(int k=0;k<numMachines;k++)
    {
      maxCompTime=max(maxCompTime,a[k]+b[k]+t[k]);
    }

  return max(maxCompTime,maxDur);
}

void verifyAndCorrectDurations(int numJobs,int numMachines)
{
  for(int j=0;j<numJobs;j++) 
    {
      for(int i=0;i<numMachines;i++) 
	{
	  int currentDuration=JSPDurations[i][j];

	  if(currentDuration<GBL_DurationLB)
	    {
	      currentDuration=GBL_DurationLB;
	    }
	  else if(currentDuration>GBL_DurationUB)
	    {
	      currentDuration=GBL_DurationUB;
	    }

	  JSPDurations[i][j]=currentDuration;
	}
    }
}

void writeProblem(int numJobs,int numMachines)
{
  cout << endl;
  cout << setw(3) << numJobs << " " << setw(3) << numMachines << endl << endl;
  for(int j=0;j<numJobs;j++)
    {
      for(int i=0;i<numMachines;i++) 
	{
	  cout << setw(3) << JSPMachines[i][j] << " " << setw(3) << JSPDurations[i][j] << " ";
	}
      cout << endl;
    }
  cout << endl;
}

void writeLowerBounds(int numJobs,int numMachines)
{
  long taillardLB=taillardLowerBound(numJobs,numMachines);

  cout << "Taillard LB: " << taillardLB << endl;
}

////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char **argv)                                    
{
  if(argc<6)
    {
      cerr << "***Incorrect number of arguments. Correct invocation is: " << endl;
      cerr << "***jspgen option number-of-jobs number-of-machines duration-seed machine-seed [OPTIONAL-ARGUMENTS]*" << endl;
      return 0;
    }

  // store the input arguments where needed.
  string problemTypeStr(argv[1]);
  string numJobsStr(argv[2]);
  string numMachinesStr(argv[3]);
  string durationSeedStr(argv[4]);
  string machineSeedStr(argv[5]);

  storeOptionalParameters(argv+6,argc-6);

  int numJobs=atoi(numJobsStr.c_str());
  int numMachines=atoi(numMachinesStr.c_str());
  long durationSeed=atol(durationSeedStr.c_str());
  long machineSeed=atol(machineSeedStr.c_str());

  if(numJobs<=0)
    {
      cerr << "***Illegal number of jobs specified: " << numJobs << endl;
      return 0;
    }

  if(numMachines<=0)
    {
      cerr << "***Illegal number of machines specified: " << numMachines << endl;
      return 0;
    }

  if(numJobs>MAX_JOBS)
    {
      cerr << "***Too many jobs specified - current hard-coded maximum is: " << MAX_JOBS << endl;
      return 0;
    }

  if(numMachines>MAX_MACHINES)
    {
      cerr << "***Too many machines specified - current hard-coded maximum is: " << MAX_MACHINES << endl;
      return 0;
    }

  if(durationSeed<0)
    {
      cerr << "***Illegal (negative) random seed specified: " << durationSeed << endl;
      return 0;      
    }

  if(machineSeed<0)
    {
      cerr << "***Illegal (negative) machine seed specified: " << machineSeed << endl;
      return 0;      
    }

  if(durationSeed==0)
    {
      durationSeed=long(time((time_t*)0));
    }

  setSeed(durationSeed);

  // when time-seeding the random number generator, the first integer value is generally
  // the same for a wide range of similar times (under this simple RNG)...
  (void) unifZeroOne(); 

  // load the operation duration bounds (if the defaults are over-ridden).
  if(loadDurationIntervalBounds()==false)
    {
      return 0;
    }

  const string TaillardTypeKeyword="taillard";
  const string GaussianTypeKeyword="gaussian";
  const string JobCorrelatedTypeKeyword="job-correlated";
  const string MachineCorrelatedTypeKeyword="machine-correlated";
  const string MixedCorrelatedTypeKeyword="mixed-correlated";

  bool genResult;

  if(problemTypeStr==TaillardTypeKeyword)
    {
      genResult=generateTaillardJobShop(numJobs,numMachines);
    }
  else if(problemTypeStr==GaussianTypeKeyword)
    {
      genResult=generateGaussianJobShop(numJobs,numMachines);
    }
  else if(problemTypeStr==JobCorrelatedTypeKeyword)
    {
      genResult=generateJobCorrelatedJobShop(numJobs,numMachines);
    }
  else if(problemTypeStr==MachineCorrelatedTypeKeyword)
    {
      genResult=generateMachineCorrelatedJobShop(numJobs,numMachines);
    }
  else if(problemTypeStr==MixedCorrelatedTypeKeyword)
    {
      genResult=generateMixedCorrelatedJobShop(numJobs,numMachines);
    }
  else
    {
      cerr << "***An unknown problem type was specified: " << problemTypeStr << endl;
      return 0;
    }

  if(genResult==false)
    {
      cerr << "***Failed to generate problem instance" << endl;
      return 0;
    }

  // verify that the operation durations actually fall into the specified
  // bounds - if they don't, modify them such that they do. currently 
  // necessary in two cases:
  // 1 - extreme outliers sampled from Gaussian distributions, which occur 
  //     relatively rarely
  // 2 - 'boundary' conditions in which the placement of distribution means
  //     causes the lower or upper duration bounds to be exceeded (i.e. in
  //     generating any kind of correlated problem).
  verifyAndCorrectDurations(numJobs,numMachines);

  if(machineSeed==0)
    {
      machineSeed=long(time((time_t*)0));
    }
  setSeed(machineSeed);

  const char *workflowOptionKeyword="workflow";
  int numMachinePartitions=1;
  if(optionPresent(workflowOptionKeyword)==true)
    {
      numMachinePartitions=optionAsInteger(workflowOptionKeyword);
    }
  if((numMachines%numMachinePartitions)!=0)
    {
      cerr << "Number of workflow partitions must be a multiple of the machine count" << endl;
      return 0;
    }

  computeOperationMachineAssignments(numJobs,numMachines,numMachinePartitions);

  writeProblem(numJobs,numMachines);

  writeLowerBounds(numJobs,numMachines);

  cout << endl;
  cout << "Duration seed: " << durationSeed << endl;
  cout << "Machine seed: " << machineSeed << endl;

  return 1;
}


