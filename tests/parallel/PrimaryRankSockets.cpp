#ifndef PRECICE_NO_MPI

#include "testing/Testing.hpp"

#include <precice/Participant.hpp>
#include <vector>

BOOST_AUTO_TEST_SUITE(Integration)
BOOST_AUTO_TEST_SUITE(Parallel)
BOOST_AUTO_TEST_CASE(PrimaryRankSockets)
{
  PRECICE_TEST("ParallelSolver"_on(3_ranks), "SerialSolver"_on(1_rank));

  std::string myMeshName;
  if (context.isNamed("ParallelSolver")) {
    myMeshName = "ParallelMesh";
  } else {
    myMeshName = "SerialMesh";
  }
  precice::Participant participant(context.name, context.config(), context.rank, context.size);
  auto                 meshName    = myMeshName;
  double               position[2] = {0, 0};
  participant.setMeshVertex(meshName, position);
  participant.initialize();
  participant.advance(1.0);
  participant.finalize();
}

BOOST_AUTO_TEST_SUITE_END() // Integration
BOOST_AUTO_TEST_SUITE_END() // Parallel

#endif // PRECICE_NO_MPI
