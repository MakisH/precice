#ifndef PRECICE_NO_MPI

#include "testing/Testing.hpp"

#include <precice/Participant.hpp>
#include <vector>

BOOST_AUTO_TEST_SUITE(Integration)
BOOST_AUTO_TEST_SUITE(Serial)
BOOST_AUTO_TEST_SUITE(MultipleMappings)
BOOST_AUTO_TEST_CASE(MultipleWriteFromMappings)
{
  PRECICE_TEST("A"_on(1_rank), "B"_on(1_rank));

  using Eigen::Vector2d;

  precice::Participant participant(context.name, context.config(), context.rank, context.size);
  Vector2d             vertex1{0.0, 0.0};
  Vector2d             vertex2{2.0, 0.0};
  Vector2d             vertex3{4.0, 0.0};

  if (context.isNamed("A")) {
    auto meshNameTop    = "MeshATop";
    auto meshNameBottom = "MeshABottom";
    int  vertexIDTop    = participant.setMeshVertex(meshNameTop, vertex1);
    int  vertexIDBottom = participant.setMeshVertex(meshNameBottom, vertex3);
    auto dataNameTop    = "Pressure";
    auto dataNameBottom = "Pressure";

    participant.initialize();
    double dt = participant.getMaxTimeStepSize();
    participant.advance(dt);
    dt              = participant.getMaxTimeStepSize();
    double pressure = -1.0;
    participant.readData(meshNameTop, dataNameTop, {&vertexIDTop, 1}, dt, {&pressure, 1});
    BOOST_TEST(pressure == 1.0);
    pressure = -1.0;
    participant.readData(meshNameBottom, dataNameBottom, {&vertexIDBottom, 1}, dt, {&pressure, 1});
    BOOST_TEST(pressure == 5.0);
    BOOST_TEST(not participant.isCouplingOngoing());
    participant.finalize();

  } else {
    BOOST_TEST(context.isNamed("B"));
    auto meshName  = "MeshB";
    int  vertexID1 = participant.setMeshVertex(meshName, vertex1);
    int  vertexID2 = participant.setMeshVertex(meshName, vertex2);
    int  vertexID3 = participant.setMeshVertex(meshName, vertex3);
    auto dataName  = "Pressure";

    participant.initialize();
    double pressure = 1.0;
    participant.writeData(meshName, dataName, {&vertexID1, 1}, {&pressure, 1});
    pressure = 4.0;
    participant.writeData(meshName, dataName, {&vertexID2, 1}, {&pressure, 1});
    pressure = 5.0;
    participant.writeData(meshName, dataName, {&vertexID3, 1}, {&pressure, 1});
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
