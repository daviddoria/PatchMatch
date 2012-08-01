#include <iostream>
#include <vector>

#include "PatchMatch.h"

// Submodules
#include <PatchComparison/SelfPatchCompare.h>
#include <PatchComparison/SSD.h>
#include <Helpers/Helpers.h>

int main(int argc, char*argv[])
{
  // Verify arguments
  if(argc < 5)
  {
    std::cerr << "Required arguments: image mask patchRadius output" << std::endl;
    return EXIT_FAILURE;
  }

  // Parse arguments
  std::stringstream ss;
  for(int i = 1; i < argc; ++i)
  {
    ss << argv[i] << " ";
  }
  std::string imageFilename;
  std::string maskFilename;
  unsigned int patchRadius;
  std::string outputFilename;

  ss >> imageFilename >> maskFilename >> patchRadius >> outputFilename;

  // Output arguments
  std::cout << "imageFilename: " << imageFilename << std::endl;
  std::cout << "maskFilename: " << maskFilename << std::endl;
  std::cout << "patchRadius: " << patchRadius << std::endl;
  std::cout << "outputFilename: " << outputFilename << std::endl;

  typedef itk::Image<itk::CovariantVector<unsigned char, 3>, 2> ImageType;
  
  typedef itk::ImageFileReader<ImageType> ImageReaderType;
  ImageReaderType::Pointer imageReader = ImageReaderType::New();
  imageReader->SetFileName(imageFilename);
  imageReader->Update();

  ImageType* image = imageReader->GetOutput();

  // The target mask should be valid in the region to fill
  Mask::Pointer targetMask = Mask::New();
  targetMask->Read(maskFilename);
  targetMask->InvertData();

  // The source mask should be valid in the region from which to take source patches
  Mask::Pointer sourceMask = Mask::New();
  sourceMask->Read(maskFilename);

  SSD<ImageType>* patchDistanceFunctor = new SSD<ImageType>;
  patchDistanceFunctor->SetImage(imageReader->GetOutput());
  
  typedef SelfPatchCompare<ImageType> SelfPatchCompareType;
  SelfPatchCompareType selfPatchCompare;
  selfPatchCompare.SetPatchDistanceFunctor(patchDistanceFunctor);
  selfPatchCompare.SetImage(image);
  selfPatchCompare.SetMask(sourceMask);

  itk::ImageRegion<2> targetMaskBoundingBox = MaskOperations::ComputeValidBoundingBox(targetMask);
  
  itk::ImageRegionIteratorWithIndex<ImageType> imageIterator(image, targetMaskBoundingBox);

  typedef PatchMatch<ImageType> PatchMatchType;
  PatchMatchType::CoordinateImageType::Pointer nnField =
       PatchMatchType::CoordinateImageType::New();
  nnField->SetRegions(image->GetLargestPossibleRegion());
  nnField->Allocate();

  unsigned int numberOfTargetPixels = targetMask->CountValidPixels();
  unsigned int numberOfCompletedPixels = 0;
  while(!imageIterator.IsAtEnd())
  {
    if(targetMask->IsValid(imageIterator.GetIndex()))
    {
      std::cout << "Computing NN for " << imageIterator.GetIndex() << std::endl;
      itk::ImageRegion<2> targetRegion =
           ITKHelpers::GetRegionInRadiusAroundPixel(imageIterator.GetIndex(), patchRadius);
      selfPatchCompare.SetTargetRegion(targetRegion);
      selfPatchCompare.ComputePatchScores();
      std::vector<SelfPatchCompareType::PatchDataType> patchData = selfPatchCompare.GetPatchData();
      std::partial_sort(patchData.begin(), patchData.begin() + 2, patchData.end(),
                        Helpers::SortBySecondAccending<SelfPatchCompareType::PatchDataType>);

      itk::Index<2> bestMatchCenter = ITKHelpers::GetRegionCenter(patchData[0].first);
      PatchMatchType::CoordinateImageType::PixelType nnFieldPixel;
      nnFieldPixel[0] = bestMatchCenter[0];
      nnFieldPixel[1] = bestMatchCenter[1];
      nnFieldPixel[2] = patchData[0].second;
      std::cout << "NN is " << nnFieldPixel << std::endl;
      nnField->SetPixel(imageIterator.GetIndex(), nnFieldPixel);
    }
    numberOfCompletedPixels++;
    std::cout << "Completed " << numberOfCompletedPixels << " of " << numberOfTargetPixels << std::endl;
    ++imageIterator;
  }

  ITKHelpers::WriteImage(nnField.GetPointer(), outputFilename);
  return EXIT_SUCCESS;
}
