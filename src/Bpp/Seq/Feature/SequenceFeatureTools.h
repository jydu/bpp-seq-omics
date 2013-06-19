//
// File: SequenceFeatureTools.h
// Created by: Julien Dutheil
// Created on: Mon Jul 30 2012
//

/*
Copyright or © or Copr. Bio++ Development Team, (November 17, 2004)

This software is a computer program whose purpose is to provide classes
for sequences analysis.

This software is governed by the CeCILL  license under French law and
abiding by the rules of distribution of free software.  You can  use, 
modify and/ or redistribute the software under the terms of the CeCILL
license as circulated by CEA, CNRS and INRIA at the following URL
"http://www.cecill.info". 

As a counterpart to the access to the source code and  rights to copy,
modify and redistribute granted by the license, users are provided only
with a limited warranty  and the software's author,  the holder of the
economic rights,  and the successive licensors  have only  limited
liability. 

In this respect, the user's attention is drawn to the risks associated
with loading,  using,  modifying and/or developing or reproducing the
software by the user in light of its specific status of free software,
that may mean  that it is complicated to manipulate,  and  that  also
therefore means  that it is reserved for developers  and  experienced
professionals having in-depth computer knowledge. Users are therefore
encouraged to load and test the software's suitability as regards their
requirements in conditions enabling the security of their systems and/or 
data to be ensured and,  more generally, to use and operate it in the 
same conditions as regards security. 

The fact that you are presently reading this means that you have had
knowledge of the CeCILL license and that you accept its terms.
*/

#ifndef _SEQUENCEFEATURETOOLS_H_
#define _SEQUENCEFEATURETOOLS_H_

#include "SequenceFeature.h"

//From bpp-seq:
#include <Bpp/Seq/Sequence.h>
#include <Bpp/Seq/GeneticCode/GeneticCode.h>

namespace bpp {

class SequenceFeatureTools
{
  public:
    /**
     * @brief Extract a sub-sequence given a SeqRange.
     *
     * The sub-sequence is revese-complemented if SeqRange is in negative
     * strand.
     *
     * @param seq The Sequence to trunc.
     * @param range The SeqRange to extract.
     * @return A new Sequence object with the given subsequence oriented
     * according to the SeqRange.
     * @author Sylvain Gaillard
     */
    static Sequence* extract(const Sequence& seq, const SeqRange& range);

    /**
     * @brief Get ORF features for a Sequence.
     *
     * @param seq The Sequence where to find ORF. Must be a nucleic sequence.
     * @param featSet A SequenceFeatureSet to fill with the annotations.
     * @param gcode The genetic code to use.
     * @return The number of ORF found.
     * @author Sylvain Gaillard
     */
    static unsigned int getOrfs(const Sequence& seq, SequenceFeatureSet& featSet, const GeneticCode& gCode);

};

} //end of namespace bpp

#endif //_SEQUENCEFEATURETOOLS_H_

