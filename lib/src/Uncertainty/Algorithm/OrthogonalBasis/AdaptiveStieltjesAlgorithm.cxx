//                                               -*- C++ -*-
/**
 *  @brief Implement the modified adaptive Stieltjes algorithm to compute
 *         the coefficients of the 3 terms recurrence relation of an
 *         orthonormal polynomial family
 *
 *  Copyright 2005-2016 Airbus-EDF-IMACS-Phimeca
 *
 *  This library is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  along with this library.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "openturns/AdaptiveStieltjesAlgorithm.hxx"
#include "openturns/PersistentObjectFactory.hxx"
#include "openturns/ResourceMap.hxx"
#include "openturns/Exception.hxx"
#include "openturns/Uniform.hxx"
#include "openturns/GaussKronrod.hxx"
#include "openturns/OrthogonalUniVariatePolynomial.hxx"
#include "openturns/MethodBoundNumericalMathEvaluationImplementation.hxx"
#include "openturns/Log.hxx"

BEGIN_NAMESPACE_OPENTURNS

CLASSNAMEINIT(AdaptiveStieltjesAlgorithm);

static const Factory<AdaptiveStieltjesAlgorithm> RegisteredFactory;

/* Default constructor */
AdaptiveStieltjesAlgorithm::AdaptiveStieltjesAlgorithm()
  : OrthonormalizationAlgorithmImplementation(Uniform(-1.0, 1.0))
  , monicRecurrenceCoefficients_(1, Coefficients(3))
  , monicSquaredNorms_(1)
  , isElliptical_(true)
{
  // Here we initialize the monic coefficients cache
  monicRecurrenceCoefficients_[0][0] = 1.0;
  monicRecurrenceCoefficients_[0][2] = 0.0;
  monicSquaredNorms_[0] = 1.0;
}


/* Parameter constructor */
AdaptiveStieltjesAlgorithm::AdaptiveStieltjesAlgorithm(const Distribution & measure)
  : OrthonormalizationAlgorithmImplementation(measure)
  , monicRecurrenceCoefficients_(1, Coefficients(3))
  , monicSquaredNorms_(1)
  , isElliptical_(measure.isElliptical())
{
  // Here we initialize the monic coefficients cache
  const NumericalScalar mu(measure.getMean()[0]);
  monicRecurrenceCoefficients_[0][0] = 1.0;
  // To avoid -0.0 in print, we test for the mean of the distribution.
  if (std::abs(mu) > ResourceMap::GetAsNumericalScalar("DistributionImplementation-DefaultQuantileEpsilon"))
    monicRecurrenceCoefficients_[0][1] = -mu;
  // The value of \beta_0 is 1 as the weight distribution is a probability distribution
  monicRecurrenceCoefficients_[0][2] = 0.0;
  monicSquaredNorms_[0] = 1.0;
}


/* Virtual constructor */
AdaptiveStieltjesAlgorithm * AdaptiveStieltjesAlgorithm::clone() const
{
  return new AdaptiveStieltjesAlgorithm(*this);
}


/* Calculate the coefficients of recurrence a0n, a1n, a2n such that
   Pn+1(x) = (a0n * x + a1n) * Pn(x) + a2n * Pn-1(x), P-1(x)=0, P0(x)=1
   We have:
   a0n = 1/sqrt(\beta_{n+1})
   a1n = -\alpha_n/sqrt(\beta_{n+1})
   a2n = -\sqrt{\beta_n/\beta_{n+1}}
   where \alpha_n and \beta_n are the recurrence coefficients of the monic orthogonal coefficients
   Qn+1(x) = (x - \alpha_n) * Qn(x) - beta_n * Qn-1(x), Q-1(x)=0, Q0(x)=1
   Rn = <Qn,Qn> for n >= 0
   \alpha_n = <xQn,Qn> / Rn
   \beta_n  = Rn / Rn-1 for n >= 1, \beta_0 = 0
   Remark: here we considere probability measures D, so R0=1, \alpha_0=E[D]=\mu, Q1(x)=x-\mu, \beta_1=R1=Var[D]
   Due to the convention of coefficients for orthonormal polynomials, the coefficients of the monic
   polynomials are stored as [1, -\alpha_n, -\beta_n]
 */
