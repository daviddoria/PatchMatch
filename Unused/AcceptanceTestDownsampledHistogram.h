
/*
template <typename TImage>
bool PatchMatch<TImage>::AddIfBetterDownsampledHistogram(const itk::Index<2>& index, const Match& potentialMatch)
{
  Match currentMatch = this->Output->GetPixel(index);
  if(potentialMatch.Score < currentMatch.Score)
  {
    const unsigned int numberOfBinsPerDimension = 20;

    itk::Index<2> smallIndex = {{static_cast<int>(index[0] * this->DownsampleFactor),
                                  static_cast<int>(index[1] * this->DownsampleFactor)}};
    unsigned int smallPatchRadius = this->PatchRadius / 2;

    itk::Index<2> queryPatchCenter = ITKHelpers::GetRegionCenter(potentialMatch.Region);
    itk::Index<2> smallQueryPatchCenter = {{static_cast<int>(queryPatchCenter[0] * this->DownsampleFactor),
                                            static_cast<int>(queryPatchCenter[1] * this->DownsampleFactor)}};

    itk::ImageRegion<2> smallQueryRegion = ITKHelpers::GetRegionInRadiusAroundPixel(smallIndex, smallPatchRadius);

    itk::ImageRegion<2> smallPotentialMatchRegion = ITKHelpers::GetRegionInRadiusAroundPixel(smallQueryPatchCenter, smallPatchRadius);

    //typedef float BinValueType;
    typedef int BinValueType;
    typedef Histogram<BinValueType>::HistogramType HistogramType;

    HistogramType queryHistogram = Histogram<BinValueType>::Compute1DConcatenatedHistogramOfMultiChannelImage(
                  this->DownsampledImage.GetPointer(), smallQueryRegion, numberOfBinsPerDimension, 0, 255);

    HistogramType potentialMatchHistogram = Histogram<BinValueType>::Compute1DConcatenatedHistogramOfMultiChannelImage(
                  this->DownsampledImage.GetPointer(), smallPotentialMatchRegion, numberOfBinsPerDimension, 0, 255);

    float potentialMatchHistogramDifference = Histogram<BinValueType>::HistogramDifference(queryHistogram, potentialMatchHistogram);


    if(potentialMatchHistogramDifference < this->HistogramAcceptanceThreshold)
    {
//       std::cout << "Match accepted. SSD " << potentialMatch.Score << " (better than " << currentMatch.Score << ", "
//                 << " Potential Match Histogram score: " << potentialMatchHistogramDifference << std::endl;
      this->Output->SetPixel(index, potentialMatch);
      return true;
    }
    else
    {
//       std::cout << "Rejected better SSD match: " << potentialMatch.Score << " (better than " << currentMatch.Score << std::endl
//                 << " Potential Match Histogram score: " << potentialMatchHistogramDifference << std::endl;
      return false;
    }

  } // end if SSD is better

  return false;
}
*/
