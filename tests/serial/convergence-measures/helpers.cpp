#ifndef PRECICE_NO_MPI

#include "helpers.hpp"
#include "testing/Testing.hpp"

#include "precice/Participant.hpp"

void testConvergenceMeasures(const std::string configFile, TestContext const &context, std::vector<int> &expectedIterations)
{
  using Eigen::Vector2d;

  std::string meshName = context.isNamed("SolverOne") ? "MeshOne" : "MeshTwo";

  precice::Participant participant(context.name, configFile, 0, 1);

  Vector2d vertex{0.0, 0.0};

  std::vector<double> writeValues = {1.0, 1.01, 2.0, 2.5, 2.8, 2.81};

  VertexID vertexID = participant.setMeshVertex(meshName, vertex);

  participant.initialize();
  int numberOfAdvanceCalls = 0;
  int numberOfIterations   = -1;
  int timestep             = 0;

  while (participant.isCouplingOngoing()) {
    if (participant.requiresWritingCheckpoint()) {
      numberOfIterations = 0;
    }

    if (context.isNamed("SolverTwo")) {
      auto dataName = "Data2";
      participant.writeData(meshName, dataName, {&vertexID, 1}, {&writeValues.at(numberOfAdvanceCalls), 1});
    }

    participant.advance(1.0);
    ++numberOfAdvanceCalls;
    ++numberOfIterations;

    if (participant.requiresReadingCheckpoint()) {
    } else { //converged
      BOOST_TEST(numberOfIterations == expectedIterations.at(timestep));
      ++timestep;
    }
  }
  participant.finalize();
}

#endif
