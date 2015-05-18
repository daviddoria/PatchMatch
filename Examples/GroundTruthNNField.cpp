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
  if(argc < 4)
  {
    std::cerr << "Required arguments: image patchRadius output" << std::endl;
    return EXIT_FAILURE;
  }

  // Parse arguments
  std::stringstream ss;
  for(int i = 1; i < argc; ++i)
  {
    ss << argv[i] << " ";
  }
  std::string imageFilename;
  unsigned int patchRadius;
  std::string outputFilename;

  ss >> imageFilename >> patchRadius >> outputFilename;

  // Output arguments
  std::cout << "imageFilename: " << imageFilename << std::endl;
  std::cout << "patchRadius: " << patchRadius << std::endl;
  std::cout << "outputFilename: " << outputFilename << std::endl;

  typedef itk::Image<itk::CovariantVector<unsigned char, 3>, 2> ImageType;
  
  typedef itk::ImageFileReader<ImageType> ImageReaderType;
  ImageReaderType::Pointer imageReader = ImageReaderType::New();
  imageReader->SetFileName(imageFilename);
  imageReader->Update();

  ImageType* image = imageReader->GetOutput();

  SSD<ImageType>* patchDistanceFunctor = new SSD<ImageType>;
  patchDistanceFunctor->SetImage(imageReader->GetOutput());
  
  typedef SelfPatchCompare<ImageType> SelfPatchCompareType;
  SelfPatchCompareType selfPatchCompare;
  selfPatchCompare.SetPatchDistanceFunctor(patchDistanceFunctor);
  selfPatchCompare.SetImage(image);
  selfPatchCompare.CreateFullyValidMask();

  itk::ImageRegionIteratorWithIndex<ImageType> imageIterator(image, image->GetLargestPossibleRegion());

  PatchMatchHelpers::CoordinateImageType::Pointer nnField =
       PatchMatchHelpers::CoordinateImageType::New();
  nnField->SetRegions(image->GetLargestPossibleRegion());
  nnField->Allocate();

  unsigned int numberOfCompletedPixels = 0;
  while(!imageIterator.IsAtEnd())
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
    PatchMatchHelpers::CoordinateImageType::PixelType nnFieldPixel;
    nnFieldPixel[0] = bestMatchCenter[0];
    nnFieldPixel[1] = bestMatchCenter[1];
    nnFieldPixel[2] = patchData[0].second;
    std::cout << "NN is " << nnFieldPixel << std::endl;
    nnField->SetPixel(imageIterator.GetIndex(), nnFieldPixel);

    numberOfCompletedPixels++;

    ++imageIterator;
  }

  ITKHelpers::WriteImage(nnField.GetPointer(), outputFilename);
  return EXIT_SUCCESS;
}
