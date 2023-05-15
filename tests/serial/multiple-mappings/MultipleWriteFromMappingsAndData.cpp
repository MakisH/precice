#ifndef PRECICE_NO_MPI

#include "testing/Testing.hpp"

#include <precice/Participant.hpp>
#include <vector>

BOOST_AUTO_TEST_SUITE(Integration)
BOOST_AUTO_TEST_SUITE(Serial)
BOOST_AUTO_TEST_SUITE(MultipleMappings)
BOOST_AUTO_TEST_CASE(MultipleWriteFromMappingsAndData)
{
  PRECICE_TEST("A"_on(1_rank), "B"_on(1_rank));

  using Eigen::Vector2d;

  precice::Participant participant(context.name, context.config(), context.rank, context.size);
  Vector2d             vertex1{0.0, 0.0};
  Vector2d             vertex2{2.0, 0.0};
  Vector2d             vertex3{4.0, 0.0};

  if (context.isNamed("A")) {
    auto meshNameTop     = "MeshATop";
    auto meshNameBottom  = "MeshABottom";
    int  vertexIDTop     = participant.setMeshVertex(meshNameTop, vertex1);
    int  vertexIDBottom  = participant.setMeshVertex(meshNameBottom, vertex3);
    auto dataNameTopP    = "Pressure";
    auto dataNameBottomP = "Pressure";
    auto dataNameTopT    = "Temperature";
    auto dataNameBottomT = "Temperature";

    participant.initialize();
    double dt = participant.getMaxTimeStepSize();
    participant.advance(dt);
    double pressure    = -1.0;
    double temperature = -1.0;
    dt                 = participant.getMaxTimeStepSize();
    participant.readData(meshNameTop, dataNameTopP, {&vertexIDTop, 1}, dt, {&pressure, 1});
    participant.readData(meshNameTop, dataNameTopT, {&vertexIDTop, 1}, dt, {&temperature, 1});
    BOOST_TEST(pressure == 1.0);
    BOOST_TEST(temperature == 331);
    pressure    = -1.0;
    temperature = -1.0;
    participant.readData(meshNameBottom, dataNameBottomP, {&vertexIDBottom, 1}, dt, {&pressure, 1});
    participant.readData(meshNameBottom, dataNameBottomT, {&vertexIDBottom, 1}, dt, {&temperature, 1});
    BOOST_TEST(temperature == 273.15);
    BOOST_TEST(pressure == 5.0);
    BOOST_TEST(not participant.isCouplingOngoing());
    participant.finalize();

  } else {
    BOOST_TEST(context.isNamed("B"));
    auto meshName  = "MeshB";
    int  vertexID1 = participant.setMeshVertex(meshName, vertex1);
    int  vertexID2 = participant.setMeshVertex(meshName, vertex2);
    int  vertexID3 = participant.setMeshVertex(meshName, vertex3);
    auto dataNameP = "Pressure";
    auto dataNameT = "Temperature";

    participant.initialize();
    double pressure    = 1.0;
    double temperature = 331;
    participant.writeData(meshName, dataNameP, {&vertexID1, 1}, {&pressure, 1});
    participant.writeData(meshName, dataNameT, {&vertexID1, 1}, {&temperature, 1});
    pressure    = 4.0;
    temperature = 335;
    participant.writeData(meshName, dataNameP, {&vertexID2, 1}, {&pressure, 1});
    participant.writeData(meshName, dataNameT, {&vertexID2, 1}, {&temperature, 1});
    pressure    = 5.0;
    temperature = 273.15;
    participant.writeData(meshName, dataNameP, {&vertexID3, 1}, {&pressure, 1});
    participant.writeData(meshName, dataNameT, {&vertexID3, 1}, {&temperature, 1});
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
