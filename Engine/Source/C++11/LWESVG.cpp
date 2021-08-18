#include "LWESVG.h"
#include "LWEXML.h"
#include <LWPlatform/LWFileStream.h>
#include <LWCore/LWMath.h>
#include <LWCore/LWSVector.h>
#include <LWCore/LWSMatrix.h>
#include "LWELogger.h"

//LWESVGStyle:
LWESVGStyle &LWESVGStyle::SetxPos(float Position, bool bIsPercent) {
	m_Position.x = Position;
	m_Flag |= bIsPercent ? xPosPercent : 0;
	m_HasFlags |= hasXPos;
	return *this;
}

LWESVGStyle &LWESVGStyle::SetyPos(float Position, bool bIsPercent) {
	m_Position.y = Position;
	m_Flag |= bIsPercent ? yPosPercent : 0;
	m_HasFlags |= hasYPos;
	return *this;
}

LWESVGStyle &LWESVGStyle::SetxSize(float Size, bool bIsPercent, bool bIsAuto) {
	m_Size.x = Size;
	m_Flag |= (bIsPercent ? xSizePercent : 0) | (bIsAuto ? WidthAuto : 0);
	m_HasFlags |= hasXSize;
	return *this;
}

LWESVGStyle &LWESVGStyle::SetySize(float Size, bool bIsPercent, bool bIsAuto) {
	m_Size.y = Size;
	m_Flag |= (bIsPercent ? ySizePercent : 0) | (bIsAuto ? HeightAuto : 0);
	m_HasFlags = hasYSize;
	return *this;
}

LWESVGStyle &LWESVGStyle::SetxRadi(float Radi, bool bIsPercent, bool bIsAuto) {
	m_Radius.x = Radi;
	m_Flag |= (bIsPercent ? xRadiPercent : 0) | (bIsAuto ? xRadiAuto : 0);
	m_HasFlags |= hasXRadi;
	return *this;
}

LWESVGStyle &LWESVGStyle::SetyRadi(float Radi, bool bIsPercent, bool bIsAuto) {
	m_Radius.y = Radi;
	m_Flag |= (bIsPercent ? yRadiPercent : 0) | (bIsAuto ? yRadiAuto : 0);
	m_HasFlags |= hasYRadi;
	return *this;
}

LWESVGStyle &LWESVGStyle::SetLineCaps(uint32_t LineCaps) {
	LWBitFieldSet(LineCap, m_Flag, LineCaps);
	m_HasFlags |= hasLineCap;
	return *this;
}

LWESVGStyle &LWESVGStyle::SetFillColor(const LWVector4f &Color) {
	m_FillColor = Color;
	m_HasFlags |= hasFillColor;
	return *this;
}

LWESVGStyle &LWESVGStyle::SetStrokeColor(const LWVector4f &Color) {
	m_StrokeColor = Color;
	m_HasFlags |= hasStrokeColor;
	return *this;
}

LWESVGStyle &LWESVGStyle::SetStrokeWidth(const float StokeWidth) {
	m_StrokeWidth = StokeWidth;
	m_HasFlags |= hasStrokeWidth;
	return *this;
}

uint32_t LWESVGStyle::GetLineCap(void) const {
	return LWBitFieldGet(LineCap, m_Flag);
}

bool LWESVGStyle::isWidthAuto(void) const {
	return (m_Flag & WidthAuto) != 0;
}

bool LWESVGStyle::isHeightAuto(void) const {
	return (m_Flag & HeightAuto) != 0;
}

bool LWESVGStyle::isxRadiAuto(void) const {
	return (m_Flag & xRadiAuto) != 0;
}

bool LWESVGStyle::isyRadiAuto(void) const {
	return (m_Flag & yRadiAuto) != 0;
}

bool LWESVGStyle::Has(uint32_t HasFlag) const {
	return (m_HasFlags & HasFlag) != 0;
}

LWESVGStyle::LWESVGStyle(const LWESVGStyle &Style, const LWESVGStyle &InheritanceStyle) : m_HasFlags(Style.m_HasFlags) {
	m_Position.x = Has(hasXPos) ? Style.m_Position.x : InheritanceStyle.m_Position.x;
	m_Flag |= (Has(hasXPos) ? Style.m_Flag : InheritanceStyle.m_Flag) & xPosPercent;

	m_Position.y = Has(hasYPos) ? Style.m_Position.y : InheritanceStyle.m_Position.y;
	m_Flag |= (Has(hasYPos) ? Style.m_Flag : InheritanceStyle.m_Flag) & yPosPercent;

	m_Size.x = Has(hasXSize) ? Style.m_Size.x : InheritanceStyle.m_Size.x;
	m_Flag |= (Has(hasXSize) ? Style.m_Flag : InheritanceStyle.m_Flag) & (xSizePercent|WidthAuto);
	
	m_Size.y = Has(hasYSize) ? Style.m_Size.y : InheritanceStyle.m_Size.y;
	m_Flag |= (Has(hasYSize) ? Style.m_Flag : InheritanceStyle.m_Flag) & (ySizePercent|HeightAuto);

	m_Radius.x = Has(hasXRadi) ? Style.m_Radius.x : InheritanceStyle.m_Radius.y;
	m_Flag |= (Has(hasXRadi) ? Style.m_Flag : InheritanceStyle.m_Flag) & (xRadiPercent|xRadiAuto);

	m_Radius.y = Has(hasYRadi) ? Style.m_Radius.y : InheritanceStyle.m_Radius.y;
	m_Flag |= (Has(hasYRadi) ? Style.m_Flag : InheritanceStyle.m_Flag) & (yRadiPercent|yRadiAuto);

	m_Flag |= (Has(hasLineCap) ? Style.m_Flag : InheritanceStyle.m_Flag) & LineCapBits;

	m_FillColor = Has(hasFillColor) ? Style.m_FillColor : InheritanceStyle.m_FillColor;
	m_StrokeColor = Has(hasStrokeColor) ? Style.m_StrokeColor : InheritanceStyle.m_StrokeColor;

	m_Opacity = Style.m_Opacity * InheritanceStyle.m_Opacity;

	m_StrokeWidth = Has(hasStrokeWidth) ? Style.m_StrokeWidth : InheritanceStyle.m_StrokeWidth;

	m_HasFlags |= InheritanceStyle.m_HasFlags;
}


