#include <ROOT/RNTuple.hxx>
#include <ROOT/RNTupleModel.hxx>
#include <ROOT/RNTupleProcessor.hxx>
#include <ROOT/RNTupleWriter.hxx>

#include <TCanvas.h>
#include <TH1.h>

#include <filesystem>

#include "timer.hxx"

using ROOT::RNTupleModel;
using ROOT::RNTupleWriter;
using ROOT::Experimental::RNTupleOpenSpec;
using ROOT::Experimental::RNTupleProcessor;

constexpr int N_SAMPLES = 4;

void compute_intermediate_result(const std::vector<std::string> &inputPaths,
                                 const std::vector<std::string> &outputPaths) {
  for (unsigned i = 0; i < inputPaths.size(); ++i) {
    RNTupleOpenSpec ntuple{"ntuple", inputPaths[i]};
    auto processor = RNTupleProcessor::Create(ntuple);

    auto xRead = processor->GetEntry().GetPtr<float>("x");
    auto yRead = processor->GetEntry().GetPtr<float>("y");
    auto zRead = processor->GetEntry().GetPtr<float>("z");

    {
      auto model = RNTupleModel::Create();
      auto rWrite = model->MakeField<float>("r");

      auto writer = RNTupleWriter::Recreate(std::move(model), "ntuple_aux",
                                            outputPaths[i]);

      for (auto &entry [[maybe_unused]] : *processor) {
        *rWrite = *xRead + *yRead + *zRead;
        writer->Fill();
      }
    }
  }
}

void run_benchmark(const std::vector<std::string> &primaryPaths,
                   const std::vector<std::string> &auxPaths,
                   bool debug = false) {
  auto canvas = std::make_unique<TCanvas>();
  auto hist = std::make_unique<TH1D>("scenario1_join_first", "scenario1_join_first", 64, -8, 8);

  std::vector<RNTupleOpenSpec> ntuples;
  std::vector<std::unique_ptr<RNTupleProcessor>> joinProcessors;

  for (unsigned i = 0; i < primaryPaths.size(); ++i) {
    joinProcessors.emplace_back(RNTupleProcessor::CreateJoin(
        {"ntuple", primaryPaths[i]}, {"ntuple_aux", auxPaths[i]}, {}));
}
  auto processor = RNTupleProcessor::CreateChain(std::move(joinProcessors));

  auto x = processor->GetEntry().GetPtr<float>("x");
  auto y = processor->GetEntry().GetPtr<float>("y");
  auto z = processor->GetEntry().GetPtr<float>("z");
  auto r = processor->GetEntry().GetPtr<float>("ntuple_aux.r");

  float xyzr;

  for (auto &entry [[maybe_unused]] : *processor) {
    xyzr = *x + *y + *z + *r;

    if (debug)
      hist->Fill(xyzr);
  }

  if (debug) {
    hist->DrawClone("SAME");
    canvas->SaveAs("scenario1_join_first.png");
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

  std::vector<std::string> inputPaths;
  std::vector<std::string> outputPaths;
  std::filesystem::create_directories(
      "/tmp/ntuple_processor_eval/scenario1_join_first/");

  for (unsigned i = 0; i < N_SAMPLES; ++i) {
    inputPaths.emplace_back("data/scenario1/" + nEvents + "_evts_sample" +
                            std::to_string(i) + ".root");
    outputPaths.emplace_back(
        "/tmp/ntuple_processor_eval/scenario1_join_first/" + nEvents +
        "_evts_sample" + std::to_string(i) + ".root");
  }

  auto timer = Timer();
  timer.start();

  compute_intermediate_result(inputPaths, outputPaths);
  run_benchmark(inputPaths, outputPaths, runDebug);

  timer.end();
  timer.print(runDebug /** humanReadable */);
}
