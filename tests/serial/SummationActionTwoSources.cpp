#ifndef PRECICE_NO_MPI

#include "testing/Testing.hpp"

#include <precice/Participant.hpp>
#include <vector>

BOOST_AUTO_TEST_SUITE(Integration)
BOOST_AUTO_TEST_SUITE(Serial)
BOOST_AUTO_TEST_CASE(SummationActionTwoSources)
{
  PRECICE_TEST("SolverTarget"_on(1_rank), "SolverSourceOne"_on(1_rank), "SolverSourceTwo"_on(1_rank));

  using Eigen::Vector3d;

  if (context.isNamed("SolverTarget")) {
    // Expected values in the target solver
    double expectedValueA = 3.0;
    double expectedValueB = 7.0;
    double expectedValueC = 11.0;
    double expectedValueD = 15.0;

    // Target solver
    precice::Participant participant(context.name, context.config(), 0, 1);

    // Set mesh
    Vector3d coordA{0.0, 0.0, 0.3};
    Vector3d coordB{1.0, 0.0, 0.3};
    Vector3d coordC{1.0, 1.0, 0.3};
    Vector3d coordD{0.0, 1.0, 0.3};

    auto meshName = "MeshTarget";

    int idA = participant.setMeshVertex(meshName, coordA);
    int idB = participant.setMeshVertex(meshName, coordB);
    int idC = participant.setMeshVertex(meshName, coordC);
    int idD = participant.setMeshVertex(meshName, coordD);

    // Initialize, the mesh
    participant.initialize();
    double dt = participant.getMaxTimeStepSize();

    // Read the summed data from the mesh.
    auto   dataAID = "Target";
    double valueA, valueB, valueC, valueD;

    while (participant.isCouplingOngoing()) {

      participant.readData(meshName, dataAID, {&idA, 1}, dt, {&valueA, 1});
      participant.readData(meshName, dataAID, {&idB, 1}, dt, {&valueB, 1});
      participant.readData(meshName, dataAID, {&idC, 1}, dt, {&valueC, 1});
      participant.readData(meshName, dataAID, {&idD, 1}, dt, {&valueD, 1});

      BOOST_TEST(valueA == expectedValueA);
      BOOST_TEST(valueB == expectedValueB);
      BOOST_TEST(valueC == expectedValueC);
      BOOST_TEST(valueD == expectedValueD);

      participant.advance(dt);
      double dt = participant.getMaxTimeStepSize();
    }

    participant.finalize();
  } else if (context.isNamed("SolverSourceOne")) {
    // Source solver one
    precice::Participant participant(context.name, context.config(), 0, 1);

    // Set mesh
    Vector3d coordA{0.0, 0.0, 0.3};
    Vector3d coordB{1.0, 0.0, 0.3};
    Vector3d coordC{1.0, 1.0, 0.3};
    Vector3d coordD{0.0, 1.0, 0.3};

    auto meshName = "MeshOne";

    int idA = participant.setMeshVertex(meshName, coordA);
    int idB = participant.setMeshVertex(meshName, coordB);
    int idC = participant.setMeshVertex(meshName, coordC);
    int idD = participant.setMeshVertex(meshName, coordD);

    // Initialize, the mesh
    participant.initialize();
    double dt = participant.getMaxTimeStepSize();

    auto   dataAID = "SourceOne";
    double valueA  = 1.0;
    double valueB  = 3.0;
    double valueC  = 5.0;
    double valueD  = 7.0;

    while (participant.isCouplingOngoing()) {

      participant.writeData(meshName, dataAID, {&idA, 1}, {&valueA, 1});
      participant.writeData(meshName, dataAID, {&idB, 1}, {&valueB, 1});
      participant.writeData(meshName, dataAID, {&idC, 1}, {&valueC, 1});
      participant.writeData(meshName, dataAID, {&idD, 1}, {&valueD, 1});

      participant.advance(dt);
      double dt = participant.getMaxTimeStepSize();
    }
    participant.finalize();
  } else {
    BOOST_REQUIRE(context.isNamed("SolverSourceTwo"));
    // Source solver two
    precice::Participant participant(context.name, context.config(), 0, 1);

    // Set mesh
    Vector3d coordA{0.0, 0.0, 0.3};
    Vector3d coordB{1.0, 0.0, 0.3};
    Vector3d coordC{1.0, 1.0, 0.3};
    Vector3d coordD{0.0, 1.0, 0.3};

    auto meshName = "MeshTwo";

    int idA = participant.setMeshVertex(meshName, coordA);
    int idB = participant.setMeshVertex(meshName, coordB);
    int idC = participant.setMeshVertex(meshName, coordC);
    int idD = participant.setMeshVertex(meshName, coordD);

    // Initialize, the mesh
    participant.initialize();
    double dt = participant.getMaxTimeStepSize();

    auto   dataAID = "SourceTwo";
    double valueA  = 2.0;
    double valueB  = 4.0;
    double valueC  = 6.0;
    double valueD  = 8.0;

    while (participant.isCouplingOngoing()) {

      participant.writeData(meshName, dataAID, {&idA, 1}, {&valueA, 1});
      participant.writeData(meshName, dataAID, {&idB, 1}, {&valueB, 1});
      participant.writeData(meshName, dataAID, {&idC, 1}, {&valueC, 1});
      participant.writeData(meshName, dataAID, {&idD, 1}, {&valueD, 1});

      participant.advance(dt);
      double dt = participant.getMaxTimeStepSize();
    }

    participant.finalize();
  }
}

BOOST_AUTO_TEST_SUITE_END() // Integration
BOOST_AUTO_TEST_SUITE_END() // Serial

#endif // PRECICE_NO_MPI
