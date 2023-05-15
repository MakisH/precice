#ifndef PRECICE_NO_MPI

#include "testing/Testing.hpp"

#include <action/RecorderAction.hpp>
#include <precice/Participant.hpp>
#include <vector>

/**
 * @brief Test to make sure that actions are called in the right order for explicit coupling via RecorderAction
 */
BOOST_AUTO_TEST_SUITE(Integration)
BOOST_AUTO_TEST_SUITE(Serial)
BOOST_AUTO_TEST_SUITE(ActionTimings)
BOOST_AUTO_TEST_CASE(ActionTimingsExplicit)
{
  PRECICE_TEST("SolverOne"_on(1_rank), "SolverTwo"_on(1_rank));

  using namespace precice;
  Participant participant(context.name, context.config(), 0, 1);

  std::string meshName;
  std::string writeDataName;
  std::string readDataName;
  double      writeValue;

  if (context.isNamed("SolverOne")) {
    meshName      = "MeshOne";
    writeDataName = "Forces";
    readDataName  = "Velocities";
    writeValue    = 1;
  } else {
    BOOST_TEST(context.isNamed("SolverTwo"));
    meshName      = "MeshTwo";
    writeDataName = "Velocities";
    readDataName  = "Forces";
    writeValue    = 2;
  }
  int                 dimensions = participant.getMeshDimensions(meshName);
  std::vector<double> vertex(dimensions, 0);
  int                 vertexID = participant.setMeshVertex(meshName, vertex);

  double dt = -1;
  BOOST_TEST(action::RecorderAction::records.empty());
  action::RecorderAction::reset();
  std::vector<double> writeData(dimensions, writeValue);
  std::vector<double> readData(dimensions, -1);

  if (participant.requiresInitialData()) {
    BOOST_TEST(context.isNamed("SolverTwo"));
    participant.writeData(meshName, writeDataName, {&vertexID, 1}, writeData);
  }

  participant.initialize();
  dt = participant.getMaxTimeStepSize();
  BOOST_TEST(dt == 1.0);

  if (context.isNamed("SolverOne")) {
    BOOST_TEST(action::RecorderAction::records.size() == 1);
    BOOST_TEST(action::RecorderAction::records.at(0).timing == action::Action::READ_MAPPING_POST);
  } else {
    BOOST_TEST(context.isNamed("SolverTwo"));
    BOOST_TEST(action::RecorderAction::records.size() == 2);
    BOOST_TEST(action::RecorderAction::records.at(0).timing == action::Action::WRITE_MAPPING_POST);
    BOOST_TEST(action::RecorderAction::records.at(1).timing == action::Action::READ_MAPPING_POST);
  }
  action::RecorderAction::reset();

  int iteration = 0;

  while (participant.isCouplingOngoing()) {
    participant.readData(meshName, readDataName, {&vertexID, 1}, dt, readData);
    participant.writeData(meshName, writeDataName, {&vertexID, 1}, writeData);
    participant.advance(dt);
    double dt = participant.getMaxTimeStepSize();
    BOOST_TEST(participant.isTimeWindowComplete());
    iteration++;
    if (context.isNamed("SolverOne") || iteration < 10) {
      BOOST_TEST(action::RecorderAction::records.size() == 2);
      BOOST_TEST(action::RecorderAction::records.at(0).timing == action::Action::WRITE_MAPPING_POST);
      BOOST_TEST(action::RecorderAction::records.at(1).timing == action::Action::READ_MAPPING_POST);
    } else { // SolverTwo only writes in very last iteration, does not read.
      BOOST_TEST(action::RecorderAction::records.size() == 1);
      BOOST_TEST(action::RecorderAction::records.at(0).timing == action::Action::WRITE_MAPPING_POST);
    }
    action::RecorderAction::reset();
  }
  participant.finalize();
}

BOOST_AUTO_TEST_SUITE_END() // Integration
BOOST_AUTO_TEST_SUITE_END() // Serial
BOOST_AUTO_TEST_SUITE_END() // ActionTimings

#endif // PRECICE_NO_MPI
