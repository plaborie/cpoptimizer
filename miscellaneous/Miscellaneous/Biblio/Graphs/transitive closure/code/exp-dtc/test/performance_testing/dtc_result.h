// --------------------------------------------------------------------
//
//  File:        dtc_result.h
//  Date:        02/98
//  Last update: 03/99
//  Description: Result class for tests (graphs and opseqs)
//
//  (C) 1998, 1999 Tobias Miller & Christos Zaroliagis
//                 {miller,zaro}@mpi-sb.mpg.de
//
// --------------------------------------------------------------------

#ifndef _DTC_RESULT_H
#define _DTC_RESULT_H

/******************** Class DTC_Result ********************/

class dtc_result
  {
    private:
      float	Time[AlgAnz][200];
      int	Step[200], StepMax;
      float	Error[AlgAnz];
      
      void print_latex_1(FILE*);
      void print_latex_2(FILE*, int);
      void print_gnuplot_1(FILE*, string);
      void print_gnuplot_2(FILE*, string, int);

    public:
      dtc_result() { clear(); }
      void clear();
      void save();
      void load(string);
      void set(int, int, float, int);				/* Set Time, Step */
      void set(int AlgNum, float E) { Error[AlgNum] += E; }	/* Set Error */
      void average(int);
      void print_latex(list<string>&, string);
      void print_gnuplot(list<string>&, string);
      void print_ps(list<string>&, string);
      void view(list<string>&);
  };

void dtc_result::clear()
  {
    for(int a=0; a<AlgAnz; a++)
      {
        for (int s=0; s<200; s++) Time[a][s] = 0;
        Error[a] = 0;
      }
    for (int s=0; s<200; s++) Step[s] = 0;
    StepMax = 0;
  }

void dtc_result::save()
  {
    system("mkdir -p "+SaveDir+"/"+SessionDir+"/"+ResultDir);
    FILE* File = fopen(SaveDir+"/"+SessionDir+"/"+ResultDir+string("/result-%i_%i", NodeAnz, EdgeAnz), "w");
    fprintf(File, "NodeAnz = %i\nEdgeAnz = %i\n", NodeAnz, EdgeAnz);
    fprintf(File, "StepMax = %i\nStep = ", StepMax);
    for (int s=0; s<StepMax; s++) fprintf(File, "%i, ", Step[s]);
    for (int a=0; a<AlgAnz; a++)
        if (AlgOn[a])
          {
            fprintf(File, "\nAlg %i = ", a);
            for (int s=0; s<StepMax; s++) fprintf(File, "%f, ", Time[a][s]);
            fprintf(File, "Error %f", Error[a]);
          }
    fprintf(File, "\n");
    fclose(File);
  }

void dtc_result::load(string FileName)
  {
    int a;
    clear();
    FILE* File = fopen(SaveDir+"/"+SessionDir+"/"+ResultDir+"/"+FileName, "r");
    fscanf(File, "NodeAnz = %i\nEdgeAnz = %i\n", &NodeAnz, &EdgeAnz);
    fscanf(File, "StepMax = %i\nStep = ", &StepMax);
    for (int s=0; s<StepMax; s++) fscanf(File, "%i, ", &(Step[s]));
    while (fscanf(File, "\nAlg %i = ", &a) != EOF)
      {
        for (int s=0; s<StepMax; s++) fscanf(File, "%f, ", &(Time[a][s]));
        fscanf(File, "Error %f", &(Error[a]));
      }
    fclose(File);
  }

void dtc_result::set(int AlgNum, int StepNum, float T, int O)
  {
    if (! in_range(StepNum, 0, 199)) return;
    Time[AlgNum][StepNum] += T;
    Step[StepNum] = O;
    if (StepMax <= StepNum) StepMax = StepNum+1;
  }

void dtc_result::average(int Anz)
  {
    if (Anz < 1) return;
    for (int a=0; a<AlgAnz; a++)
        if (AlgOn[a])
          {
            for (int s=0; s<StepMax; s++) Time[a][s] = Time[a][s]/Anz;
            Error[a] = Error[a]/Anz;
          }
  }

