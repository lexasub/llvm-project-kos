//===- Writer.cpp ---------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "Writer.h"
#include "Config.h"
#include "InputFiles.h"
#include "SymbolTable.h"
#include "Symbols.h"
#include "lld/Common/ErrorHandler.h"
#include "lld/Common/Timer.h"
#include "llvm/Support/Parallel.h"

using namespace llvm;
using namespace llvm::COFF;
using namespace llvm::object;
using namespace lld;
using namespace lld::kos;

struct Menuet02Header {
  char magic[8] = {'M', 'E', 'N', 'U', 'E', 'T', '0', '2'};
  uint32_t version = 1;
  uint32_t start;
  uint32_t end;
  uint32_t memory;
  uint32_t stack;
  uint32_t params;
  uint32_t path;
  uint32_t tls;
};

// Global vector of all output sections. After output sections are finalized,
// this can be indexed by Chunk::getOutputSection.
static std::vector<OutputSection *> outputSections;

OutputSection *Chunk::getOutputSection() const {
  return osidx == 0 ? nullptr : outputSections[osidx - 1];
}

void OutputSection::clear() { outputSections.clear(); }

namespace {

// PartialSection represents a group of chunks that contribute to an
// OutputSection. Collating a collection of PartialSections of same name and
// characteristics constitutes the OutputSection.
class PartialSectionKey {
public:
  StringRef name;
  unsigned characteristics;

  bool operator<(const PartialSectionKey &other) const {
    int c = name.compare(other.name);
    if (c == 1)
      return false;
    if (c == 0)
      return characteristics < other.characteristics;
    return true;
  }
};

// The writer writes a SymbolTable result to a file.
class Writer {
public:
  Writer() : buffer(errorHandler().outputBuffer) {}
  void run();

private:
  void createSections();
  void assignAddresses();
  void finalizeAddresses();
  void removeEmptySections();
  void assignOutputSectionIndices();
  void openFile(StringRef outputPath);
  void writeHeader();
  void writeSections();
  PartialSection *createPartialSection(StringRef name, uint32_t outChars);

  std::unique_ptr<FileOutputBuffer> &buffer;
  std::map<PartialSectionKey, PartialSection *> partialSections;

  uint64_t fileSize;
  uint64_t sizeOfImage;
  uint64_t sizeOfHeaders;

  OutputSection *textSec;
  OutputSection *rdataSec;
  OutputSection *dataSec;
};
} // anonymous namespace

static Timer codeLayoutTimer("Code Layout", Timer::root());
static Timer diskCommitTimer("Commit Output File", Timer::root());

void lld::kos::writeResult() { Writer().run(); }

void OutputSection::addChunk(Chunk *c) { chunks.push_back(c); }

void OutputSection::insertChunkAtStart(Chunk *c) {
  chunks.insert(chunks.begin(), c);
}

void OutputSection::setPermissions(uint32_t c) {
  header.Characteristics &= ~permMask;
  header.Characteristics |= c;
}

void OutputSection::merge(OutputSection *other) {
  chunks.insert(chunks.end(), other->chunks.begin(), other->chunks.end());
  other->chunks.clear();
  contribSections.insert(contribSections.end(), other->contribSections.begin(),
                         other->contribSections.end());
  other->contribSections.clear();
}

void OutputSection::addContributingPartialSection(PartialSection *sec) {
  contribSections.push_back(sec);
}

// Assign addresses and add thunks if necessary.
void Writer::finalizeAddresses() {
  assignAddresses();
  if (config->machine != ARMNT && config->machine != ARM64)
    return;

  fatal("ARM stuff is cutted, fuck off");
}

// The main function of the writer.
void Writer::run() {
  ScopedTimer t1(codeLayoutTimer);

  createSections();
  finalizeAddresses();

  dyn_cast<DefinedAbsolute>(symtab->findUnderscore("__hdr_params"))->setVA(28);
  dyn_cast<DefinedAbsolute>(symtab->findUnderscore("__hdr_path"))->setVA(32);

  dyn_cast<DefinedAbsolute>(symtab->findUnderscore("__params"))
      ->setVA(sizeOfImage);
  sizeOfImage += 1024;
  dyn_cast<DefinedAbsolute>(symtab->findUnderscore("__path"))
      ->setVA(sizeOfImage);
  sizeOfImage += 4096;

  removeEmptySections();
  assignOutputSectionIndices();

  openFile(config->outputFile);
  writeHeader();
  writeSections();

  t1.stop();

  if (errorCount())
    return;

  ScopedTimer t2(diskCommitTimer);
  if (auto e = buffer->commit())
    fatal("failed to write the output file: " + toString(std::move(e)));
}

