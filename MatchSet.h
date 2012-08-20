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

#ifndef MatchSet_H
#define MatchSet_H

// ITK
#include "itkImageRegion.h"

// Custom
#include "Match.h"

class MatchSet
{
public:
  /** Constructor. */
  MatchSet() : MaximumMatches(1)
  {
  }

  /** Clear/delete all matches. */
  void Clear()
  {
    this->Matches.clear();
  }

  /** Get the specified match if it is in the valid range. */
  Match GetMatch(const unsigned int matchId) const
  {
    assert(matchId < this->MaximumMatches);
    return this->Matches[matchId];
  }

  /** Get a reference to the specified match if it is in the valid range. */
  Match& GetMatch(const unsigned int matchId)
  {
    assert(matchId < this->MaximumMatches);
    return this->Matches[matchId];
  }

  /** Get the number of matches in this set. */
  unsigned int GetNumberOfMatches() const
  {
    return this->Matches.size();
  }

  /** Test if any of the contained matches are verified.
    * Note that this correctly returns false if there are no matches at all.*/
  bool HasVerifiedMatch() const
  {
    for(size_t i = 0; i < this->Matches.size(); ++i)
    {
      if(this->Matches[i].IsVerified())
      {
        return true;
      }
    }
    return false;
  }

  /** Add this match to the set if it's SSD is better than the worst stored match, or if the container is not yet full. */
  void AddMatch(const Match& potentialMatch)
  {
    // Insert 'match' to Matches if it is better than any of the existing saved matches.
    // Delete (by trimming) any matches that are now not in the top MaximumMatches matches.

    // Check if a Match object of the same region is already in the container.
    // (We don't want to store the same match multipe times)
    auto result = std::find_if(this->Matches.begin(), this->Matches.end(), [&potentialMatch](const Match& match) {
          return match.GetRegion() == potentialMatch.GetRegion();});

    // If the item was found
    if(result != this->Matches.end()) // (The 'result' iterator matches the .end() if the item was not found)
    {
      // Replace the match with the new match data.
      *result = potentialMatch;
    }
    else // If the item was not found
    {
      // Add the item to the container
      this->Matches.push_back(potentialMatch);

      // Keep the top MaximumMatches patches according to SSD
      // Sort the container
      // NaN does not sort correctly, so replace NaN's with max() before sorting
      auto ssdSortFunctor = [](const Match& match1, const Match& match2)
                            {
                              float score1 = match1.GetSSDScore();
                              float score2 = match2.GetSSDScore();
                              if(Helpers::IsNaN(score1))
                              {
                                score1 = std::numeric_limits<float>::max();
                              }
                              if(Helpers::IsNaN(score2))
                              {
                                score2 = std::numeric_limits<float>::max();
                              }
                              return score1 < score2;
                            };
      std::sort(this->Matches.begin(), this->Matches.end(), ssdSortFunctor);

      // Trim the container
      if(this->Matches.size() > this->MaximumMatches)
      {
        this->Matches.resize(this->MaximumMatches);
      }

      // NaN does not sort correctly, so replace NaN's with max() before sorting
      auto verificationSortFunctor = [](const Match& match1, const Match& match2)
                          {
                          float score1 = match1.GetVerificationScore();
                          float score2 = match2.GetVerificationScore();
                          if(Helpers::IsNaN(score1))
                          {
                            score1 = std::numeric_limits<float>::max();
                          }
                          if(Helpers::IsNaN(score2))
                          {
                            score2 = std::numeric_limits<float>::max();
                          }
                          return score1 < score2;
                          };
      // stable_sort ensures that the order of identical elements is preserved. That is, we want the matches to remain
      // ordered by their SSD scores if the verification scores are max().
      std::stable_sort(this->Matches.begin(), this->Matches.end(), verificationSortFunctor);
    }
  }

  void SetMaximumMatches(const unsigned int maximumMatches)
  {
    this->MaximumMatches = maximumMatches;
  }

  unsigned int GetMaximumMatches()
  {
    return this->MaximumMatches;
  }

private:
  std::vector<Match> Matches;
  unsigned int MaximumMatches;
};

#endif
