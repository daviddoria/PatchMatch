/*=========================================================================
 *
 *  Copyright David Doria 2012 daviddoria@gmail.com
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *         http://www.apache.org/licenses/LICENSE-2.0.txt
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *=========================================================================*/

#ifndef AcceptanceTestComposite_H
#define AcceptanceTestComposite_H

// Custom
#include "AcceptanceTest.h"

// Boost
#include <boost/signals2/signal.hpp>

class AcceptanceTestComposite : public AcceptanceTest
{
public:
  virtual bool IsBetterWithScore(const itk::ImageRegion<2>& queryRegion, const Match& oldMatch,
                        const Match& potentialBetterMatch, float& score)
  {
    // Run the tests in a way that if one fails the rest are not run at all.
    for(size_t i = 0; i < this->AcceptanceTests.size(); ++i)
    {
      AcceptanceTest* acceptanceTest = this->AcceptanceTests[i];
      if(!acceptanceTest->IsBetterWithScore(queryRegion, oldMatch, potentialBetterMatch, score))
      {
        WhichFailedSignal(i);
        return false;
      }
    }

    return true;
  }

  /** Add an acceptance test. */
  void AddAcceptanceTest(AcceptanceTest* const acceptanceTest)
  {
    this->AcceptanceTests.push_back(acceptanceTest);
  }

  /** Remove an acceptance test. */
  void RemoveAcceptanceTest(AcceptanceTest* const acceptanceTestToRemove)
  {
    std::cout << "Before RemoveAcceptanceTest() there are " << this->AcceptanceTests.size() << std::endl;
    this->AcceptanceTests.erase(std::remove_if(this->AcceptanceTests.begin(), this->AcceptanceTests.end(),
                                                [acceptanceTestToRemove](AcceptanceTest* queryTest)
                                                {
                                                  return queryTest == acceptanceTestToRemove;
                                                }),
                                 this->AcceptanceTests.end());

    std::cout << "After RemoveAcceptanceTest() there are " << this->AcceptanceTests.size() << std::endl;
  }

  boost::signals2::signal<void (unsigned int)> WhichFailedSignal;

private:
  std::vector<AcceptanceTest*> AcceptanceTests;
};
#endif
