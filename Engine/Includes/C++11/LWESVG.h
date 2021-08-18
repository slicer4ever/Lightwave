#ifndef LWESVG_H
#define LWESVG_H
#include <LWCore/LWTypes.h>
#include <LWCore/LWVector.h>
#include <LWCore/LWUnicode.h>
#include <LWVideo/LWMesh.h>
#include <LWEXML.h>
#include <functional>

//TODO: Finish SVG implementation.

struct LWESVGElement;

class LWESVG;

/*!< \brief callback function for iterating over SVG elements. return true to continue iterating, or false to stop.
	 \params: ID, Element, Parent, SVG.
*/
typedef std::function<bool(uint32_t, LWESVGElement &, LWESVGElement *, LWESVG &)> LWESVGIterateFunc;
typedef std::function<bool(uint32_t, const LWESVGElement &, const LWESVGElement *, const LWESVG &)> LWESVGConstIterateFunc;

struct LWESVGStyle {
	LWBitField32(LineCap, 2, 0);

	//Flags:
	static const uint32_t WidthAuto      = 0x400000;
	static const uint32_t HeightAuto     = 0x800000;
	static const uint32_t xRadiAuto     = 0x1000000;
	static const uint32_t yRadiAuto     = 0x2000000;
	static const uint32_t xPosPercent   = 0x4000000;
	static const uint32_t yPosPercent   = 0x8000000;
	static const uint32_t xSizePercent = 0x10000000;
	static const uint32_t ySizePercent = 0x20000000;
	static const uint32_t xRadiPercent = 0x40000000;
	static const uint32_t yRadiPercent = 0x80000000;

	//Line-Caps:
	static const uint32_t CapButt = 0;
	static const uint32_t CapRound = 1;
	static const uint32_t CapSquare = 2;

	//Inheritance flags(all set functions will raise the relevant flag):
	static const uint32_t hasXPos = 0x1;
	static const uint32_t hasYPos = 0x2;
	static const uint32_t hasXSize = 0x4;
	static const uint32_t hasYSize = 0x8;
	static const uint32_t hasXRadi = 0x10;
	static const uint32_t hasYRadi = 0x20;
	static const uint32_t hasLineCap = 0x40;
	static const uint32_t hasFillColor = 0x80;
	static const uint32_t hasStrokeColor = 0x100;
	static const uint32_t hasStrokeWidth = 0x200;

	LWESVGStyle &SetxPos(float Position, bool bIsPercent);

	LWESVGStyle &SetyPos(float Position, bool bIsPercent);

	LWESVGStyle &SetxSize(float Size, bool bIsPercent, bool bIsAuto);

	LWESVGStyle &SetySize(float Size, bool bIsPercent, bool bIsAuto);

	LWESVGStyle &SetxRadi(float Radi, bool bIsPercent, bool bIsAuto);

	LWESVGStyle &SetyRadi(float Radi, bool bIsPercent, bool bIsAuto);

	LWESVGStyle &SetFillColor(const LWVector4f &Color);

	LWESVGStyle &SetStrokeColor(const LWVector4f &Color);

	LWESVGStyle &SetStrokeWidth(const float StokeWidth);

	LWESVGStyle &SetLineCaps(uint32_t LineCaps);

	uint32_t GetLineCap(void) const;

	bool isWidthAuto(void) const;

	bool isHeightAuto(void) const;

	bool isxRadiAuto(void) const;

	bool isyRadiAuto(void) const;

	bool Has(uint32_t HasFlag) const;

	//Produces a style which uses Style propertys if has* flag is set, otherwise uses the property in InheritanceStyle.
	LWESVGStyle(const LWESVGStyle &Style, const LWESVGStyle &InheritanceStyle);

	LWESVGStyle() = default;

	LWVector4f m_FillColor = LWVector4f(0.0f);
	LWVector4f m_StrokeColor = LWVector4f(0.0f, 0.0f, 0.0f, 1.0f);
	LWVector2f m_Position;
	LWVector2f m_Size;
	LWVector2f m_Radius; //doubles for rectangle corner roundness.
	float m_Opacity = 1.0f;
	float m_StrokeWidth = 1.0f;
	uint32_t m_Flag = 0;
	uint32_t m_HasFlags = 0;
};

