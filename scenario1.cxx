#include <ROOT/RNTuple.hxx>
#include <ROOT/RNTupleProcessor.hxx>

#include <TCanvas.h>
#include <TH1.h>

#include "timer.hxx"

using ROOT::Experimental::RNTupleOpenSpec;
using ROOT::Experimental::RNTupleProcessor;

void run_benchmark(std::string_view samplePath, bool debug = false) {
  const RNTupleOpenSpec ntuple{"ntuple", samplePath};
  auto processor = RNTupleProcessor::Create(ntuple);

  auto x = processor->GetEntry().GetPtr<float>("x");
  auto y = processor->GetEntry().GetPtr<float>("y");
  auto z = processor->GetEntry().GetPtr<float>("z");

  auto canvas = std::make_unique<TCanvas>();
  auto hist = std::make_unique<TH1D>("scenario1", "scenario1", 64, -8, 8);

  float xyz;

  for (const auto &entry [[maybe_unused]] : *processor) {
    if (const auto &nEntries = processor->GetNEntriesProcessed();
        debug && nEntries % 1000 == 0)
      std::cout << nEntries << " entries processed" << std::endl;

    xyz = *x + *y + *z;

    if (debug)
      hist->Fill(xyz);
  }

  if (debug) {
    hist->DrawClone("SAME");
    canvas->SaveAs("scenario1.png");
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

  auto timer = Timer();
  timer.start();

  run_benchmark("data/scenario1/" + nEvents + "_evts.root", runDebug);

  timer.end();
  timer.print(runDebug /** humanReadable */);
}