//LWESVGElement:
uint32_t LWESVGElement::GetType(void) const {
	return LWBitFieldGet(Type, m_Flag);
}

LWESVGElement::LWESVGElement(const LWESVGStyle &Style, uint32_t Type) : m_Style(Style), m_Flag(Type) {}

//LWESVG:
bool LWESVG::LoadFile(LWESVG &Object, const LWUTF8Iterator &Filepath, LWAllocator &Allocator, LWFileStream *ExistingStream) {
	LWEXML X;
	if (!LWEXML::LoadFile(X, Allocator, Filepath, true, ExistingStream)) return false;
	X.PushParser("svg", &LWESVG::XMLParser, &Object);
	X.Process();
	return true;
}

bool LWESVG::XMLParser(LWEXMLNode *Node, void *UserData, LWEXML *XML) {
	LWESVG *SVG = (LWESVG*)UserData;

	LWEXMLAttribute *widthAttr = Node->FindAttribute("width");
	LWEXMLAttribute *heightAttr = Node->FindAttribute("height");
	LWVector2f SVGSize = LWVector2f();
	bool bIsPercent = false;
	if (widthAttr) SVGSize.x = ParseLengthValue(widthAttr->GetValue(), bIsPercent);
	if (heightAttr) SVGSize.y = ParseLengthValue(heightAttr->GetValue(), bIsPercent);
	*SVG = LWESVG(SVGSize);
	for (LWEXMLNode *C = Node->m_FirstChild; C; C = C->m_Next) {
		if (!XMLParseBaseElement(C, *SVG, -1, XML)) {
			LWELogCritical<256>("failed to parse node: '{}'", C->GetName());
		}
	}	
	return true;
}

bool LWESVG::XMLParseBaseElement(LWEXMLNode *Node, LWESVG &SVG, uint32_t ParentElemID, LWEXML *XML) {
	uint32_t ID = -1;
	uint32_t NodeType = Node->GetName().CompareList("metadata", "desc", "g", "defs", "circle", "ellipse", "rect", "line", "path", "polygon", "polyline");
	if (NodeType == -1) return false;
	else if (NodeType == 0) return true; //we don't care about metadata, or it's children.
	else if (NodeType == 1) return true; //we don't care about description.
	LWESVGElement Elem;
	bool Result = false;
	if (NodeType == 2) Result = XMLParseGroupElement(Node, Elem, ParentElemID, XML);
	else if (NodeType == 3) Result = XMLParseDefineElement(Node, Elem, ParentElemID, XML);
	else if (NodeType == 4) Result = XMLParseCircleElement(Node, Elem, ParentElemID, XML);
	else if (NodeType == 5) Result = XMLParseEllipseElement(Node, Elem, ParentElemID, XML);
	else if (NodeType == 6) Result = XMLParseRectElement(Node, Elem, ParentElemID, XML);
	else if (NodeType == 7) Result = XMLParseLineElement(Node, Elem, ParentElemID, XML);
	else if (NodeType == 8) Result = XMLParsePathElement(Node, Elem, ParentElemID, XML);
	else if (NodeType == 9) Result = XMLParsePolygonElement(Node, Elem, ParentElemID, XML);
	else if (NodeType == 10) Result = XMLParsePolylineElement(Node, Elem, ParentElemID, XML);

	if (!Result) return false;
	ID = SVG.PushElementLast(Elem, ParentElemID);

	for (LWEXMLNode *C = Node->m_FirstChild; C; C = C->m_Next) {
		if (!XMLParseBaseElement(C, SVG, ID, XML)) {
			LWELogCritical<256>("failed to parse node: '{}'", C->GetName());
		}
	}
	return false;
}

bool LWESVG::XMLParseCoreAttributes(LWEXMLNode *Node, LWESVGElement &Elem, uint32_t ParentElemID, LWEXML *XML) {
	LWEXMLAttribute *idAttr = Node->FindAttribute("id");
	LWEXMLAttribute *classAttr = Node->FindAttribute("class");
	if (idAttr) Elem.m_IDHash = idAttr->GetValue().Hash();
	if (classAttr) Elem.m_ClassHash = classAttr->GetValue().Hash();
	return true;
}

