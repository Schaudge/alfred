/*
============================================================================
Alfred: BAM alignment statistics
============================================================================
Copyright (C) 2017-2018 Tobias Rausch

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
============================================================================
Contact: Tobias Rausch (rausch@embl.de)
============================================================================
*/

#ifndef BAMSTATS_H
#define BAMSTATS_H

#include <limits>

#include <boost/dynamic_bitset.hpp>
#include <boost/unordered_map.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/progress.hpp>
#include <boost/iostreams/device/file.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/zlib.hpp>
#include <boost/iostreams/filter/gzip.hpp>

#include <htslib/sam.h>
#include <htslib/faidx.h>

#include "tenX.h"
#include "util.h"

namespace bamstats
{

  struct ChrGC {
    uint32_t ncount;
    uint32_t gccount;
  };
  
  struct BaseCounts {
    typedef uint32_t TCountType;
    typedef std::vector<TCountType> TCoverageBp;
    
    typedef uint16_t TMaxCoverage;
    typedef std::vector<TMaxCoverage> TBpCoverage;

    uint32_t maxCoverage;
    uint32_t maxIndelSize;
    uint64_t n1;
    uint64_t n2;
    uint64_t nd;
    uint64_t matchCount;
    uint64_t mismatchCount;
    uint64_t delCount;
    uint64_t insCount;
    uint64_t softClipCount;
    uint64_t hardClipCount;
    std::vector<uint32_t> delHomACGTN;  // A:0, C:1, G:2, T:3, N:4, none:5
    std::vector<uint32_t> insHomACGTN;  // A:0, C:1, G:2, T:3, N:4, none:5
    std::vector<uint32_t> delSize;
    std::vector<uint32_t> insSize;
    TCoverageBp bpWithCoverage;
    TBpCoverage cov;

    BaseCounts() : maxCoverage(std::numeric_limits<TMaxCoverage>::max()), maxIndelSize(50), n1(0), n2(0), nd(0), matchCount(0), mismatchCount(0), delCount(0), insCount(0), softClipCount(0), hardClipCount(0) {
      delHomACGTN.resize(6, 0);
      insHomACGTN.resize(6, 0);
      bpWithCoverage.resize(maxCoverage + 1, 0);
      delSize.resize(maxIndelSize + 1, 0);
      insSize.resize(maxIndelSize + 1, 0);
      cov.clear();
    }
  };

  struct ReadCounts {
    typedef uint16_t TMaxReadLength;
    typedef uint32_t TCountType;
    typedef std::vector<TCountType> TLengthReadCount;
    typedef std::vector<uint64_t> TBaseQualitySum;
    typedef std::vector<uint64_t> TGCContent;
    typedef boost::dynamic_bitset<> TBitSet;
    typedef std::pair<int32_t, int32_t> TStartEndPair;
    typedef std::map<int32_t, TStartEndPair> TBlockRange;
    typedef std::vector<TBlockRange> TGenomicBlockRange;
    typedef std::vector<uint64_t> TMappedChr;
    
    int32_t maxReadLength;
    int32_t maxUMI;
    int64_t secondary;
    int64_t qcfail;
    int64_t dup;
    int64_t supplementary;
    int64_t unmap;
    int64_t forward;
    int64_t reverse;
    int64_t spliced;
    int64_t mapped1;
    int64_t mapped2;
    int64_t haplotagged;
    int64_t mitagged;
    TMappedChr mappedchr;
    TLengthReadCount lRc;
    TLengthReadCount nCount;
    TLengthReadCount aCount;
    TLengthReadCount cCount;
    TLengthReadCount gCount;
    TLengthReadCount tCount;
    TBaseQualitySum bqCount;
    TGCContent gcContent;
    TBitSet umi;
    TGenomicBlockRange brange;

    ReadCounts(uint32_t const n_targets) : maxReadLength(std::numeric_limits<TMaxReadLength>::max()), maxUMI(10000000), secondary(0), qcfail(0), dup(0), supplementary(0), unmap(0), forward(0), reverse(0), spliced(0), mapped1(0), mapped2(0), haplotagged(0), mitagged(0) {
      mappedchr.resize(n_targets, 0);
      lRc.resize(maxReadLength + 1, 0);
      aCount.resize(maxReadLength + 1, 0);
      cCount.resize(maxReadLength + 1, 0);
      gCount.resize(maxReadLength + 1, 0);
      tCount.resize(maxReadLength + 1, 0);
      nCount.resize(maxReadLength + 1, 0);
      bqCount.resize(maxReadLength + 1, 0);
      gcContent.resize(101, 0);
    }
  };

  struct PairCounts {
    typedef uint16_t TMaxInsertSize;
    typedef uint32_t TCountType;
    typedef std::vector<TCountType> TISizePairCount;
    int32_t maxInsertSize;
    int64_t paired;
    int64_t mapped;
    int64_t mappedSameChr;
    int64_t mappedProper;
    int64_t orient[4];
    int64_t totalISizeCount;
    TISizePairCount fPlus;
    TISizePairCount rPlus;
    TISizePairCount fMinus;
    TISizePairCount rMinus;
    
    
    PairCounts() : maxInsertSize(std::numeric_limits<TMaxInsertSize>::max()), paired(0), mapped(0), mappedSameChr(0), mappedProper(0), totalISizeCount(0) {
      orient[0] = 0;
      orient[1] = 0;
      orient[2] = 0;
      orient[3] = 0;
      fPlus.resize(maxInsertSize + 1, 0);
      rPlus.resize(maxInsertSize + 1, 0);
      fMinus.resize(maxInsertSize + 1, 0);
      rMinus.resize(maxInsertSize + 1, 0);
    }
  };

  
  struct QualCounts {
    typedef uint8_t TMaxQuality;
    typedef uint32_t TCountType;
    typedef std::vector<TCountType> TQualCount;
    int32_t maxQuality;
    TQualCount qcount;

    QualCounts() : maxQuality(std::numeric_limits<TMaxQuality>::max()) {
      qcount.resize(maxQuality + 1, 0);
    }
  };
    
  
  struct ReadGroupStats {
    BaseCounts bc;
    ReadCounts rc;
    PairCounts pc;
    QualCounts qc;
    
  ReadGroupStats(uint32_t const n_targets) : bc(BaseCounts()), rc(ReadCounts(n_targets)), pc(PairCounts()), qc(QualCounts()) {}
  };


  struct BedCounts {
    typedef double TAvgCov;
    typedef std::vector<TAvgCov> TBpCov;
    typedef boost::unordered_map<std::string, TBpCov> TRgBpMap;
    typedef std::vector<TRgBpMap> TGenomicBp;

    typedef std::vector<int64_t> TOnTargetBp;
    typedef boost::unordered_map<std::string, TOnTargetBp> TOnTargetMap;
    
    int32_t stepsize;
    int32_t onTSize;
    TGenomicBp gCov;
    TOnTargetMap onTarget;
    
    BedCounts(int32_t nchr, int32_t s, int32_t vs) : stepsize(s), onTSize(vs) {
      gCov.resize(nchr, TRgBpMap());
    }
  };
  

  template<typename TChromosomeRegions, typename TBpCoverage, typename TBedCounts>
  inline void
  _summarizeBedCoverage(TChromosomeRegions const& chrRegions, TBpCoverage const& cov, int32_t refIndex, std::string const& rg, TBedCounts& be) {
    typename BedCounts::TRgBpMap::iterator itChr = be.gCov[refIndex].find(rg);
    typename BedCounts::TOnTargetMap::iterator itOT = be.onTarget.find(rg);
    for(int32_t s = 0; s < (int32_t) itOT->second.size(); ++s) {
      // Avoid over-counting
      typedef boost::dynamic_bitset<> TBitSet;
      TBitSet used(cov.size());
      for(uint32_t i = 0; i<chrRegions.size(); ++i) {
	int64_t avgCov = 0;
	int32_t rStart = std::max(0, chrRegions[i].start - s * be.stepsize);
	int32_t rEnd = std::min((int32_t) cov.size(), chrRegions[i].end + s * be.stepsize);
	for(int32_t k = rStart; k < rEnd; ++k) {
	  if (!used[k]) {
	    avgCov += cov[k];
	    used[k] = 1;
	  }
	}
	itOT->second[s] += avgCov;
	if (s == 0) {
	  if (chrRegions[i].start < chrRegions[i].end) itChr->second[i] = (typename BedCounts::TAvgCov) ( (double) avgCov / (double) (chrRegions[i].end - chrRegions[i].start));
	  else itChr->second[i] = (typename BedCounts::TAvgCov) (0);
	}
      }
    }
  }

