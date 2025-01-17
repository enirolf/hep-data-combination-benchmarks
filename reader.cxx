#include <ROOT/RNTuple.hxx>
#include <ROOT/RNTupleMerger.hxx>
#include <ROOT/RNTupleModel.hxx>
#include <ROOT/RNTupleProcessor.hxx>
#include <ROOT/RNTupleReader.hxx>
#include <ROOT/RNTupleWriter.hxx>
#include <ROOT/RPageStorageFile.hxx>

#include <TCanvas.h>
#include <TH1.h>

#include <filesystem>

#include "timer.hxx"

using ROOT::Experimental::RNTupleModel;
using ROOT::Experimental::RNTupleOpenSpec;
using ROOT::Experimental::RNTupleProcessor;
using ROOT::Experimental::RNTupleReader;
using ROOT::Experimental::RNTupleWriter;
using ROOT::Experimental::RNTupleWriteOptions;
using ROOT::Experimental::Internal::RNTupleMerger;
using ROOT::Experimental::Internal::RPageSource;
using ROOT::Experimental::Internal::RPageSinkFile;

constexpr int N_SAMPLES = 4;

void run_benchmark(std::string_view inputPath, bool debug = false) {
  auto canvas = std::make_unique<TCanvas>();
  auto hist = std::make_unique<TH1D>("baseline", "baseline",
                                     64, -8, 8);

  auto reader = RNTupleReader::Open("ntuple", inputPath);
  reader->EnableMetrics();

  auto x = reader->GetView<float>("x");
  auto y = reader->GetView<float>("y");
  auto z = reader->GetView<float>("z");
  auto r = reader->GetView<float>("r");

  float xyzr;

  for (const auto &entryNumber : reader->GetEntryRange()) {
    xyzr = x(entryNumber) + y(entryNumber) + z(entryNumber) + r(entryNumber);

    if (debug)
      hist->Fill(xyzr);
  }

  reader->PrintInfo(ROOT::Experimental::ENTupleInfo::kMetrics);

  if (debug) {
    hist->DrawClone("SAME");
    canvas->SaveAs("baseline.png");
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

  std::string inputPath = "data/baseline/" + nEvents + "_evts.root";

  auto timer = Timer();
  timer.start();

  run_benchmark(inputPath, runDebug);

  timer.end();
  timer.print(runDebug /** humanReadable */);
}
