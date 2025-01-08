#include <ROOT/RNTuple.hxx>
#include <ROOT/RNTupleProcessor.hxx>

#include <TCanvas.h>
#include <TH1.h>

#include "timer.hxx"

using ROOT::Experimental::RNTupleOpenSpec;
using ROOT::Experimental::RNTupleProcessor;

void run_benchmark(std::string_view samplePath, bool makeHist = false,
                   bool writeLog = false) {
  const RNTupleOpenSpec ntuple{"ntuple", samplePath};
  auto processor = RNTupleProcessor::Create(ntuple);

  auto x = processor->GetEntry().GetPtr<float>("x");
  auto y = processor->GetEntry().GetPtr<float>("y");
  auto z = processor->GetEntry().GetPtr<float>("z");

  auto canvas = std::make_unique<TCanvas>();
  auto hist = std::make_unique<TH1D>("scenario1", "scenario1", 64, -8, 8);

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
    canvas->SaveAs("scenario1.png");
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

  run_benchmark("data/scenario1/" + nEvents + "_evts.root");
}
