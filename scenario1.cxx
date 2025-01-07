#include <ROOT/RNTuple.hxx>
#include <ROOT/RNTupleProcessor.hxx>

#include <TH1.h>
#include <TCanvas.h>

using ROOT::Experimental::RNTupleProcessor;
using ROOT::Experimental::RNTupleOpenSpec;

void run_benchmark(std::string_view samplePath, bool makeHist = false) {
   const RNTupleOpenSpec ntuple{"ntuple", samplePath};
   auto processor = RNTupleProcessor::Create(ntuple);

   auto x = processor->GetEntry().GetPtr<float>("x");
   auto y = processor->GetEntry().GetPtr<float>("y");
   auto z = processor->GetEntry().GetPtr<float>("z");

   auto canvas = std::make_unique<TCanvas>();
   auto hist = std::make_unique<TH1D>("scenario1", "scenario1", 64, -8, 8);

   for (const auto &entry [[maybe_unused]] : *processor) {
      if (const auto &nEntries = processor->GetNEntriesProcessed(); nEntries % 1000 == 0)
        std::cout << nEntries << " entries processed" << std::endl;

      if (makeHist)
         hist->Fill(*x + *y + *z);
   }

   if (makeHist) {
      hist->DrawClone("SAME");
      canvas->SaveAs("scenario1.png");
   }
}

int main() {
   run_benchmark("data/scenario1/10k_evts.root", true);
}
