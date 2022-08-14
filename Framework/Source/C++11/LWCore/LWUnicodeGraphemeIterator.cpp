#include "LWCore/LWUnicodeGraphemeIterator.h"
#include "LWCore/LWGraphemeTable.h"
#include "LWCore/LWByteBuffer.h"
#include <algorithm>
#include <vector>

//LWGraphemeBlock
bool operator<(const LWGraphemeBlock &B, uint32_t CodePoint) {
	return B.m_Max < CodePoint;
}

bool LWGraphemeBlock::operator<(const LWGraphemeBlock &B) {
	return m_Max <= B.m_Max;
}

uint32_t LWGraphemeBlock::InRange(uint32_t CodePoint) const {
	return CodePoint >= m_Min && CodePoint <= m_Max ? m_Type : LWGRAPHEME_ANY;
}

LWGraphemeBlock::LWGraphemeBlock(uint32_t CodePoint, uint32_t Type) : m_Min(CodePoint), m_Max(CodePoint), m_Type(Type) {}

LWGraphemeBlock::LWGraphemeBlock(uint32_t MinCodePoint, uint32_t MaxCodePoint, uint32_t Type) : m_Min(MinCodePoint), m_Max(MaxCodePoint), m_Type(Type) {}

//LWGraphemeTable:
const uint32_t LWGraphemeTable::BlockSize;
const uint32_t LWGraphemeTable::TableBlocks;


bool LWGraphemeTable::EvaluateGraphemeRules(uint32_t LeftGraphemeType, uint32_t RightGraphemeType) {
	//Follows rules and matrix as outlined here: https://www.unicode.org/Public/UCD/latest/ucd/auxiliary/GraphemeBreakTest.html
	//Rule table matrix, true means break, false means stay.
	const bool RulesMatrix[LWGRAPHEME_TYPECOUNT][LWGRAPHEME_TYPECOUNT] = {
		//Any x ANY,   CR,   LF,    Control, Extend, ZWJ,   RI,    Prepend, Spacing, L,     V,     T,     LV,    LVT
		{       true,  true, true,  true,    false,  false, true,  true,    false,   true,  true,  true,  true,  true },
		//CR x "                                                                                                   "
		{       true,  true, false, true,    true,   true,  true,  true,    true,    true,  true,  true,  true,  true },
		//LF x " "
		{       true,  true, true,  true,    true,   true,  true,  true,    true,    true,  true,  true,  true,  true },
		//Control x " "
		{       true,  true, true,  true,    true,   true,  true,  true,    true,    true,  true,  true,  true,  true },
		//Extend x " "
		{       true,  true, true,  true,    false,  false, true,  true,    false,   true,  true,  true,  true,  true },
		//ZWJ x " "
		{       true,  true, true,  true,    false,  false, true,  true,    false,   true,  true,  true,  true,  true },
		//RI x " "
		{       true,  true, true,  true,    false,  false, false, true,    false,   true,  true,  true,  true,  true },
		//Prepend x " "
		{       false, true, true,  true,    false,  false, false, false,   false,   false, false, false, false, false},
		//Spacing x " "
		{       true,  true, true,  true,    false,  false, true,  true,    false,   true,  true,  true,  true,  true },
		//L x " "
		{       true,  true, true,  true,    false,  false, true,  true,    false,   false, false, true,  false, false },
		//V x " "
		{       true,  true, true,  true,    false,  false, true,  true,    false,   true,  false, false, true,  true },
		//T x " "
		{       true,  true, true,  true,    false,  false, true,  true,    false,   false, false, true,  false, false },
		//LV x " "
		{       true,  true, true,  true,    false,  false, true,  true,    false,   true,  false, false, true,  true },
		//LVT x " "
		{       true,  true, true,  true,    false,  false, true,  true,    false,   true,  true,  false, true,  true }
	};
	LWVerify(LeftGraphemeType < LWGRAPHEME_TYPECOUNT && RightGraphemeType < LWGRAPHEME_TYPECOUNT);
	return RulesMatrix[LeftGraphemeType][RightGraphemeType];
}

uint32_t LWGraphemeTable::GetCodepointType(uint32_t CodePoint) {
	uint32_t Block = CodePoint / BlockSize;
	if (Block >= LWGraphemeTable::TableBlocks) return LWGRAPHEME_ANY;
	const LWGraphemeBlockTable &B = LWGBlockTable[Block];
	if (!B.m_BlockCount) return LWGRAPHEME_ANY;
	const LWGraphemeBlock *Lower = LWGBlocks + B.m_BlockOffset;
	const LWGraphemeBlock *Upper = LWGBlocks + (B.m_BlockOffset + B.m_BlockCount);
	//Use binary search to find table, since table is in linear order:
	const LWGraphemeBlock *Res = std::lower_bound(Lower, Upper, CodePoint);
	if (Res == Upper) return LWGRAPHEME_ANY;
	return Res->InRange(CodePoint);
}

