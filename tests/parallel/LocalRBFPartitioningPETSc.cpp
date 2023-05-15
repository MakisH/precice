#ifndef PRECICE_NO_MPI
#ifndef PRECICE_NO_PETSC

#include "testing/Testing.hpp"

#include <precice/Participant.hpp>
#include <vector>

BOOST_AUTO_TEST_SUITE(Integration)
BOOST_AUTO_TEST_SUITE(Parallel)
BOOST_AUTO_TEST_CASE(LocalRBFPartitioningPETSc)
{
  PRECICE_TEST("SolverOne"_on(3_ranks), "SolverTwo"_on(1_rank));

  if (context.name == "SolverOne") {
    precice::Participant participant(context.name, context.config(), context.rank, context.size);
    auto                 meshName = "MeshOne";
    auto                 dataName = "Data2";

    int    vertexIDs[2];
    double xCoord       = context.rank * 0.4;
    double positions[4] = {xCoord, 0.0, xCoord + 0.2, 0.0};
    participant.setMeshVertices(meshName, positions, vertexIDs);
    participant.initialize();
    double values[2];
    participant.advance(1.0);
    double preciceDt = participant.getMaxTimeStepSize();
    participant.readData(meshName, dataName, vertexIDs, preciceDt, values);
    participant.finalize();
  } else {
    BOOST_REQUIRE(context.isNamed("SolverTwo"));
    precice::Participant participant(context.name, context.config(), context.rank, context.size);
    auto                 meshName = "MeshTwo";
    int                  vertexIDs[6];
    double               positions[12] = {0.0, 0.0, 0.2, 0.0, 0.4, 0.0, 0.6, 0.0, 0.8, 0.0, 1.0, 0.0};
    participant.setMeshVertices(meshName, positions, vertexIDs);
    participant.initialize();
    auto   dataName  = "Data2";
    double values[6] = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0};
    participant.writeData(meshName, dataName, vertexIDs, values);
    participant.advance(1.0);
    participant.finalize();
  }
}

BOOST_AUTO_TEST_SUITE_END() // Integration
BOOST_AUTO_TEST_SUITE_END() // Parallel

#endif
#endif // PRECICE_NO_MPI
