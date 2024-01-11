#include "join.hpp"

#include <cstdint>
#include <vector>
#include <algorithm>

JoinAlgorithm getJoinAlgorithm() {
  return JOIN_ALGORITHM_SMJ;
}

int join(File &file, int numPagesR, int numPagesS, char *buffer,
         int numFrames) {
  // Step 1: Sort relations R and S using external merge sort
    int pageIndexR = 0;
    int pageIndexS = pageIndexR + numPagesR;
    int pageIndexOut = pageIndexS + numPagesS;

    std::vector<Tuple> tuplesR(numPagesR * 512);
    file.read(tuplesR.data(), pageIndexR, numPagesR);

    std::vector<Tuple> tuplesS(numPagesS * 512);
    file.read(tuplesS.data(), pageIndexS, numPagesS);

    std::sort(tuplesR.begin(), tuplesR.end(), [](const Tuple& a, const Tuple& b) {
        return a.first < b.first;
    });

    std::sort(tuplesS.begin(), tuplesS.end(), [](const Tuple& a, const Tuple& b) {
        return a.first < b.first;
    });

  auto r = tuplesR.begin();
  auto s = tuplesS.begin();

  // Step 3: Process tuples in R and S
  std::vector<Tuple> tuplesOut;
  while (r != tuplesR.end() && s != tuplesS.end()) {
    // Step 4: Advance r pointer to tuple with key >= s key
    while (r != tuplesR.end() && r->first < s->first) {
      ++r;
    }

    // Step 5: Advance s pointer to tuple with key >= r key
    while (s != tuplesS.end() && s->first < r->first) {
      ++s;
    }

    // Step 6: Output matching tuples and advance s pointer
    if (r != tuplesR.end() && s != tuplesS.end() && r->first == s->first) {
      auto t = s;
      while (s != tuplesS.end() && s->first == r->first) {
        tuplesOut.emplace_back(r->second, s->second);
        ++s;
      }
      s = t;
      ++r;
    }
  }

  // Step 7: Write output tuples to disk
  int numTuplesOut = (int)tuplesOut.size();
  int numPagesOut = numTuplesOut / 512 + (numTuplesOut % 512 != 0);
  file.write(tuplesOut.data(), pageIndexOut, numPagesOut);

  return numTuplesOut;
}
