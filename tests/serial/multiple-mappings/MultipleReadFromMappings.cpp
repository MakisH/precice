#ifndef PRECICE_NO_MPI

#include "testing/Testing.hpp"

#include <precice/Participant.hpp>
#include <vector>

BOOST_AUTO_TEST_SUITE(Integration)
BOOST_AUTO_TEST_SUITE(Serial)
BOOST_AUTO_TEST_SUITE(MultipleMappings)
BOOST_AUTO_TEST_CASE(MultipleReadFromMappings)
{
  PRECICE_TEST("A"_on(1_rank), "B"_on(1_rank));

  using Eigen::Vector2d;

  precice::Participant participant(context.name, context.config(), context.rank, context.size);
  Vector2d             vertex{0.0, 0.0};

  if (context.isNamed("A")) {
    auto meshNameTop    = "MeshATop";
    auto meshNameBottom = "MeshABottom";
    int  vertexIDTop    = participant.setMeshVertex(meshNameTop, vertex);
    int  vertexIDBottom = participant.setMeshVertex(meshNameBottom, vertex);
    auto dataNameTop    = "Pressure";
    auto dataNameBottom = "Pressure";

    participant.initialize();
    double dt = participant.getMaxTimeStepSize();
    participant.advance(dt);
    double pressure = -1.0;
    dt              = participant.getMaxTimeStepSize();
    participant.readData(meshNameTop, dataNameTop, {&vertexIDTop, 1}, dt, {&pressure, 1});
    BOOST_TEST(pressure == 1.0);
    pressure = -1.0;
    participant.readData(meshNameBottom, dataNameBottom, {&vertexIDBottom, 1}, dt, {&pressure, 1});
    BOOST_TEST(pressure == 1.0);
    BOOST_TEST(not participant.isCouplingOngoing());
    participant.finalize();

  } else {
    BOOST_TEST(context.isNamed("B"));
    auto meshName = "MeshB";
    int  vertexID = participant.setMeshVertex(meshName, vertex);
    auto dataName = "Pressure";

    participant.initialize();
    double pressure = 1.0;
    participant.writeData(meshName, dataName, {&vertexID, 1}, {&pressure, 1});
    double dt = participant.getMaxTimeStepSize();
    participant.advance(dt);
    BOOST_TEST(not participant.isCouplingOngoing());
    participant.finalize();
  }
}

BOOST_AUTO_TEST_SUITE_END() // Integration
BOOST_AUTO_TEST_SUITE_END() // Serial
BOOST_AUTO_TEST_SUITE_END() // MultipleMappings

#endif // PRECICE_NO_MPI
