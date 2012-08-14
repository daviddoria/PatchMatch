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
#include <vector>

// Custom
#include "Match.h"
#include "PatchMatchHelpers.h"

// Submodules
#include <Mask/Mask.h>

struct Process
{

  Process() : NNField(NULL), MaskImage(NULL), Forward(true)
  {}

  Process(Mask* const mask, PatchMatchHelpers::NNFieldType* const nnField) : NNField(nnField), MaskImage(mask), Forward(true)
  {}

  Process(Mask* const mask) : NNField(NULL), MaskImage(mask), Forward(true)
  {}

  void SetNNField(PatchMatchHelpers::NNFieldType* const nnField)
  {
    this->NNField = nnField;
  }

  void SetMask(Mask* const mask)
  {
    this->MaskImage = mask;
  }

  virtual bool ShouldProcess(const itk::Index<2>& queryIndex) = 0;

  virtual std::vector<itk::Index<2> > GetPixelsToProcess() = 0;

  void SetForward(const bool forward)
  {
    this->Forward = forward;
  }

protected:
  PatchMatchHelpers::NNFieldType* NNField;
  Mask* MaskImage;
  bool Forward;
};

struct ProcessValidMaskPixels : public Process
{
  ProcessValidMaskPixels(Mask* const mask) : Process(mask)
  {
    this->MaskImage = mask;
  }

  bool ShouldProcess(const itk::Index<2>& queryIndex)
  {
    return this->MaskImage->IsValid(queryIndex);
  }

  std::vector<itk::Index<2> > GetPixelsToProcess()
  {
    return GetPixelsToProcess(this->MaskImage);
  }

  std::vector<itk::Index<2> > GetPixelsToProcess(const Mask* const mask)
  {
    std::vector<itk::Index<2> > validPixels = mask->GetValidPixels(this->Forward);
    return validPixels;
  }

};

struct ProcessUnverifiedValidMaskPixels : public Process
{
  ProcessUnverifiedValidMaskPixels(PatchMatchHelpers::NNFieldType* const nnField, Mask* const mask) : Process(mask), NNField(nnField)
  {
  }

  bool ShouldProcess(const itk::Index<2>& queryIndex)
  {
    bool shouldProcess = this->MaskImage->IsValid(queryIndex) && !this->NNField->GetPixel(queryIndex).HasVerifiedMatch();
    return shouldProcess;
  }

  std::vector<itk::Index<2> > GetPixelsToProcess()
  {
    return GetPixelsToProcess(this->MaskImage);
  }

  std::vector<itk::Index<2> > GetPixelsToProcess(const Mask* const mask)
  {
    std::vector<itk::Index<2> > validMaskPixels = mask->GetValidPixels(this->Forward);

    std::vector<itk::Index<2> > pixelsToProcess;
    for(size_t i = 0; i < validMaskPixels.size(); ++i)
    {
      if(ShouldProcess(validMaskPixels[i]))
      {
        pixelsToProcess.push_back(validMaskPixels[i]);
      }
    }
    return pixelsToProcess;
  }

private:
  PatchMatchHelpers::NNFieldType* NNField;

};

#endif
