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

// STL
#include <vector>

// Custom
#include "Match.h"
#include "PatchMatchHelpers.h"

// Submodules
#include <Mask/Mask.h>


struct NeighborTest
{
  virtual bool TestNeighbor(const itk::Index<2>& currentPixel, const itk::Index<2>& queryNeighbor) const = 0;
};

struct Neighbors
{
  std::vector<itk::Index<2> > GetNeighbors(const itk::Index<2>& queryIndex) const
  {
    std::vector<itk::Index<2> > potentialNeighbors =
        ITKHelpers::Get8NeighborsInRegion(this->Region, queryIndex);

    // The neighbors that pass all tests will be stored here
    std::vector<itk::Index<2> > passedNeighbors;

    for(size_t potentialNeighborId = 0; potentialNeighborId < potentialNeighbors.size(); ++potentialNeighborId)
    {
      bool passed = true;
      for(size_t testId = 0; testId < this->NeighborTests.size(); ++testId)
      {
        if(!this->NeighborTests[testId]->TestNeighbor(queryIndex, potentialNeighbors[potentialNeighborId]))
        {
          passed = false;
          break;
        }
      }

      if(passed)
      {
        passedNeighbors.push_back(potentialNeighbors[potentialNeighborId]);
      }
    }

    return passedNeighbors;
  }

  void AddNeighborTest(NeighborTest* const neighborTest)
  {
    this->NeighborTests.push_back(neighborTest);
  }

  void SetRegion(const itk::ImageRegion<2> region)
  {
    this->Region = region;
  }

private:
  /** A collection of tests to be performed to determine if the neighbor should be included.*/
  std::vector<NeighborTest*> NeighborTests;

  /** The region in which to search for neighbors.*/
  itk::ImageRegion<2> Region;
};

struct NeighborTestAllowedPropagation : public NeighborTest
{
  NeighborTestAllowedPropagation(PatchMatchHelpers::NNFieldType* const nnField) :
  NNField(nnField)
  {
  }

  bool TestNeighbor(const itk::Index<2>& currentPixel, const itk::Index<2>& queryNeighbor) const
  {
    assert(this->NNField);

    MatchSet matchSet = this->NNField->GetPixel(queryNeighbor);
    if(matchSet.GetNumberOfMatches() == 0)
    {
      return false;
    }

    if(matchSet.GetMatch(0).GetAllowPropagation())
    {
      return true;
    }

    return false;
  }

private:
  PatchMatchHelpers::NNFieldType* NNField;
};

struct NeighborTestValidMask : public NeighborTest
{
  NeighborTestValidMask(Mask* const mask) :
  MaskImage(mask)
  {
  }

  bool TestNeighbor(const itk::Index<2>& currentPixel, const itk::Index<2>& queryNeighbor) const
  {
    assert(this->MaskImage);
    return this->MaskImage->IsValid(queryNeighbor);
  }

private:
  Mask* MaskImage;
};

struct NeighborTestVerified : public NeighborTest
{
  NeighborTestVerified(PatchMatchHelpers::NNFieldType* const nnField) :
  NNField(nnField)
  {
  }

  bool TestNeighbor(const itk::Index<2>& currentPixel, const itk::Index<2>& queryNeighbor) const
  {
    assert(this->NNField);

    return this->NNField->GetPixel(queryNeighbor).HasVerifiedMatch();
  }

private:
  PatchMatchHelpers::NNFieldType* NNField;
};

struct NeighborTestValidScore : public NeighborTest
{
  NeighborTestValidScore(PatchMatchHelpers::NNFieldType* const nnField) :
  NNField(nnField)
  {
  }

  bool TestNeighbor(const itk::Index<2>& currentPixel, const itk::Index<2>& queryNeighbor) const
  {
    assert(this->TargetMask);
    return this->NNField->GetPixel(queryNeighbor).GetMatch(0).IsValid();
  }

private:
  PatchMatchHelpers::NNFieldType* NNField;
  Mask* TargetMask;
};


struct NeighborTestForward : public NeighborTest
{
  bool TestNeighbor(const itk::Index<2>& currentPixel, const itk::Index<2>& queryNeighbor) const
  {
    itk::Offset<2> offset = queryNeighbor - currentPixel;

    itk::Offset<2> leftPixelOffset = {{-1, 0}};

    itk::Offset<2> upPixelOffset = {{0, -1}};

    if(offset == leftPixelOffset || offset == upPixelOffset)
    {
      return true;
    }

    return false;
  }
};

struct NeighborTestBackward : public NeighborTest
{
  bool TestNeighbor(const itk::Index<2>& currentPixel, const itk::Index<2>& queryNeighbor) const
  {
    itk::Offset<2> offset = queryNeighbor - currentPixel;

    itk::Offset<2> rightPixelOffset = {{1, 0}};

    itk::Offset<2> downPixelOffset = {{0, 1}};

    if(offset == rightPixelOffset || offset == downPixelOffset)
    {
      return true;
    }

    return false;
  }
};


#endif
