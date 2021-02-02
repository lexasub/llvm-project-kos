template <typename> struct vector {};

#include "Types.h"
#include <map>

struct TString {
  TString(char *);
};

struct TreeInfo {};

class DataInputHandler {
  void AddTree();
  void SignalTreeInfo() {
    fInputTrees[(char *)""];
  }
  map<TString, vector<TreeInfo>> fInputTrees;
  map<string, bool> fExplicitTrainTest;
};