static StringRef getOutputSectionName(StringRef name) {
  StringRef s = name.split('$').first;

  // Treat a later period as a separator for MinGW, for sections like
  // ".ctors.01234".
  return s.substr(0, s.find('.', 1));
}

// Return whether a SectionChunk's suffix (the dollar and any trailing
// suffix) should be removed and sorted into the main suffixless
// PartialSection.
static bool shouldStripSectionSuffix(SectionChunk *sc, StringRef name) {
  // On MinGW, comdat groups are formed by putting the comdat group name
  // after the '$' in the section name. For .eh_frame$<symbol>, that must
  // still be sorted before the .eh_frame trailer from crtend.o, thus just
  // strip the section name trailer. For other sections, such as
  // .tls$$<symbol> (where non-comdat .tls symbols are otherwise stored in
  // ".tls$"), they must be strictly sorted after .tls. And for the
  // hypothetical case of comdat .CRT$XCU, we definitely need to keep the
  // suffix for sorting. Thus, to play it safe, only strip the suffix for
  // the standard sections.
  if (!config->mingw)
    return false;
  if (!sc || !sc->isCOMDAT())
    return false;
  return name.startswith(".text$") || name.startswith(".data$") ||
         name.startswith(".rdata$") || name.startswith(".pdata$") ||
         name.startswith(".xdata$") || name.startswith(".eh_frame$");
}

// Create output section objects and add them to OutputSections.
void Writer::createSections() {
  // First, create the builtin sections.
  const uint32_t data = IMAGE_SCN_CNT_INITIALIZED_DATA;
  const uint32_t bss = IMAGE_SCN_CNT_UNINITIALIZED_DATA;
  const uint32_t code = IMAGE_SCN_CNT_CODE;
  const uint32_t r = IMAGE_SCN_MEM_READ;
  const uint32_t w = IMAGE_SCN_MEM_WRITE;
  const uint32_t x = IMAGE_SCN_MEM_EXECUTE;

  SmallDenseMap<std::pair<StringRef, uint32_t>, OutputSection *> sections;
  auto createSection = [&](StringRef name, uint32_t outChars) {
    OutputSection *&sec = sections[{name, outChars}];
    if (!sec) {
      sec = make<OutputSection>(name, outChars);
      outputSections.push_back(sec);
    }
    return sec;
  };

  // Try to match the section order used by link.exe.
  textSec = createSection(".text", code | r | x);
  createSection(".bss", bss | r | w);
  rdataSec = createSection(".rdata", data | r);
  dataSec = createSection(".data", data | r | w);

  // Then bin chunks by name and output characteristics.
  for (Chunk *c : symtab->getChunks()) {
    auto *sc = dyn_cast<SectionChunk>(c);
    if (sc && !sc->live) {
      if (config->verbose)
        sc->printDiscardedMessage();
      continue;
    }
    StringRef name = c->getSectionName();
    if (shouldStripSectionSuffix(sc, name))
      name = name.split('$').first;

    PartialSection *pSec =
        createPartialSection(name, c->getOutputCharacteristics());
    pSec->chunks.push_back(c);
  }

  // Then create an OutputSection for each section.
  // '$' and all following characters in input section names are
  // discarded when determining output section. So, .text$foo
  // contributes to .text, for example. See PE/COFF spec 3.2.
  for (auto it : partialSections) {
    PartialSection *pSec = it.second;
    StringRef name = getOutputSectionName(pSec->name);
    uint32_t outChars = pSec->characteristics;

    OutputSection *sec = createSection(name, outChars);
    for (Chunk *c : pSec->chunks)
      sec->addChunk(c);

    sec->addContributingPartialSection(pSec);
  }
}

// The Windows loader doesn't seem to like empty sections,
// so we remove them if any.
void Writer::removeEmptySections() {
  auto isEmpty = [](OutputSection *s) { return s->getVirtualSize() == 0; };
  outputSections.erase(
      std::remove_if(outputSections.begin(), outputSections.end(), isEmpty),
      outputSections.end());
}

void Writer::assignOutputSectionIndices() {
  // Assign final output section indices, and assign each chunk to its output
  // section.
  uint32_t idx = 1;
  for (OutputSection *os : outputSections) {
    os->sectionIndex = idx;
    for (Chunk *c : os->chunks)
      c->setOutputSectionIdx(idx);
    ++idx;
  }

  // Merge chunks are containers of chunks, so assign those an output section
  // too.
  for (MergeChunk *mc : MergeChunk::instances)
    if (mc)
      for (SectionChunk *sc : mc->sections)
        if (sc && sc->live)
          sc->setOutputSectionIdx(mc->getOutputSectionIdx());
}

