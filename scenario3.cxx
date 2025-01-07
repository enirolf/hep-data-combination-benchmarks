#include <ROOT/RNTuple.hxx>
#include <ROOT/RNTupleProcessor.hxx>

#include <TH1.h>
#include <TCanvas.h>

using ROOT::Experimental::RNTupleProcessor;
using ROOT::Experimental::RNTupleOpenSpec;

void run_benchmark(std::string_view primarySamplePath, std::string_view auxSamplePath, bool makeHist = false) {
   const std::vector<RNTupleOpenSpec> ntuples{{"ntuple", primarySamplePath}, {"ntuple_aux", auxSamplePath}};
   auto processor = RNTupleProcessor::CreateJoin(ntuples, {});

   auto x = processor->GetEntry().GetPtr<float>("x");
   auto y = processor->GetEntry().GetPtr<float>("y");
   auto z = processor->GetEntry().GetPtr<float>("ntuple_aux.z");

   auto canvas = std::make_unique<TCanvas>();
   auto hist = std::make_unique<TH1D>("scenario3", "scenario3", 64, -8, 8);

   for (const auto &entry [[maybe_unused]] : *processor) {
      if (const auto &nEntries = processor->GetNEntriesProcessed(); nEntries % 1000 == 0)
        std::cout << nEntries << " entries processed" << std::endl;

      if (makeHist)
         hist->Fill(*x + *y + *z);
   }

   if (makeHist) {
      hist->DrawClone("SAME");
      canvas->SaveAs("scenario3.png");
   }
}

int main() {
  run_benchmark("data/scenario3/10k_evts_primary.root",
                "data/scenario3/10k_evts_auxiliary.root", true);
}
