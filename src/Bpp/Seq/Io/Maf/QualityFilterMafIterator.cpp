// SPDX-FileCopyrightText: The Bio++ Development Group
//
// SPDX-License-Identifier: CECILL-2.1

#include "QualityFilterMafIterator.h"

// From bpp-seq:
#include <Bpp/Seq/SequenceWithQuality.h>

using namespace bpp;

// From the STL:
#include <string>
#include <numeric>

using namespace std;

unique_ptr<MafBlock> QualityFilterMafIterator::analyseCurrentBlock_()
{
  if (blockBuffer_.size() == 0)
  {
    do
    {
      // Else there is no more block in the buffer, we need parse more:
      auto block = iterator_->nextBlock();
      if (!block)
        return 0; // No more block.

      // Parse block.
      vector<vector<int>> aln;
      for (size_t i = 0; i < species_.size(); ++i)
      {
        const MafSequence& seq = block->sequenceForSpecies(species_[i]);
        if (seq.hasAnnotation(SequenceQuality::QUALITY_SCORE))
        {
          aln.push_back(dynamic_cast<const SequenceQuality&>(seq.annotation(SequenceQuality::QUALITY_SCORE)).getScores());
        }
      }
      if (aln.size() != species_.size())
      {
        blockBuffer_.push_back(move(block));
        if (logstream_)
        {
          (*logstream_ << "QUAL CLEANER: block is missing quality score for at least one species and will therefore not be filtered.").endLine();
        }
        // NB here we could decide to discard the block instead!
      }
      else
      {
        size_t nr = aln.size();
        size_t nc = block->getNumberOfSites();
        // First we create a mask:
        vector<size_t> pos;
        vector<int> col(nr);
        // Reset window:
        window_.clear();
        // Init window:
        size_t i;
        for (i = 0; i < windowSize_; ++i)
        {
          for (size_t j = 0; j < nr; ++j)
          {
            col[j] = aln[j][i];
          }
          window_.push_back(col);
        }
        // Slide window:
        if (verbose_)
        {
          ApplicationTools::message->endLine();
          ApplicationTools::displayTask("Sliding window for quality filter", true);
        }
        while (i + step_ < nc)
        {
          if (verbose_)
            ApplicationTools::displayGauge(i - windowSize_, nc - windowSize_ - 1, '>');
          // Evaluate current window:
          double mean = 0;
          double n = static_cast<double>(aln.size() * windowSize_);
          for (size_t u = 0; u < window_.size(); ++u)
          {
            for (size_t v = 0; v < window_[u].size(); ++v)
            {
              mean += (window_[u][v] > 0 ? static_cast<double>(window_[u][v]) : 0.);
              if (window_[u][v] == -1)
                n--;
            }
          }
          if (n > 0 && (mean / n) < minQual_)
          {
            if (pos.size() == 0)
            {
              pos.push_back(i - windowSize_);
              pos.push_back(i);
            }
            else
            {
              if (i - windowSize_ <= pos[pos.size() - 1])
              {
                pos[pos.size() - 1] = i; // Windows are overlapping and we extend previous region
              }
              else // This is a new region
              {
                pos.push_back(i - windowSize_);
                pos.push_back(i);
              }
            }
          }

          // Move forward:
          for (size_t k = 0; k < step_; ++k)
          {
            for (size_t j = 0; j < nr; ++j)
            {
              col[j] = aln[j][i];
            }
            window_.push_back(col);
            window_.pop_front();
            ++i;
          }
        }

        // Evaluate last window:
        double mean = 0;
        double n = static_cast<double>(aln.size() * windowSize_);
        for (size_t u = 0; u < window_.size(); ++u)
        {
          for (size_t v = 0; v < window_[u].size(); ++v)
          {
            mean += (window_[u][v] > 0 ? static_cast<double>(window_[u][v]) : 0.);
            if (window_[u][v] == -1)
              n--;
          }
        }
        if (n > 0 && (mean / n) < minQual_)
        {
          if (pos.size() == 0)
          {
            pos.push_back(i - windowSize_);
            pos.push_back(i);
          }
          else
          {
            if (i - windowSize_ < pos[pos.size() - 1])
            {
              pos[pos.size() - 1] = i; // Windows are overlapping and we extend previous region
            }
            else // This is a new region
            {
              pos.push_back(i - windowSize_);
              pos.push_back(i);
            }
          }
        }
        if (verbose_)
          ApplicationTools::displayTaskDone();

        // Now we remove regions with two many gaps, using a sliding window:
        if (pos.size() == 0)
        {
          blockBuffer_.push_back(move(block));
          if (logstream_)
          {
            (*logstream_ << "QUAL CLEANER: block is clean and kept as is.").endLine();
          }
        }
        else if (pos.size() == 2 && pos.front() == 0 && pos.back() == block->getNumberOfSites())
        {
          // Everything is removed:
          if (logstream_)
          {
            (*logstream_ << "QUAL CLEANER: block was entirely removed. Tried to get the next one.").endLine();
          }
        }
        else
        {
          if (logstream_)
          {
            (*logstream_ << "QUAL CLEANER: block with size " << block->getNumberOfSites() << " will be split into " << (pos.size() / 2 + 1) << " blocks.").endLine();
          }
          if (verbose_)
          {
            ApplicationTools::message->endLine();
            ApplicationTools::displayTask("Spliting block", true);
          }
          for (i = 0; i < pos.size(); i += 2)
          {
            if (verbose_)
              ApplicationTools::displayGauge(i, pos.size() - 2, '=');
            if (logstream_)
            {
              (*logstream_ << "QUAL CLEANER: removing region (" << pos[i] << ", " << pos[i + 1] << ") from block.").endLine();
            }
            if (pos[i] > 0)
            {
              auto newBlock = make_unique<MafBlock>();
              newBlock->setScore(block->getScore());
              newBlock->setPass(block->getPass());
              for (size_t j = 0; j < block->getNumberOfSequences(); ++j)
              {
                unique_ptr<MafSequence> subseq;
                if (i == 0)
                {
                  subseq = block->sequence(j).subSequence(0, pos[i]);
                }
                else
                {
                  subseq = block->sequence(j).subSequence(pos[i - 1], pos[i] - pos[i - 1]);
                }
                newBlock->addSequence(subseq);
              }
              blockBuffer_.push_back(move(newBlock));
            }

            if (keepTrashedBlocks_)
            {
              auto outBlock = make_unique<MafBlock>();
              outBlock->setScore(block->getScore());
              outBlock->setPass(block->getPass());
              for (size_t j = 0; j < block->getNumberOfSequences(); ++j)
              {
                auto outseq = block->sequence(j).subSequence(pos[i], pos[i + 1] - pos[i]);
                outBlock->addSequence(outseq);
              }
              trashBuffer_.push_back(move(outBlock));
            }
          }
          // Add last block:
          if (pos[pos.size() - 1] < block->getNumberOfSites())
          {
            auto newBlock = make_unique<MafBlock>();
            newBlock->setScore(block->getScore());
            newBlock->setPass(block->getPass());
            for (size_t j = 0; j < block->getNumberOfSequences(); ++j)
            {
              auto subseq = block->sequence(j).subSequence(pos[pos.size() - 1], block->getNumberOfSites() - pos[pos.size() - 1]);
              newBlock->addSequence(subseq);
            }
            blockBuffer_.push_back(move(newBlock));
          }
          if (verbose_)
            ApplicationTools::displayTaskDone();
        }
      }
    }
    while (blockBuffer_.size() == 0);
  }

  auto block = move(blockBuffer_.front());
  blockBuffer_.pop_front();
  return block;
}
