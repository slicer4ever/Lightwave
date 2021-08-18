#include "LWCore/LWMath.h"
#include "LWCore/LWVector.h"

bool LWMatrix4_UseDXOrtho = false;

LWVector4f LWHSLAtoRGBA(const LWVector4f &HSLA) {
	//Source: https://www.rapidtables.com/convert/color/hsl-to-rgb.html
	if (HSLA.y <= 0.0f) return LWVector4f(0.0f, 0.0f, 0.0f, HSLA.w);
	float hh = fmodf(HSLA.x, 360.0f) / 60.0f;
	int32_t i = (int32_t)hh;

	float ff = hh - i;
	float c = (1.0f - fabs(2.0f * HSLA.z - 1.0f)) * HSLA.y;
	float x = c * (1.0f - ff);
	float m = HSLA.z - c * 0.5f;
	if (i == 0) return LWVector4f(c + m, x + m, m, HSLA.w);
	else if (i == 1) return LWVector4f(x + m, c + m, m, HSLA.w);
	else if (i == 2) return LWVector4f(m, c + m, x + m, HSLA.w);
	else if (i == 3) return LWVector4f(m, x + m, c + m, HSLA.w);
	else if (i == 4) return LWVector4f(x + m, m, c + m, HSLA.w);
	return LWVector4f(c + m, m, x + m, HSLA.w);
}

LWVector4f LWRGBAtoHSLA(const LWVector4f &RGBA) {
	const float e = std::numeric_limits<float>::epsilon();
	float cMax = std::max<float>(std::max<float>(RGBA.x, RGBA.y), RGBA.z);
	float cMin = std::min<float>(std::min<float>(RGBA.x, RGBA.y), RGBA.z);
	float cDelta = cMax - cMin;
	float l = (cMax + cMin) * 0.5f;
	float h = 0.0f;
	if (cDelta <= e) return LWVector4f(0.0f, 0.0f, l, RGBA.w);

	float s = cDelta / (1.0f - fabs(2.0f * l - 1.0f));
	if (RGBA.x >= cMax) h = (RGBA.y - RGBA.z) / cDelta;
	else if (RGBA.y >= cMax) h = 2.0f + (RGBA.z - RGBA.x) / cDelta;
	else h = 4.0f + (RGBA.x - RGBA.y) / cDelta;
	h *= 60.0f;
	if (h < 0.0f) h += 360.0f;
	return LWVector4f(h, s, l, RGBA.w);
}

LWVector4f LWHSLAInterpolate(const LWVector4f &Src, const LWVector4f &Dst, float p) {
	const float i360 = 1.0f / 360.0f;
	float sh = Src.x * 360.0f;
	float dh = Dst.x * 360.0f;
	float Dis = dh - sh;
	if (Dis < -180.0f) Dis += 360.0f;
	else if (Dis > 180.0f) Dis -= 360.0f;
	float h = sh + Dis * p;
	if (h < 0.0f) h += 360.0f;
	else if (h >= 360.0f) h -= 360.0f;
	return LWVector4f(h*i360, Src.y + (Dst.y - Src.y) * p, Src.z + (Dst.z - Src.z) * p, Src.w + (Dst.w - Src.w) * p);
}

uint8_t LWNext2N(uint8_t v) {
	v--;
	v |= v >> 1;
	v |= v >> 2;
	v |= v >> 4;
	v++;
	return v;
}

uint16_t LWNext2N(uint16_t v) {
	v--;
	v |= v >> 1;
	v |= v >> 2;
	v |= v >> 4;
	v |= v >> 8;
	v++;
	return v;
}

uint32_t LWNext2N(uint32_t v) {
	v--;
	v |= v >> 1;
	v |= v >> 2;
	v |= v >> 4;
	v |= v >> 8;
	v |= v >> 16;
	v++;
	return v;
}

uint64_t LWNext2N(uint64_t v) {
	v--;
	v |= v >> 1;
	v |= v >> 2;
	v |= v >> 4;
	v |= v >> 8;
	v |= v >> 16;
	v |= v >> 32;
	v++;
	return v;
}