/******************** Print LaTeX ********************/

void dtc_result::print_latex(list<string> &ResultList, string FileName)
  {
    if (ResultList.empty()) return;
    load(ResultList.front());

    int a;
    FILE* File = fopen(FileName+".tex", "w");
    switch (OutputAxes)
      {
        case _ops_:   fprintf(File, "%i vertices, %i edges, %i different data sets, %i iterations\n", NodeAnz, EdgeAnz, GraphAnz*OpSeqAnz, IterAnz); break;
        case _nodes_: fprintf(File, "%i edges, %i different data sets, %i iterations\n", EdgeAnz, GraphAnz*OpSeqAnz, IterAnz); break;
        case _edges_: fprintf(File, "%i vertices, %i different data sets, %i iterations\n", NodeAnz, GraphAnz*OpSeqAnz, IterAnz); break;
      }

    fprintf(File, "\\begin{tabular}{|c|");
    for (a=0; a<AlgAnz; a++)
        if (AlgOn[a]) fprintf(File, "c|");
    fprintf(File, "} \\hline\n      ");
    for (a=0; a<AlgAnz; a++)
        if (AlgOn[a]) fprintf(File, " & "+AlgName[a]);
    fprintf(File, " \\\\ \\hline \\hline\n");
    
    string s;
    switch (OutputAxes)
      {
        case _ops_:   print_latex_1(File); break;
        case _nodes_: forall (s, ResultList) { load(s); print_latex_2(File, NodeAnz); } break;
        case _edges_: forall (s, ResultList) { load(s); print_latex_2(File, EdgeAnz); } break;
      }
    
    fprintf(File, "\\end{tabular}\n");
    fclose(File);
  }

void dtc_result::print_latex_1(FILE* File)
  {
    for (int s=0; s<StepMax; s++)
      {
        if (s == 0) fprintf(File, "  Init");
        else fprintf(File, "%6i", Step[s]);
        for (int a=0; a<AlgAnz; a++)
            if (AlgOn[a])
              {
                switch (OutputTime)
                  {
                    case 0: fprintf(File, " & %7.3f", Time[a][s]); break;
                    case 1: fprintf(File, " & %7.3f", Time[a][s]-Time[a][0]); break;
                  }
              }
        fprintf(File, " \\\\ \\hline\n");
      }
  }

void dtc_result::print_latex_2(FILE* File, int step)
  {
    fprintf(File, "%6i", step);
    for (int a=0; a<AlgAnz; a++)
        if (AlgOn[a])
          {
            if (OutputType == _time_)
                switch (OutputTime)
                  {
                    case 0: fprintf(File, " & %7.3f", Time[a][StepMax-1]); break;
                    case 1: fprintf(File, " & %7.3f", Time[a][StepMax-1]-Time[a][0]); break;
                    case 2: fprintf(File, " & %7.3f", Time[a][0]); break;
                  }
            else fprintf(File, " & %7.3f", Error[a]);
          }
    fprintf(File, " \\\\ \\hline\n");
  }

/******************** Print GnuPlot ********************/

