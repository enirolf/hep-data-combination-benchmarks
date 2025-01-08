#include <ROOT/RNTuple.hxx>
#include <ROOT/RNTupleProcessor.hxx>

#include <TCanvas.h>
#include <TH1.h>

#include <filesystem>

#include "timer.hxx"

using ROOT::Experimental::RNTupleOpenSpec;
using ROOT::Experimental::RNTupleProcessor;

constexpr int N_SAMPLES = 4;

void run_benchmark(const std::vector<std::string> samplePaths,
                   bool makeHist = false, bool writeLog = false) {
  std::vector<RNTupleOpenSpec> ntuples;
  for (const auto &path : samplePaths) {
    ntuples.emplace_back("ntuple", path);
  }
  auto processor = RNTupleProcessor::CreateChain(ntuples);

  auto x = processor->GetEntry().GetPtr<float>("x");
  auto y = processor->GetEntry().GetPtr<float>("y");
  auto z = processor->GetEntry().GetPtr<float>("z");

  auto canvas = std::make_unique<TCanvas>();
  auto hist = std::make_unique<TH1D>("scenario2", "scenario2", 64, -8, 8);

  float xyz;

  auto timer = Timer();
  timer.start();

  for (const auto &entry [[maybe_unused]] : *processor) {
    if (const auto &nEntries = processor->GetNEntriesProcessed();
        writeLog && nEntries % 1000 == 0)
      std::cout << nEntries << " entries processed" << std::endl;

    xyz = *x + *y + *z;

    if (makeHist)
      hist->Fill(xyz);
  }

  if (makeHist) {
    hist->DrawClone("SAME");
    canvas->SaveAs("scenario2.png");
  }

  timer.end();
  timer.print(/* true */ /** humanReadable */);
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    std::cerr << "please provide the number of events to run with (in "
                 "abbreviated string format, e.g. '10k')"
              << std::endl;
    return 1;
  }

  std::string nEvents = argv[1];
  std::vector<std::string> samplePaths;

  for (unsigned i = 0; i < N_SAMPLES; ++i) {
    samplePaths.emplace_back("data/scenario2/" + nEvents + "_evts_sample" +
                             std::to_string(i) + ".root");
  }

  run_benchmark(samplePaths);
}
