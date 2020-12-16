// Flow Vector Correction Framework
//
// Copyright (C) 2020  Lukas Kreis Ilya Selyuzhenkov
// Contact: l.kreis@gsi.de; ilya.selyuzhenkov@gmail.com
// For a full list of contributors please see docs/Credits
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include <ROOT/RDataFrame.hxx>
#include <gtest/gtest.h>

#include "AverageHelper.hpp"
#include "AxesConfiguration.hpp"
#include "CorrelationAction.hpp"
#include "CorrelationBuilder.hpp"
#include "ReSampleFunctor.hpp"

TEST(CorrelationBuilderUnitTest, RDataFrame) {
  ROOT::RDataFrame df(100);
  std::size_t phi_size = 4;
  Qn::DataContainerQVector qvec_proto;
  auto event_axis = Qn::AxisD("event", 2, 0, 2);
  for (auto& q : qvec_proto) {
    q.ActivateHarmonic(1);
    q.InitializeHarmonics();
  }
  auto df0 = df.DefineSlotEntry("event",
                                [](unsigned int, ULong64_t entry) {
                                  double a = 0;
                                  if (entry > 90) a = 1.;
                                  return a;
                                },
                                {});
  auto df1 = df0.Define("q",
                        [qvec_proto](double event) -> Qn::DataContainerQVector {
                          auto ret = qvec_proto;
                          for (int i = 0; i < 100; ++i) {
                            ret.At(0).Add(0, 1.);
                          }
                          ret.At(0).CheckQuality();
                          ret.At(0).Normal(Qn::QVector::Normalization::M);
                          return ret;
                        },
                        {"event"});
  auto axes = Qn::MakeAxes(event_axis);
  auto c1 = [](const Qn::QVector& a) { return a.x(1); };
  std::vector<Qn::DataContainerQVector> initializations{qvec_proto};
  int n = 5;
  auto df2 = Qn::Correlation::Resample(df1, n);
  auto y = Qn::Correlation::UseWeights::Yes;
  std::array<std::string, 1> qs = {"q"};
  auto correlationvector = Qn::Correlation::CorrelationBuilder(&df2, n, axes);
  correlationvector.AddCorrelationWithInitializationObject("test", c1, c1, y,
                                                           qs, initializations);

  auto results = correlationvector.GetResults();
  auto x = results[0]->GetDataContainer();
  auto xx = Qn::DataContainerStatCalculate(x);
}