bool LWESVG::XMLParseGeometryAttributes(LWEXMLNode *Node, LWESVGElement &Elem, uint32_t ParentElemID, LWEXML *XML) {
	LWEXMLAttribute *cxAttr = Node->FindAttribute("cx");
	LWEXMLAttribute *cyAttr = Node->FindAttribute("cy");
	LWEXMLAttribute *heightAttr = Node->FindAttribute("height");
	LWEXMLAttribute *widthAttr = Node->FindAttribute("width");
	LWEXMLAttribute *rAttr = Node->FindAttribute("r");
	LWEXMLAttribute *rxAttr = Node->FindAttribute("rx");
	LWEXMLAttribute *ryAttr = Node->FindAttribute("ry");
	LWEXMLAttribute *xAttr = Node->FindAttribute("x");
	LWEXMLAttribute *yAttr = Node->FindAttribute("y");
	LWESVGStyle &ES = Elem.m_Style;
	bool bIsPercent = false;
	if (xAttr) {
		float Value = ParseLengthValue(xAttr->GetValue(), bIsPercent);
		ES.SetxPos(Value, bIsPercent);
	}
	if (yAttr) {
		float Value = ParseLengthValue(yAttr->GetValue(), bIsPercent);
		ES.SetyPos(Value, bIsPercent);
	}
	if (cxAttr) {
		float Value = ParseLengthValue(cxAttr->GetValue(), bIsPercent);
		ES.SetxPos(Value, bIsPercent);
	}
	if (cyAttr) {
		float Value = ParseLengthValue(cyAttr->GetValue(), bIsPercent);
		ES.SetyPos(Value, bIsPercent);
	}
	if (rAttr) {
		float Value = ParseLengthValue(rAttr->GetValue(), bIsPercent);
		ES.SetxRadi(Value, bIsPercent, false);
	}
	if (widthAttr) {
		LWUTF8I wValue = widthAttr->GetValue().NextWord(true);
		if (wValue.Compare("auto")) ES.SetxSize(0.0f, false, true);
		else {
			float Value = ParseLengthValue(wValue, bIsPercent);
			ES.SetxSize(Value, bIsPercent, false);
		}
	}
	if (heightAttr) {
		LWUTF8I hValue = heightAttr->GetValue().NextWord(true);
		if (hValue.Compare("auto")) ES.SetySize(0.0f, false, true);
		else {
			float Value = ParseLengthValue(hValue, bIsPercent);
			ES.SetySize(Value, bIsPercent, false);
		}
	}
	if (rxAttr) {
		LWUTF8I rxValue = rxAttr->GetValue().NextWord(true);
		if (rxValue.Compare("auto")) ES.SetxRadi(0.0f, false, true);
		else {
			float Value = ParseLengthValue(rxValue, bIsPercent);
			ES.SetxRadi(Value, bIsPercent, false);
		}
	}
	if (ryAttr) {
		LWUTF8I ryValue = ryAttr->GetValue().NextWord(true);
		if (ryValue.Compare("auto")) ES.SetyRadi(0.0f, false, true);
		else {
			float Value = ParseLengthValue(ryValue, bIsPercent);
			ES.SetyRadi(Value, bIsPercent, false);
		}
	}

	return true;
}

bool LWESVG::XMLParsePresentationAttributes(LWEXMLNode *Node, LWESVGElement &Elem, uint32_t ParentElemID, LWEXML *XML) {
	LWEXMLAttribute *fillAttr = Node->FindAttribute("fill");
	LWEXMLAttribute *OpacityAttr = Node->FindAttribute("opacity");
	LWEXMLAttribute *StrokeLineCapAttr = Node->FindAttribute("stroke-linecap");
	LWEXMLAttribute *StrokeWidth = Node->FindAttribute("stroke-width");

	LWESVGStyle &ES = Elem.m_Style;
	bool bIsPercent = false;
	if (fillAttr) ES.SetFillColor(ParseColorValue(fillAttr->GetValue(), ES.m_FillColor.w));
	if (OpacityAttr) ES.m_Opacity = ParseLengthValue(OpacityAttr->GetValue(), bIsPercent);
	if (StrokeLineCapAttr) {
		uint32_t LineCap = StrokeLineCapAttr->GetValue().NextWord(true).CompareList("butt", "round", "square");
		if (LineCap != -1) ES.SetLineCaps(LineCap);
	}
	if (StrokeWidth) ES.SetStrokeWidth(ParseLengthValue(StrokeWidth->GetValue(), bIsPercent));
	return true;
}

bool LWESVG::XMLParseGroupElement(LWEXMLNode *Node, LWESVGElement &Elem, uint32_t ParentElemID, LWEXML *XML) {
	Elem = LWESVGElement(LWESVGStyle(), LWESVGElement::Group);
	if (!XMLParseCoreAttributes(Node, Elem, ParentElemID, XML)) return false;
	if (!XMLParseGeometryAttributes(Node, Elem, ParentElemID, XML)) return false;
	if (!XMLParsePresentationAttributes(Node, Elem, ParentElemID, XML)) return false;
	return true;
}

bool LWESVG::XMLParseDefineElement(LWEXMLNode *Node, LWESVGElement &Elem, uint32_t ParentElemID, LWEXML *XML) {
	Elem = LWESVGElement(LWESVGStyle(), LWESVGElement::Group);
	if (!XMLParseCoreAttributes(Node, Elem, ParentElemID, XML)) return false;
	if (!XMLParseGeometryAttributes(Node, Elem, ParentElemID, XML)) return false;
	if (!XMLParsePresentationAttributes(Node, Elem, ParentElemID, XML)) return false;
	return true;
}

bool LWESVG::XMLParseRectElement(LWEXMLNode *Node, LWESVGElement &Elem, uint32_t ParentElemID, LWEXML *XML) {
	Elem = LWESVGElement(LWESVGStyle(), LWESVGElement::Rect);
	if (!XMLParseCoreAttributes(Node, Elem, ParentElemID, XML)) return false;
	if (!XMLParseGeometryAttributes(Node, Elem, ParentElemID, XML)) return false;
	if (!XMLParsePresentationAttributes(Node, Elem, ParentElemID, XML)) return false;
	return true;
}

