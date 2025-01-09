#include <ROOT/RNTuple.hxx>
#include <ROOT/RNTupleProcessor.hxx>

#include <TCanvas.h>
#include <TH1.h>

#include "timer.hxx"

using ROOT::Experimental::RNTupleOpenSpec;
using ROOT::Experimental::RNTupleProcessor;

void run_benchmark(std::string_view primarySamplePath,
                   std::string_view auxSamplePath, bool debug = false) {
  const std::vector<RNTupleOpenSpec> ntuples{{"ntuple", primarySamplePath},
                                             {"ntuple_aux", auxSamplePath}};
  auto processor = RNTupleProcessor::CreateJoin(ntuples, {});

  auto x = processor->GetEntry().GetPtr<float>("x");
  auto y = processor->GetEntry().GetPtr<float>("y");
  auto z = processor->GetEntry().GetPtr<float>("ntuple_aux.z");

  auto canvas = std::make_unique<TCanvas>();
  auto hist = std::make_unique<TH1D>("scenario3", "scenario3", 64, -8, 8);

  float xyz;

  auto timer = Timer();
  timer.start();

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
    canvas->SaveAs("scenario3.png");
  }

  timer.end();
  timer.print(debug /** humanReadable */);
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

  run_benchmark("data/scenario4/" + nEvents + "_evts_primary.root",
                "data/scenario4/" + nEvents + "_evts_auxiliary.root", runDebug);
}
