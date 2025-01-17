#include <ROOT/RNTuple.hxx>
#include <ROOT/RNTupleProcessor.hxx>

#include <TCanvas.h>
#include <TH1.h>

#include <filesystem>

#include "timer.hxx"

using ROOT::Experimental::RNTupleOpenSpec;
using ROOT::Experimental::RNTupleProcessor;

constexpr int N_SAMPLES = 4;

void run_benchmark(
    const std::vector<std::pair<std::string, std::string>> &samplePaths,
    bool debug = false) {
  std::vector<RNTupleOpenSpec> ntuples;
  std::vector<std::unique_ptr<RNTupleProcessor>> joinProcessors;

  for (const auto &[primaryPath, auxPath] : samplePaths) {
    ntuples = {{"ntuple", primaryPath}, {"ntuple_aux", auxPath}};
    joinProcessors.emplace_back(RNTupleProcessor::CreateJoin(ntuples, {}));
  }
  auto processor = RNTupleProcessor::CreateChain(joinProcessors);

  auto x = processor->GetEntry().GetPtr<float>("x");
  auto y = processor->GetEntry().GetPtr<float>("y");
  auto z = processor->GetEntry().GetPtr<float>("ntuple_aux.z");

  auto canvas = std::make_unique<TCanvas>();
  auto hist = std::make_unique<TH1D>("scenario2_join_first",
                                     "scenario2_join_first", 64, -8, 8);

  float xyz;

  for (const auto &entry [[maybe_unused]] : *processor) {
    if (const auto &nEntries = processor->GetNEntriesProcessed();
        debug && nEntries % 50000 == 0)
      std::cout << nEntries << " entries processed" << std::endl;

    xyz = *x + *y + *z;

    if (debug)
      hist->Fill(xyz);
  }

  if (debug) {
    hist->DrawClone("SAME");
    canvas->SaveAs("scenario2_join_first.png");
  }
}

int main(int argc, char *argv[]) {
  bool runDebug = false;

  int c;

  while ((c = getopt(argc, argv, "d")) != -1) {
    switch (c) {
    case 'd':
      runDebug = true;
      break;
    default:
      break;
    }
  }

  if ((argc - optind) < 1) {
    std::cerr << "please provide the number of events to run with (in "
                 "abbreviated string format, e.g. '10k')"
              << std::endl;
    return 1;
  }

  std::string nEvents = argv[optind];

  std::vector<std::pair<std::string, std::string>> samplePaths;

  for (unsigned i = 0; i < N_SAMPLES; ++i) {
    samplePaths.emplace_back(
        "data/scenario2/" + nEvents + "_evts_primary_sample" +
            std::to_string(i) + ".root",
        "data/scenario2/" + nEvents + "_evts_auxiliary_sample" +
            std::to_string(i) + ".root");
  }

  auto timer = Timer();
  timer.start();

  run_benchmark(samplePaths, runDebug);

  timer.end();
  timer.print(runDebug /** humanReadable */);
}