bool LWESVG::XMLParseCircleElement(LWEXMLNode *Node, LWESVGElement &Elem, uint32_t ParentElemID, LWEXML *XML) {
	Elem = LWESVGElement(LWESVGStyle(), LWESVGElement::Circle);
	if (!XMLParseCoreAttributes(Node, Elem, ParentElemID, XML)) return false;
	if (!XMLParseGeometryAttributes(Node, Elem, ParentElemID, XML)) return false;
	if (!XMLParsePresentationAttributes(Node, Elem, ParentElemID, XML)) return false;
	return true;
}

bool LWESVG::XMLParseEllipseElement(LWEXMLNode *Node, LWESVGElement &Elem, uint32_t ParentElemID, LWEXML *XML) {
	Elem = LWESVGElement(LWESVGStyle(), LWESVGElement::Ellipse);
	if (!XMLParseCoreAttributes(Node, Elem, ParentElemID, XML)) return false;
	if (!XMLParseGeometryAttributes(Node, Elem, ParentElemID, XML)) return false;
	if (!XMLParsePresentationAttributes(Node, Elem, ParentElemID, XML)) return false;
	return true;
}

bool LWESVG::XMLParseLineElement(LWEXMLNode *Node, LWESVGElement &Elem, uint32_t ParentElemID, LWEXML *XML) {
	Elem = LWESVGElement(LWESVGStyle(), LWESVGElement::Line);
	if (!XMLParseCoreAttributes(Node, Elem, ParentElemID, XML)) return false;
	if (!XMLParseGeometryAttributes(Node, Elem, ParentElemID, XML)) return false;
	if (!XMLParsePresentationAttributes(Node, Elem, ParentElemID, XML)) return false;
	return true;
}

bool LWESVG::XMLParsePathElement(LWEXMLNode *Node, LWESVGElement &Elem, uint32_t ParentElemID, LWEXML *XML) {
	Elem = LWESVGElement(LWESVGStyle(), LWESVGElement::Path);
	if (!XMLParseCoreAttributes(Node, Elem, ParentElemID, XML)) return false;
	if (!XMLParseGeometryAttributes(Node, Elem, ParentElemID, XML)) return false;
	if (!XMLParsePresentationAttributes(Node, Elem, ParentElemID, XML)) return false;
	return true;
}

bool LWESVG::XMLParsePolygonElement(LWEXMLNode *Node, LWESVGElement &Elem, uint32_t ParentElemID, LWEXML *XML) {
	Elem = LWESVGElement(LWESVGStyle(), LWESVGElement::Polygon);
	if (!XMLParseCoreAttributes(Node, Elem, ParentElemID, XML)) return false;
	if (!XMLParseGeometryAttributes(Node, Elem, ParentElemID, XML)) return false;
	if (!XMLParsePresentationAttributes(Node, Elem, ParentElemID, XML)) return false;
	return true;
}

bool LWESVG::XMLParsePolylineElement(LWEXMLNode *Node, LWESVGElement &Elem, uint32_t ParentElemID, LWEXML *XML) {
	Elem = LWESVGElement(LWESVGStyle(), LWESVGElement::Polyline);
	if (!XMLParseCoreAttributes(Node, Elem, ParentElemID, XML)) return false;
	if (!XMLParseGeometryAttributes(Node, Elem, ParentElemID, XML)) return false;
	if (!XMLParsePresentationAttributes(Node, Elem, ParentElemID, XML)) return false;
	return true;
}


float LWESVG::ParseLengthValue(const LWUTF8Iterator &Value, bool &bIsPercent) {
	const float PxEMScalar = 1.0f / 16.0f;
	const float PxPtScalar = 0.75f;
	const float PxEXScalar = PxEMScalar * 0.5f; //Half the size of an em generally.
	const float DPI = 96.0f; //Standard dpi value under windows.
	const float DPcm = DPI * 1.0f / 2.54f;
	const float DPmm = DPI * 1.0f / 25.4f;
	char FormatBuffer[32] = {};
	float V = 0.0f;
	bIsPercent = false;
	int32_t r = sscanf_s(Value.NextWord(true).c_str(), "%f%s", &V, FormatBuffer, (uint32_t)sizeof(FormatBuffer));
	if (r <= 1) return V;
	uint32_t FormatType = LWUTF8I(FormatBuffer).CompareList("%", "cm", "mm", "in", "pt", "pc", "em", "ex");
	if (FormatType == 0) bIsPercent = true;
	else if (FormatType == 1) V *= DPcm; //cm
	else if (FormatType == 2) V *= DPmm; //mm
	else if (FormatType == 3) V *= DPI; //In
	else if (FormatType == 4) V *= PxPtScalar; //pt
	else if (FormatType == 5) V *= PxEMScalar; //em
	else if (FormatType == 6) V *= PxEXScalar; //ex
	return V;
}

