#include "ReadDataContext.hpp"

#include "time/Waveform.hpp"

namespace precice::impl {

logging::Logger ReadDataContext::_log{"impl::ReadDataContext"};

ReadDataContext::ReadDataContext(
    mesh::PtrData data,
    mesh::PtrMesh mesh,
    int           interpolationOrder)
    : DataContext(data, mesh)
{
  _waveform = std::make_shared<time::Waveform>(interpolationOrder);
}

void ReadDataContext::appendMappingConfiguration(MappingContext &mappingContext, const MeshContext &meshContext)
{
  PRECICE_ASSERT(!hasReadMapping(), "The read data context must be unique. Otherwise we would have an ambiguous read data operation on the user side.")
  PRECICE_ASSERT(meshContext.mesh->hasDataName(getDataName()));
  mesh::PtrData data = meshContext.mesh->data(getDataName());
  PRECICE_ASSERT(data != _providedData, "Data the read mapping is mapping from needs to be different from _providedData");
  mappingContext.fromData = data;
  mappingContext.toData   = _providedData;
  appendMapping(mappingContext);
  PRECICE_ASSERT(hasReadMapping());
}

void ReadDataContext::readValues(::precice::span<const VertexID> vertices, double normalizedDt, ::precice::span<double> values) const
{
  Eigen::Map<Eigen::MatrixXd>       outputData(values.data(), getDataDimensions(), values.size());
  const Eigen::MatrixXd             sample{_waveform->sample(normalizedDt)};
  Eigen::Map<const Eigen::MatrixXd> localData(sample.data(), getDataDimensions(), getMesh().vertices().size());
  for (int i = 0; i < vertices.size(); ++i) {
    const auto vid = vertices[i];
    PRECICE_CHECK(getMesh().isValidVertexID(vid),
                  "Cannot read data \"{}\" from invalid Vertex ID ({}) of mesh \"{}\". "
                  "Please make sure you only use the results from calls to setMeshVertex/Vertices().",
                  getDataName(), vid, getMeshName());
    outputData.col(i) = localData.col(vid);
  }
}

int ReadDataContext::getInterpolationOrder() const
{
  return _waveform->getInterpolationOrder();
}

void ReadDataContext::storeDataInWaveform()
{
  _waveform->store(_providedData->values()); // store mapped or received _providedData in the _waveform
}

void ReadDataContext::initializeWaveform()
{
  PRECICE_ASSERT(not hasWriteMapping(), "Write mapping does not need waveforms.");
  _waveform->initialize(_providedData->values());
}

void ReadDataContext::moveToNextWindow()
{
  _waveform->moveToNextWindow();
}

} // namespace precice::impl
