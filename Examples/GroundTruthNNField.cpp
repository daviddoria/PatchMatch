#include <iostream>
#include <vector>

#include "PatchMatch.h"
#include "Propagator.h"
#include "RandomSearch.h"

// Submodules
#include <PatchComparison/SelfPatchCompare.h>
#include <PatchComparison/SSD.h>
#include <Helpers/Helpers.h>

template<typename TImage>
void WriteExactNNField(TImage* image, const unsigned int patchRadius);

template<typename TImage>
void WriteTrivialNNField(TImage* image, const unsigned int patchRadius);

template<typename TImage>
void WritePatchMatchNNField(TImage* image, const unsigned int patchRadius);

int main(int argc, char*argv[])
{
  // Verify arguments
  if(argc < 3)
  {
    std::cerr << "Required arguments: image patchRadius" << std::endl;
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

  ss >> imageFilename >> patchRadius;

  // Output arguments
  std::cout << "imageFilename: " << imageFilename << std::endl;
  std::cout << "patchRadius: " << patchRadius << std::endl;

  typedef itk::Image<itk::CovariantVector<unsigned char, 3>, 2> ImageType;
  
  typedef itk::ImageFileReader<ImageType> ImageReaderType;
  ImageReaderType::Pointer imageReader = ImageReaderType::New();
  imageReader->SetFileName(imageFilename);
  imageReader->Update();

  ImageType* image = imageReader->GetOutput();

  //WriteExactNNField(image, patchRadius);
  //WriteTrivialNNField(image, patchRadius);
  WritePatchMatchNNField(image, patchRadius);

  return EXIT_SUCCESS;
}


template<typename TImage>
void WriteTrivialNNField(TImage* image, const unsigned int patchRadius)
{
    PatchMatchHelpers::CoordinateImageType::Pointer nnField =
         PatchMatchHelpers::CoordinateImageType::New();
    nnField->SetRegions(image->GetLargestPossibleRegion());
    nnField->Allocate();

    // Blank the NN field
    PatchMatchHelpers::CoordinateImageType::PixelType zero;
    zero.Fill(0);
    ITKHelpers::SetImageToConstant(nnField.GetPointer(), zero);

    itk::ImageRegion<2> internalRegion = ITKHelpers::GetInternalRegion(image->GetLargestPossibleRegion(), patchRadius);

    itk::ImageRegionIteratorWithIndex<TImage> imageIterator(image, internalRegion);

    while(!imageIterator.IsAtEnd())
    {
      itk::Index<2> bestMatchCenter = imageIterator.GetIndex();
      PatchMatchHelpers::CoordinateImageType::PixelType nnFieldPixel;
      nnFieldPixel[0] = bestMatchCenter[0];
      nnFieldPixel[1] = bestMatchCenter[1];
      std::cout << "NN is " << nnFieldPixel << std::endl;
      nnField->SetPixel(imageIterator.GetIndex(), nnFieldPixel);

      ++imageIterator;
    }

    ITKHelpers::WriteImage(nnField.GetPointer(), "TrivialNNField.mha");
}

template<typename TImage>
void WriteExactNNField(TImage* image, const unsigned int patchRadius)
{
    SSD<TImage>* patchDistanceFunctor = new SSD<TImage>;
    patchDistanceFunctor->SetImage(image);

    typedef SelfPatchCompare<TImage> SelfPatchCompareType;
    SelfPatchCompareType selfPatchCompare;
    selfPatchCompare.SetPatchDistanceFunctor(patchDistanceFunctor);
    selfPatchCompare.SetImage(image);
    selfPatchCompare.CreateFullyValidMask();

    PatchMatchHelpers::CoordinateImageType::Pointer nnField =
         PatchMatchHelpers::CoordinateImageType::New();
    nnField->SetRegions(image->GetLargestPossibleRegion());
    nnField->Allocate();

    // Blank the NN field
    PatchMatchHelpers::CoordinateImageType::PixelType zero;
    zero.Fill(0);
    ITKHelpers::SetImageToConstant(nnField.GetPointer(), zero);

    unsigned int numberOfCompletedPixels = 0;

    itk::ImageRegion<2> internalRegion = ITKHelpers::GetInternalRegion(image->GetLargestPossibleRegion(), patchRadius);
    itk::ImageRegionIteratorWithIndex<TImage> imageIterator(image, internalRegion);

    while(!imageIterator.IsAtEnd())
    {
      std::cout << "Computing NN for " << imageIterator.GetIndex() << std::endl;
      itk::ImageRegion<2> targetRegion =
         ITKHelpers::GetRegionInRadiusAroundPixel(imageIterator.GetIndex(), patchRadius);

      selfPatchCompare.SetTargetRegion(targetRegion);
      selfPatchCompare.ComputePatchScores();
      std::vector<typename SelfPatchCompareType::PatchDataType> patchData = selfPatchCompare.GetPatchData();
      std::partial_sort(patchData.begin(), patchData.begin() + 2, patchData.end(),
                      Helpers::SortBySecondAccending<typename SelfPatchCompareType::PatchDataType>);

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

    ITKHelpers::WriteImage(nnField.GetPointer(), "ExactNNField.mha");
}

template<typename TImage>
void WritePatchMatchNNField(TImage* image, const unsigned int patchRadius)
{
    typedef SSD<TImage> PatchDistanceFunctorType;
    PatchDistanceFunctorType* patchDistanceFunctor = new PatchDistanceFunctorType;
    patchDistanceFunctor->SetImage(image);

    typedef Propagator<PatchDistanceFunctorType> PropagatorType;
    PropagatorType* propagator = new PropagatorType;
    propagator->SetPatchDistanceFunctor(patchDistanceFunctor);

    typedef RandomSearch<TImage, PatchDistanceFunctorType> RandomSearchType;
    RandomSearchType* randomSearchFunctor = new RandomSearchType;
    randomSearchFunctor->SetPatchDistanceFunctor(patchDistanceFunctor);
    randomSearchFunctor->SetImage(image);
    randomSearchFunctor->SetPatchRadius(patchRadius);

    typedef PatchMatch<TImage, PropagatorType, RandomSearchType> PatchMatchType;
    PatchMatchType patchMatch;
    patchMatch.SetImage(image);
    patchMatch.SetPatchRadius(patchRadius);
    patchMatch.SetPropagationFunctor(propagator);
    patchMatch.SetRandomSearchFunctor(randomSearchFunctor);
    patchMatch.Compute();

    PatchMatchHelpers::WriteNNField(patchMatch.GetNNField(), "PatchMatchNNField.mha");
}
