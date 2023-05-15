#ifndef PRECICE_NO_MPI

#include "testing/Testing.hpp"

#include <precice/Participant.hpp>
#include <vector>

BOOST_AUTO_TEST_SUITE(Integration)
BOOST_AUTO_TEST_SUITE(Serial)
BOOST_AUTO_TEST_CASE(AitkenAcceleration)
{
  PRECICE_TEST("A"_on(1_rank), "B"_on(1_rank));

  using Eigen::Vector2d;

  precice::Participant participant(context.name, context.config(), context.rank, context.size);
  Vector2d             vertex{0.0, 0.0};

  if (context.isNamed("A")) {
    auto meshName = "A-Mesh";
    int  vertexID = participant.setMeshVertex(meshName, vertex);
    auto dataName = "Data";

    participant.initialize();
    double dt    = participant.getMaxTimeStepSize();
    double value = 1.0;
    participant.writeData(meshName, dataName, {&vertexID, 1}, {&value, 1});

    participant.requiresWritingCheckpoint();
    participant.advance(dt);
    participant.requiresReadingCheckpoint();

    participant.requiresWritingCheckpoint();
    participant.advance(dt);
    participant.requiresReadingCheckpoint();

    BOOST_TEST(not participant.isCouplingOngoing());
    participant.finalize();

  } else {
    BOOST_TEST(context.isNamed("B"));
    auto meshName = "B-Mesh";
    int  vertexID = participant.setMeshVertex(meshName, vertex);
    auto dataName = "Data";

    participant.initialize();
    double dt = participant.getMaxTimeStepSize();
    participant.requiresWritingCheckpoint();
    participant.advance(dt);
    participant.requiresReadingCheckpoint();

    double value = -1.0;

    dt = participant.getMaxTimeStepSize();
    participant.readData(meshName, dataName, {&vertexID, 1}, dt, {&value, 1});
    BOOST_TEST(value == 0.1); // due to initial underrelaxation

    participant.requiresWritingCheckpoint();
    participant.advance(dt);
    participant.requiresReadingCheckpoint();

    BOOST_TEST(not participant.isCouplingOngoing());
    participant.finalize();
  }
}

BOOST_AUTO_TEST_SUITE_END() // Integration
BOOST_AUTO_TEST_SUITE_END() // Serial

#endif // PRECICE_NO_MPI