LWVector4f LWESVG::ParseColorValue(const LWUTF8Iterator &Value, float CurrentAlpha) {
	const float i255 = 1.0f / 255.0f;
	LWVector4f Result = LWVector4f(0.0f, 0.0f, 0.0f, CurrentAlpha);
	//Color table taken from: https://www.w3.org/TR/css-color-3/
	LWUTF8I ColorTableNames[]  = { "black", "silver",    "gray",     "white",     "maroon",   "red",      "purple",   "fuchsia", "green",    "lime",     "olive",    "yellow",   "navy",     "blue",     "teal",     "aqua",     "aliceblue", "antiquewhite", "aquamarine", "azure",    "beige",    "bisque",   "blanchedalmond", "blueviolet", "brown",     "burlywood",  "cadetblue", "chartreuse", "chocolate", "coral",    "cornflowerblue", "cornsilk", "crimson",  "cyan",     "darkblue", "darkcyan", "darkgoldenrod", "darkgray", "darkgreen", "darkgrey", "darkkhaki", "darkmagenta", "darkolivegreen", "darkorange", "darkorchid", "darkred",  "darksalmon", "darkseagreen", "darkslateblue", "darkslategray", "darkslategrey", "darkturquoise", "darkviolet", "deeppink", "deepskyblue", "dimgray",  "dimgrey",  "dodgerblue", "firebrick", "floralwhite", "forestgreen", "gainsboro", "ghostwhite", "gold",      "goldenrod", "greenyellow", "grey",     "honeydew",  "hotpink",  "indianred", "indigo",   "ivory",    "khaki",    "lavender", "lavenderblush", "lawngreen", "lemonchiffon", "lightblue", "lightcoral", "lightcyan", "lightgoldenrodyellow", "lightgray", "lightgreen", "lightgrey", "lightpink", "lightsalmon", "lightseagreen", "lightskyblue", "lightslategray", "lightslategrey", "lightsteelblue", "lightyellow", "limegreen", "linen",    "magenta",  "mediumaquamarine", "mediumblue", "mediumorchid", "mediumpurple", "mediumseagreen", "mediumslateblue", "mediumspringgreen", "mediumturquoise", "mediumvioletred", "midnightblue", "mintcream", "mistyrose", "moccasin", "navajowhite", "oldlace",  "olivedrab", "orange",   "orangered", "orchid"    "palegoldenrod", "palegreen", "paleturquoise", "palevioletred", "papayawhip", "peachpuff", "peru",     "pink",     "plum",     "powderblue", "rosybrown", "royalblue", "saddlebrown", "salmon",   "sandybrown", "seagreen", "seashell", "sienna",   "skyblue",  "slateblue", "slategray", "slategrey", "snow",     "springgreen", "steelblue", "tan",      "thistle",  "tomato",   "turquoise", "violet",   "wheat",    "whitesmoke", "yellowgreen" };
	const uint32_t ColorTable[] = { 0xFF,    0xC0C0C0ff,  0x808080ff, 0xffffffff,  0x800000ff, 0xFF0000ff, 0x800080ff, 0xFF00FF,  0x008000ff, 0x00FF00ff, 0x808000ff, 0xFFFF00ff, 0x000080ff, 0x0000FFff, 0x008080ff, 0x00FFFFff, 0xf0f8ffff,  0xfaebd7ff,     0x7fffd4ff,   0xf0ffffff, 0xf5f5dcff, 0xffe4c4ff, 0xffebcdff,       0x8a2be2ff,   0xa52a2aff,  0xdeb887ff,   0x5f9ea0ff,  0x7fff00ff,   0xd2691eff,  0xff7f50ff, 0x6495edff,       0xfff8dcff, 0xdc143cff, 0x00ffffff, 0x00008bff, 0x008b8bff, 0xb8860bff,      0xa9a9a9ff, 0x006400ff,  0xa9a9a9ff, 0xbdb76bff,  0x8b008bff,   0x556b2fff,        0xff8c00ff,   0x9932ccff,   0x8b0000ff, 0xe9967aff,   0x8fbc8fff,     0x483d8bff,      0x2f4f4fff,      0x2f4f4fff,      0x00ced1ff,      0x9400d3ff,   0xff1493ff, 0x00bfffff,    0x696969ff, 0x696969ff, 0x1e90ffff,   0xb22222ff,  0xfffaf0ff,    0x228b22ff,    0xdcdcdcff,  0xf8f8ffff,   0xffd700ff,  0xdaa520ff,  0xadff2fff,    0x808080ff, 0xf0fff0ff,  0xff69b4ff, 0xcd5c5cff,  0x4b0082ff, 0xfffff0ff, 0xf0e68cff, 0xe6e6faff, 0xfff0f5ff,      0x7cfc00ff,  0xfffacdff,     0xadd8e6ff,  0xf08080ff,   0xe0ffffff,  0xfafad2ff,             0xd3d3d3ff,  0x90ee90ff,   0xd3d3d3ff,  0xffb6c1ff,  0xffa07aff,    0x20b2aaff,      0x87cefaff,     0x778899ff,       0x778899ff,       0xb0c4deff,       0xffffe0ff,    0x32cd32ff,  0xfaf0e6ff, 0xff00ffff, 0x66cdaaff,         0x0000cdff,   0xba55d3ff,     0x9370dbff,     0x3cb371ff,       0x7b68eeff,        0x00fa9aff,          0x48d1ccff,        0xc71585ff,        0x191970ff,     0xf5fffaff,  0xffe4e1ff,  0xffe4b5ff, 0xffdeadff,    0xfdf5e6ff, 0x6b8e23ff,  0xffa500ff, 0xff4500ff,  0xda70d6ff, 0xeee8aaff,      0x98fb98ff,  0xafeeeeff,      0xdb7093ff,      0xffefd5ff,   0xffdab9ff,  0xcd853fff, 0xffc0cbff, 0xdda0ddff, 0xb0e0e6ff,   0xbc8f8fff,  0x4169e1ff,  0x8b4513ff,    0xfa8072ff, 0xf4a460ff,   0x2e8b57ff, 0xfff5eeff, 0xa0522dff, 0x87ceebff, 0x6a5acdff,  0x708090ff,  0x708090ff,  0xfffafaff, 0x00ff7fff,    0x4682b4ff,  0xd2b48cff, 0xd8bfd8ff, 0xff6347ff, 0x40e0d0ff,  0xee82eeff, 0xf5deb3ff, 0xf5f5f5ff, 0x9acd32ff };
	const uint32_t ColorTableCount = sizeof(ColorTable) / sizeof(uint32_t);
		
	uint32_t HexColor = 0;
	LWUTF8Iterator wValue = Value.NextWord(true);
	const char *cValue = wValue.c_str();
	if (sscanf_s(cValue, "#%x", &HexColor) == 1) Result = LWVector4f(LWUNPACK_COLOR(HexColor, 16), LWUNPACK_COLOR(HexColor, 8), LWUNPACK_COLOR(HexColor, 0), CurrentAlpha);
	else if (sscanf_s(cValue, "rgb( %f , %f , %f)", &Result.x, &Result.y, &Result.z) == 3) Result = LWVector4f(Result.x * i255, Result.y * i255, Result.z * i255, CurrentAlpha);
	else if (sscanf_s(cValue, "rgb( %f%% , %f%% , %f)", &Result.x, &Result.y, &Result.z) == 3) Result = LWVector4f(Result.x * 0.01f, Result.y * 0.01f, Result.z * 0.01f, CurrentAlpha);
	else if (sscanf_s(cValue, "rgba( %f , %f , %f , %f)", &Result.x, &Result.y, &Result.z, &Result.w) == 4) Result = LWVector4f(Result.x * i255, Result.y * i255, Result.z * i255, Result.w);
	else if (sscanf_s(cValue, "rgba( %f%% , %f%% , %f%% , %f)", &Result.x, &Result.y, &Result.z, &Result.w) == 4) Result = LWVector4f(Result.x * 0.01f, Result.y * 0.01f, Result.z * 0.01f, Result.w);
	else {
		//Check if hsl value:
		bool bHSL = false;
		if (bHSL = (sscanf_s(cValue, "hsl( %f , %f%% , %f%%)", &Result.x, &Result.y, &Result.z) == 3)) Result = LWVector4f(Result.x, Result.y * 0.01f, Result.z * 0.01f, CurrentAlpha);
		else if (bHSL = (sscanf_s(cValue, "hsla( %f , %f%% , %f%% , %f)", &Result.x, &Result.y, &Result.z, &Result.w) == 4)) Result = LWVector4f(Result.x, Result.y * 0.01f, Result.z * 0.01f, Result.w);
		if (bHSL) return HSLAtoRGBA(Result);
		else {
			uint32_t Idx = wValue.CompareLista(ColorTableCount, ColorTableNames);
			if (Idx != -1) Result = LWUNPACK_COLORVEC4f(ColorTable[Idx]);
		}
	}
	return Result;
}

