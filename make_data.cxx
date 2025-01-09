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

void create_single_sample(std::string_view sampleName,
                          std::string_view samplePath,
                          std::uint64_t nEntries = 1e4,
                          std::uint64_t kStart = 0,
                          const std::vector<std::string> &fieldNames = {
                              "x", "y", "z"}) {
  auto model = RNTupleModel::Create();

  auto fK = model->MakeField<std::uint64_t>("k");

  std::vector<std::shared_ptr<float>> fields;
  for (const auto &fieldName : fieldNames) {
    fields.emplace_back(model->MakeField<float>(fieldName));
  }

  auto ntuple = RNTupleWriter::Recreate(std::move(model), sampleName, samplePath);

  for (unsigned i = 0; i < nEntries; ++i) {
    *fK = i + kStart; // Maybe change to something more sophisticated?

    for (auto &field : fields) {
      *field = dist(gen);
    }

    ntuple->Fill();
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

void scenario1(std::uint64_t nEntries = 1e4) {
  gen.seed(SEED);

  auto nEntriesAsString = format_n_entries(nEntries);
  std::cout << "creating data set for scenario 1 (" << nEntriesAsString << " total entries)..." << std::flush;

  create_single_sample("ntuple", "data/scenario1/" + nEntriesAsString + "_evts.root", nEntries);

  std::cout << " done!" << std::endl;
}

void scenario2(std::uint64_t nEntries = 1e4, std::uint32_t nSamples = 4) {
  gen.seed(SEED);

  const std::uint64_t nEntriesPerSample = nEntries / nSamples;
  auto nEntriesAsString = format_n_entries(nEntries);
  auto nEntriesPerSampleAsString = format_n_entries(nEntriesPerSample);
  std::cout << "creating data set for scenario 2 (" << nEntriesAsString
            << " total entries, " << nSamples << " samples, "
            << nEntriesPerSampleAsString << " entries per sample)..."
            << std::flush;

  for (unsigned i = 0; i < nSamples; ++i) {
    create_single_sample("ntuple", "data/scenario2/" + nEntriesAsString + "_evts_sample" + i + ".root", nEntriesPerSample, i * nEntriesPerSample);
  }

  std::cout << " done!" << std::endl;
}

void scenario3(std::uint64_t nEntries = 1e4) {
  gen.seed(SEED); // TODO figure out how to make x y z values consistent with the others

  auto nEntriesAsString = format_n_entries(nEntries);
  std::cout << "creating data set for scenario 3 (" << nEntriesAsString << " total entries)..." << std::flush;

  create_single_sample("ntuple", "data/scenario3/" + nEntriesAsString + "_evts_primary.root", nEntries, 0, {"x", "y"});
  create_single_sample("ntuple_aux", "data/scenario3/" + nEntriesAsString + "_evts_auxiliary.root", nEntries, 0, {"z"});

  std::cout << " done!" << std::endl;
}

void scenario4(std::uint64_t nEntries = 1e4) {
  gen.seed(SEED);
  auto nEntriesAsString = format_n_entries(nEntries);
  std::cout << "creating data set for scenario 4 (" << nEntriesAsString << " total entries)..." << std::flush;

  create_single_sample("ntuple", "data/scenario4/" + nEntriesAsString + "_evts_primary.root", nEntries, 0, {"x", "y"});

  constexpr unsigned nThreads = 8;
  const unsigned nEntriesPerThread = nEntries / nThreads;

  auto fnFill = [&nEntriesPerThread](RNTupleParallelWriter *writer) {
    static std::atomic<std::uint32_t> globalThreadId;
    const auto threadId = ++globalThreadId;

    auto fillContext = writer->CreateFillContext();
    auto entry = fillContext->CreateEntry();

    auto fK = entry->GetPtr<std::uint64_t>("k");
    auto fZ = entry->GetPtr<float>("z");

    for (unsigned i = 0;  i < nEntriesPerThread; ++i) {
      *fK = nEntriesPerThread * threadId + i;
      *fZ = dist(gen);
      fillContext->Fill(*entry);
    }
  };

  auto model = RNTupleModel::CreateBare();
  model->MakeField<std::uint64_t>("k");
  model->MakeField<float>("z");

  auto writer = RNTupleParallelWriter::Recreate(
      std::move(model), "ntuple_aux",
      "data/scenario4/" + nEntriesAsString + "_evts_auxiliary.root");

  std::vector<std::thread> threads;
  for (unsigned i = 0; i < nThreads; ++i) {
    threads.emplace_back(fnFill, writer.get());
  }
  for (unsigned i = 0; i < nThreads; ++i) {
    threads[i].join();
  }

  std::cout << " done!" << std::endl;
}

void scenario5(std::uint64_t nEntries = 1e4) {
  // TODO
}

int main() {
  std::filesystem::create_directories("data/scenario1");
  std::filesystem::create_directories("data/scenario2");
  std::filesystem::create_directories("data/scenario3");
  std::filesystem::create_directories("data/scenario4");
  std::filesystem::create_directories("data/scenario5");

  for (const auto &nEntries : {1e6, 1e7, 5e7, 1e8}) {
    scenario1(nEntries);
    scenario2(nEntries);
    scenario3(nEntries);
    scenario4(nEntries);
  }
}
