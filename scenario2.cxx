#include <ROOT/RNTuple.hxx>
#include <ROOT/RNTupleProcessor.hxx>

#include <TH1.h>
#include <TCanvas.h>

#include <filesystem>

using ROOT::Experimental::RNTupleProcessor;
using ROOT::Experimental::RNTupleOpenSpec;

constexpr int N_SAMPLES = 4;

void run_benchmark(const std::vector<std::string> samplePaths, bool makeHist = false) {
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

   for (const auto &entry [[maybe_unused]] : *processor) {
      if (const auto &nEntries = processor->GetNEntriesProcessed(); nEntries % 1000 == 0)
        std::cout << nEntries << " entries processed" << std::endl;

      if (makeHist)
         hist->Fill(*x + *y + *z);
   }

   if (makeHist) {
      hist->DrawClone("SAME");
      canvas->SaveAs("scenario2.png");
   }
}

int main() {
   std::vector<std::string> samplePaths;

   for (unsigned i = 0; i < N_SAMPLES; ++i) {
      samplePaths.emplace_back("data/scenario2/2k_evts_sample" + std::to_string(i) + ".root");
   }

   run_benchmark(samplePaths, true);
}
