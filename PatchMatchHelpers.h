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

namespace PatchMatchHelpers
{
  
itk::Offset<2> RandomNeighborNonZeroOffset()
{
  int randomOffsetX = 0;
  int randomOffsetY = 0;
  while((randomOffsetX == 0) && (randomOffsetY == 0) ) // We don't want the random offset to be (0,0), because the histogram difference would be zero!
  {
    // Generate random numbers in the set (-1,0,1) by generating a number in (0,1,2) and then subtracting 1
    randomOffsetX = rand()%3 - 1;
    randomOffsetY = rand()%3 - 1;
  }

  itk::Offset<2> randomNeighborNonZeroOffset = {{randomOffsetX, randomOffsetY}};

  return randomNeighborNonZeroOffset;
}

} // end PatchMatchHelpers namespace

#endif