/*!< \brief each element in the dom structure of the svg object. */
struct LWESVGElement {
	LWBitField32(Type, 4, 0);

	//SVG Types:
	static const uint32_t Group = 0;
	static const uint32_t Define = 1;
	static const uint32_t Circle = 2;
	static const uint32_t Ellipse = 3;
	static const uint32_t Rect = 4;
	static const uint32_t Line = 5;
	static const uint32_t Path = 6;
	static const uint32_t Polygon = 7;
	static const uint32_t Polyline = 8;

	uint32_t GetType(void) const;

	LWESVGElement(const LWESVGStyle &Style, uint32_t Type);

	LWESVGElement() = default;

	//Common propertys:
	std::vector<LWVector2f> m_PointList;
	LWESVGStyle m_Style;

	uint32_t m_IDHash = LWCrypto::FNV1AHash; //ID name hash'd.
	uint32_t m_ClassHash = LWCrypto::FNV1AHash; //Class name hash'd.
	uint32_t m_Flag = 0;
	uint32_t m_InheritFlag = 0; //Flag used to indicate which propertys should be inherited.

	//Used to iterate over the element tree, should not be touched by application code in general.
	uint32_t m_ParentID = -1;
	uint32_t m_NextID = -1;
	uint32_t m_PrevID = -1;
	uint32_t m_FirstChildID = -1;
	uint32_t m_LastChildID = -1;
};

/*!< \brief processes svg structure for rendering. */
class LWESVG {
public:
	/*!< \brief load's a file at the specified file path. */
	static bool LoadFile(LWESVG &Object, const LWUTF8Iterator &Filepath, LWAllocator &Allocator, LWFileStream *ExistingStream = nullptr);

	/*!< \brief parse's an "svg" node, and all children.  UserData should be a pointer to an LWESVG object. */
	static bool XMLParser(LWEXMLNode *Node, void *UserData, LWEXML *XML);

	static bool XMLParseBaseElement(LWEXMLNode *Node, LWESVG &SVG, uint32_t ParentElemID, LWEXML *XML);

	static bool XMLParseCoreAttributes(LWEXMLNode *Node, LWESVGElement &Elem, uint32_t ParentElemID, LWEXML *XML);

	static bool XMLParseGeometryAttributes(LWEXMLNode *Node, LWESVGElement &Elem, uint32_t ParentElemID, LWEXML *XML);

	static bool XMLParsePresentationAttributes(LWEXMLNode *Node, LWESVGElement &Elem, uint32_t ParentElemID, LWEXML *XML);

	static bool XMLParseGroupElement(LWEXMLNode *Node, LWESVGElement &Elem, uint32_t ParentElemID, LWEXML *XML);

	static bool XMLParseDefineElement(LWEXMLNode *Node, LWESVGElement &Elem, uint32_t ParentElemID, LWEXML *XML);

	static bool XMLParseRectElement(LWEXMLNode *Node, LWESVGElement &Elem, uint32_t ParentElemID, LWEXML *XML);

	static bool XMLParseCircleElement(LWEXMLNode *Node, LWESVGElement &Elem, uint32_t ParentElemID, LWEXML *XML);

	static bool XMLParseEllipseElement(LWEXMLNode *Node, LWESVGElement &Elem, uint32_t ParentElemID, LWEXML *XML);

	static bool XMLParseLineElement(LWEXMLNode *Node, LWESVGElement &Elem, uint32_t ParentElemID, LWEXML *XML);

	static bool XMLParsePathElement(LWEXMLNode *Node, LWESVGElement &Elem, uint32_t ParentElemID, LWEXML *XML);

	static bool XMLParsePolygonElement(LWEXMLNode *Node, LWESVGElement &Elem, uint32_t ParentElemID, LWEXML *XML);

	static bool XMLParsePolylineElement(LWEXMLNode *Node, LWESVGElement &Elem, uint32_t ParentElemID, LWEXML *XML);