LWVector4f LWESVG::HSLAtoRGBA(const LWVector4f &HSLA) {
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

LWVector4f LWESVG::RGBAtoHSLA(const LWVector4f &RGBA) {
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


uint32_t LWESVG::BuildVertices(float Time, LWVertexUI *Vertices, const LWSMatrix4f &Transform) {


	auto DrawArc = [](LWVertexUI *V, const LWVector2f &Center, const LWVector2f &Radi, float ArcStartTheta, float ArcEndTheta, uint32_t Steps, const LWVector4f &Color, const LWSMatrix4f &Transform)->uint32_t {
		if (!V) return Steps * 3;
		float ThetaStep = (ArcEndTheta - ArcStartTheta)/(float)Steps;
		LWSVector4f ctrPos = LWSVector4f(Center, 0.0f, 1.0f) * Transform;
		LWSVector4f pPos = LWSVector4f(Center + LWVector2f::MakeTheta(ArcStartTheta)*Radi, 0.0f, 1.0f) * Transform;
		for (uint32_t i = 1; i <= Steps; i++) {
			LWSVector4f cPos = LWSVector4f(Center + LWVector2f::MakeTheta(ArcStartTheta + ThetaStep * (float)i)*Radi, 0.0f, 1.0f) * Transform;
			*(V++) = LWVertexUI(ctrPos, Color, LWVector4f(0.0f));
			*(V++) = LWVertexUI(pPos, Color, LWVector4f(0.0f));
			*(V++) = LWVertexUI(cPos, Color, LWVector4f(0.0f));
			pPos = cPos;
		}

		return Steps * 3;
	};


	auto DrawLine = [](LWVertexUI *V, const LWVector2f &APnt, const LWVector2f &BPnt, uint32_t ACapType, uint32_t BCapType, const LWVector4f &Color, float Thickness, LWSMatrix4f &Transform)->uint32_t {
		static const uint32_t RoundSteps = 4;
		uint32_t Verts = 6;
		if (ACapType == LWESVGStyle::CapRound) Verts += 3 * RoundSteps;
		if (BCapType == LWESVGStyle::CapRound) Verts += 3 * RoundSteps;
		if (!V) return Verts;
		LWVector2f Dir = BPnt - APnt;
		LWVector2f nDir = Dir.Normalize();
		//if(ACapType==LWESVGStyle::CapSquare) APnt -= nDir

		LWVector2f pnDir = nDir.Perpindicular() * Thickness;


		*(V + 0) = LWVertexUI(LWSVector4f(APnt + pnDir, 0.0f, 1.0f) * Transform, Color, LWVector4f(0.0f));
		*(V + 1) = LWVertexUI(LWSVector4f(APnt - pnDir, 0.0f, 1.0f) * Transform, Color, LWVector4f(0.0f));
		*(V + 2) = LWVertexUI(LWSVector4f(BPnt - pnDir, 0.0f, 1.0f) * Transform, Color, LWVector4f(0.0f));
		
		*(V + 3) = LWVertexUI(LWSVector4f(BPnt - pnDir, 0.0f, 1.0f) * Transform, Color, LWVector4f(0.0f));
		*(V + 4) = LWVertexUI(LWSVector4f(BPnt + pnDir, 0.0f, 1.0f) * Transform, Color, LWVector4f(0.0f));
		*(V + 5) = LWVertexUI(LWSVector4f(APnt + pnDir, 0.0f, 1.0f) * Transform, Color, LWVector4f(0.0f));

		return Verts;
	};

	//auto DrawLineList = [](LWVertexUI *V, const LWVector2f &APnt, const 


	uint32_t o = 0;
	//IterateElements([&DrawLine, &o](uint32_t ID, LWESVGElement &Elem, LWESVGElement*, LWESVG*) {
		//uint32_t Type = Elem.GetType();

		//Draw stroke, fill
		

	//});
	return o;
}

uint32_t LWESVG::PushElementFirst(const LWESVGElement &Elem, uint32_t ParentID) {
	return InsertElementAt(Elem, ParentID, -1);
}

uint32_t LWESVG::PushElementLast(const LWESVGElement &Elem, uint32_t ParentID) {
	return InsertElementAt(Elem, ParentID, ParentID == -1 ? m_LastChildID : m_Elements[ParentID].m_LastChildID);
}

uint32_t LWESVG::InsertElementAt(const LWESVGElement &Elem, uint32_t ParentID, uint32_t PrevID) {
	uint32_t ID = (uint32_t)m_Elements.size();
	m_Elements.push_back(Elem);
	MoveComponentTo(ID, ParentID, PrevID);
	return ID;
}

LWESVG &LWESVG::MoveComponentTo(uint32_t SourceID, uint32_t ParentID, uint32_t PrevID) {
	LWESVGElement &E = m_Elements[SourceID];
	RemoveComponent(SourceID);
	E.m_ParentID = ParentID;
	E.m_PrevID = PrevID;
	E.m_NextID = -1;
	if (ParentID != -1) {
		LWESVGElement &PE = m_Elements[ParentID];
		if (PrevID == -1) {
			if (PE.m_FirstChildID != -1) {
				LWESVGElement &PFirst = m_Elements[PE.m_FirstChildID];
				PFirst.m_PrevID = SourceID;
				E.m_NextID = PE.m_FirstChildID;
			}
			PE.m_FirstChildID = SourceID;
		}
		if (PrevID == PE.m_LastChildID) PE.m_LastChildID = SourceID;
	} else {
		if (PrevID == -1) {
			if (m_FirstChildID != -1) {
				LWESVGElement &PFirst = m_Elements[m_FirstChildID];
				PFirst.m_PrevID = SourceID;
				E.m_NextID = m_FirstChildID;
			}
			m_FirstChildID = SourceID;
		}
		if (PrevID == m_LastChildID) m_LastChildID = SourceID;
	}
	if (PrevID != -1) {
		LWESVGElement &PrE = m_Elements[PrevID];
		if (PrE.m_NextID != -1) {
			LWESVGElement &NxE = m_Elements[PrE.m_NextID];
			NxE.m_PrevID = SourceID;
			E.m_NextID = PrE.m_NextID;
		}
		PrE.m_NextID = SourceID;
	}
	return *this;
}

LWESVG &LWESVG::RemoveComponent(uint32_t ID) {
	LWESVGElement &E = m_Elements[ID];
	if (E.m_ParentID != -1) {
		LWESVGElement &PE = m_Elements[E.m_ParentID];
		if (PE.m_FirstChildID == ID) PE.m_FirstChildID = E.m_NextID;
		if (PE.m_LastChildID == ID) PE.m_LastChildID = E.m_PrevID;
	} else {
		if (m_FirstChildID == ID) m_FirstChildID = E.m_NextID;
		if (m_LastChildID == ID) m_LastChildID = E.m_PrevID;
	}
	if (E.m_PrevID != -1) {
		LWESVGElement &PrE = m_Elements[E.m_PrevID];
		PrE.m_NextID = E.m_NextID;
	}
	if (E.m_NextID != -1) {
		LWESVGElement &NxE = m_Elements[E.m_NextID];
		NxE.m_PrevID = E.m_PrevID;
	}
	return *this;
}

LWESVG &LWESVG::Prune(void) {
	std::vector<LWESVGElement> NewList;
	std::function<uint32_t(uint32_t, uint32_t)> ProcessElement = [&NewList, this, &ProcessElement](uint32_t ID, uint32_t ParentID)->uint32_t {
		LWESVGElement &E = m_Elements[ID];
		uint32_t nID = (uint32_t)NewList.size();
		NewList.push_back(E);
		LWESVGElement &nE = NewList[nID];
		nE.m_ParentID = ParentID;
		uint32_t c = E.m_FirstChildID;
		uint32_t PrevNID = -1;
		while (c != -1) {
			LWESVGElement &cE = m_Elements[c];
			uint32_t cnID = ProcessElement(c, nID);
			LWESVGElement &ncE = NewList[cnID];
			ncE.m_PrevID = PrevNID;
			if (PrevNID == -1) nE.m_FirstChildID = cnID;
			else {
				LWESVGElement &ncPrev = NewList[PrevNID];
				ncPrev.m_NextID = cnID;
			}
			PrevNID = cnID;
			c = cE.m_NextID;
		}
		nE.m_LastChildID = PrevNID;
		return nID;
	};
	NewList.reserve(m_Elements.size());
	uint32_t c = m_FirstChildID;
	uint32_t PrevID = -1;
	while (c != -1) {
		LWESVGElement &cE = m_Elements[c];
		uint32_t ID = ProcessElement(c, -1);
		LWESVGElement &ncE = NewList[ID];
		ncE.m_PrevID = PrevID;
		if (PrevID == -1) m_FirstChildID = ID;
		else {
			LWESVGElement &ncPrev = NewList[PrevID];
			ncPrev.m_NextID = ID;
		}
		PrevID = ID;
		c = cE.m_NextID;
	}
	m_LastChildID = PrevID;
	m_Elements = NewList;
	return *this;
}

LWESVG &LWESVG::IterateElements(LWESVGIterateFunc Func) {
	std::function<bool(uint32_t, LWESVGElement *)> IterateFunc = [&Func, &IterateFunc, this](uint32_t ID, LWESVGElement *Parent)->bool {
		if (ID == -1) return true;
		LWESVGElement &E = m_Elements[ID];
		if (!Func(ID, E, Parent, *this)) return false;
		if (!IterateFunc(E.m_FirstChildID, &E)) return false;
		if (!IterateFunc(E.m_NextID, Parent)) return false;
		return true;
	};
	IterateFunc(m_FirstChildID, nullptr);
	return *this;
}

const LWESVG &LWESVG::IterateElements(LWESVGConstIterateFunc Func) const {
	std::function<bool(uint32_t, const LWESVGElement *)> IterateFunc = [&Func, &IterateFunc, this](uint32_t ID, const LWESVGElement *Parent)->bool {
		if (ID == -1) return true;
		const LWESVGElement &E = m_Elements[ID];
		if (!Func(ID, E, Parent, *this)) return false;
		if (!IterateFunc(E.m_FirstChildID, &E)) return false;
		if (!IterateFunc(E.m_NextID, Parent)) return false;
		return true;
	};
	IterateFunc(m_FirstChildID, nullptr);
	return *this;
}

LWESVG &LWESVG::IterateElementsBackwards(LWESVGIterateFunc Func) {
	std::function<bool(uint32_t, LWESVGElement *)> IterateFunc = [&Func, &IterateFunc, this](uint32_t ID, LWESVGElement *Parent)->bool {
		if (ID == -1) return true;
		LWESVGElement &E = m_Elements[ID];
		if (!Func(ID, E, Parent, *this)) return false;
		if (!IterateFunc(E.m_LastChildID, &E)) return false;
		if (!IterateFunc(E.m_PrevID, Parent)) return false;
		return true;
	};
	IterateFunc(m_LastChildID, nullptr);
	return *this;
}

const LWESVG &LWESVG::IterateElementsBackwards(LWESVGConstIterateFunc Func) const {
	std::function<bool(uint32_t, const LWESVGElement *)> IterateFunc = [&Func, &IterateFunc, this](uint32_t ID, const LWESVGElement *Parent)->bool {
		if (ID == -1) return true;
		const LWESVGElement &E = m_Elements[ID];
		if (!Func(ID, E, Parent, *this)) return false;
		if (!IterateFunc(E.m_LastChildID, &E)) return false;
		if (!IterateFunc(E.m_PrevID, Parent)) return false;
		return true;
	};
	IterateFunc(m_LastChildID, nullptr);
	return *this;
}

LWVector2f LWESVG::GetSize(void) const {
	return m_Size;
}

LWESVGElement *LWESVG::GetElement(uint32_t ID) {
	return ID >= (uint32_t)m_Elements.size() ? nullptr : &m_Elements[ID];
}

const LWESVGElement *LWESVG::GetElement(uint32_t ID) const {
	return ID >= (uint32_t)m_Elements.size() ? nullptr : &m_Elements[ID];
}

LWESVGElement *LWESVG::FindElementByID(const LWUTF8Iterator &IDName) {
	return FindElementByID(IDName.Hash());
}

LWESVGElement *LWESVG::FindElementByID(uint32_t IDNameHash) {
	uint32_t Res = -1;
	IterateElements([&Res, &IDNameHash](uint32_t ID, LWESVGElement &Elem, LWESVGElement *, LWESVG &)->bool {
		if (Elem.m_IDHash == IDNameHash) {
			Res = ID;
			return false;
		}
		return true;
	});
	return GetElement(Res);
}


const LWESVGElement *LWESVG::FindElementByID(const LWUTF8Iterator &IDName) const {
	return FindElementByID(IDName.Hash());
}

const LWESVGElement *LWESVG::FindElementByID(uint32_t IDNameHash) const {
	uint32_t Res = -1;
	IterateElements([&Res, &IDNameHash](uint32_t ID, const LWESVGElement &Elem, const LWESVGElement*, const LWESVG&)->bool {
		if (Elem.m_IDHash == IDNameHash) {
			Res = ID;
			return false;
		}
		return true;
	});
	return GetElement(Res);
}

uint32_t LWESVG::GetFirstChildID(void) const {
	return m_FirstChildID;
}

uint32_t LWESVG::GetLastChildID(void) const {
	return m_LastChildID;
}

LWESVG::LWESVG(const LWVector2f &Size, uint32_t ReserveCount) : m_Size(Size) {
	m_Elements.reserve(ReserveCount);
}
