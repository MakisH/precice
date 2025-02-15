#include "time/Waveform.hpp"
#include <algorithm>
#include <unsupported/Eigen/Splines>
#include "cplscheme/CouplingScheme.hpp"
#include "logging/LogMacros.hpp"
#include "math/differences.hpp"
#include "time/Time.hpp"
#include "utils/EigenHelperFunctions.hpp"

namespace precice::time {

Waveform::Waveform(
    const int interpolationOrder)
    : _interpolationOrder(interpolationOrder)
{
  PRECICE_ASSERT(Time::MIN_INTERPOLATION_ORDER <= _interpolationOrder && _interpolationOrder <= Time::MAX_INTERPOLATION_ORDER);
}

int Waveform::getInterpolationOrder() const
{
  return _interpolationOrder;
}

void Waveform::initialize(const Eigen::VectorXd &values)
{
  PRECICE_ASSERT(_storage.nTimes() == 0);
  _storage.initialize(values);
  PRECICE_ASSERT(_interpolationOrder >= Time::MIN_INTERPOLATION_ORDER);
}

void Waveform::store(const Eigen::VectorXd &values, double normalizedDt)
{
  if (math::equals(_storage.maxStoredNormalizedDt(), time::Storage::WINDOW_END)) { // reached end of window and trying to write new data from next window. Clearing window first.
    _storage.clear();
  }
  PRECICE_ASSERT(values.size() == _storage.nDofs());
  _storage.setValuesAtTime(normalizedDt, values);
}

// helper function to compute x(t) from given data (x0,t0), (x1,t1), ..., (xn,tn) via B-spline interpolation (implemented using Eigen).
Eigen::VectorXd bSplineInterpolationAt(double t, Eigen::VectorXd ts, Eigen::MatrixXd xs, int splineDegree)
{
  // organize data in columns. Each column represents one sample in time.
  PRECICE_ASSERT(xs.cols() == ts.size());
  const int ndofs = xs.rows(); // number of dofs. Each dof needs it's own interpolant.

  Eigen::VectorXd interpolated(ndofs);

  const int splineDimension = 1;

  for (int i = 0; i < ndofs; i++) {
    const auto spline = Eigen::SplineFitting<Eigen::Spline<double, splineDimension>>::Interpolate(xs.row(i), splineDegree, ts);
    interpolated[i]   = spline(t)[0]; // get component of spline associated with xs.row(i)
  }

  return interpolated;
}

Eigen::VectorXd Waveform::sample(double normalizedDt) const
{
  const int usedOrder = computeUsedOrder(_interpolationOrder, _storage.nTimes());

  PRECICE_ASSERT(math::equals(this->_storage.maxStoredNormalizedDt(), time::Storage::WINDOW_END), this->_storage.maxStoredNormalizedDt()); // sampling is only allowed, if a window is complete.

  if (_interpolationOrder == 0) {
    return this->_storage.getValuesAtOrAfter(normalizedDt);
  }

  PRECICE_ASSERT(usedOrder >= 1);

  const auto data = _storage.getTimesAndValues();

  return bSplineInterpolationAt(normalizedDt, data.first, data.second, usedOrder);
}

void Waveform::moveToNextWindow()
{
  _storage.move();
}

int Waveform::computeUsedOrder(int requestedOrder, int numberOfAvailableSamples) const
{
  int usedOrder = -1;
  PRECICE_ASSERT(requestedOrder <= 3);
  if (requestedOrder == 0 || numberOfAvailableSamples < 2) {
    usedOrder = 0;
  } else if (requestedOrder == 1 || numberOfAvailableSamples < 3) {
    usedOrder = 1;
  } else if (requestedOrder == 2 || numberOfAvailableSamples < 4) {
    usedOrder = 2;
  } else if (requestedOrder == 3 || numberOfAvailableSamples < 5) {
    usedOrder = 3;
  } else {
    PRECICE_ASSERT(false); // not supported
  }
  return usedOrder;
}

} // namespace precice::time
