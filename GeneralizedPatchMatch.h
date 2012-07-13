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

#ifndef GeneralizedPatchMatch_H
#define GeneralizedPatchMatch_H

#include "PatchMatch.h"

template<typename TImage>
class GeneralizedPatchMatch : public PatchMatch<TImage>
{
public:

  /** Constructor. */
  GeneralizedPatchMatch();

  /** The type that is used to store the nearest neighbor field. */
  typedef itk::Image<std::vector<Match>, 2> GeneralizedPMImageType;

  /** Get an image where the channels are (x component, y component, score) from the nearest neighbor field. */
  static void GetPatchCentersImage(GeneralizedPMImageType* const pmImage,
                                   typename PatchMatch<TImage>::CoordinateImageType* const output);

  /** Get the Output. */
  GeneralizedPMImageType* GetOutput();

  /** Add this match to the top matches if it is better than any of the current matches. */
  void AddIfBetter(const itk::Index<2>& index, const Match& match);

  /** Set the number of bestcandidate matches to store at each pixel. */
  void SetNumberOfCandidates(const unsigned int numberOfCandidates);

private:
  /** The intermediate and final output. This is a different type than in the parent class. */
  GeneralizedPMImageType::Pointer Output;

  /** The number of bestcandidate matches to store at each pixel. */
  unsigned int NumberOfCandidates;
};

#include "GeneralizedPatchMatch.hpp"

#endif
