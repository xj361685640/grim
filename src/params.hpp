#ifndef GRIM_PARAMS_H_
#define GRIM_PARAMS_H_

const int NDIM = 4;
const int LOCATIONS = 7;

namespace vars
{
  enum
  {
    RHO, U, U1, U2, U3, B1, B2, B3
  };
  extern int Q, DP;
  extern int dof;
};

namespace locations
{
  enum
  {
    CENTER, LEFT, RIGHT, TOP, BOTTOM, FRONT, BACK
  };
};

namespace directions
{
  enum
  {
    X1, X2, X3
  };
};

namespace boundaries
{
  enum
  {
    PERIODIC, OUTFLOW, MIRROR, DIRICHLET
  };
};

namespace metrics
{
  enum
  {
    MINKOWSKI, MODIFIED_KERR_SCHILD
  };
};

namespace timeStepping
{
  enum
  {
    EXPLICIT, IMEX, IMPLICIT
  };
};

namespace reconstructionOptions
{
  enum
  {
    MINMOD, WENO5
  };
};

namespace params
{
  extern int N1;
  extern int N2;
  extern int N3;
  extern int dim;
  extern int numGhost;

  extern int timeStepper;
  extern double dt;
  extern double Time;
  extern double finalTime;
  extern int metric;
  extern double hSlope;
  extern double blackHoleSpin;

  extern double X1Start, X1End;
  extern double X2Start, X2End;
  extern double X3Start, X3End;

  extern int boundaryLeft;
  extern int boundaryRight;

  extern int boundaryTop;
  extern int boundaryBottom;

  extern int boundaryFront;
  extern int boundaryBack;

  extern double rhoFloorInFluidElement;
  extern double uFloorInFluidElement;
  extern double bSqrFloorInFluidElement;
  extern double temperatureFloorInFluidElement;

  extern int conduction;
  extern int viscosity;
  extern int highOrderTermsConduction;
  extern int highOrderTermsViscosity;
  extern double adiabaticIndex;

  extern double slopeLimTheta;
  extern int reconstruction;

  extern int maxNonLinearIter;
  extern int maxLineSearchIters;

  extern double nonlinearsolve_atol;
  extern double JacobianAssembleEpsilon;
  extern double linesearchfloor;

};

#endif /* GRIM_PARAMS_H_ */