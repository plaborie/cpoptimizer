#include <ilsim/ilurandom.h>

IloNum normal(IloNum x) {
	IloNum z = (x - 20.0)/ 10.0;
	IloNum coeff = 1.0/sqrt(2 * IloPi);
	return coeff*exp(-0.5*pow(z,2));
}

int main() {
	IloEnv env;
	IluRandomEnv rand(env);
	IloInt i;
	IluNormalVariableI::Init();
	
//	IluRandomVariableI* v  = new IluFunctionVariableI(rand, 0, 40, normal,100000);
	IluRandomVariableI* v  = new IluNormalVariableI(rand, 0, 70, 20, 10);
	IluHistogramI* h = new IluHistogramI();
	
	h->clean();
	for (i=0; i < 1000000; i++)
		h->setValue(v->getValue());
	
	cout << 0 << "\t"
			 << h->getAverage() << "\t"
			 << v->getAverage() << endl;
	
	for (IloNum t=0; t<=40; t++) {	
 		h->clean();
 		for (i=0; i < 1000000; i++)
 			h->setValue(v->getValue(t));
		
 		cout << t << "\t"
 				 << h->getAverage() << "\t"
 				 << v->getAverage(t) << endl;
 	}
	
	h->clean();
	for (i=0; i < 1000000; i++)
		h->setValue(v->getValue(39.9));
	
	cout << 39.9 << "\t"
			 << h->getAverage() << "\t"
			 << v->getAverage(39.9) << endl;
//	h->display("H.dat");
	
	return 0;

}