// Visits all sections to assign incremental, non-overlapping RVAs and
// file offsets.
void Writer::assignAddresses() {
  sizeOfHeaders = sizeof(Menuet02Header);
  fileSize = sizeOfHeaders;

  // The first page is kept unmapped.
  uint64_t rva = sizeOfHeaders;

  for (OutputSection *sec : outputSections) {
    uint64_t rawSize = 0, virtualSize = 0;
    sec->header.VirtualAddress = rva;

    // If /FUNCTIONPADMIN is used, functions are padded in order to create a
    // hotpatchable image.
    const bool isCodeSection =
        (sec->header.Characteristics & IMAGE_SCN_CNT_CODE) &&
        (sec->header.Characteristics & IMAGE_SCN_MEM_READ) &&
        (sec->header.Characteristics & IMAGE_SCN_MEM_EXECUTE);
    uint32_t padding = isCodeSection ? config->functionPadMin : 0;

    for (Chunk *c : sec->chunks) {
      if (padding && c->isHotPatchable())
        virtualSize += padding;
      c->setRVA(rva + virtualSize);
      virtualSize += c->getSize();
      if (c->hasData)
        rawSize = virtualSize;
    }
    if (virtualSize > UINT32_MAX)
      error("section larger than 4 GiB: " + sec->name);
    sec->header.VirtualSize = virtualSize;
    sec->header.SizeOfRawData = rawSize;
    if (rawSize != 0)
      sec->header.PointerToRawData = fileSize;
    rva += virtualSize;
    fileSize += rawSize;
  }
  sizeOfImage = rva;

  // Assign addresses to sections in MergeChunks.
  for (MergeChunk *mc : MergeChunk::instances)
    if (mc)
      mc->assignSubsectionRVAs();
}

void Writer::writeHeader() {
  uint8_t *buf = buffer->getBufferStart();

  Menuet02Header hdr;
  hdr.start = dyn_cast<Defined>(symtab->findUnderscore("__start"))->getRVA();
  hdr.end = fileSize;
  hdr.memory = sizeOfImage + config->stackSize;
  hdr.stack = sizeOfImage + config->stackSize;
  hdr.params = dyn_cast<Defined>(symtab->findUnderscore("__params"))->getRVA();
  hdr.path = dyn_cast<Defined>(symtab->findUnderscore("__path"))->getRVA();

  // Prevent UB, reinterpret cast sucks
  auto put32 = [&buf](uint32_t v) {
    *buf++ = (v & 0x000000ff) >> 0;
    *buf++ = (v & 0x0000ff00) >> 8;
    *buf++ = (v & 0x00ff0000) >> 16;
    *buf++ = (v & 0xff000000) >> 24;
  };

  memcpy(buf, hdr.magic, sizeof(hdr.magic));
  buf += sizeof(hdr.magic);

  put32(hdr.version); // version
  put32(hdr.start);
  put32(hdr.end);
  put32(hdr.memory);
  put32(hdr.stack);
  put32(hdr.params);
  put32(hdr.path);
}

void Writer::openFile(StringRef path) {
  buffer = CHECK(
      FileOutputBuffer::create(path, fileSize, FileOutputBuffer::F_executable),
      "failed to open " + path);
}

// Write section contents to a mmap'ed file.
void Writer::writeSections() {
  // Record the number of sections to apply section index relocations
  // against absolute symbols. See applySecIdx in Chunks.cpp..
  DefinedAbsolute::numOutputSections = outputSections.size();

  uint8_t *buf = buffer->getBufferStart();
  for (OutputSection *sec : outputSections) {
    uint8_t *secBuf = buf + sec->getFileOff();
    // Fill gaps between functions in .text with INT3 instructions
    // instead of leaving as NUL bytes (which can be interpreted as
    // ADD instructions).
    if (sec->header.Characteristics & IMAGE_SCN_CNT_CODE)
      memset(secBuf, 0xCC, sec->getRawSize());
    parallelForEach(sec->chunks, [&](Chunk *c) {
      c->writeTo(secBuf + c->getRVA() - sec->getRVA());
    });
  }
}

PartialSection *Writer::createPartialSection(StringRef name,
                                             uint32_t outChars) {
  PartialSection *&pSec = partialSections[{name, outChars}];
  if (pSec)
    return pSec;
  pSec = make<PartialSection>(name, outChars);
  return pSec;
}
