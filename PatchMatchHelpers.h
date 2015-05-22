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

#ifndef PatchMatchHelpers_H
#define PatchMatchHelpers_H

// ITK
#include "itkImageRegionConstIterator.h"
#include "itkImage.h"
#include "itkCovariantVector.h"
#include "itkVectorImage.h"

// Submodules
#include <Mask/Mask.h>
#include <ITKHelpers/ITKHelpers.h>

// Custom
#include "Match.h"
#include "NNField.h"

namespace PatchMatchHelpers
{
///////// Types //////////
typedef itk::Image<itk::CovariantVector<unsigned int, 2>, 2> CoordinateImageType;

///////// Function templates (defined in PatchMatchHelpers.hpp) //////////

template <typename NNFieldType, typename CoordinateImageType>
void GetPatchCentersImage(const NNFieldType* const matchImage, CoordinateImageType* const output);

/** Get an image where the channels are (x component, y component, score) from the nearest
  * neighbor field struct. */
template <typename NNFieldType>
void WriteNNField(const NNFieldType* const nnField, const std::string& fileName);

/////////// Non-template functions (defined in PatchMatchHelpers.cpp) /////////////

/** Read a nearest neighbor field from a file. */
void ReadNNField(const std::string& fileName, const unsigned int patchRadius,
                 NNFieldType* const nnField);

/** Get a random region inside of a specified 'region'. */
itk::ImageRegion<2> GetRandomRegionInRegion(const itk::ImageRegion<2>& region, const unsigned int patchRadius);

/** Get a random pixel index in a 'region'. */
itk::Index<2> GetRandomPixelInRegion(const itk::ImageRegion<2>& region);

/** Get a list of all of the indices in a 'region' in raster scan order. */
std::vector<itk::Index<2> > GetAllPixelIndices(const itk::ImageRegion<2>& region);

} // end PatchMatchHelpers namespace

#include "PatchMatchHelpers.hpp"

#endif
