#ifndef CHEBY_TABLE_H
#define CHEBY_TABLE_H

struct coeffentry {
	double fc, r;
	int highpass;
	unsigned int npole;
	double num[8];
	double den[8];
};

// Result coming from DSP guide chap 20
struct coeffentry chebytable[] = {
	{ .fc = 0.01, .npole = 2, .highpass = 0, .r = 0.005,
	  .num = {8.663387E-04, 1.732678E-03, 8.663387E-04},
	  .den = {1.0, -1.919129E+00, 9.225943E-01}
	},
	{ .fc = 0.15, .npole = 2, .highpass = 0, .r = 0.005,
	  .num = {1.254285E-01, 2.508570E-01, 1.254285E-01},
	  .den = {1.0, -8.070778E-01, 3.087918E-01}
	},
	{ .fc = 0.01, .npole = 2, .highpass = 1, .r = 0.005,
	  .num = {9.567529E-01, -1.913506E+00, 9.567529E-01},
	  .den = {1.0, -1.911437E+00, 9.155749E-01}
	},
	{ .fc = 0.15, .npole = 2, .highpass = 1, .r = 0.005,
	  .num = {5.001024E-01, -1.000205E+00, 5.001024E-01},
	  .den = {1.0, -7.158993E-01, 2.845103E-01}
	},
	{ .fc = 0.2, .npole = 6, .highpass = 0, .r = 0.005,
	  .num = {4.187408E-03, 2.512445E-02, 6.281112E-02, 8.374816E-02, 6.281112E-02, 2.512445E-02, 4.187408E-03},
	  .den = {1.0, -2.315806E+00, 3.293726E+00, -2.904826E+00, 1.694128E+00, -6.021426E-01, 1.029147E-01}
	},
	{ .fc = 0.4, .npole = 6, .highpass = 1, .r = 0.005,
	  .num = {9.086141E-05, -5.451685E-04, 1.362921E-03, -1.817228E-03, 1.362921E-03, -5.451685E-04, 9.086141E-05},
	  .den = {1.0, 4.470118E+00, 8.755595E+00, 9.543712E+00, 6.079377E+00, 2.140062E+00, 3.247363E-01}
	}
};

#define NUMCASE	(sizeof(chebytable)/sizeof(chebytable[0]))


#endif // CHEBY_TABLE_H