	/*!< \brief parse's a length value, returning value, if the value is a percentage then StyleFlags will raise the percentstylebit. */
	static float ParseLengthValue(const LWUTF8Iterator &Value, bool &bIsPercent);

	/*!< \brief parse's for any of the folowing format's: named color(as defined by css3 colors), #rrggbb, rgb(255,255,255), rgb(100%, 100%, 100%), rgba(255,255,255,1), rgba(100%, 100%, 100%, 1), hsl(0-360, 100%, 100%), hsla(0-360, 100%, 100%) */
	static LWVector4f ParseColorValue(const LWUTF8Iterator &Value, float CurrentAlpha);

	static LWVector4f HSLAtoRGBA(const LWVector4f &HSLA);

	static LWVector4f RGBAtoHSLA(const LWVector4f &RGBA);

	uint32_t BuildVertices(float Time, LWVertexUI *Vertices, const LWSMatrix4f &Transform);

	/*!< \brief inserts the element as the last child of the specified parent(if parent is -1, the root element is used.) 
		 \return the id of the created component.
	*/
	uint32_t PushElementLast(const LWESVGElement &Elem, uint32_t ParentID = -1);

	/*!< \brief inserts the element as the first child of the specified parent(if parent is -1, the root element is used.) 
		 \return the id of the created component.
	*/
	uint32_t PushElementFirst(const LWESVGElement &Elem, uint32_t ParentID = -1);

	/*!< \brief inserts the element at the specified location, after the prev node of the parent child, if prev is -1 then the element is placed as the first child of parent.
		 \return the id of the created component.
	*/
	uint32_t InsertElementAt(const LWESVGElement &Elem, uint32_t ParentID = -1, uint32_t PrevID = -1);

	/*!< \brief moves a component from it's current position, to the specified position, if PrevID=-1 then the element will be placed as the first child under parent, otherwise it's placed after the specified child. */
	LWESVG &MoveComponentTo(uint32_t SourceID, uint32_t ParentID, uint32_t PrevID);

	/*!< \brief removes the specified component from the tree, this will leave a hole in the vector container of elements, call prune to rebuild the tree. */
	LWESVG &RemoveComponent(uint32_t ID);

	/*!< \brief rebuild's the tree for linear ordering of elements in the tree structure, this will change id of components, but keep their relative positions, as such any references to elements will potentially be invalid after this call. 
		 \note this function should not be needed if strictly loading in an svg file, or creation occurs in top down first-last tree structure.
	*/
	LWESVG &Prune(void);

	LWESVG &IterateElements(LWESVGIterateFunc Func);

	const LWESVG &IterateElements(LWESVGConstIterateFunc Func) const;

	LWESVG &IterateElementsBackwards(LWESVGIterateFunc Func);

	const LWESVG &IterateElementsBackwards(LWESVGConstIterateFunc Func) const;

	LWVector2f GetSize(void) const;

	/*!< \brief returns an element at the specified id, if id >= container then returns null.
		 \note: if the svg object is being modified, storing this pointer will become invalid if the modification causes the container to resize, or prune changes the elements position.
	*/
	LWESVGElement *GetElement(uint32_t ID);

	/*!< \brief returns an const element at the specified id, if id >= container then returns null.
		 \note: if the svg object is being modified, storing this pointer will become invalid if the modification causes the container to resize, or prune changes the elements position.
	*/
	const LWESVGElement *GetElement(uint32_t ID) const;

	/*!< \brief returns the first element with the specified id name. returns null if not found. */
	LWESVGElement *FindElementByID(const LWUTF8Iterator &IDName);

	LWESVGElement *FindElementByID(uint32_t IDNameHash);

	const LWESVGElement *FindElementByID(const LWUTF8Iterator &IDName) const;
	
	const LWESVGElement *FindElementByID(uint32_t IDNameHash) const;

	uint32_t GetFirstChildID(void) const;

	uint32_t GetLastChildID(void) const;

	LWESVG(const LWVector2f &Size, uint32_t ReserveCount = 0);

	LWESVG() = default;

private:
	std::vector<LWESVGElement> m_Elements;
	LWVector2f m_Size;
	uint32_t m_FirstChildID = -1;
	uint32_t m_LastChildID = -1;
};

#endif