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
#include <string>

#include "timer.hxx"

using ROOT::RNTupleModel;
using ROOT::RNTupleReader;
using ROOT::RNTupleWriteOptions;
using ROOT::RNTupleWriter;
using ROOT::Experimental::RNTupleOpenSpec;
using ROOT::Experimental::RNTupleProcessor;
using ROOT::Experimental::Internal::RNTupleMerger;
using ROOT::Internal::RPageSinkFile;
using ROOT::Internal::RPageSource;

constexpr int N_SAMPLES = 4;

void merge_samples(const std::vector<std::string> &mainSamplePaths,
                   const std::vector<std::string> &auxSamplePaths,
                   std::string_view nEventsAsString,
                   std::string_view outputDir) {
  assert(mainSamplePaths.size() == auxSamplePaths.size());

  std::vector<std::string> outputSamplePaths;

  // "join"
  for (unsigned i = 0; i < mainSamplePaths.size(); ++i) {
    auto readerMain = RNTupleReader::Open("ntuple", mainSamplePaths[i]);
    auto readerAux = RNTupleReader::Open("ntuple_aux", auxSamplePaths[i]);

    auto xRead = readerMain->GetView<float>("x");
    auto yRead = readerMain->GetView<float>("y");
    auto zRead = readerAux->GetView<float>("z");

    {
      auto model = RNTupleModel::Create();
      auto xWrite = model->MakeField<float>("x");
      auto yWrite = model->MakeField<float>("y");
      auto zWrite = model->MakeField<float>("z");

      outputSamplePaths.emplace_back(std::string(outputDir) +
                                     std::string(nEventsAsString) +
                                     "_evts_sample" + i + ".root");

      auto writer = RNTupleWriter::Recreate(std::move(model), "ntuple",
                                            outputSamplePaths[i]);

      for (const auto &entryNum : readerMain->GetEntryRange()) {
        *xWrite = xRead(entryNum);
        *yWrite = yRead(entryNum);
        *zWrite = zRead(entryNum);
        writer->Fill();
      }
    }
  }

  // "union"
  const std::string outputPath =
      std::string(outputDir) + std::string(nEventsAsString) + "_evts.root";

  std::vector<std::unique_ptr<RPageSource>> sampleSources;
  std::vector<RPageSource *> sampleSourcePtrs;
  for (unsigned i = 0; i < outputSamplePaths.size(); ++i) {
    sampleSources.push_back(
        RPageSource::Create("ntuple", outputSamplePaths[i]));
    sampleSourcePtrs.push_back(sampleSources[i].get());
  }

  auto destination = std::make_unique<RPageSinkFile>("ntuple", outputPath,
                                                     RNTupleWriteOptions());
  RNTupleMerger merger(std::move(destination));
  merger.Merge(sampleSourcePtrs);
}

void run_benchmark(const std::string &samplePath, bool debug = false) {
  const RNTupleOpenSpec ntuple{"ntuple", samplePath};
  auto processor = RNTupleProcessor::Create(ntuple);

  auto x = processor->GetEntry().GetPtr<float>("x");
  auto y = processor->GetEntry().GetPtr<float>("y");
  auto z = processor->GetEntry().GetPtr<float>("z");

  auto canvas = std::make_unique<TCanvas>();
  auto hist = std::make_unique<TH1D>("scenario2_upper_bound",
                                     "scenario2_upper_bound", 64, -8, 8);

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
    canvas->SaveAs("scenario2_upper_bound.png");
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

  std::vector<std::string> mainSamplePaths;
  std::vector<std::string> auxSamplePaths;
  std::filesystem::create_directories(
      "/tmp/ntuple_processor_eval/scenario2_upper_bound/");

  for (unsigned i = 0; i < N_SAMPLES; ++i) {
    mainSamplePaths.emplace_back("data/scenario2/" + nEvents +
                                 "_evts_primary_sample" + std::to_string(i) +
                                 ".root");
    auxSamplePaths.emplace_back("data/scenario2/" + nEvents +
                                "_evts_auxiliary_sample" + std::to_string(i) +
                                ".root");
  }

  auto timer = Timer();
  timer.start();

  merge_samples(mainSamplePaths, auxSamplePaths, nEvents,
                "/tmp/ntuple_processor_eval/scenario2_upper_bound/");
  run_benchmark("/tmp/ntuple_processor_eval/scenario2_upper_bound/" + nEvents +
                    "_evts.root",
                runDebug);

  timer.end();
  timer.print(runDebug /** humanReadable */);
}
