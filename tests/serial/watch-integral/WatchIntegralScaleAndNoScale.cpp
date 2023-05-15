#ifndef PRECICE_NO_MPI

#include "testing/Testing.hpp"

#include <precice/Participant.hpp>
#include <vector>
#include "helpers.hpp"
#include "io/TXTTableWriter.hpp"

BOOST_AUTO_TEST_SUITE(Integration)
BOOST_AUTO_TEST_SUITE(Serial)
BOOST_AUTO_TEST_CASE(WatchIntegralScaleAndNoScale)
{
  PRECICE_TEST("SolverOne"_on(1_rank), "SolverTwo"_on(1_rank));

  using Eigen::Vector2d;

  if (context.isNamed("SolverOne")) {
    precice::Participant participant(context.name, context.config(), 0, 1);

    // Set mesh
    Vector2d coordA{0.0, 0.0};
    Vector2d coordB{1.0, 0.0};
    Vector2d coordC{1.0, 2.0};

    auto meshName = "MeshOne";

    int idA = participant.setMeshVertex(meshName, coordA);
    int idB = participant.setMeshVertex(meshName, coordB);
    int idC = participant.setMeshVertex(meshName, coordC);

    participant.setMeshEdge(meshName, idA, idB);
    participant.setMeshEdge(meshName, idB, idC);

    // Initialize, the mesh
    participant.initialize();
    double dt = participant.getMaxTimeStepSize();

    auto   dataOneID = "DataOne";
    double valueA    = 1.0;
    double valueB    = 2.0;
    double valueC    = 3.0;

    double increment = 1.0;

    while (participant.isCouplingOngoing()) {

      participant.writeData(meshName, dataOneID, {&idA, 1}, {&valueA, 1});
      participant.writeData(meshName, dataOneID, {&idB, 1}, {&valueB, 1});
      participant.writeData(meshName, dataOneID, {&idC, 1}, {&valueC, 1});

      participant.advance(dt);
      double dt = participant.getMaxTimeStepSize();

      valueA += increment;
      valueB += increment;
      valueC += increment;
    }
    participant.finalize();
  } else if (context.isNamed("SolverTwo")) {

    precice::Participant participant(context.name, context.config(), 0, 1);

    // Set mesh
    Vector2d coordA{0.0, 0.0};
    Vector2d coordB{1.0, 0.0};
    Vector2d coordC{1.0, 2.0};

    auto meshTwoID = "MeshTwo";

    int idA = participant.setMeshVertex(meshTwoID, coordA);
    int idB = participant.setMeshVertex(meshTwoID, coordB);
    int idC = participant.setMeshVertex(meshTwoID, coordC);

    participant.setMeshEdge(meshTwoID, idA, idB);
    participant.setMeshEdge(meshTwoID, idB, idC);

    // Initialize the mesh
    participant.initialize();
    double dt = participant.getMaxTimeStepSize();

    auto   dataOneID = "DataOne";
    double valueA, valueB, valueC;

    while (participant.isCouplingOngoing()) {

      participant.readData(meshTwoID, dataOneID, {&idA, 1}, dt, {&valueA, 1});
      participant.readData(meshTwoID, dataOneID, {&idB, 1}, dt, {&valueB, 1});
      participant.readData(meshTwoID, dataOneID, {&idC, 1}, dt, {&valueC, 1});

      participant.advance(dt);
      double dt = participant.getMaxTimeStepSize();
    }
    participant.finalize();

    {
      std::string fileName = "precice-SolverTwo-watchintegral-WatchIntegral.log";
      auto        result   = readDoublesFromTXTFile(fileName, 3);
      auto        expected = std::vector<double>{
          1.0, 9.5, 3.0,
          2.0, 12.5, 3.0,
          3.0, 12.5, 3.0};
      BOOST_TEST(result.size() == expected.size());
      for (size_t i = 0; i < result.size(); ++i) {
        BOOST_TEST_CONTEXT("entry index: " << i)
        {
          using testing::equals;
          BOOST_TEST(equals(result.at(i), expected.at(i)));
        }
      }
    }

    {
      std::string fileName = "precice-SolverTwo-watchintegral-WatchIntegralNoScale.log";
      auto        result   = readDoublesFromTXTFile(fileName, 3);
      auto        expected = std::vector<double>{
          1.0, 9.0, 3.0,
          2.0, 12.0, 3.0,
          3.0, 12.0, 3.0};
      BOOST_TEST(result.size() == expected.size());
      for (size_t i = 0; i < result.size(); ++i) {
        BOOST_TEST_CONTEXT("entry index: " << i)
        {
          using testing::equals;
          BOOST_TEST(equals(result.at(i), expected.at(i)));
        }
      }
    }
  }
}

BOOST_AUTO_TEST_SUITE_END() // Integration
BOOST_AUTO_TEST_SUITE_END() // Serial

#endif // PRECICE_NO_MPI
