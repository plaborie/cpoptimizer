#include <ilsim/ilupgsim.h>
#include <stdlib.h>
#include <stdio.h>


class EventI {
public:
	IloInt  _id;
	IloNum  _t;
	IloBool _start;
	EventI(IloInt id, IloNum t, IloBool start)
		:_id (id), _t(t), _start(start){}
	~EventI(){};
};

static int compareTime(const void* fir, const void* sec) {
  IloNum firVal = (*((EventI**) fir))->_t;
  IloNum secVal = (*((EventI**) sec))->_t;
  return (firVal < secVal)?-1:((firVal > secVal)?1:0);
}


int main(int argc, char** argv) {
	try {
		IloInt seed = 1;
		/*if (argc > 1)
			seed = atoi(argv[1]);*/
		
		IloEnv env;
		IluPrecGraphI* g = new (env) IluPrecGraphI(5);
		g->addArc(3, 1);
		g->addArc(1, 0);
		g->addArc(2, 4);
		g->addArc(2, 1);
		g->addArc(4, 0);


		IloRandom rand(env);
		rand.reSeed(seed);
		IluNormalVariableI::Init();
		IluNormalVariableI* d0 = new (env) IluNormalVariableI(env, rand, 0.0,   0.0,   0.0,  0.0);
		IluNormalVariableI* d1 = new (env) IluNormalVariableI(env, rand, 1.0, 161.0,  81.0, 32.0);
		IluNormalVariableI* d2 = new (env) IluNormalVariableI(env, rand, 1.0, 101.0,  51.0, 20.0);
		IluNormalVariableI* d3 = new (env) IluNormalVariableI(env, rand, 1.0, 201.0, 101.0, 40.0);
		IluNormalVariableI* d4 = new (env) IluNormalVariableI(env, rand, 1.0, 101.0,  51.0, 20.0);

		g->setDurationVariable(0, d0);
		g->setDurationVariable(1, d1);
		g->setDurationVariable(2, d2);
		g->setDurationVariable(3, d3);
		g->setDurationVariable(4, d4);

		IluHistogramI* h0 = new IluHistogramI(env, 5000);
		g->setHistogram(0, h0);
		
		g->topologicalSort();

		h0->clean();
		IloNum now = 0.0;
	//	IloNum temp = env.getTime();
		g->simulate(now, 5000); // By default 1,000 simualtions are run.
		printf("Makespan: mean = %.14f and standard deviation = %.14f\n", h0->getAverage(), h0->getStandardDev());

	/*	IloNum* durs = new (env) IloNum[5];
		durs[0] = d0->getValue();
		durs[1] = d1->getValue();
		durs[2] = d2->getValue();
		durs[3] = d3->getValue();
		durs[4] = d4->getValue();
		
		IloNum* starts = new (env) IloNum[5];
		IloNum* ends   = new (env) IloNum[5];

		starts[1] = 0;
		starts[3] = 0;
		ends[1] = durs[1];
		ends[3] = durs[3];
		starts[2] = ends[1] + 1e-3;
		starts[4] = IloMax(ends[1], ends[3]) + 1e-3;
		ends[2] = starts[2] + durs[2];
		ends[4] = starts[4] + durs[4];
		starts[0] = IloMax(ends[2], ends[4]) + 1e-3;
		ends[0] = starts[0] + durs[0];

		EventI** events = new (env) EventI*[10];
		IloInt i;
		for (i=0; i<5; ++i) {
			EventI* event = new (env) EventI(i,starts[i],1);
			events[i]=event;
		}
		for (i=0; i<5; ++i) {
			EventI* event = new (env) EventI(i,ends[i],0);
			events[5+i]=event;
		}

		qsort(events, 10, sizeof(EventI*), compareTime);

		const char* filename = "default.graph";
		ofstream out(filename);
		
		IloNum now = 0.0;
		for (i=0; i<10;) {
			for (;now<events[i]->_t; now+=1) {
				h0->clean();
				for (IloInt j=0; j<100000; j++)
					g->simulate(now); // By default 1,000 simulations are run.
				cout << now;
				cout << "\t... avg=" << h0->getAverage();
				cout << ";\t dev=" << h0->getStandardDev() << endl;
				out << now << "\t" <<  h0->getAverage() << "\t" <<  h0->getStandardDev() << endl;
			}
			while ((i < 10) && (events[i]->_t <= now)) {
				IloInt id = events[i]->_id;
				IloNum t  = events[i]->_t;
				IloInt start = events[i]->_start;
				if (0 < start) {
					cout << t << "\t START \t" << id << endl;
					g->setStartTime(id,t);
				} else {
					cout << t << "\t END   \t" << id << endl;
					g->setEndTime(id,t);
				}
				i++;
			}
		}

		out.close();*/
		env.end();
		
		
/*		const char* filename = "default.graph";
		if (argc > 1)
			filename = argv[1];

		IloNum alpha = 0.1;
		if (argc > 2)
			alpha = atof(argv[2]);

		IloInt n = 1000000;
		if (argc > 3)
			n = atol(argv[3]);
		
		IloEnv env;
		IluRandomEnv rand(env);

		IluPrecGraphI* g = new (env) IluPrecGraphI();
		g->readJobShop(rand, filename, alpha);

		IluHistogramI* h = new (env) IluHistogramI(env, n);
		g->setHistogram(1,h);
	
		g->topologicalSort();

		h->clean();

		IloNum temp = env.getTime();
		g->simulate(n);
		IloNum temp1 = env.getTime();
		cout << "Time spent for running 1,000 simulations:" << temp1-temp << endl;
		
		cout << h->getAverage() << "\t" << h->getStandardDev() << endl;

		env.end();*/
		
		
	} catch (IloException& e) {
		cout << e << endl;
	}
	
	return 0;
}
