#include "timestepper.hpp"
#include <sys/time.h>

void timeStepper::solve(grid &primGuess)
{
  for (int nonLinearIter=0;
       nonLinearIter < params::maxNonLinearIter; nonLinearIter++
       )
  {
    //True residual, with explicit terms (not needed for Jacobian)
    computeResidual(primGuess, *residual,true);
    for (int var=0; var < vars::dof; var++)
    {
      /* Need residualSoA to compute norms */
      residualSoA(span, span, span, var) = residual->vars[var];

      /* Initialize primGuessPlusEps. Needed to numerically assemble the
       * Jacobian */
      primGuessPlusEps->vars[var]        = primGuess.vars[var];
    }

    /* Still yet to put in code to compute global norm over mpi procs */
    double globalL2Norm =  af::norm(af::flat(residualSoA));
    printf("Nonliner iter = %i, error = %e\n", nonLinearIter, globalL2Norm);
    if(globalL2Norm<params::nonlinearsolve_atol)
      break;

    /* First assemble b. Need to reorder from SoA -> AoS. */
    for (int var=0; var < vars::dof; var++)
    {
      bSoA(span, span, span, var) = -residual->vars[var];
    }
    array bAoS        = af::reorder(bSoA, 3, 0, 1, 2);

    //Residual without explicit terms, for faster Jacobian assembly
    computeResidual(primGuess, *residual,false);

    /* Assemble the Jacobian in Struct of Arrays format where the physics
     * operations are all vectorized */
    for (int row=0; row < vars::dof; row++)
    {
      /* Recommended value of Jacobian differencing parameter to achieve fp64
       * machine precision */
      double epsilon = params::JacobianAssembleEpsilon;

      array smallPrim = af::abs(primGuess.vars[row])<.5*epsilon;
      primGuessPlusEps->vars[row]  = (1. + epsilon)*primGuess.vars[row]*(1.-smallPrim)
	+smallPrim*epsilon; 

      computeResidual(*primGuessPlusEps, *residualPlusEps,false);

      for (int column=0; column < vars::dof; column++)
      {
        jacobianSoA(span, span, span, column + vars::dof*row)
          = (  residualPlusEps->vars[column] 
             - residual->vars[column]
            )
            /(primGuessPlusEps->vars[row]-primGuess.vars[row]);
      }
      /* reset */
      primGuessPlusEps->vars[row]  = primGuess.vars[row]; 
    }
    /* Jacobian assembly complete */

    /* Solve the linear system Jacobian * deltaPrim = -residual for the
     * correction deltaPrim */

    array jacobianAoS = af::reorder(jacobianSoA, 3, 0, 1, 2);

    /* Now solve Ax = b using direct inversion, where
     * A = Jacobian
     * x = deltaPrim
     * b = -residual 
     *
     * Currently inverting locally by looping over individual zones. Need to
     * call the batch function magma_dgesv_batched() from the MAGMA library
     * for optimal use on NVIDIA cards */
    for (int k=0; k<residual->vars[0].dims(2); k++)
    {
      for (int j=0; j<residual->vars[0].dims(1); j++)
      {
        for (int i=0; i<residual->vars[0].dims(0); i++)
        {
          array A = af::moddims(jacobianAoS(span, i, j, k), 
                                vars::dof, vars::dof
                               );

          deltaPrimAoS(span, i, j, k) = af::solve(A, bAoS(span, i, j, k));
        }
      }
    }

    /* Done with the solve. Now rearrange from AoS -> SoA */
    array deltaPrimSoA = af::reorder(deltaPrimAoS, 1, 2, 3, 0);

    /* Quartic backtracking :
     We minimize f(u+stepLength*du) = 0.5*sqr(residual[u+stepLength*du]).
     We use
       f0 = f(u)
       fPrime0 = df/d(stepLength)(u) = -2*f0
       [because stepLength=1 is the solution of the linearized problem...]
       f1 = f(u+stepLength0*du)
     to reconstruct
       f = (f1-f0-fPrime0*stepLength0)*(stepLength/stepLength0)^2 + fPrime0*stepLength + f0
     which has a minimum at the new value of stepLength,
       stepLength = -fPrime0*stepLength0^2 / (f1-f0-fPrime0*stepLength0)/2
     */
    array f0      = 0.5 * af::sum(af::pow(residualSoA, 2.), 3);
    array fPrime0 = -2.*f0;
    
    array residualAoS = af::reorder(residualSoA, 3, 0, 1, 2);

    /* Start with a full step */
    stepLength = 1.;
    int lineSearchIter=0;
    for (;lineSearchIter < params::maxLineSearchIters; lineSearchIter++
        )
    {
      /* 1) First take current step stepLength */
      for (int var=0; var<vars::dof; var++)
      {
        primGuessLineSearchTrial->vars[var] =  
          primGuess.vars[var] + stepLength*deltaPrimSoA(span, span, span, var);
      } 

      /* ...and then compute the norm */
      computeResidual(*primGuessLineSearchTrial, *residual,true);
      for (int var=0; var<vars::dof; var++)
      {
        residualSoA(span, span, span, var) = residual->vars[var];
      }
      array f1 = 0.5 * af::sum(af::pow(residualSoA, 2.), 3);

      /* We have 3 pieces of information:
       * a) f(0)
       * b) f'(0) 
       * c) f(stepLength) 
       */
    
      const double alpha    = 1e-4;
      const double EPS      = params::linesearchfloor;
      array condition = f1 > (f0*(1. - alpha*stepLength) +EPS);
      array denom     = (f1-f0-fPrime0*stepLength) * condition + (1.-condition);
      array nextStepLength = -fPrime0*stepLength*stepLength/denom/2.;
      stepLength      = stepLength*(1. - condition) + condition*nextStepLength;
      
      array conditionIndices = where(condition > 0);
      if (conditionIndices.elements() == 0)
      {
        break;
      }
    }

    //printf("Stopped linesearch after %i its\n",lineSearchIter);
    /* stepLength has now been set */
    for (int var=0; var<vars::dof; var++)
    {
      primGuess.vars[var] = 
        primGuess.vars[var] + stepLength*deltaPrimSoA(span, span, span, var);
    }
  }

}