AdaptiveStieltjesAlgorithm::Coefficients AdaptiveStieltjesAlgorithm::getRecurrenceCoefficients(const UnsignedInteger n) const
{
  // The cache size is at least 1
  const UnsignedInteger cacheSize = monicRecurrenceCoefficients_.getSize();
  // Get the coefficients from the cache if possible
  if (n < cacheSize - 1)
    {
      const NumericalScalar inverseSqrtBetaNp1(1.0 / sqrt(-monicRecurrenceCoefficients_[n + 1][2]));
      Coefficients coefficients(3);
      coefficients[0] = inverseSqrtBetaNp1;
      if (std::abs(monicRecurrenceCoefficients_[n][1]) > 0.0)
	coefficients[1] = monicRecurrenceCoefficients_[n][1] * inverseSqrtBetaNp1;
      coefficients[2] = -sqrt(-monicRecurrenceCoefficients_[n][2]) * inverseSqrtBetaNp1;
      return coefficients;
    }
  // This loop is to go to the first coefficients not in the cache. Here we cannot use cacheSize as
  // the size of the cache is increased by each call to getRecurrenceCoefficients()
  while (n >= monicRecurrenceCoefficients_.getSize()) getRecurrenceCoefficients(n - 1);
  // Here we know that n == cacheSize - 1. In order to compute the recurrence coefficients of the orthonormal polynomial pN
  // we need \alpha_{n-1}, \beta_{n-1} and \beta_n. The first two values are in the cache, we need \beta_n. We compute both
  // \alpha_n and \beta_n by numerical integration.
  Coefficients monicCoefficients(3);
  monicCoefficients[0] = 1.0;
  // Build the monic orthogonal polynomial of degree cacheSize
  const OrthogonalUniVariatePolynomial qN(monicRecurrenceCoefficients_);
  const GaussKronrod algo(ResourceMap::GetAsUnsignedInteger("AdaptiveStieltjesAlgorithm-MaximumSubIntervalsBetweenRoots") * (n + 1), ResourceMap::GetAsNumericalScalar("AdaptiveStieltjesAlgorithm-MaximumError"), GaussKronrodRule(GaussKronrodRule::G7K15));
  const DotProductWrapper dotProductWrapper(qN, measure_);
  if (isElliptical_)
    {
      // In the case of elliptical distributions, the coefficient \alpha is
      // always equal to the mean of the distribution
      monicCoefficients[1] = monicRecurrenceCoefficients_[n][1];
      // For n == 1 beta_1 is the variance
      if (n == 0)
	{
	  monicSquaredNorms_.add(measure_.getCovariance()(0, 0));
	  monicCoefficients[2] = -monicSquaredNorms_[1];
	}
      else
	{
	  const NumericalMathFunction dotProductKernel(bindMethod<DotProductWrapper, NumericalPoint, NumericalPoint>(dotProductWrapper, &DotProductWrapper::kernelSym, 1, 1));
	  monicSquaredNorms_.add(algo.integrate(dotProductKernel, measure_.getRange())[0]);
	  monicCoefficients[2] = -monicSquaredNorms_[n + 1] / monicSquaredNorms_[n];
	} // n != 1
    } // isElliptical_
  else
    {
      // \beta_n = Rn / Rn-1 with Rn-1 = \beta_{n-1}Rn-2 = \beta_{n-1}\beta_{n-2}Rn-3 = ... = \prod_{k=0}^{n-1}\beta_k
      // Compute Rn and <x.Qn, Qn>
      const NumericalMathFunction dotProductKernel(bindMethod<DotProductWrapper, NumericalPoint, NumericalPoint>(dotProductWrapper, &DotProductWrapper::kernelGen, 1, 2));
      NumericalPoint dotProduct(algo.integrate(dotProductKernel, measure_.getRange()));
      monicSquaredNorms_.add(dotProduct[0]);
      monicCoefficients[1] = -dotProduct[1] / monicSquaredNorms_[n + 1];
      monicCoefficients[2] = -monicSquaredNorms_[n + 1] / monicSquaredNorms_[n];
    } // !isElliptical_
  monicRecurrenceCoefficients_.add(monicCoefficients);
  // Now n == cacheSize - 2
  return getRecurrenceCoefficients(n);
}

/* String converter */
String AdaptiveStieltjesAlgorithm::__repr__() const
{
  return OSS() << "class=" << getClassName()
         << " measure=" << measure_
         << " monicRecurrenceCoefficients=" << monicRecurrenceCoefficients_
         << " monicSquaredNorms=" << monicSquaredNorms_
         << " isElliptical=" << isElliptical_;
}


/* Method save() stores the object through the StorageManager */
void AdaptiveStieltjesAlgorithm::save(Advocate & adv) const
{
  OrthonormalizationAlgorithmImplementation::save(adv);
  adv.saveAttribute( "monicRecurrenceCoefficients_", monicRecurrenceCoefficients_ );
  adv.saveAttribute( "monicSquaredNorms_", monicSquaredNorms_ );
  adv.saveAttribute( "isElliptical_", isElliptical_ );
}


/* Method load() reloads the object from the StorageManager */
void AdaptiveStieltjesAlgorithm::load(Advocate & adv)
{
  OrthonormalizationAlgorithmImplementation::load(adv);
  adv.loadAttribute( "monicRecurrenceCoefficients_", monicRecurrenceCoefficients_ );
  adv.loadAttribute( "monicSquaredNorms_", monicSquaredNorms_ );
  adv.loadAttribute( "isElliptical_", isElliptical_ );
}

END_NAMESPACE_OPENTURNS
