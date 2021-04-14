// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

///
/// \file   RawDataQcTask.cxx
/// \author Marek Bombara
///

#include <TCanvas.h>
#include <TPad.h>
#include <TH1.h>
#include <TLatex.h>

#include "QualityControl/QcInfoLogger.h"
#include "CTP/RawDataQcTask.h"
#include <Framework/InputRecord.h>

namespace o2::quality_control_modules::ctp
{

RawDataQcTask::~RawDataQcTask()
{
  delete mHistogram;
  delete mHistogram2;
  delete mCanvasHB1;
}

void RawDataQcTask::initialize(o2::framework::InitContext& /*ctx*/)
{
  ILOG(Info, Support) << "initialize RawDataQcTask" << ENDM; // QcInfoLogger is used. FairMQ logs will go to there as well.

  // this is how to get access to custom parameters defined in the config file at qc.tasks.<task_name>.taskParameters
  if (auto param = mCustomParameters.find("myOwnKey"); param != mCustomParameters.end()) {
    ILOG(Info, Devel) << "Custom parameter - myOwnKey: " << param->second << ENDM;
  }

  mHistogram = new TH1F("example", "example", 20, 0, 30000);
  mHistogram2 = new TH1F("example2", "example2", 20, 0, 20);
  mCanvasHB1 = new TCanvas("hbmaptest", "hbmaptest", 800, 600);
  getObjectsManager()->startPublishing(mHistogram);
  getObjectsManager()->startPublishing(mHistogram2);
  getObjectsManager()->startPublishing(mCanvasHB1);
  try {
    getObjectsManager()->addMetadata(mHistogram->GetName(), "custom", "34");
    getObjectsManager()->addMetadata(mHistogram2->GetName(), "custom", "34");
    getObjectsManager()->addMetadata(mCanvasHB1->GetName(), "custom", "34");
  } catch (...) {
    // some methods can throw exceptions, it is indicated in their doxygen.
    // In case it is recoverable, it is recommended to catch them and do something meaningful.
    // Here we don't care that the metadata was not added and just log the event.
    ILOG(Warning, Support) << "Metadata could not be added to " << mHistogram->GetName() << ENDM;
    ILOG(Warning, Support) << "Metadata could not be added to " << mHistogram2->GetName() << ENDM;
    ILOG(Warning, Support) << "Metadata could not be added to " << mCanvasHB1->GetName() << ENDM;
  }
}

void RawDataQcTask::startOfActivity(Activity& activity)
{
  ILOG(Info, Support) << "startOfActivity" << activity.mId << ENDM;
  mHistogram->Reset();
  mHistogram2->Reset();
}

void RawDataQcTask::startOfCycle()
{
  ILOG(Info, Support) << "startOfCycle" << ENDM;
}

void RawDataQcTask::monitorData(o2::framework::ProcessingContext& ctx)
{
  // In this function you can access data inputs specified in the JSON config file, for example:
  //   "query": "random:ITS/RAWDATA/0"
  // which is correspondingly <binding>:<dataOrigin>/<dataDescription>/<subSpecification
  // One can also access conditions from CCDB, via separate API (see point 3)

  // Use Framework/DataRefUtils.h or Framework/InputRecord.h to access and unpack inputs (both are documented)
  // One can find additional examples at:
  // https://github.com/AliceO2Group/AliceO2/blob/dev/Framework/Core/README.md#using-inputs---the-inputrecord-api

  // Some examples:

  // 1. In a loop
  for (auto&& input : ctx.inputs()) {
    // get message header
    if (input.header != nullptr && input.payload != nullptr) {
      const auto* header = header::get<header::DataHeader*>(input.header);
      // get payload of a specific input, which is a char array.
      // const char* payload = input.payload;

      // for the sake of an example, let's fill the histogram with payload sizes
      mHistogram->Fill(header->payloadSize);
      mHistogram2->Fill(4.);
      mCanvasHB1->Divide(2,1);
      mCanvasHB1->cd(1);
      //TPad * pad1 = new TPad("pad1","The pad with the function",0.05,0.50,0.95,0.95,21); 
      mCanvasHB1->SetFillColor(42);
      TLatex l1;
      l1.SetTextSize(0.3);
      l1.DrawLatex(0.1,0.5,"ITS1");
      mCanvasHB1->cd(2);
      TLatex l2;      
      l2.SetTextSize(0.3);
      l2.DrawLatex(0.1,0.5,"ITS2");
      /*
      mCanvasHB1->cd(3);
      TLatex l3;
      l3.SetTextSize(0.3);
      l3.DrawLatex(0.1,0.5,"ITS3");
      mCanvasHB1->cd(4);
      TLatex l4;
      l4.SetTextSize(0.3);
      l4.DrawLatex(0.1,0.5,"ITS4");
      */
      mCanvasHB1->Update();
    }
  }

  // 2. Using get("<binding>")

  // get the payload of a specific input, which is a char array. "random" is the binding specified in the config file.
  //   auto payload = ctx.inputs().get("random").payload;

  // get payload of a specific input, which is a structure array:
  //  const auto* header = header::get<header::DataHeader*>(ctx.inputs().get("random").header);
  //  struct s {int a; double b;};
  //  auto array = ctx.inputs().get<s*>("random");
  //  for (int j = 0; j < header->payloadSize / sizeof(s); ++j) {
  //    int i = array.get()[j].a;
  //  }

  // get payload of a specific input, which is a root object
  //   auto h = ctx.inputs().get<TH1F*>("histos");
  //   Double_t stats[4];
  //   h->GetStats(stats);
  //   auto s = ctx.inputs().get<TObjString*>("string");
  //   LOG(INFO) << "String is " << s->GetString().Data();

  // 3. Access CCDB. If it is enough to retrieve it once, do it in initialize().
  // Remember to delete the object when the pointer goes out of scope or it is no longer needed.
  //   TObject* condition = TaskInterface::retrieveCondition("QcTask/example"); // put a valid condition path here
  //   if (condition) {
  //     LOG(INFO) << "Retrieved " << condition->ClassName();
  //     delete condition;
  //   }
}

void RawDataQcTask::endOfCycle()
{
  ILOG(Info, Support) << "endOfCycle" << ENDM;
}

void RawDataQcTask::endOfActivity(Activity& /*activity*/)
{
  ILOG(Info, Support) << "endOfActivity" << ENDM;
}

void RawDataQcTask::reset()
{
  // clean all the monitor objects here

  ILOG(Info, Support) << "Resetting the histogram" << ENDM;
  mHistogram->Reset();
  mHistogram2->Reset();
}

} // namespace o2::quality_control_modules::ctp
