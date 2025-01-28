#include <ROOT/RNTuple.hxx>
#include <ROOT/RNTupleModel.hxx>
#include <ROOT/RNTupleReader.hxx>
#include <ROOT/RNTupleWriter.hxx>
#include <ROOT/RNTupleParallelWriter.hxx>

#include <atomic>
#include <filesystem>
#include <iostream>
#include <memory>
#include <mutex>
#include <random>
#include <string>
#include <thread>
#include <vector>
#include <utility>

using ROOT::Experimental::RNTupleModel;
using ROOT::Experimental::RNTupleParallelWriter;
using ROOT::Experimental::RNTupleReader;
using ROOT::Experimental::RNTupleWriteOptions;
using ROOT::Experimental::RNTupleWriter;

constexpr std::uint64_t SEED=12091997;
std::mt19937 gen{SEED};
std::normal_distribution dist;

void print_ntuple_info(std::string_view sampleName, std::string_view samplePath, std::uint64_t nEntries = 5) {
  auto ntuple = RNTupleReader::Open(sampleName, samplePath);

  // ntuple->PrintInfo();

  for (unsigned i = 0; i < nEntries; ++i) {
    ntuple->Show(i);
  }
}

std::string format_n_entries(std::uint64_t nEntries) {
  if (nEntries / 1e9 >= 1)
    return std::to_string(static_cast<int>(nEntries / 1e9)) + "B";
  else if (nEntries / 1e6 >= 1)
    return std::to_string(static_cast<int>(nEntries / 1e6)) + "M";
  else if (nEntries / 1e3 >= 1)
    return std::to_string(static_cast<int>(nEntries / 1e3)) + "k";
  else
    return std::to_string(nEntries);
}

void scenario1(std::uint64_t nEntries = 1e4, std::uint32_t nSamples = 4) {
  std::string basePath = "data/scenario1/";
  std::filesystem::create_directories(basePath);
  gen.seed(SEED);

  const std::uint64_t nEntriesPerSample = nEntries / nSamples;
  auto nEntriesAsString = format_n_entries(nEntries);
  auto nEntriesPerSampleAsString = format_n_entries(nEntriesPerSample);
  std::cout << "creating scenario 1 data (" << nEntriesAsString
            << " total entries, " << nSamples << " samples, "
            << nEntriesPerSampleAsString << " entries per sample)..."
            << std::flush;

  for (unsigned i = 0; i < nSamples; ++i) {
    {
      auto model = RNTupleModel::Create();

      auto fX = model->MakeField<float>("x");
      auto fY = model->MakeField<float>("y");
      auto fZ = model->MakeField<float>("z");

      auto ntuple = RNTupleWriter::Recreate(std::move(model), "ntuple",
                                            basePath + nEntriesAsString +
                                                "_evts_sample" + i + ".root");

      for (unsigned i = 0; i < nEntriesPerSample; ++i) {
        *fX = dist(gen);
        *fY = dist(gen);
        *fZ = dist(gen);

        ntuple->Fill();
      }
    }
  }

  std::cout << " done!" << std::endl;
}

void scenario2_lower_bound(std::uint64_t nEntries = 1e4) {
  std::string basePath = "data/scenario2_lower_bound/";
  std::filesystem::create_directories(basePath);
  gen.seed(SEED);

  auto nEntriesAsString = format_n_entries(nEntries);
  std::cout << "creating scenario 2 lower bound data (" << nEntriesAsString
            << " total entries)..." << std::flush;

  {
    auto model = RNTupleModel::Create();

    auto fX = model->MakeField<float>("x");
    auto fY = model->MakeField<float>("y");
    auto fZ = model->MakeField<float>("z");

    auto ntuple = RNTupleWriter::Recreate(
        std::move(model), "ntuple", basePath + nEntriesAsString + "_evts.root");

    for (unsigned i = 0; i < nEntries; ++i) {
      *fX = dist(gen);
      *fY = dist(gen);
      *fZ = dist(gen);

      ntuple->Fill();
    }
  }

  std::cout << " done!" << std::endl;
}

void scenario2(std::uint64_t nEntries = 1e4, std::uint32_t nSamples = 4) {
    std::string basePath = "data/scenario2/";
  std::filesystem::create_directories(basePath);
  gen.seed(SEED);

  const std::uint64_t nEntriesPerSample = nEntries / nSamples;
  auto nEntriesAsString = format_n_entries(nEntries);
  auto nEntriesPerSampleAsString = format_n_entries(nEntriesPerSample);
  std::cout << "creating scenario 2 data (" << nEntriesAsString
            << " total entries, " << nSamples << " samples, "
            << nEntriesPerSampleAsString << " entries per sample)..."
            << std::flush;

  for (unsigned i = 0; i < nSamples; ++i) {
    {
      auto primaryModel = RNTupleModel::Create();
      auto fX = primaryModel->MakeField<float>("x");
      auto fY = primaryModel->MakeField<float>("y");

      auto auxiliaryModel = RNTupleModel::Create();
      auto fZ = auxiliaryModel->MakeField<float>("z");

      auto primaryNTuple = RNTupleWriter::Recreate(
          std::move(primaryModel), "ntuple",
          basePath + nEntriesAsString + "_evts_primary_sample" + i + ".root");

      auto auxiliaryNTuple = RNTupleWriter::Recreate(
          std::move(auxiliaryModel), "ntuple_aux",
          basePath + nEntriesAsString + "_evts_auxiliary_sample" + i + ".root");

      for (unsigned i = 0; i < nEntriesPerSample; ++i) {
        *fX = dist(gen);
        *fY = dist(gen);
        primaryNTuple->Fill();
        *fZ = dist(gen);
        auxiliaryNTuple->Fill();
      }
    }
  }

  std::cout << " done!" << std::endl;
}

int main() {
  for (const auto &nEntries : {1e6, 1e7, 5e7, 1e8}) {
    scenario1(nEntries);
    scenario2_lower_bound(nEntries);
    scenario2(nEntries);
  }
}
