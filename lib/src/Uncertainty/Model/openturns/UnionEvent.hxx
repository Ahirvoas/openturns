//                                               -*- C++ -*-
/**
 * @brief Union of several events
 *
 *  Copyright 2005-2019 Airbus-EDF-IMACS-Phimeca
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
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this library.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef OPENTURNS_UNIONEVENT_HXX
#define OPENTURNS_UNIONEVENT_HXX

#include "openturns/RandomVectorImplementation.hxx"
#include "openturns/RandomVector.hxx"

BEGIN_NAMESPACE_OPENTURNS


/**
 * @class UnionEvent
 *
 *
 */
class OT_API UnionEvent
  : public RandomVectorImplementation
{
  CLASSNAME

public:
  typedef Collection<RandomVector> RandomVectorCollection;
  typedef PersistentCollection<RandomVector> RandomVectorPersistentCollection;

  /** Default constructor */
  UnionEvent();

  /** Constructor from collection */
  explicit UnionEvent(const RandomVectorCollection & collection);

  /** Virtual constructor */
  virtual UnionEvent * clone() const;

  /** String converter */
  String __repr__() const;

  /** Dimension accessor */
  virtual UnsignedInteger getDimension() const;

  /** Collection accessor */
  void setEventCollection(const RandomVectorCollection & collection);
  RandomVectorCollection getEventCollection() const;

  /** Realization accessor */
  virtual Point getRealization() const;

  /** Whether it is an event */
  virtual Bool isEvent() const;

  /** Method save() stores the object through the StorageManager */
  void save(Advocate & adv) const;

  /** Method load() reloads the object from the StorageManager */
  void load(Advocate & adv);


private:

  /** The process of the eventProcess */
  RandomVectorPersistentCollection eventCollection_;

  /** id of the root cause */
  UnsignedInteger rootCauseId_;

  RandomVector aggregatedEvent_;
}; /* class UnionEvent */


END_NAMESPACE_OPENTURNS

#endif /* OPENTURNS_UNIONEVENT_HXX */
