#ifndef __LBFGS_H__
#define __LBFGS_H__

#include <vector>

extern "C" {
	extern void lbfgs(int* n, int* m, double* x, double* f, double* g, 
										int* diagco, double* diag, int* iprint, double* eps, 
										double* xtol, double* w, int* iflag);
}

class LBFGS {

	public:
	LBFGS(int n, int m = 5): _n(n), _m(m), _iflag(0)
	{
		_diag = new double[n];
		_w = new double[ n * (2 * m + 1) + 2 * m];
	}
	~LBFGS()
	{
		delete[] _diag;
		delete[] _w;
	}
	
	int optimize (double x[], double *f, double g[])   // vector, object, gradient
	{
		//int iprint[] = {-1, 0};
		int iprint[] = {1, 3};
		double eta  = 1e-7;
		double xtol = 1e-7;
		int diagco = 0;
		lbfgs(&_n, &_m, x, f, g, &diagco, _diag, iprint, &eta, &xtol, _w, &_iflag);
	   
		return _iflag; 	
		if (_iflag < 0)
			return -1;

		if (_iflag == 0) 
			return 0; // terminate

		return 1; // evaluate next f and g
	}

private:
	int _n;
	int _m;

	double *_diag;
	double *_w;

	int _iflag;
};

#endif
