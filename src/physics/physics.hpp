#ifndef GRIM_PHYSICS_H_
#define GRIM_PHYSICS_H_

#include "../params.hpp"
#include "../grid/grid.hpp"
#include "../geometry/geometry.hpp"
#include "../reconstruction/reconstruction.hpp"

inline int DELTA(int const &mu, int const &nu)
{
  return (mu==nu ? 1 : 0);
}

class fluidElement
{
  private:
    const std::vector<std::vector<int>> indicesToLoopOver 
      = {{0},    {0, 1}, {0, 1}, {0, 2}, {0, 2}, {0, 3}, {0, 3}};
    /*   CENTER, LEFT,   RIGHT,  TOP,    BOTTOM, FRONT,  BACK*/

    array one;
  public:
    int loc;

    /* fluidElement parameters */
    array tau, chi_emhd, nu_emhd;
    
    array rho, u, u1, u2, u3, B1, B2, B3;
    array pressure, temperature;
    array qTilde, deltaPTilde;
    array q, deltaP;
  
    array gammaLorentzFactor, uCon[NDIM], uCov[NDIM];
    array bSqr, bCon[NDIM], bCov[NDIM];
    array soundSpeed;
    
    array NUp[NDIM];
    array TUpDown[NDIM][NDIM];

    array gradT[NDIM];
    array dtuCov[NDIM];
    array graduCov[NDIM][NDIM];
    array divuCov;
    array deltaP0;
    array q0;
    array bNorm;

    fluidElement(const array prim[vars::dof],
                 const geometry &geom
                );
    void set(const array prim[vars::dof],
             const geometry &geom
            );
    void setFluidElementParameters(const geometry &geom);
    void computeFluxes(const geometry &geom, 
                       const int direction,
                       array flux[vars::dof]
                      );
//
//    void computeMinMaxCharSpeeds(const geometry &geom,
//			       const int dir,
//			       array &MinSpeed,
//				 array &MaxSpeed);
//
//    void computeTimeDerivSources(const geometry &geom,
//				const fluidElement &elemOld,
//				const fluidElement &elemNew,
//				const double dt,
//				grid &sources
//				);
//
//    void computeImplicitSources(const geometry &geom,
//				grid &sources
//				);
//  void computeExplicitSources(const geometry &geom,
//			      grid &sources
//			      );
//  void computeEMHDGradients(const geometry &geom);
};

//class riemannSolver
//{
//  public:
//    fluidElement *elemLeft, *elemRight;
//
//    grid *primLeft, *primRight;
//    grid *fluxLeft, *fluxRight;
//    grid *consLeft, *consRight;
//
//    array MinSpeedLeft,MaxSpeedLeft;
//    array MinSpeedRight,MaxSpeedRight;
//
//    riemannSolver(const geometry &geom);
//    ~riemannSolver();
//
//    void solve(const grid &prim,
//               const geometry &geom,
//               const int dir,
//               grid &fluxes
//              );
//};

#endif /* GRIM_PHYSICS_H_ */