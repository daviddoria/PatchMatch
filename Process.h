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

#ifndef Process_H
#define Process_H

// STL
#include <functional>

// Custom
#include "Match.h"

// Submodules
#include <Mask/Mask.h>

struct Process
{
  Process(){}

  typedef itk::Image<Match, 2> NNFieldType;

  void SetNNField(NNFieldType* const nnField)
  {
    this->NNField = nnField;
  }

  virtual bool ShouldProcess(const itk::Index<2>& queryIndex) = 0;

  virtual std::vector<itk::Index<2> > GetPixelsToProcess(const Mask* mask) = 0;

  NNFieldType* NNField;
};

struct ProcessValidMaskPixels : public Process
{
  ProcessValidMaskPixels(Mask* const mask)
  {
    this->MaskImage = mask;
  }
  
  bool ShouldProcess(const itk::Index<2>& queryIndex)
  {
    return true;
  }

  std::vector<itk::Index<2> > GetPixelsToProcess()
  {
    return GetPixelsToProcess(this->MaskImage);
  }
  
  std::vector<itk::Index<2> > GetPixelsToProcess(const Mask* const mask)
  {
    std::vector<itk::Index<2> > validPixels = mask->GetValidPixels();
    return validPixels;
  }

private:
  Mask* MaskImage;
};

struct ProcessInvalid : public Process
{
  ProcessInvalid(){}

  typedef itk::Image<Match, 2> MatchImageType;

  void SetNNField(MatchImageType* const nnField)
  {
    this->MatchImage = nnField;
  }

  bool ShouldProcess(const itk::Index<2>& queryIndex)
  {
    assert(this->MatchImage);
    if(!this->MatchImage->GetPixel(queryIndex).IsValid())
    {
      return true;
    }
    return false;
  }

  std::vector<itk::Index<2> > GetPixelsToProcess(const Mask* mask)
  {
    std::vector<itk::Index<2> > validPixels = mask->GetValidPixels();
    validPixels.erase(std::remove_if(validPixels.begin(), validPixels.end(),
                       [this](const itk::Index<2>& queryIndex)
                       {
                         return !this->ShouldProcess(queryIndex);
                       }),
                       validPixels.end());

    return validPixels;
  }

  MatchImageType* MatchImage;
};

#endif
  