  template<typename TConfig>
  inline int32_t
  bamStatsRun(TConfig& c) {
    // Load bam file
    samFile* samfile = sam_open(c.bamFile.string().c_str(), "r");
    hts_set_fai_filename(samfile, c.genome.string().c_str());
    bam_hdr_t* hdr = sam_hdr_read(samfile);

    // One list of regions for every chromosome
    typedef std::vector<Interval> TChromosomeRegions;
    typedef std::vector<TChromosomeRegions> TGenomicRegions;
    TGenomicRegions gRegions;
    gRegions.resize(hdr->n_targets, TChromosomeRegions());

    // Parse regions from BED file or create one region per chromosome
    int32_t totalBedSize = 0;
    if (c.hasRegionFile) {
      if (is_gz(c.regionFile)) {
	std::ifstream file(c.regionFile.string().c_str(), std::ios_base::in | std::ios_base::binary);
	boost::iostreams::filtering_streambuf<boost::iostreams::input> dataIn;
	dataIn.push(boost::iostreams::gzip_decompressor());
	dataIn.push(file);
	std::istream instream(&dataIn);
	std::string line;
	while(std::getline(instream, line)) {
	  typedef boost::tokenizer< boost::char_separator<char> > Tokenizer;
	  boost::char_separator<char> sep(" \t,;");
	  Tokenizer tokens(line, sep);
	  Tokenizer::iterator tokIter = tokens.begin();
	  std::string chrName = *tokIter++;
	  // Map chromosome names to the bam header chromosome IDs
	  int32_t chrid = bam_name2id(hdr, chrName.c_str());
	  // Valid ID?
	  if (chrid >= 0) {
	    if (tokIter!=tokens.end()) {
	      int32_t start = boost::lexical_cast<int32_t>(*tokIter++);
	      int32_t end = boost::lexical_cast<int32_t>(*tokIter++);
	      gRegions[chrid].push_back(Interval(start, end));
	    }
	  }
	}
	dataIn.pop();
      } else {
	// Parse regions from BED file
	std::ifstream bedFile(c.regionFile.string().c_str(), std::ifstream::in);
	if (bedFile.is_open()) {
	  while (bedFile.good()) {
	    std::string line;
	    getline(bedFile, line);
	    typedef boost::tokenizer< boost::char_separator<char> > Tokenizer;
	    boost::char_separator<char> sep(" \t,;");
	    Tokenizer tokens(line, sep);
	    Tokenizer::iterator tokIter = tokens.begin();
	    std::string chrName = *tokIter++;
	    // Map chromosome names to the bam header chromosome IDs
	    int32_t chrid = bam_name2id(hdr, chrName.c_str());
	    // Valid ID?
	    if (chrid >= 0) {
	      if (tokIter!=tokens.end()) {
		int32_t start = boost::lexical_cast<int32_t>(*tokIter++);
		int32_t end = boost::lexical_cast<int32_t>(*tokIter++);
		gRegions[chrid].push_back(Interval(start, end));
	      }
	    }
	  }
	  bedFile.close();
	}
      }

      // Get total bed size
      for(int32_t refIndex = 0; refIndex < hdr->n_targets; ++refIndex) {
	typedef boost::dynamic_bitset<> TBitSet;
	TBitSet bedcovered(hdr->target_len[refIndex]);
	for(uint32_t i = 0; i < gRegions[refIndex].size(); ++i)
	  for(int32_t k = gRegions[refIndex][i].start; (k < gRegions[refIndex][i].end) && (k < (int32_t) hdr->target_len[refIndex]); ++k) bedcovered[k] = 1;
	totalBedSize += bedcovered.count();
      }
    }
    
    // Debug code
    //uint32_t rIndex = 0;
    //for(TGenomicRegions::const_iterator itG = gRegions.begin(); itG != gRegions.end(); ++itG, ++rIndex) {
    //for(TChromosomeRegions::const_iterator itC = itG->begin(); itC != itG->end(); ++itC) {
    //std::cout << rIndex << ',' << hdr->target_name[rIndex] << ',' << itC->start << ',' << itC->end << std::endl;
    //}
    //}

    // BED file statistics
    BedCounts be(hdr->n_targets, 25, 20);
    
    // Read group statistics
    typedef std::set<std::string> TRgSet;
    TRgSet rgs;
    getRGs(std::string(hdr->text), rgs);
    if (c.ignoreRG) rgs.insert("DefaultLib");
    if ((c.singleRG) && (rgs.find(c.rgname) == rgs.end())) {
	std::cerr << "Read group is not present in BAM file: " << c.rgname << std::endl;
	return 1;
    }
    typedef boost::unordered_map<std::string, ReadGroupStats> TRGMap;
    TRGMap rgMap;
    for(typename TRgSet::const_iterator itRg = rgs.begin(); itRg != rgs.end(); ++itRg) {
      if (((c.ignoreRG) && (*itRg == "DefaultLib")) || ((c.singleRG) && (*itRg == c.rgname)) || ((!c.ignoreRG) && (!c.singleRG))) {
	rgMap.insert(std::make_pair(*itRg, ReadGroupStats(hdr->n_targets)));
	for(int32_t refIndex = 0; refIndex < hdr->n_targets; ++refIndex) {
	  typename BedCounts::TRgBpMap::iterator itChr = be.gCov[refIndex].insert(std::make_pair(*itRg, typename BedCounts::TBpCov())).first;
	  itChr->second.resize(gRegions[refIndex].size());
	  typename BedCounts::TOnTargetMap::iterator itOT = be.onTarget.insert(std::make_pair(*itRg, typename BedCounts::TOnTargetBp())).first;
	  itOT->second.resize(be.onTSize, 0);
	}
      }
    }

    // Parse reference and BAM file
    boost::posix_time::ptime now = boost::posix_time::second_clock::local_time();
    std::cout << '[' << boost::posix_time::to_simple_string(now) << "] " << "BAM file parsing" << std::endl;
    boost::progress_display show_progress( hdr->n_targets );

    // GC- and N-content
    typedef boost::dynamic_bitset<> TBitSet;
    TBitSet nrun;
    TBitSet gcref;
    uint64_t referencebp = 0;
    uint64_t ncount = 0;
    int32_t gcRunnerIdx = 0;
    int32_t gcRunnerCount = 0;
    std::vector<ChrGC> chrGC(hdr->n_targets, ChrGC());
    ReadCounts::TGCContent refGcContent(101, 0);

    // Find N90 chromosome length
    uint32_t minChrLen = 0;
    {
      std::vector<uint32_t> chrlen(hdr->n_targets, 0);
      uint64_t genomelen = 0;
      for(int32_t refIndex = 0; refIndex < hdr->n_targets; ++refIndex) {
	chrlen[refIndex] = hdr->target_len[refIndex];
	genomelen += hdr->target_len[refIndex];
      }
      std::sort(chrlen.begin(), chrlen.end(), std::greater<uint32_t>());
      uint64_t cumsum = 0;
      for(uint32_t i = 0; i < chrlen.size(); ++i) {
	cumsum += chrlen[i];
	if (cumsum > genomelen * 0.9) {
	  minChrLen = chrlen[i];
	  break;
	}
      }
    }
    
    // Parse genome
    int32_t refIndex = -1;
    char* seq = NULL;
    faidx_t* fai = fai_load(c.genome.string().c_str());
    bam1_t* rec = bam_init1();
    while (sam_read1(samfile, hdr, rec) >= 0) {
      // New chromosome?
      if ((!(rec->core.flag & BAM_FUNMAP)) && (rec->core.tid != refIndex)) {
	++show_progress;
	
	// Summarize bp-level coverage
	if (refIndex != -1) {
	  for(typename TRGMap::iterator itRg = rgMap.begin(); itRg != rgMap.end(); ++itRg) {
	    if ((c.hasRegionFile) && (!gRegions[refIndex].empty())) _summarizeBedCoverage(gRegions[refIndex], itRg->second.bc.cov, refIndex, itRg->first, be);
	    for(uint32_t i = 0; i < hdr->target_len[refIndex]; ++i) {
	      if (itRg->second.bc.cov[i] >= 1) {
		++itRg->second.bc.nd;
		if (itRg->second.bc.cov[i] == 1) ++itRg->second.bc.n1;
		if (itRg->second.bc.cov[i] == 2) ++itRg->second.bc.n2;
	      }
	      if (!nrun[i]) ++itRg->second.bc.bpWithCoverage[itRg->second.bc.cov[i]];
	    }
	    itRg->second.bc.cov.clear();
	  }
	  if (seq != NULL) free(seq);
	}
	refIndex = rec->core.tid;
	
	// Load chromosome
	int32_t seqlen = -1;
	std::string tname(hdr->target_name[refIndex]);
	seq = faidx_fetch_seq(fai, tname.c_str(), 0, hdr->target_len[refIndex], &seqlen);

	// Set N-mask
	nrun.clear();
	nrun.resize(hdr->target_len[refIndex], false);
	gcref.clear();
	gcref.resize(hdr->target_len[refIndex], false);
	referencebp += hdr->target_len[refIndex];
	for(uint32_t i = 0; i < hdr->target_len[refIndex]; ++i) {
	  if ((seq[i] == 'c') || (seq[i] == 'C') || (seq[i] == 'g') || (seq[i] == 'G')) gcref[i] = 1;
	  if ((seq[i] == 'n') || (seq[i] == 'N')) {
	    nrun[i] = 1;
	    ++ncount;
	  }
	}
	// Reference GC
	chrGC[refIndex].ncount = nrun.count();
	chrGC[refIndex].gccount = gcref.count();
	if ((hdr->target_len[refIndex] > 100) && (hdr->target_len[refIndex] >= minChrLen)) {
	  uint32_t nsum = 0;
	  uint32_t gcsum = 0;
	  for(uint32_t pos = 0; pos < hdr->target_len[refIndex] - 100; ++pos) {
	    if (pos == 0) {
	      for(uint32_t i = 0; i < 100; ++i) {
		nsum += nrun[i];
		gcsum += gcref[i];
	      }
	    } else {
	      nsum -= nrun[pos - 1];
	      gcsum -= gcref[pos - 1];
	      nsum += nrun[pos + 99];
	      gcsum += gcref[pos + 99];
	    }
	    if (!nsum) ++refGcContent[gcsum];
	  }
	}
	
	// Resize coverage vectors
	for(typename TRGMap::iterator itRg = rgMap.begin(); itRg != rgMap.end(); ++itRg) itRg->second.bc.cov.resize(hdr->target_len[refIndex], 0);
      }
      
      // Get the library information
      std::string rG = "DefaultLib";
      if (!c.ignoreRG) {
	uint8_t *rgptr = bam_aux_get(rec, "RG");
	if (rgptr) {
	  char* rg = (char*) (rgptr + 1);
	  rG = std::string(rg);
	}
	if ((c.singleRG) && (rG != c.rgname)) continue;
      }
      typename TRGMap::iterator itRg = rgMap.find(rG);
      if (itRg == rgMap.end()) {
	std::cerr << "Missing read group: " << rG << std::endl;
	return 1;
      }

      // Alignments behind the reference end
      if ((!(rec->core.flag & BAM_FUNMAP)) && (((rec->core.pos >= (int32_t) hdr->target_len[refIndex]) || (lastAlignedPosition(rec) > hdr->target_len[refIndex])))) {
	std::cerr << "Alignment is past the reference end: " << hdr->target_name[refIndex] << ':' << rec->core.pos << std::endl;
	continue;
      }

      // Paired counts
      if (rec->core.flag & BAM_FPAIRED) {
	++itRg->second.pc.paired;
	if (!((rec->core.flag & BAM_FUNMAP) || (rec->core.flag & BAM_FMUNMAP))) {
	  ++itRg->second.pc.mapped;
	  if (rec->core.tid == rec->core.mtid) {
	    ++itRg->second.pc.mappedSameChr;
	    if (rec->core.flag & BAM_FPROPER_PAIR) ++itRg->second.pc.mappedProper;
	  }
	  if (rec->core.pos > rec->core.mpos) {
	    ++itRg->second.pc.totalISizeCount;
	    int32_t outerISize = rec->core.pos - rec->core.mpos + alignmentLength(rec);
	    switch(layout(rec)) {
	    case 0:
	      ++itRg->second.pc.orient[0];
	      if (outerISize < itRg->second.pc.maxInsertSize) ++itRg->second.pc.fPlus[outerISize];
	      else ++itRg->second.pc.fPlus[itRg->second.pc.maxInsertSize];
	      break;
	    case 1:
	      if (outerISize < itRg->second.pc.maxInsertSize) ++itRg->second.pc.fMinus[outerISize];
	      else ++itRg->second.pc.fMinus[itRg->second.pc.maxInsertSize];
	      break;
	    case 2:
	      ++itRg->second.pc.orient[2];
	      if (outerISize < itRg->second.pc.maxInsertSize) ++itRg->second.pc.rPlus[outerISize];
	      else ++itRg->second.pc.rPlus[itRg->second.pc.maxInsertSize];
	      break;
	    case 3:
	      ++itRg->second.pc.orient[3];
	      if (outerISize < itRg->second.pc.maxInsertSize) ++itRg->second.pc.rMinus[outerISize];
	      else ++itRg->second.pc.rMinus[itRg->second.pc.maxInsertSize];
	      break;
	    default:
	      break;
	    }
	  }
	}
      }

      // Read counts
      if (rec->core.flag & (BAM_FSECONDARY | BAM_FQCFAIL | BAM_FDUP | BAM_FSUPPLEMENTARY | BAM_FUNMAP)) {
	if (rec->core.flag & BAM_FSECONDARY) ++itRg->second.rc.secondary;
	if (rec->core.flag & BAM_FQCFAIL) ++itRg->second.rc.qcfail;
	if (rec->core.flag & BAM_FDUP) ++itRg->second.rc.dup;
	if (rec->core.flag & BAM_FSUPPLEMENTARY) ++itRg->second.rc.supplementary;
	if (rec->core.flag & BAM_FUNMAP) {
	  ++itRg->second.rc.unmap;
	} else {
	  ++itRg->second.rc.mappedchr[refIndex];
	}
	continue;
      }
      ++itRg->second.qc.qcount[(int32_t) rec->core.qual];
      ++itRg->second.rc.mappedchr[refIndex];
      if (rec->core.flag & BAM_FREAD2) ++itRg->second.rc.mapped2;
      else ++itRg->second.rc.mapped1;
      if (rec->core.flag & BAM_FREVERSE) ++itRg->second.rc.reverse;
      else ++itRg->second.rc.forward;
      if (rec->core.l_qseq < itRg->second.rc.maxReadLength) ++itRg->second.rc.lRc[rec->core.l_qseq];
      else ++itRg->second.rc.lRc[itRg->second.rc.maxReadLength];

      // Fetch molecule identifier
      uint8_t* miptr = bam_aux_get(rec, "MI");
      if (miptr) {
	c.isMitagged = true;
	++itRg->second.rc.mitagged;
	int32_t mitag = bam_aux2i(miptr);
	if ((mitag>=0) && (mitag < itRg->second.rc.maxUMI)) {
	  // Lazy resize
	  if (itRg->second.rc.umi.empty()) itRg->second.rc.umi.resize(itRg->second.rc.maxUMI, false);
	  itRg->second.rc.umi[mitag] = true;
	}
      }
      
      // Fetch haplotype tag
      uint8_t* hpptr = bam_aux_get(rec, "HP");
      if (hpptr) {
	c.isHaplotagged = true;
	++itRg->second.rc.haplotagged;

	// If no phased block assume all in one phased block
	int32_t psId = 0;
	uint8_t* psptr = bam_aux_get(rec, "PS");
	if (psptr) psId = bam_aux2i(psptr);
	if ((int32_t) itRg->second.rc.brange.size() <= refIndex) itRg->second.rc.brange.resize(refIndex + 1, ReadCounts::TBlockRange());
	if (itRg->second.rc.brange[refIndex].find(psId) == itRg->second.rc.brange[refIndex].end()) {
	  itRg->second.rc.brange[refIndex].insert(std::make_pair(psId, std::make_pair(rec->core.pos, lastAlignedPosition(rec))));
	} else {
	  itRg->second.rc.brange[refIndex][psId].first = std::min(rec->core.pos, itRg->second.rc.brange[refIndex][psId].first);
	  itRg->second.rc.brange[refIndex][psId].second = std::max((int32_t) lastAlignedPosition(rec), itRg->second.rc.brange[refIndex][psId].second);
	}
      }
      
      // Get the read sequence
      typedef std::vector<uint8_t> TQuality;
      TQuality quality;
      quality.resize(rec->core.l_qseq);
      std::string sequence;
      sequence.resize(rec->core.l_qseq);
      uint8_t* seqptr = bam_get_seq(rec);
      uint8_t* qualptr = bam_get_qual(rec);
      for (int32_t i = 0; i < rec->core.l_qseq; ++i) {
	quality[i] = qualptr[i];
	sequence[i] = "=ACMGRSVTWYHKDBN"[bam_seqi(seqptr, i)];
	//char c = 33 + quality[i];
	int32_t relpos = i;
	if (rec->core.flag & BAM_FREVERSE) {
	  relpos = rec->core.l_qseq - i - 1;
	  if (relpos < itRg->second.rc.maxReadLength) {
	    itRg->second.rc.bqCount[relpos] += (uint64_t) quality[i];
	    if ((sequence[i] == 'N') || (sequence[i] == 'n')) ++itRg->second.rc.nCount[relpos];
	    else if ((sequence[i] == 'A') || (sequence[i] == 'a')) ++itRg->second.rc.tCount[relpos];
	    else if ((sequence[i] == 'C') || (sequence[i] == 'c')) ++itRg->second.rc.gCount[relpos];
	    else if ((sequence[i] == 'G') || (sequence[i] == 'g')) ++itRg->second.rc.cCount[relpos];
	    else if ((sequence[i] == 'T') || (sequence[i] == 't')) ++itRg->second.rc.aCount[relpos];
	  } else {
	    itRg->second.rc.bqCount[itRg->second.rc.maxReadLength] += (uint64_t) quality[i];
	    if ((sequence[i] == 'N') || (sequence[i] == 'n')) ++itRg->second.rc.nCount[itRg->second.rc.maxReadLength];
	    else if ((sequence[i] == 'A') || (sequence[i] == 'a')) ++itRg->second.rc.tCount[itRg->second.rc.maxReadLength];
	    else if ((sequence[i] == 'C') || (sequence[i] == 'c')) ++itRg->second.rc.gCount[itRg->second.rc.maxReadLength];
	    else if ((sequence[i] == 'G') || (sequence[i] == 'g')) ++itRg->second.rc.cCount[itRg->second.rc.maxReadLength];
	    else if ((sequence[i] == 'T') || (sequence[i] == 't')) ++itRg->second.rc.aCount[itRg->second.rc.maxReadLength];
	  }
	} else {
	  if (relpos < itRg->second.rc.maxReadLength) {
	    itRg->second.rc.bqCount[relpos] += (uint64_t) quality[i];
	    if ((sequence[i] == 'N') || (sequence[i] == 'n')) ++itRg->second.rc.nCount[relpos];
	    else if ((sequence[i] == 'A') || (sequence[i] == 'a')) ++itRg->second.rc.aCount[relpos];
	    else if ((sequence[i] == 'C') || (sequence[i] == 'c')) ++itRg->second.rc.cCount[relpos];
	    else if ((sequence[i] == 'G') || (sequence[i] == 'g')) ++itRg->second.rc.gCount[relpos];
	    else if ((sequence[i] == 'T') || (sequence[i] == 't')) ++itRg->second.rc.tCount[relpos];
	  } else {
	    itRg->second.rc.bqCount[itRg->second.rc.maxReadLength] += (uint64_t) quality[i];
	    if ((sequence[i] == 'N') || (sequence[i] == 'n')) ++itRg->second.rc.nCount[itRg->second.rc.maxReadLength];
	    else if ((sequence[i] == 'A') || (sequence[i] == 'a')) ++itRg->second.rc.aCount[itRg->second.rc.maxReadLength];
	    else if ((sequence[i] == 'C') || (sequence[i] == 'c')) ++itRg->second.rc.cCount[itRg->second.rc.maxReadLength];
	    else if ((sequence[i] == 'G') || (sequence[i] == 'g')) ++itRg->second.rc.gCount[itRg->second.rc.maxReadLength];
	    else if ((sequence[i] == 'T') || (sequence[i] == 't')) ++itRg->second.rc.tCount[itRg->second.rc.maxReadLength];
	  }
	}
      }

      // Sequence GC content
      if (sequence.size() > 100) {
	uint32_t offset = (sequence.size() - 100) / 2;
	int32_t gccont = 0;
	for(uint32_t i = offset; i < (offset + 100); ++i) {
	  if ((sequence[i] == 'c') || (sequence[i] == 'C') || (sequence[i] == 'g') || (sequence[i] == 'G')) ++gccont;
	}
	++itRg->second.rc.gcContent[gccont];
      } else {
	// Most likely some tag-counting application, ignore the first 20bp
	if (!(rec->core.flag & BAM_FREVERSE)) {
	  for(uint32_t i = 20; i < sequence.size(); ++i, ++gcRunnerIdx) {
	    if ((sequence[i] == 'c') || (sequence[i] == 'C') || (sequence[i] == 'g') || (sequence[i] == 'G')) ++gcRunnerCount;
	    if (gcRunnerIdx == 100) {
	      ++itRg->second.rc.gcContent[gcRunnerCount];
	      gcRunnerIdx = 0;
	      gcRunnerCount = 0;
	    }
	  }
	}
      }
    
      // Get the reference slice
      std::string refslice = boost::to_upper_copy(std::string(seq + rec->core.pos, seq + lastAlignedPosition(rec)));
      
      // Debug 
      //std::cout << matchCount << ',' << mismatchCount << ',' << delCount << ',' << insCount << ',' << softClipCount << ',' << hardClipCount << std::endl;
      //std::cout << refslice << std::endl;
      //std::cout << sequence << std::endl;

      uint32_t rp = 0; // reference pointer
      uint32_t sp = 0; // sequence pointer
      
      // Parse the CIGAR
      uint32_t* cigar = bam_get_cigar(rec);
      bool spliced = false;
      for (std::size_t i = 0; i < rec->core.n_cigar; ++i) {
	if ((bam_cigar_op(cigar[i]) == BAM_CMATCH) || (bam_cigar_op(cigar[i]) == BAM_CEQUAL) || (bam_cigar_op(cigar[i]) == BAM_CDIFF)) {
	  // match or mismatch
	  for(std::size_t k = 0; k<bam_cigar_oplen(cigar[i]);++k) {
	    if (sequence[sp] == refslice[rp]) ++itRg->second.bc.matchCount;
	    else ++itRg->second.bc.mismatchCount;
	    // Count bp-level coverage
	    if (itRg->second.bc.cov[rec->core.pos + rp] < itRg->second.bc.maxCoverage) ++itRg->second.bc.cov[rec->core.pos + rp];
	    ++sp;
	    ++rp;
	  }
	} else if (bam_cigar_op(cigar[i]) == BAM_CDEL) {
	  ++itRg->second.bc.delCount;
	  ++itRg->second.bc.delHomACGTN[homopolymerContext(sequence, sp, 3)];
	  if (bam_cigar_oplen(cigar[i]) < itRg->second.bc.maxIndelSize) ++itRg->second.bc.delSize[bam_cigar_oplen(cigar[i])];
	  else ++itRg->second.bc.delSize[itRg->second.bc.maxIndelSize];
	  rp += bam_cigar_oplen(cigar[i]);
	} else if (bam_cigar_op(cigar[i]) == BAM_CINS) {
	  ++itRg->second.bc.insCount;
	  ++itRg->second.bc.insHomACGTN[homopolymerContext(sequence, sp, 3)];
	  if (bam_cigar_oplen(cigar[i]) < itRg->second.bc.maxIndelSize) ++itRg->second.bc.insSize[bam_cigar_oplen(cigar[i])];
	  else ++itRg->second.bc.insSize[itRg->second.bc.maxIndelSize];
	  sp += bam_cigar_oplen(cigar[i]);
	} else if (bam_cigar_op(cigar[i]) == BAM_CSOFT_CLIP) {
	  ++itRg->second.bc.softClipCount;
	  sp += bam_cigar_oplen(cigar[i]);
	} else if(bam_cigar_op(cigar[i]) == BAM_CHARD_CLIP) {
	  ++itRg->second.bc.hardClipCount;
	} else if (bam_cigar_op(cigar[i]) == BAM_CREF_SKIP) {
	  if (!spliced) {
	    ++itRg->second.rc.spliced;
	    spliced = true;
	  }
	  rp += bam_cigar_oplen(cigar[i]);
	} else {
	  std::cerr << "Unknown Cigar options" << std::endl;
	  return 1;
	}
      }
    }
    // Summarize bp-level coverage
    if (refIndex != -1) {
      for(typename TRGMap::iterator itRg = rgMap.begin(); itRg != rgMap.end(); ++itRg) {
	if ((c.hasRegionFile) && (!gRegions[refIndex].empty())) _summarizeBedCoverage(gRegions[refIndex], itRg->second.bc.cov, refIndex, itRg->first, be);
	for(uint32_t i = 0; i < hdr->target_len[refIndex]; ++i) {
	  if (itRg->second.bc.cov[i] >= 1) {
	    ++itRg->second.bc.nd;
	    if (itRg->second.bc.cov[i] == 1) ++itRg->second.bc.n1;
	    if (itRg->second.bc.cov[i] == 2) ++itRg->second.bc.n2;
	  }
	  if (!nrun[i]) ++itRg->second.bc.bpWithCoverage[itRg->second.bc.cov[i]];
	}
	itRg->second.bc.cov.clear();
      }
      if (seq != NULL) free(seq);
    }
    

    // Outfile
    boost::iostreams::filtering_ostream rcfile;
    rcfile.push(boost::iostreams::gzip_compressor());
    rcfile.push(boost::iostreams::file_sink(c.outfile.string().c_str(), std::ios_base::out | std::ios_base::binary));

    // Output header
    rcfile << "# This file was produced by alfred v" << alfredVersionNumber << "." << std::endl;
    rcfile << "# This file contains alignment statistics for all reads." << std::endl;

    // Output metrics
    rcfile << "# Alignment summary metrics (ME)." << std::endl;
    rcfile << "# Use `zgrep ^ME <outfile> | cut -f 2-` to extract this part." << std::endl;
    rcfile << "ME\tSample\tLibrary\t#QCFail\tQCFailFraction\t#DuplicateMarked\tDuplicateFraction\t#Unmapped\tUnmappedFraction\t#Mapped\tMappedFraction\t#MappedRead1\t#MappedRead2\tRatioMapped2vsMapped1\t#MappedForward\tMappedForwardFraction\t#MappedReverse\tMappedReverseFraction\t#SecondaryAlignments\tSecondaryAlignmentFraction\t#SupplementaryAlignments\tSupplementaryAlignmentFraction\t#SplicedAlignments\tSplicedAlignmentFraction" << "\t";
    rcfile << "#Pairs\t#MappedPairs\tMappedPairsFraction\t#MappedSameChr\tMappedSameChrFraction\t#MappedProperPair\tMappedProperFraction" << "\t";
    rcfile << "#ReferenceBp\t#ReferenceNs\t#AlignedBases\t#MatchedBases\tMatchRate\t#MismatchedBases\tMismatchRate\t#DeletionsCigarD\tDeletionRate\tHomopolymerContextDel\t#InsertionsCigarI\tInsertionRate\tHomopolymerContextIns\t#SoftClippedBases\tSoftClipRate\t#HardClippedBases\tHardClipRate\tErrorRate" << "\t";
    rcfile << "MedianReadLength\tDefaultLibraryLayout\tMedianInsertSize\tMedianCoverage\tSDCoverage\tCoveredBp\tFractionCovered\tBpCov1ToCovNRatio\tBpCov1ToCov2Ratio\tMedianMAPQ";
    if (c.hasRegionFile) rcfile << "\t#TotalBedBp\t#AlignedBasesInBed\tFractionInBed\tEnrichmentOverBed";
    if (c.isMitagged) rcfile << "\t#MItagged\tFractionMItagged\t#UMIs";
    if (c.isHaplotagged) rcfile << "\t#HaploTagged\tFractionHaploTagged\t#PhasedBlocks\tN50PhasedBlockLength"; 
    rcfile << std::endl;
    for(typename TRGMap::iterator itRg = rgMap.begin(); itRg != rgMap.end(); ++itRg) {
      // Read counts
      uint64_t totalReadCount = itRg->second.rc.qcfail + itRg->second.rc.dup + itRg->second.rc.unmap + itRg->second.rc.mapped1 + itRg->second.rc.mapped2;
      uint64_t mappedCount = itRg->second.rc.mapped1 + itRg->second.rc.mapped2;
      rcfile << "ME\t" << c.sampleName << "\t" << itRg->first << "\t" << itRg->second.rc.qcfail << "\t" << (double) itRg->second.rc.qcfail / (double) totalReadCount << "\t" << itRg->second.rc.dup << "\t" << (double) itRg->second.rc.dup / (double) totalReadCount << "\t" << itRg->second.rc.unmap << "\t" << (double) itRg->second.rc.unmap / (double) totalReadCount << "\t" << mappedCount << "\t" << (double) mappedCount / (double) totalReadCount << "\t" << itRg->second.rc.mapped1 << "\t" << itRg->second.rc.mapped2 << "\t" << (double) itRg->second.rc.mapped2 / (double) itRg->second.rc.mapped1 << "\t" << itRg->second.rc.forward << "\t" << (double) itRg->second.rc.forward / (double) mappedCount << "\t" << itRg->second.rc.reverse << "\t" << (double) itRg->second.rc.reverse / (double) mappedCount << "\t" << itRg->second.rc.secondary << "\t" << (double) itRg->second.rc.secondary / (double) mappedCount << "\t" << itRg->second.rc.supplementary << "\t" << (double) itRg->second.rc.supplementary / (double) mappedCount << "\t" << itRg->second.rc.spliced << "\t" << (double) itRg->second.rc.spliced / (double) mappedCount << "\t";

      // Paired counts
      int64_t paired = itRg->second.pc.paired / 2;
      int64_t mapped = itRg->second.pc.mapped / 2;
      int64_t mappedSameChr = itRg->second.pc.mappedSameChr / 2;
      int64_t mappedProper = itRg->second.pc.mappedProper / 2;
      int32_t deflayout = 0;
      int32_t maxcount = itRg->second.pc.orient[0];
      for(int32_t i = 1; i<4; ++i) {
	if (itRg->second.pc.orient[i] > maxcount) {
	  maxcount = itRg->second.pc.orient[i];
	  deflayout = i;
	}
      }
      double mappedpairedfrac = 0;
      if (paired > 0) mappedpairedfrac = (double) mapped / (double) paired;
      double mappedpairedchrfrac = 0;
      if (paired > 0) mappedpairedchrfrac = (double) mappedSameChr / (double) paired;
      double mappedproperfrac = 0;
      if (paired > 0) mappedproperfrac = (double) mappedProper / (double) paired;
      rcfile << paired << "\t" << mapped << "\t" << mappedpairedfrac << "\t" << mappedSameChr << "\t" << mappedpairedchrfrac << "\t" << mappedProper << "\t" << mappedproperfrac << "\t";

      // Homopolymer Context of InDels
      double insTotal = 0;
      for(uint32_t i = 0; i < itRg->second.bc.insHomACGTN.size(); ++i) insTotal += itRg->second.bc.insHomACGTN[i];
      double insFrac = 0;
      if (insTotal > 0) {
	for(uint32_t i = 0; i < itRg->second.bc.insHomACGTN.size() - 1; ++i) insFrac += itRg->second.bc.insHomACGTN[i];
	insFrac /= insTotal;
      }
      double delTotal = 0;
      for(uint32_t i = 0; i < itRg->second.bc.delHomACGTN.size(); ++i) delTotal += itRg->second.bc.delHomACGTN[i];
      double delFrac = 0;
      if (delTotal > 0) {
	for(uint32_t i = 0; i < itRg->second.bc.delHomACGTN.size() - 1; ++i) delFrac += itRg->second.bc.delHomACGTN[i];
	delFrac /= delTotal;
      }
	
      // Error rates
      uint64_t alignedbases = itRg->second.bc.matchCount + itRg->second.bc.mismatchCount;
      rcfile << referencebp << "\t" << ncount << "\t" << alignedbases << "\t" << itRg->second.bc.matchCount << "\t" << (double) itRg->second.bc.matchCount / (double) alignedbases << "\t" << itRg->second.bc.mismatchCount << "\t" << (double) itRg->second.bc.mismatchCount / (double) alignedbases << "\t" << itRg->second.bc.delCount << "\t" << (double) itRg->second.bc.delCount / (double) alignedbases << "\t" << delFrac << "\t" << itRg->second.bc.insCount << "\t" << (double) itRg->second.bc.insCount / (double) alignedbases << "\t" << insFrac << "\t" << itRg->second.bc.softClipCount << "\t" << (double) itRg->second.bc.softClipCount / (double) alignedbases << "\t" << itRg->second.bc.hardClipCount << "\t" << (double) itRg->second.bc.hardClipCount / (double) alignedbases << "\t" << (double) (itRg->second.bc.mismatchCount + itRg->second.bc.delCount + itRg->second.bc.insCount + itRg->second.bc.softClipCount + itRg->second.bc.hardClipCount) / (double) alignedbases  << "\t";

      // Median coverage, read length, etc.
      int32_t medISize = 0;
      switch(deflayout) {
      case 0:
	medISize = medianFromHistogram(itRg->second.pc.fPlus);
	break;
      case 1:
	medISize = medianFromHistogram(itRg->second.pc.fMinus);
	break;
      case 2:
	medISize = medianFromHistogram(itRg->second.pc.rPlus);
	break;
      case 3:
	medISize = medianFromHistogram(itRg->second.pc.rMinus);
	break;
      default:
	break;
      }

      // Standardized SD of genomic coverage
      double ssdcov = 1000 * sdFromHistogram(itRg->second.bc.bpWithCoverage) / std::sqrt((double) mappedCount);
      double fraccovbp = (double) itRg->second.bc.nd / (double) (referencebp - ncount);
      double pbc1 = (double) itRg->second.bc.n1 / (double) itRg->second.bc.nd;
      double pbc2 = (double) itRg->second.bc.n1 / (double) itRg->second.bc.n2;

      rcfile << medianFromHistogram(itRg->second.rc.lRc) << "\t" << deflayout << "\t" << medISize << "\t" << medianFromHistogram(itRg->second.bc.bpWithCoverage) << "\t" << ssdcov << "\t" << itRg->second.bc.nd << "\t" << fraccovbp << "\t" << pbc1 << "\t" << pbc2 << "\t" << medianFromHistogram(itRg->second.qc.qcount);
      
      // Bed metrics
      if (c.hasRegionFile) {
	uint64_t nonN = referencebp - ncount;
	typename BedCounts::TOnTargetMap::iterator itOT = be.onTarget.find(itRg->first);
	uint64_t alignedBedBases = itOT->second[0];
	double fractioninbed = (double) alignedBedBases / (double) alignedbases;
	double enrichment = fractioninbed / ((double) totalBedSize / (double) nonN);
	rcfile << "\t" << totalBedSize << "\t" << alignedBedBases << "\t" << fractioninbed << "\t" << enrichment;
      }
      if (c.isMitagged) {
	rcfile << "\t" << itRg->second.rc.mitagged << "\t" << (double) itRg->second.rc.mitagged / (double) totalReadCount << "\t" << itRg->second.rc.umi.count();
      }
      if (c.isHaplotagged) {
	int32_t n50ps = n50PhasedBlockLength(itRg->second.rc.brange);
	rcfile << "\t" << itRg->second.rc.haplotagged << "\t" << (double) itRg->second.rc.haplotagged / (double) totalReadCount << "\t" << phasedBlocks(itRg->second.rc.brange) << "\t" << n50ps;
      }
      rcfile << std::endl;
    }

    // Output read length histogram
    rcfile << "# Read length distribution (RL)." << std::endl;
    rcfile << "# Use `zgrep ^RL <outfile> | cut -f 2-` to extract this part." << std::endl;
    rcfile << "RL\tSample\tReadlength\tCount\tFraction\tLibrary" << std::endl;
    for(typename TRGMap::iterator itRg = rgMap.begin(); itRg != rgMap.end(); ++itRg) {
      uint32_t lastValidRL = 0;
      for(uint32_t i = 0; i < itRg->second.rc.lRc.size(); ++i)
	if (itRg->second.rc.lRc[i] > 0) lastValidRL = i;
      double total = 0;
      for(uint32_t i = 0; i <= lastValidRL; ++i) total += itRg->second.rc.lRc[i];
      for(uint32_t i = 0; i <= lastValidRL; ++i) {
	double frac = 0;
	if (total > 0) frac = (double) itRg->second.rc.lRc[i] / total;
	rcfile << "RL\t" << c.sampleName << "\t" << i << "\t" << itRg->second.rc.lRc[i] << "\t" << frac << "\t" << itRg->first << std::endl;
      }
    }

    // Output mean base quality
    rcfile << "# Mean base quality (BQ)." << std::endl;
    rcfile << "# Use `zgrep ^BQ <outfile> | cut -f 2-` to extract this part." << std::endl;
    rcfile << "BQ\tSample\tPosition\tBaseQual\tLibrary" << std::endl;
    for(typename TRGMap::iterator itRg = rgMap.begin(); itRg != rgMap.end(); ++itRg) {
      uint32_t lastValidBQIdx = 0;
      for(uint32_t i = lastValidBQIdx + 1; i < itRg->second.rc.nCount.size(); ++i) {
	uint64_t bcount = itRg->second.rc.aCount[i] + itRg->second.rc.cCount[i] + itRg->second.rc.gCount[i] + itRg->second.rc.tCount[i] + itRg->second.rc.nCount[i];
	if (bcount > 0) lastValidBQIdx = i;
      }
      for(uint32_t i = 0; i <= lastValidBQIdx; ++i) {
	uint64_t bcount = itRg->second.rc.aCount[i] + itRg->second.rc.cCount[i] + itRg->second.rc.gCount[i] + itRg->second.rc.tCount[i] + itRg->second.rc.nCount[i];
	if (bcount > 0) rcfile << "BQ\t" << c.sampleName << "\t" << i << "\t" << (double) (itRg->second.rc.bqCount[i]) / (double) (bcount) << "\t" << itRg->first << std::endl;
	else rcfile << "BQ\t" << c.sampleName << "\t" << i << "\tNA\t" << itRg->first << std::endl;
      }
    }

    // Output per base ACGTN content
    rcfile << "# Base content (BC)." << std::endl;
    rcfile << "# Use `zgrep ^BC <outfile> | cut -f 2-` to extract this part." << std::endl;
    rcfile << "BC\tSample\tPosition\tBase\tCount\tFraction\tLibrary" << std::endl;
    for(typename TRGMap::iterator itRg = rgMap.begin(); itRg != rgMap.end(); ++itRg) {
      uint32_t lastValidBQIdx = 0;
      for(uint32_t i = lastValidBQIdx + 1; i < itRg->second.rc.nCount.size(); ++i) {
	uint64_t bcount = itRg->second.rc.aCount[i] + itRg->second.rc.cCount[i] + itRg->second.rc.gCount[i] + itRg->second.rc.tCount[i] + itRg->second.rc.nCount[i];
	if (bcount > 0) lastValidBQIdx = i;
      }
      for(uint32_t i = 0; i <= lastValidBQIdx; ++i) {
	uint64_t bcount = itRg->second.rc.aCount[i] + itRg->second.rc.cCount[i] + itRg->second.rc.gCount[i] + itRg->second.rc.tCount[i] + itRg->second.rc.nCount[i];
	if (bcount > 0) {
	  rcfile << "BC\t" << c.sampleName << "\t" << i << "\tA\t" << itRg->second.rc.aCount[i] << "\t" << (double) itRg->second.rc.aCount[i] / (double) bcount << "\t" << itRg->first << std::endl;
	  rcfile << "BC\t" << c.sampleName << "\t" << i << "\tC\t" << itRg->second.rc.cCount[i] << "\t" << (double) itRg->second.rc.cCount[i] / (double) bcount << "\t" << itRg->first << std::endl;
	  rcfile << "BC\t" << c.sampleName << "\t" << i << "\tG\t" << itRg->second.rc.gCount[i] << "\t" << (double) itRg->second.rc.gCount[i] / (double) bcount << "\t" << itRg->first << std::endl;
	  rcfile << "BC\t" << c.sampleName << "\t" << i << "\tT\t" << itRg->second.rc.tCount[i] << "\t" << (double) itRg->second.rc.tCount[i] / (double) bcount << "\t" << itRg->first << std::endl;
	  rcfile << "BC\t" << c.sampleName << "\t" << i << "\tN\t" << itRg->second.rc.nCount[i] << "\t" << (double) itRg->second.rc.nCount[i] / (double) bcount << "\t" << itRg->first << std::endl;
	} else {
	  rcfile << "BC\t" << c.sampleName << "\t" << i << "\tA\t" << itRg->second.rc.aCount[i] << "\tNA\t" << itRg->first << std::endl;
	  rcfile << "BC\t" << c.sampleName << "\t" << i << "\tC\t" << itRg->second.rc.cCount[i] << "\tNA\t" << itRg->first << std::endl;
	  rcfile << "BC\t" << c.sampleName << "\t" << i << "\tG\t" << itRg->second.rc.gCount[i] << "\tNA\t" << itRg->first << std::endl;
	  rcfile << "BC\t" << c.sampleName << "\t" << i << "\tT\t" << itRg->second.rc.tCount[i] << "\tNA\t" << itRg->first << std::endl;
	  rcfile << "BC\t" << c.sampleName << "\t" << i << "\tN\t" << itRg->second.rc.nCount[i] << "\tNA\t" << itRg->first << std::endl;
	}
      }
    }


    // Output mapping quality histogram
    rcfile << "# Mapping quality histogram (MQ)." << std::endl;
    rcfile << "# Use `zgrep ^MQ <outfile> | cut -f 2-` to extract this part." << std::endl;
    rcfile << "MQ\tSample\tMappingQuality\tCount\tFraction\tLibrary" << std::endl;
    for(typename TRGMap::iterator itRg = rgMap.begin(); itRg != rgMap.end(); ++itRg) {
      uint32_t lastValidMQ = 0;
      for(uint32_t i = 0; i < itRg->second.qc.qcount.size(); ++i)
	if (itRg->second.qc.qcount[i] > 0) lastValidMQ = i;
      double total = 0;
      for(uint32_t i = 0; i <= lastValidMQ; ++i) total += itRg->second.qc.qcount[i];
      for(uint32_t i = 0; i <= lastValidMQ; ++i) {
	double frac = 0;
	if (total > 0) frac = (double) itRg->second.qc.qcount[i] / total;
	rcfile << "MQ\t" << c.sampleName << "\t" << i << "\t" << itRg->second.qc.qcount[i] << "\t" << frac << "\t" << itRg->first << std::endl;
      }
    }    

    // Output coverage histograms
    rcfile << "# Coverage histogram (CO)." << std::endl;
    rcfile << "# Use `zgrep ^CO <outfile> | cut -f 2-` to extract this part." << std::endl;
    rcfile << "CO\tSample\tCoverage\tCount\tQuantile\tLibrary" << std::endl;
    for(typename TRGMap::iterator itRg = rgMap.begin(); itRg != rgMap.end(); ++itRg) {
      uint32_t lastValidCO = 0;
      for(uint32_t i = 0; i < itRg->second.bc.bpWithCoverage.size(); ++i)
	if (itRg->second.bc.bpWithCoverage[i] > 0) lastValidCO = i;
      // Ignore last bucket that collects all higher coverage bases
      uint64_t totalCO = 0;
      for(uint32_t i = 0; i < itRg->second.bc.bpWithCoverage.size() - 1; ++i) totalCO += itRg->second.bc.bpWithCoverage[i];
      uint64_t cumsum = 0;
      for(uint32_t i = 0; i <= lastValidCO; ++i) {
	double quant = 0;
	if (totalCO > 0) quant = (double) cumsum / (double) totalCO;
	rcfile << "CO\t" << c.sampleName << "\t" << i << "\t" << itRg->second.bc.bpWithCoverage[i] << "\t" << quant << "\t" << itRg->first << std::endl;
	cumsum += itRg->second.bc.bpWithCoverage[i];
      }
    }

    // Output mapping statistics by chromosome
    rcfile << "# Chromosome mapping statistics (CM)." << std::endl;
    rcfile << "# Use `zgrep ^CM <outfile> | cut -f 2-` to extract this part." << std::endl;
    rcfile << "CM\tSample\tLibrary\tChrom\tSize\tMapped\tMappedFraction\tObsExpRatio" << std::endl;
    for(typename TRGMap::iterator itRg = rgMap.begin(); itRg != rgMap.end(); ++itRg) {
      uint64_t totalMappedChr = 0;
      for(uint32_t i = 0; i < itRg->second.rc.mappedchr.size(); ++i) totalMappedChr += itRg->second.rc.mappedchr[i];
      for(uint32_t i = 0; i < itRg->second.rc.mappedchr.size(); ++i) {
	double frac = 0;
	if (totalMappedChr > 0) frac = (double) itRg->second.rc.mappedchr[i] / (double) totalMappedChr;
	double expect = (double) (hdr->target_len[i] - chrGC[i].ncount) / (double) (referencebp - ncount);
	double obsexprat = frac / expect;
	rcfile << "CM\t" << c.sampleName << "\t" << itRg->first << "\t" << hdr->target_name[i] << "\t" << hdr->target_len[i] << "\t" << itRg->second.rc.mappedchr[i] << "\t" << frac << "\t" << obsexprat << std::endl;
      }
    }
    
    // Output insert size histograms
    rcfile << "# Insert size histogram (IS)." << std::endl;
    rcfile << "# Use `zgrep ^IS <outfile> | cut -f 2-` to extract this part." << std::endl;
    rcfile << "IS\tSample\tInsertSize\tCount\tLayout\tQuantile\tLibrary" << std::endl;
    for(typename TRGMap::iterator itRg = rgMap.begin(); itRg != rgMap.end(); ++itRg) {
      uint32_t lastValidISIdx = 0;
      for(uint32_t i = 0; i < itRg->second.pc.fPlus.size(); ++i) {
	uint64_t tpecount = itRg->second.pc.fPlus[i] + itRg->second.pc.fMinus[i] + itRg->second.pc.rPlus[i] + itRg->second.pc.rMinus[i];
	if (tpecount > 0) lastValidISIdx = i;
      }
      // Ignore last bucket that collects all other pairs
      uint64_t totalFR = 0;
      for(uint32_t i = 0; i < itRg->second.pc.fPlus.size() - 1; ++i) totalFR += itRg->second.pc.fPlus[i] + itRg->second.pc.fMinus[i] + itRg->second.pc.rPlus[i] + itRg->second.pc.rMinus[i];
      uint64_t cumsum = 0;
      for(uint32_t i = 0; i <= lastValidISIdx; ++i) {
	double quant = 0;
	if (totalFR > 0) quant = (double) cumsum / (double) totalFR;
	rcfile << "IS\t" << c.sampleName << "\t" << i << "\t" << itRg->second.pc.fPlus[i] << "\tF+\t" << quant << "\t" << itRg->first << std::endl;
	rcfile << "IS\t" << c.sampleName << "\t" << i << "\t" << itRg->second.pc.fMinus[i] << "\tF-\t" << quant << "\t" << itRg->first << std::endl;
	rcfile << "IS\t" << c.sampleName << "\t" << i << "\t" << itRg->second.pc.rPlus[i] << "\tR+\t" << quant << "\t" << itRg->first << std::endl;
	rcfile << "IS\t" << c.sampleName << "\t" << i << "\t" << itRg->second.pc.rMinus[i] << "\tR-\t" << quant << "\t" << itRg->first << std::endl;		
	cumsum += itRg->second.pc.fPlus[i] + itRg->second.pc.fMinus[i] + itRg->second.pc.rPlus[i] + itRg->second.pc.rMinus[i];
      }
    }

    // Homopolymer InDel context
    rcfile << "# InDel context (IC)." << std::endl;
    rcfile << "# Use `zgrep ^IC <outfile> | cut -f 2-` to extract this part." << std::endl;
    rcfile << "IC\tSample\tLibrary\tInDel\tHomopolymer\tCount\tFraction" << std::endl;
    for(typename TRGMap::iterator itRg = rgMap.begin(); itRg != rgMap.end(); ++itRg) {
      double total = 0;
      for(uint32_t i = 0; i < itRg->second.bc.delHomACGTN.size(); ++i) total += itRg->second.bc.delHomACGTN[i];
      for(uint32_t i = 0; i < itRg->second.bc.delHomACGTN.size(); ++i) {
	double frac = 0;
	if (total > 0) frac = (double) itRg->second.bc.delHomACGTN[i] / total;
	rcfile << "IC\t" << c.sampleName << "\t" << itRg->first << "\tDEL\t";
	if (i == 0) rcfile << 'A';
	else if (i == 1) rcfile << 'C';
	else if (i == 2) rcfile << 'G';
	else if (i == 3) rcfile << 'T';
	else if (i == 4) rcfile << 'N';
	else rcfile << "None";
	rcfile << "\t" << itRg->second.bc.delHomACGTN[i] << "\t" << frac << std::endl;
      }
    }
    for(typename TRGMap::iterator itRg = rgMap.begin(); itRg != rgMap.end(); ++itRg) {
      double total = 0;
      for(uint32_t i = 0; i < itRg->second.bc.insHomACGTN.size(); ++i) total += itRg->second.bc.insHomACGTN[i];
      for(uint32_t i = 0; i < itRg->second.bc.insHomACGTN.size(); ++i) {
	double frac = 0;
	if (total > 0) frac = (double) itRg->second.bc.insHomACGTN[i] / total;
	rcfile << "IC\t" << c.sampleName << "\t" << itRg->first << "\tINS\t";
	if (i == 0) rcfile << 'A';
	else if (i == 1) rcfile << 'C';
	else if (i == 2) rcfile << 'G';
	else if (i == 3) rcfile << 'T';
	else if (i == 4) rcfile << 'N';
	else rcfile << "None";
	rcfile << "\t" << itRg->second.bc.insHomACGTN[i] << "\t" << frac << std::endl;
      }
    }

    // InDel size
    rcfile << "# InDel size (IZ)." << std::endl;
    rcfile << "# Use `zgrep ^IZ <outfile> | cut -f 2-` to extract this part." << std::endl;
    rcfile << "IZ\tSample\tLibrary\tInDel\tSize\tCount" << std::endl;
    for(typename TRGMap::iterator itRg = rgMap.begin(); itRg != rgMap.end(); ++itRg) {
      for(uint32_t i = 1; i < itRg->second.bc.delSize.size(); ++i) 
	rcfile << "IZ\t" << c.sampleName << "\t" << itRg->first << "\tDEL\t" << i << "\t" << itRg->second.bc.delSize[i] << std::endl;
      for(uint32_t i = 1; i < itRg->second.bc.insSize.size(); ++i) 
	rcfile << "IZ\t" << c.sampleName << "\t" << itRg->first << "\tINS\t" << i << "\t" << itRg->second.bc.insSize[i] << std::endl;
    }

    // Reference GC content
    rcfile << "# Chromosome GC-content (CG)." << std::endl;
    rcfile << "# Use `zgrep ^CG <outfile> | cut -f 2-` to extract this part." << std::endl;
    rcfile << "CG\tChromosome\tSize\tnumN\tnumGC\tGCfraction" << std::endl;
    for(uint32_t i = 0; i < chrGC.size(); ++i) {
      if (chrGC[i].ncount + chrGC[i].gccount > 0) {
	// Only chromosomes with mapped data
	double frac = 0;
	double total = hdr->target_len[i] - chrGC[i].ncount;
	if (total > 0) frac = (double) chrGC[i].gccount / total;
	rcfile << "CG\t" << hdr->target_name[i] << "\t" << hdr->target_len[i] << "\t" << chrGC[i].ncount << "\t" << chrGC[i].gccount << "\t" << frac << std::endl;
      }
    }

    // GC-content
    rcfile << "# GC-content (GC)." << std::endl;
    rcfile << "# Use `zgrep ^GC <outfile> | cut -f 2-` to extract this part." << std::endl;
    rcfile << "GC\tSample\tLibrary\tGCcontent\tfractionOfReads" << std::endl;
    {
      double total = 0;
      for(uint32_t i = 0; i < 101; ++i) total += refGcContent[i];
      for(uint32_t i = 0; i < 101; ++i) {
	double frac = 0;
	if (total > 0) frac = (double) refGcContent[i] / total;
	rcfile << "GC\tReference\tReference\t" << (double) i / (double) 100 << "\t" << frac << std::endl;
      }
    }
    for(typename TRGMap::iterator itRg = rgMap.begin(); itRg != rgMap.end(); ++itRg) {
      double total = 0;
      for(uint32_t i = 0; i < 101; ++i) total += itRg->second.rc.gcContent[i];
      for(uint32_t i = 0; i < 101; ++i) {
	double frac = 0;
	if (total > 0) frac = (double) itRg->second.rc.gcContent[i] / total;
	rcfile << "GC\t" << c.sampleName << "\t" << itRg->first << "\t" << (double) i / (double) 100 << "\t" << frac << std::endl;
      }
    }

    if (c.hasRegionFile) {
      // Output avg. bed coverage
      rcfile << "# Avg. target coverage (TC)." << std::endl;
      rcfile << "# Use `zgrep ^TC <outfile> | cut -f 2-` to extract this part." << std::endl;
      rcfile << "TC\tSample\tLibrary\tChr\tStart\tEnd\tAvgCov" << std::endl;
      for(int32_t refIndex = 0; refIndex < hdr->n_targets; ++refIndex) {
	for(typename BedCounts::TRgBpMap::const_iterator itChr = be.gCov[refIndex].begin(); itChr != be.gCov[refIndex].end(); ++itChr) {
	  for(uint32_t i = 0; i < gRegions[refIndex].size(); ++i) {
	    rcfile << "TC\t" << c.sampleName << "\t" << itChr->first << "\t" << hdr->target_name[refIndex] << "\t" << gRegions[refIndex][i].start << "\t" << gRegions[refIndex][i].end << "\t" << itChr->second[i] << std::endl;
	  }
	}
      }
      
      // Output on-target rate
      rcfile << "# On target rate (OT)." << std::endl;
      rcfile << "# Use `zgrep ^OT <outfile> | cut -f 2-` to extract this part." << std::endl;
      rcfile << "OT\tSample\tLibrary\tExtension\tOnTarget" << std::endl;
      for(typename TRGMap::iterator itRg = rgMap.begin(); itRg != rgMap.end(); ++itRg) {
	uint64_t alignedbases = itRg->second.bc.matchCount + itRg->second.bc.mismatchCount;
	typename BedCounts::TOnTargetMap::iterator itOT = be.onTarget.find(itRg->first);
	for(uint32_t k = 0; k < itOT->second.size(); ++k) {
	  rcfile << "OT\t" << c.sampleName << "\t" << itRg->first << "\t" << k * be.stepsize << "\t" << (double) itOT->second[k] / (double) alignedbases << std::endl;
	}
      }
    }

    if (c.isHaplotagged) {
      // Output phased block length histogram
      rcfile << "# Phased block length (PS)." << std::endl;
      rcfile << "# Use `zgrep ^PS <outfile> | cut -f 2-` to extract this part." << std::endl;
      rcfile << "PS\tSample\tChr\tStart\tEnd\tPSid\tSize\tLibrary" << std::endl;
      for(typename TRGMap::iterator itRg = rgMap.begin(); itRg != rgMap.end(); ++itRg) {
	for(int32_t refIndex = 0; refIndex < (int32_t) itRg->second.rc.brange.size(); ++refIndex) {
	  for(typename ReadCounts::TBlockRange::const_iterator itBR = itRg->second.rc.brange[refIndex].begin(); itBR != itRg->second.rc.brange[refIndex].end(); ++itBR) {
	    if (itBR->second.first < itBR->second.second) {
	      rcfile << "PS\t" << c.sampleName << "\t" <<  hdr->target_name[refIndex] << "\t" << itBR->second.first << "\t" << itBR->second.second << "\t" << itBR->first << "\t" << (itBR->second.second - itBR->second.first) << "\t" << itRg->first << std::endl;
	    }
	  }
	}
      }
    }
  

    // clean-up
    bam_destroy1(rec);
    fai_destroy(fai);
    bam_hdr_destroy(hdr);
    sam_close(samfile);
    
    now = boost::posix_time::second_clock::local_time();
    std::cout << '[' << boost::posix_time::to_simple_string(now) << "] Done." << std::endl;
    
#ifdef PROFILE
    ProfilerStop();
#endif


    return 0;
  }

}

#endif