uint32_t LWGraphemeTable::GenerateTable(char *Buffer, uint32_t BufferSize, LWUTF8Iterator &FileIter) {
	const char *TypeNames[] = { "CR", "LF", "Control", "Extend", "ZWJ", "Regional_Indicator", "Prepend", "SpacingMark", "L", "V", "T", "LV", "LVT" };
	const uint32_t TypeIDs[] = { LWGRAPHEME_CR, LWGRAPHEME_LF, LWGRAPHEME_CONTROL, LWGRAPHEME_EXTEND, LWGRAPHEME_ZWJ, LWGRAPHEME_RI, LWGRAPHEME_PREPEND, LWGRAPHEME_SPACING, LWGRAPHEME_L, LWGRAPHEME_V, LWGRAPHEME_T, LWGRAPHEME_LV, LWGRAPHEME_LVT };
	const uint32_t TypeCount = sizeof(TypeIDs) / sizeof(uint32_t);
	const uint32_t LinePackSize = 10; //Number of elements to place on each line.
	uint32_t BlockIndex = 0;
	LWUTF8Iterator Line = FileIter.NextLine(true);
	std::vector<LWGraphemeBlock> m_BlockList;
	std::vector<LWGraphemeBlockTable> m_TableList;
	for(LWUTF8Iterator NextLine = Line.NextLine(); !Line.AtEnd(); Line = NextLine, NextLine.AdvanceLine()) {
		LWUTF8Iterator Word = LWUTF8Iterator(Line, NextLine).AdvanceWord(true);
		if(Word.AtEnd() || *Word=='#') continue;
		uint32_t Min, Max;
		uint32_t r = sscanf((char*)Word.GetPosition(), "%x..%x", &Min, &Max);
		if (r == 0) {
			return 0; //something went wrong.
		} else if (r == 1) Max = Min;
		Word.AdvanceWord();
		if(Word.AtEnd() || *Word=='#') continue;
		if (*Word != ';') {
			return 0; //Something went wrong?
		}
		Word.AdvanceWord();
		if(Word.AtEnd() || *Word=='#') continue;
		uint32_t i = 0;
		for (; i < TypeCount; i++) {
			LWUTF8Iterator TypeNameIter;
			uint32_t Len, RawLen;
			if (!LWUTF8Iterator::Create(TypeNameIter, (const char8_t*)TypeNames[i], Len, RawLen)) return 0;
			if (Word.Compare(TypeNameIter, Len)) break;
		}
		if (i >= TypeCount) return 0; //Something went wrong.
		m_BlockList.push_back({ Min, Max, TypeIDs[i] });
	}
	m_TableList.reserve(TableBlocks);
	//Split any blocks that cross block tables:
	for(uint32_t i = 0; i < m_BlockList.size();i++){
		auto &B = m_BlockList[i];
		uint32_t minBlock = B.m_Min / BlockSize;
		uint32_t maxBlock = B.m_Max / BlockSize;
		if(minBlock==maxBlock) continue;
		uint32_t maxID = B.m_Max;
		uint32_t blockMin = B.m_Min;
		uint32_t blockMax = (minBlock+1) * BlockSize - 1;
		B.m_Max = blockMax;
		//future splits will automatically be handled in loop:
		m_BlockList.push_back({ blockMax + 1, maxID, B.m_Type });
	}
	std::sort(m_BlockList.begin(), m_BlockList.end());
	uint32_t MaxBlocks = 0;
	//Build Table:
	for (uint32_t i = 0; i < TableBlocks; i++) {
		//Find first block, and total blocks:
		uint32_t BlockOffset = 0;
		uint32_t BlockCount = 0;
		for (uint32_t n = 0; n < (uint32_t)m_BlockList.size(); n++) {
			uint32_t Block = m_BlockList[n].m_Min / BlockSize;
			if (Block != i) continue;
			if (!BlockCount) BlockOffset = n;
			BlockCount++;
		}
		MaxBlocks = std::max<uint32_t>(MaxBlocks, BlockCount);
		m_TableList.push_back({ BlockCount, BlockOffset });
	}
	uint32_t o = 0;
	o += snprintf(Buffer + o, BufferSize > o ? BufferSize - o : 0, "//!!This file is autogenerated by LWGraphemeTable::GenerateTable. BlockSize = %d!!\n", BlockSize);
	o += snprintf(Buffer + o, BufferSize > o ? BufferSize - o : 0, "const LWGraphemeBlockTable LWGBlockTable[] = {");
	for (uint32_t i = 0; i < m_TableList.size(); i++) {
		LWGraphemeBlockTable &T = m_TableList[i];
		o += snprintf(Buffer + o, BufferSize > o ? BufferSize - o : 0, "%s%s", i == 0 ? "" : ",", (i % LinePackSize) == 0 ? "\n" : "");

		if (T.m_BlockCount == 0) o += snprintf(Buffer + o, BufferSize > o ? BufferSize - o : 0, "{}");
		else o += snprintf(Buffer + o, BufferSize > o ? BufferSize - o : 0, "{%d, %d}", T.m_BlockCount, T.m_BlockOffset);
	}
	o += snprintf(Buffer + o, BufferSize > o ? BufferSize - o : 0, "\n};\nconst LWGraphemeBlock LWGBlocks[] = {");
	for (uint32_t i = 0; i < m_BlockList.size(); i++) {
		LWGraphemeBlock &B = m_BlockList[i];
		o += snprintf(Buffer + o, BufferSize > o ? BufferSize - o : 0, "%s%s", i == 0 ? "" : ",", (i % LinePackSize) == 0 ? "\n" : "");
		if (B.m_Min == B.m_Max) o += snprintf(Buffer + o, BufferSize > o ? BufferSize - o : 0, "{0x%X, %d}", B.m_Min, B.m_Type);
		else o += snprintf(Buffer + o, BufferSize > o ? BufferSize - o : 0, "{0x%X, 0x%X, %d}", B.m_Min, B.m_Max, B.m_Type);
	}
	o += snprintf(Buffer + o, BufferSize > o ? BufferSize - o : 0, "\n};");
	return o;
}