void dtc_result::print_gnuplot(list<string> &ResultList, string FileName)
  {
    if (ResultList.empty()) return;
    load(ResultList.front());
    
    FILE *File = fopen(FileName+".gnu", "w");
    fprintf(File, "set terminal postscript ");
    if (OutputLandscp) fprintf(File, "landscape "); else fprintf(File, "portrait ");
    if (OutputColor) fprintf(File, "color ");
    if (OutputSolid) fprintf(File, "solid ");
    fprintf(File, "\"Times-Roman\" %i\n", OutputFSize);
    fprintf(File, string("set output \"")+FileName+".ps\"\n");
    fprintf(File, "set size 1,1\nset format x \"%cg\"\nset format y \"%cg\"\n", '%', '%');
    
    switch (OutputAxes)
      {
        case _ops_:   fprintf(File, "set title \"%i Vertices,  %i Edges   (%i different data sets,  %i iterations)\"\nset xlabel \"Operations\"\n",
                          NodeAnz, EdgeAnz, GraphAnz*OpSeqAnz, IterAnz); break;
        case _nodes_: fprintf(File, "set title \"%i Edges   (%i different data sets,  %i iterations)\"\nset xlabel \"Vertices\"\n",
                          EdgeAnz, GraphAnz*OpSeqAnz, IterAnz); break;
        case _edges_: fprintf(File, "set title \"%i Vertices   (%i different data sets,  %i iterations)\"\nset xlabel \"Edges\"\n",
                          NodeAnz, GraphAnz*OpSeqAnz, IterAnz); break;
      }
    if (OutputType == _time_)  fprintf(File, "set ylabel \"Time (sec)\"\nplot");
    else fprintf(File, "set ylabel \"Error Percentage\"\nplot");
    bool First = true;
    for (int a=0; a<AlgAnz; a++)
        if (AlgOn[a])
          {
            system("rm -f "+FileName+"-"+AlgSuffix[a]+".dat");
            if (First) fprintf(File, string(" \"")+FileName+"-"+AlgSuffix[a]+".dat\" title \""+AlgName[a]+"\" with linespoints");
            else fprintf(File, string(", \"")+FileName+"-"+AlgSuffix[a]+".dat\" title \""+AlgName[a]+"\" with linespoints");
            First = false;
          }
    fclose(File);
    
    string s;
    switch (OutputAxes)
      {
        case _ops_:   print_gnuplot_1(File, FileName); break;
        case _nodes_: forall (s, ResultList) { load(s); print_gnuplot_2(File, FileName, NodeAnz); } break;
        case _edges_: forall (s, ResultList) { load(s); print_gnuplot_2(File, FileName, EdgeAnz); } break;
      }
  }

void dtc_result::print_gnuplot_1(FILE* File, string FileName)
  {
    for (int a=0; a<AlgAnz; a++)
        if (AlgOn[a])
          {
            File = fopen(FileName+"-"+AlgSuffix[a]+".dat", "w");
            for (int s=0; s<StepMax; s++)
              {
                switch (OutputTime)
                  {
                    case 0: fprintf(File, "%6i %7.5f\n", Step[s], Time[a][s]); break;
                    case 1: fprintf(File, "%6i %7.5f\n", Step[s], Time[a][s]-Time[a][0]); break;
                  }
              }
            fclose(File);
          }
  }

void dtc_result::print_gnuplot_2(FILE* File, string FileName, int step)
  {
    for (int a=0; a<AlgAnz; a++)
        if (AlgOn[a])
          {
            File = fopen(FileName+"-"+AlgSuffix[a]+".dat", "a");
            if (OutputType == _time_)
                switch (OutputTime)
                  {
                    case 0: fprintf(File, "%6i %7.5f\n", step, Time[a][StepMax-1]); break;
                    case 1: fprintf(File, "%6i %7.5f\n", step, Time[a][StepMax-1]-Time[a][0]); break;
                    case 2: fprintf(File, "%6i %7.5f\n", step, Time[a][0]); break;
                  }
            else fprintf(File, "%6i %7.5f\n", step, Error[a]);
            fclose(File);
          }
  }

/******************** Print PostScript ********************/

void dtc_result::print_ps(list<string> &ResultList, string FileName)
  {
    print_gnuplot(ResultList, "_tmp_");
    system("gnuplot _tmp_.gnu");
    system("mv _tmp_.ps "+FileName+".ps");
    system("rm -f _tmp_*");
  }

/******************** View ********************/

void dtc_result::view(list<string> &ResultList)
  {
    print_gnuplot(ResultList, "_tmp_");
    system("gnuplot _tmp_.gnu");
    system("ghostview -color -landscape -a4 _tmp_.ps");
    system("rm -f _tmp_*");
  }

#endif _DTC_RESULT_H
