//                                               -*- C++ -*-
/**
 * @brief Class for Box Cox function implementation
 *
 * Copyright 2005-2018 Airbus-EDF-IMACS-Phimeca
 *
 * Permission to copy, use, modify, sell and distribute this software
 * is granted provided this copyright notice appears in all copies.
 * This software is provided "as is" without express or implied
 * warranty, and with no claim as to its suitability for any purpose.
 *
 *
 */

#ifndef OPENTURNS_INVERSEBOXCOXGRADIENT_HXX
#define OPENTURNS_INVERSEBOXCOXGRADIENT_HXX

#include "openturns/GradientImplementation.hxx"
#include "openturns/InverseBoxCoxEvaluation.hxx"

BEGIN_NAMESPACE_OPENTURNS

/**
 * @class InverseBoxCoxGradient
 *
 * This class offers an easy gradient of the Box Cox function defined by :
 *  \Delta(h)(x) = (x-s)^{\lambda - 1} for \lambda non zero, 1/(x-s) otherwise
 *  Care that x should be > s
 */

class OT_API InverseBoxCoxGradient
  : public GradientImplementation
{
  CLASSNAME
public:


  /** Default constructor */
  InverseBoxCoxGradient();

  /** Parameter constructor */
  explicit InverseBoxCoxGradient(const InverseBoxCoxEvaluation & evaluation);

#ifndef SWIG
  /** Parameter constructor */
  explicit InverseBoxCoxGradient(const Pointer<InverseBoxCoxEvaluation> & p_evaluation);
#endif

  /** Virtual constructor */
  virtual InverseBoxCoxGradient * clone() const;

  /** Comparison operator */
  Bool operator ==(const InverseBoxCoxGradient & other) const;

  /** String converter */
  String __repr__() const;
  String __str__(const String & offset = "") const;

  /** Gradient evaluation method */
  using GradientImplementation::gradient;
  Matrix gradient(const Point & inP) const;

  /** Accessor for input point dimension */
  UnsignedInteger getInputDimension() const;

  /** Accessor for output point dimension */
  UnsignedInteger getOutputDimension() const;

  /** Accessor for the lambda point */
  Point getLambda() const;

  /** Accessor for the shift */
  Point getShift() const;

  /** Method save() stores the object through the StorageManager */
  void save(Advocate & adv) const;

  /** Method load() reloads the object from the StorageManager */
  void load(Advocate & adv);

protected:
  /** The underlying evaluation */
  Pointer<InverseBoxCoxEvaluation> p_evaluation_;

}; /* class InverseBoxCoxGradient */


END_NAMESPACE_OPENTURNS

#endif /* OPENTURNS_INVERSEBOXCOXGRADIENT_HXX */
