#include "LWCore/LWUnicodeIterator.h"

const uint32_t LWUTF8Iterator::EmptyHash;
const uint32_t LWUTF8Iterator::MaxCodePoints;
const uint32_t LWUTF16Iterator::EmptyHash;
const uint32_t LWUTF16Iterator::MaxCodePoints;
const uint32_t LWUTF32Iterator::EmptyHash;
const uint32_t LWUTF32Iterator::MaxCodePoints;

std::ostream &operator << (std::ostream &o, const LWUTF8Iterator &Iter) {
	LWUTF8Iterator C = Iter;
	for (; !C.AtEnd(); ++C) {
		const char8_t *p = C.GetPosition();
		o.write((char*)p, LWUTF8Iterator::CodePointUnitSize(p)); //write one utf-8 character at a time.
	}
	return o;
}

std::ostream &operator << (std::ostream &o, const LWUTF16Iterator &Iter) {
	char8_t Buffer[4]; //we Convert to utf-8 format to pass into ostream.
	LWUTF16Iterator C = Iter;
	for (; !C.AtEnd(); ++C) {
		uint32_t Cnt = LWUTF8Iterator::EncodeCodePoint(Buffer, sizeof(Buffer), *C);
		o.write((char*)Buffer, Cnt);
	}
	return o;
}

std::ostream &operator << (std::ostream &o, const LWUTF32Iterator &Iter) {
	char8_t Buffer[4]; //we convert to utf-8 format to pass into ostream.
	LWUTF32Iterator C = Iter;
	for (; !C.AtEnd(); ++C) {
		uint32_t Cnt = LWUTF8Iterator::EncodeCodePoint(Buffer, sizeof(Buffer), *C);
		o.write((char*)Buffer, Cnt);
	}
	return o;
}