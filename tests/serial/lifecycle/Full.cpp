#ifndef PRECICE_NO_MPI

#include "testing/Testing.hpp"

#include <precice/Participant.hpp>
#include <vector>

BOOST_AUTO_TEST_SUITE(Integration)
BOOST_AUTO_TEST_SUITE(Serial)
BOOST_AUTO_TEST_SUITE(Lifecycle)
// Test representing the full explicit lifecycle of a Participant
BOOST_AUTO_TEST_CASE(Full)
{
  PRECICE_TEST("SolverOne"_on(1_rank), "SolverTwo"_on(1_rank));
  precice::Participant participant(context.name, context.config(), context.rank, context.size);

  if (context.isNamed("SolverOne")) {
    auto   meshName = "MeshOne";
    double coords[] = {0.1, 1.2, 2.3};
    auto   vertexid = participant.setMeshVertex(meshName, coords);

    auto   dataName = "DataOne";
    double data[]   = {3.4, 4.5, 5.6};
    participant.writeData(meshName, dataName, {&vertexid, 1}, data);
  } else {
    auto   meshName = "MeshTwo";
    double coords[] = {0.12, 1.21, 2.2};
    auto   vertexid = participant.setMeshVertex(meshName, coords);

    auto   dataName = "DataTwo";
    double data[]   = {7.8};
    participant.writeData(meshName, dataName, {&vertexid, 1}, data);
  }
  participant.initialize();
  BOOST_TEST(participant.isCouplingOngoing());
  participant.finalize();
}

BOOST_AUTO_TEST_SUITE_END() // Lifecycle
BOOST_AUTO_TEST_SUITE_END() // Serial
BOOST_AUTO_TEST_SUITE_END() // Integration

#endif // PRECICE_NO_MPI
