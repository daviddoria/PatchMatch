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

#ifndef Neighbors_H
#define Neighbors_H

struct AllowedPropagationNeighbors
{
  AllowedPropagationNeighbors(Mask* const allowedPropagationMask, Mask* const targetMask)
  {
    this->AllowedPropagationMask = allowedPropagationMask;
    this->TargetMask = targetMask;
  }

  std::vector<itk::Index<2> > operator() (const itk::Index<2>& queryIndex) const
  {
    std::vector<itk::Index<2> > potentialPropagationNeighbors = ITKHelpers::Get8NeighborsInRegion(AllowedPropagationMask->GetLargestPossibleRegion(),
                                                                                                queryIndex);

    std::vector<itk::Index<2> > allowedPropagationNeighbors;
    for(size_t i = 0; i < potentialPropagationNeighbors.size(); ++i)
    {
      if(this->AllowedPropagationMask->IsValid(potentialPropagationNeighbors[i]) ||
          this->TargetMask->IsValid(potentialPropagationNeighbors[i]) )
      {
        allowedPropagationNeighbors.push_back(potentialPropagationNeighbors[i]);
      }
    }
    return allowedPropagationNeighbors;
  }

private:
  Mask* AllowedPropagationMask;
  Mask* TargetMask;
};

struct ForwardPropagationNeighbors
{
  std::vector<itk::Index<2> > operator() (const itk::Index<2>& queryIndex) const
  {
    std::vector<itk::Index<2> > allowedPropagationNeighbors;

    itk::Offset<2> leftPixelOffset = {{-1, 0}};
    allowedPropagationNeighbors.push_back(queryIndex + leftPixelOffset);

    itk::Offset<2> upPixelOffset = {{0, -1}};
    allowedPropagationNeighbors.push_back(queryIndex + upPixelOffset);

    return allowedPropagationNeighbors;
  }
};

struct BackwardPropagationNeighbors
{
  std::vector<itk::Index<2> > operator() (const itk::Index<2>& queryIndex) const
  {
    std::vector<itk::Index<2> > allowedPropagationNeighbors;

    itk::Offset<2> leftPixelOffset = {{1, 0}};
    allowedPropagationNeighbors.push_back(queryIndex + leftPixelOffset);

    itk::Offset<2> upPixelOffset = {{0, 1}};
    allowedPropagationNeighbors.push_back(queryIndex + upPixelOffset);

    return allowedPropagationNeighbors;
  }
};


#endif
