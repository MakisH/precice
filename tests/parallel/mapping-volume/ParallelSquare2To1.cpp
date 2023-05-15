#ifndef PRECICE_NO_MPI

#include "testing/Testing.hpp"

#include <precice/Participant.hpp>
#include <vector>

BOOST_AUTO_TEST_SUITE(Integration)
BOOST_AUTO_TEST_SUITE(Parallel)
BOOST_AUTO_TEST_SUITE(MappingVolume)
BOOST_AUTO_TEST_CASE(ParallelSquare2To1)
{
  PRECICE_TEST("SolverOne"_on(2_ranks), "SolverTwo"_on(1_rank));

  using precice::VertexID;
  using precice::testing::equals;

  precice::Participant participant(context.name, context.config(), context.rank, context.size);

  std::vector<VertexID> vertexIDs;
  double                dt;

  if (context.isNamed("SolverOne")) {
    auto meshName = "MeshOne";
    auto dataName = "DataOne";

    std::vector<double> coords;

    // Create a square with top left corner (rank 0) or bottom right. Diagonal "y = x" is shared.
    if (context.rank == 0) {
      coords = {0.0, 0.0,
                1.0, 1.0,
                0.0, 1.0};
    } else {
      coords = {0.0, 0.0,
                1.0, 1.0,
                1.0, 0.0};
    }

    vertexIDs.resize(coords.size() / 2);
    participant.setMeshVertices(meshName, coords, vertexIDs);
    participant.setMeshTriangle(meshName, vertexIDs[0], vertexIDs[1], vertexIDs[2]);

    participant.initialize();
    dt = participant.getMaxTimeStepSize();

    // Run a step and write data with f(x) = x+2*y
    BOOST_TEST(participant.isCouplingOngoing(), "Sending participant must advance once.");

    std::vector<double> values;
    if (context.rank == 0) {
      values = {0.0,
                3.0,
                2.0};
    } else {
      values = {0.0,
                3.0,
                1.0};
    }

    participant.writeData(meshName, dataName, vertexIDs, values);

    participant.advance(dt);
    BOOST_TEST(!participant.isCouplingOngoing(), "Sending participant must advance only once.");
    participant.finalize();

  } else {
    auto meshName = "MeshTwo";
    auto dataName = "DataOne";

    std::vector<double> coords;

    /* Each ranks reads points from both input meshes
    Rank 0: (1/6, 1/2) and (1/2, 1/6)
    Rank 1: (5/6, 1/2) and (1/2, 5/6)

    */
    coords = {1. / 6, 1. / 2,
              1. / 2, 1. / 6,
              5. / 6, 1. / 2,
              1. / 2, 5. / 6};

    vertexIDs.resize(coords.size() / 2);
    participant.setMeshVertices(meshName, coords, vertexIDs);

    participant.initialize();
    dt = participant.getMaxTimeStepSize();

    // Run a step and read data expected to be f(x) = x+2*y
    BOOST_TEST(participant.isCouplingOngoing(), "Receiving participant must advance once.");

    participant.advance(dt);
    BOOST_TEST(!participant.isCouplingOngoing(), "Receiving participant must advance only once.");

    //Check expected VS read
    Eigen::VectorXd expected(4);
    Eigen::VectorXd readData(4);
    expected << 7. / 6, 5. / 6, 11. / 6, 13. / 6;

    dt = participant.getMaxTimeStepSize();
    participant.readData(meshName, dataName, vertexIDs, dt, readData);
    BOOST_CHECK(equals(expected, readData));
    participant.finalize();
  }
}

BOOST_AUTO_TEST_SUITE_END() // Integration
BOOST_AUTO_TEST_SUITE_END() // Parallel
BOOST_AUTO_TEST_SUITE_END() // MappingVolume

#endif // PRECICE_NO_MPI
