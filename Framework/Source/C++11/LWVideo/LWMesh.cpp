#include "LWVideo/LWMesh.h"
#include "LWVideo/LWVideoBuffer.h"
#include "LWCore/LWAllocator.h"

uint32_t LWVertexUI::WriteVertex(LWBaseMesh *Mesh, const LWVector2f &Position, const LWVector4f &Color, const LWVector2f &TexCoord){
	if (!Mesh->CanWriteVertices(1)) return 0;
	LWVertexUI *V = ((LWMesh<LWVertexUI>*)Mesh)->GetVertexAt(Mesh->WriteVertices(1));
	V[0] = { LWVector4f(Position, 0.0f, 1.0f), Color, LWVector4f(TexCoord, 0.0f, 0.0f) };
	return 1;
}

uint32_t LWVertexUI::WriteVertex(LWBaseMesh *Mesh, const LWVector3f &Position, const LWVector4f &Color, const LWVector2f &TexCoord){
	if (!Mesh->CanWriteVertices(1)) return 0;
	LWVertexUI *V = ((LWMesh<LWVertexUI>*)Mesh)->GetVertexAt(Mesh->WriteVertices(1));
	V[0] = { LWVector4f(Position,  1.0f), Color, LWVector4f(TexCoord, 0.0f, 0.0f) };
	return 1;
}


uint32_t LWVertexUI::WriteVertex(LWBaseMesh *Mesh, const LWVector4f &Position, const LWVector4f &Color, const LWVector2f &TexCoord){
	if (!Mesh->CanWriteVertices(1)) return 0;
	LWVertexUI *V = ((LWMesh<LWVertexUI>*)Mesh)->GetVertexAt(Mesh->WriteVertices(1));
	V[0] = { Position, Color, LWVector4f(TexCoord, 0.0f, 0.0f) };
	return 1;
}

uint32_t LWVertexUI::WriteRect(LWBaseMesh *Mesh, const LWVector2f &CtrPnt, const LWVector2f &Size, const LWVector4f &Color, const LWVector2f &TexCtrPnt, const LWVector2f &TexSize) {
	if (!Mesh->CanWriteVertices(6)) return 0;
	LWVertexUI *V = ((LWMesh<LWVertexUI>*)Mesh)->GetVertexAt(Mesh->WriteVertices(6));
	LWVector4f TopLeftPnt = LWVector4f(CtrPnt + LWVector2f(-Size.x, Size.y), 0.0f, 1.0f);
	LWVector4f TopRightPnt = LWVector4f(CtrPnt + LWVector2f(Size.x, Size.y), 0.0f, 1.0f);
	LWVector4f BtmLeftPnt = LWVector4f(CtrPnt + LWVector2f(-Size.x,-Size.y), 0.0f, 1.0f);
	LWVector4f BtmRightPnt = LWVector4f(CtrPnt + LWVector2f(Size.x,-Size.y), 0.0f, 1.0f);

	LWVector4f TopLeftTC = LWVector4f(TexCtrPnt + LWVector2f(-TexSize.x,  TexSize.y), 0.0f, 0.0f);
	LWVector4f TopRightTC = LWVector4f(TexCtrPnt + LWVector2f(TexSize.x,  TexSize.y), 0.0f, 0.0f);
	LWVector4f BtmLeftTC = LWVector4f(TexCtrPnt + LWVector2f(-TexSize.x, -TexSize.y), 0.0f, 0.0f);
	LWVector4f BtmRightTC = LWVector4f(TexCtrPnt + LWVector2f(TexSize.x, -TexSize.y), 0.0f, 0.0f);
	V[0] = { TopLeftPnt, Color, TopLeftTC };
	V[1] = { BtmLeftPnt, Color, BtmLeftTC };
	V[2] = { BtmRightPnt, Color, BtmRightTC };
	V[3] = { TopLeftPnt, Color, TopLeftTC };
	V[4] = { BtmRightPnt, Color, BtmRightTC };
	V[5] = { TopRightPnt, Color, TopRightTC };
	return 6;
}

uint32_t LWVertexUI::WriteRect(LWBaseMesh *Mesh, const LWVector2f &CtrPnt, const LWVector2f &Size, float Theta, const LWVector4f &Color, const LWVector2f &TexCtrPnt, const LWVector2f &TexSize) {
	if (!Mesh->CanWriteVertices(6)) return 0;
	LWVertexUI *V = ((LWMesh<LWVertexUI>*)Mesh)->GetVertexAt(Mesh->WriteVertices(6));
	LWVector2f Rot = LWVector2f::MakeTheta(Theta);
	LWVector2f TL = Rot.Rotate(LWVector2f(-Size.x,-Size.y))+CtrPnt;
	LWVector2f TR = Rot.Rotate(LWVector2f( Size.x,-Size.y))+CtrPnt;
	LWVector2f BL = Rot.Rotate(LWVector2f(-Size.x, Size.y))+CtrPnt;
	LWVector2f BR = Rot.Rotate(LWVector2f( Size.x, Size.y))+CtrPnt;
	
	LWVector4f TopLeftPnt = LWVector4f(TL, 0.0f, 1.0f);
	LWVector4f TopRightPnt = LWVector4f(TR, 0.0f, 1.0f);
	LWVector4f BtmLeftPnt = LWVector4f(BL, 0.0f, 1.0f);
	LWVector4f BtmRightPnt = LWVector4f(BR, 0.0f, 1.0f);

	LWVector4f TopLeftTC = LWVector4f(TexCtrPnt + LWVector2f(-TexSize.x, TexSize.y), 0.0f, 0.0f);
	LWVector4f TopRightTC = LWVector4f(TexCtrPnt + LWVector2f(TexSize.x, TexSize.y), 0.0f, 0.0f);
	LWVector4f BtmLeftTC = LWVector4f(TexCtrPnt + LWVector2f(-TexSize.x, -TexSize.y), 0.0f, 0.0f);
	LWVector4f BtmRightTC = LWVector4f(TexCtrPnt + LWVector2f(TexSize.x, -TexSize.y), 0.0f, 0.0f);
	V[0] = { TopLeftPnt, Color, TopLeftTC };
	V[1] = { BtmLeftPnt, Color, BtmLeftTC };
	V[2] = { BtmRightPnt, Color, BtmRightTC };
	V[3] = { TopLeftPnt, Color, TopLeftTC };
	V[4] = { BtmRightPnt, Color, BtmRightTC };
	V[5] = { TopRightPnt, Color, TopRightTC };
	return 6;
}

uint32_t LWVertexUI::WriteRectangle(LWBaseMesh *Mesh, const LWVector2f &TopLeftPoint, const LWVector2f &BtmRightPoint, const LWVector4f &Color, const LWVector2f &TopLeftTexCoord, const LWVector2f &BtmRightTexCoord){
	if (!Mesh->CanWriteVertices(6)) return 0;

	LWVertexUI *V = ((LWMesh<LWVertexUI>*)Mesh)->GetVertexAt(Mesh->WriteVertices(6));
	LWVector4f TopLeftPnt  = LWVector4f(TopLeftPoint, 0.0f, 1.0f);
	LWVector4f TopRightPnt = LWVector4f(BtmRightPoint.x, TopLeftPoint.y, 0.0f, 1.0f);
	LWVector4f BtmLeftPnt  = LWVector4f(TopLeftPoint.x, BtmRightPoint.y, 0.0f, 1.0f);
	LWVector4f BtmRightPnt = LWVector4f(BtmRightPoint, 0.0f, 1.0f);

	LWVector4f TopLeftTC  = LWVector4f(TopLeftTexCoord, 0.0f, 0.0f);
	LWVector4f TopRightTC = LWVector4f(BtmRightTexCoord.x, TopLeftTexCoord.y, 0.0f, 0.0f);
	LWVector4f BtmLeftTC  = LWVector4f(TopLeftTexCoord.x, BtmRightTexCoord.y, 0.0f, 0.0f);
	LWVector4f BtmRightTC = LWVector4f(BtmRightTexCoord, 0.0f, 0.0f);
	V[0] = { TopLeftPnt, Color, TopLeftTC };
	V[1] = { BtmLeftPnt, Color, BtmLeftTC };
	V[2] = { BtmRightPnt, Color, BtmRightTC };
	V[3] = { TopLeftPnt, Color, TopLeftTC };
	V[4] = { BtmRightPnt, Color, BtmRightTC };
	V[5] = { TopRightPnt, Color, TopRightTC };
	return 6;

}	

uint32_t LWVertexUI::WriteRectangle(LWBaseMesh *Mesh, const LWVector2f &TopLeftPoint, const LWVector2f &BtmRightPoint, const LWVector4f &Color, const LWVector2f &TopLeftTexCoord, const LWVector2f &BtmRightTexCoord, const LWVector4f &AABB) {
	if (!Mesh->CanWriteVertices(6)) return 0;
	if (TopLeftPoint.x >= (AABB.x + AABB.z) || TopLeftPoint.y < AABB.y || BtmRightPoint.x < AABB.x || BtmRightPoint.y >= (AABB.y+AABB.w)) return 0;

	float Width = BtmRightPoint.x - TopLeftPoint.x;
	float Height = TopLeftPoint.y - BtmRightPoint.y;
	float TexWidth = BtmRightTexCoord.x - TopLeftTexCoord.x;
	float TexHeight = TopLeftTexCoord.y - BtmRightTexCoord.y;

	float LeftRatio = (TopLeftPoint.x < AABB.x) ? (AABB.x - TopLeftPoint.x) / Width : 0.0f;
	float RightRatio = (BtmRightPoint.x >= (AABB.x + AABB.z)) ? 1.0f - ((BtmRightPoint.x - (AABB.x + AABB.z)) / Width) : 1.0f;
	float TopRatio = (TopLeftPoint.y >= (AABB.y + AABB.w)) ? 1.0f - ((TopLeftPoint.y - (AABB.y + AABB.w)) / Height) : 1.0f;
	float BtmRatio = (BtmRightPoint.y < AABB.y) ? (AABB.y - BtmRightPoint.y) / Height : 0.0f;


	LWVector2f BottomLeft = LWVector2f(TopLeftPoint.x, BtmRightPoint.y);
	LWVector2f BottomLeftTexCoord = LWVector2f(TopLeftTexCoord.x, BtmRightTexCoord.y);

	LWVector4f TL = LWVector4f(BottomLeft + LWVector2f(Width, Height)*LWVector2f(LeftRatio, TopRatio), 0.0f, 1.0f);
	LWVector4f BR = LWVector4f(BottomLeft + LWVector2f(Width, Height)*LWVector2f(RightRatio, BtmRatio), 0.0f, 1.0f);
	LWVector4f TR = LWVector4f(BR.x, TL.y, 0.0f, 1.0f);
	LWVector4f BL = LWVector4f(TL.x, BR.y, 0.0f, 1.0f);

	LWVector4f TLTexCoord = LWVector4f(BottomLeftTexCoord + LWVector2f(TexWidth, TexHeight)*LWVector2f(LeftRatio, TopRatio), 0.0f, 0.0f);
	LWVector4f BRTexCoord = LWVector4f(BottomLeftTexCoord + LWVector2f(TexWidth, TexHeight)*LWVector2f(RightRatio, BtmRatio), 0.0f, 0.0f);
	LWVector4f TRTexCoord = LWVector4f(BRTexCoord.x, TopLeftTexCoord.y, 0.0f, 0.0f);
	LWVector4f BLTexCoord = LWVector4f(TLTexCoord.x, BRTexCoord.y, 0.0f, 0.0f);

	LWVertexUI *V = ((LWMesh<LWVertexUI>*)Mesh)->GetVertexAt(Mesh->WriteVertices(6));

	V[0] = { TL, Color, TLTexCoord };
	V[1] = { BL, Color, BLTexCoord };
	V[2] = { BR, Color, BRTexCoord };
	V[3] = { TL, Color, TLTexCoord };
	V[4] = { BR, Color, BRTexCoord };
	V[5] = { TR, Color, TRTexCoord };

	return 6;
}

LWMesh<LWVertexUI> *LWVertexUI::MakeMesh(LWAllocator &Allocator, LWVideoBuffer *VertexBuffer, LWVideoBuffer *IndiceBuffer, uint32_t CurrentVertexCount, uint32_t CurrentIndiceCount) {
	return Allocator.Allocate<LWMesh<LWVertexUI>>(VertexBuffer, IndiceBuffer, CurrentVertexCount, CurrentIndiceCount);
}

LWMesh<LWVertexUI> *LWVertexUI::MakeMesh(LWAllocator &Allocator, LWVideoBuffer *VertexBuffer, uint32_t CurrentVertexCount){
	return Allocator.Allocate<LWMesh<LWVertexUI>>(VertexBuffer, CurrentVertexCount);
}

LWVertexUI::LWVertexUI(const LWVector4f &Position, const LWVector4f &Color, const LWVector4f &TexCoord) : m_Position(Position), m_Color(Color), m_TexCoord(TexCoord) {}

uint32_t LWVertexPosition::WriteVertex(LWBaseMesh *Mesh, const LWVector4f &Position){
	if (!Mesh->CanWriteVertices(1)) return 0;
	LWVertexPosition *V = ((LWMesh<LWVertexPosition>*)Mesh)->GetVertexAt(Mesh->WriteVertices(1));
	V[0] = { Position };
	return 1;
}

uint32_t LWVertexPosition::WriteRectangle(LWBaseMesh *Mesh, const LWVector2f &TopLeftPoint, const LWVector2f &BtmRightPoint){
	if (!Mesh->CanWriteVertices(6)) return 0;
	LWVertexPosition *V = ((LWMesh<LWVertexPosition>*)Mesh)->GetVertexAt(Mesh->WriteVertices(6));
	LWVector4f TopLeftPnt = LWVector4f(TopLeftPoint, 0.0f, 1.0f);
	LWVector4f TopRightPnt = LWVector4f(BtmRightPoint.x, TopLeftPoint.y, 0.0f, 1.0f);
	LWVector4f BtmLeftPnt = LWVector4f(TopLeftPoint.x, BtmRightPoint.y, 0.0f, 1.0f);
	LWVector4f BtmRightPnt = LWVector4f(BtmRightPoint, 0.0f, 1.0f);
	V[0] = { TopLeftPnt };
	V[1] = { BtmLeftPnt };
	V[2] = { BtmRightPnt };
	V[3] = { TopLeftPnt };
	V[4] = { BtmRightPnt };
	V[5] = { TopRightPnt };
	return 6;
}

uint32_t LWVertexPosition::WriteCircle(LWBaseMesh *Mesh, const LWVector2f &CenterPnt, float Radius, uint32_t Steps){
	uint32_t VertexCount = Steps * 3;
	if (!Mesh->CanWriteVertices(VertexCount)) return 0;
	LWVertexPosition *V = ((LWMesh<LWVertexPosition>*)Mesh)->GetVertexAt(Mesh->WriteVertices(VertexCount));
	LWVector2f p = LWVector2f::MakeTheta(0.0f)*Radius;
	float ThetaInc = LW_2PI / Steps;
	for (uint32_t n = 1; n <= Steps;n++){
		LWVector2f c = LWVector2f::MakeTheta(ThetaInc*n)*Radius;
		*V++ = { LWVector4f(CenterPnt + c, 0.0f, 1.0f) };
		*V++ = { LWVector4f(CenterPnt, 0.0f, 1.0f) };
		*V++ = { LWVector4f(CenterPnt + p, 0.0f, 1.0f) };
		p = c;
	}
	return VertexCount;
}

LWMesh<LWVertexPosition> *LWVertexPosition::MakeMesh(LWAllocator &Allocator, LWVideoBuffer *VertexBuffer, LWVideoBuffer *IndiceBuffer, uint32_t CurrentVertexCount, uint32_t CurrentIndiceCount) {
	return Allocator.Allocate<LWMesh<LWVertexPosition>>(VertexBuffer, IndiceBuffer, CurrentVertexCount, CurrentIndiceCount);
}

LWMesh<LWVertexPosition> *LWVertexPosition::MakeMesh(LWAllocator &Allocator, LWVideoBuffer *VertexBuffer, uint32_t CurrentVertexCount){
	return Allocator.Allocate<LWMesh<LWVertexPosition>>(VertexBuffer, CurrentVertexCount);
}

LWVertexPosition::LWVertexPosition(const LWVector4f &Position) : m_Position(Position) {}

uint32_t LWVertexColor::WriteVertex(LWBaseMesh *Mesh, const LWVector4f &Position, const LWVector4f &Color){
	if (!Mesh->CanWriteVertices(1)) return 0;
	LWVertexColor *V = ((LWMesh<LWVertexColor>*)Mesh)->GetVertexAt(Mesh->WriteVertices(1));
	*V = { Position, Color };
	return 1;
}

uint32_t LWVertexColor::WriteRectangle(LWBaseMesh *Mesh, const LWVector2f &TopLeftPoint, const LWVector2f &BtmRightPoint, const LWVector4f &Color){
	if (!Mesh->CanWriteVertices(6)) return 0;
	LWVertexColor *V = ((LWMesh<LWVertexColor>*)Mesh)->GetVertexAt(Mesh->WriteVertices(6));
	LWVector4f TopLeftPnt = LWVector4f(TopLeftPoint, 0.0f, 1.0f);
	LWVector4f TopRightPnt = LWVector4f(BtmRightPoint.x, TopLeftPoint.y, 0.0f, 1.0f);
	LWVector4f BtmLeftPnt = LWVector4f(TopLeftPoint.x, BtmRightPoint.y, 0.0f, 1.0f);
	LWVector4f BtmRightPnt = LWVector4f(BtmRightPoint, 0.0f, 1.0f);
	V[0] = { TopLeftPnt, Color };
	V[1] = { BtmLeftPnt, Color };
	V[2] = { BtmRightPnt, Color };
	V[3] = { TopLeftPnt, Color };
	V[4] = { BtmRightPnt, Color };
	V[5] = { TopRightPnt, Color };
	return 6;
}

uint32_t LWVertexColor::WriteCircle(LWBaseMesh *Mesh, const LWVector2f &CenterPnt, const LWVector4f &Color, float Radius, uint32_t Steps, float MinTheta, float MaxTheta){
	uint32_t VertexCount = Steps * 3;
	if (!Mesh->CanWriteVertices(VertexCount)) return 0;
	LWVertexColor *V = ((LWMesh<LWVertexColor>*)Mesh)->GetVertexAt(Mesh->WriteVertices(VertexCount));
	LWVector2f p = LWVector2f::MakeTheta(MinTheta)*Radius;
	float ThetaInc = (MaxTheta-MinTheta) / (Steps-1);
	for (uint32_t n = 1; n <= Steps; n++){
		LWVector2f c = LWVector2f::MakeTheta(MinTheta+ThetaInc*n)*Radius;
		*V++ = { LWVector4f(CenterPnt + c, 0.0f, 1.0f), Color };
		*V++ = { LWVector4f(CenterPnt, 0.0f, 1.0f), Color };
		*V++ = { LWVector4f(CenterPnt + p, 0.0f, 1.0f), Color };
		p = c;
	}
	return VertexCount;
}

uint32_t LWVertexColor::WriteRectangle(LWBaseMesh *Mesh, const LWVector2f &CenterPoint, const LWVector2f &HalfSize, float Theta, const LWVector4f &Color) {
	if (!Mesh->CanWriteVertices(6)) return 0;
	LWVertexColor *V = ((LWMesh<LWVertexColor>*)Mesh)->GetVertexAt(Mesh->WriteVertices(6));
	LWVector2f Dir = LWVector2f::MakeTheta(Theta);
	LWVector2f TopLeft =  LWVector2f(-HalfSize.x, HalfSize.y);
	LWVector2f TopRight = LWVector2f(HalfSize.x, HalfSize.y);
	LWVector2f BtmLeft = LWVector2f(-HalfSize.x, -HalfSize.y);
	LWVector2f BtmRight = LWVector2f(HalfSize.x, -HalfSize.y);

	TopLeft = LWVector2f(TopLeft.x*Dir.x - TopLeft.y*Dir.y, TopLeft.x*Dir.y + TopLeft.y*Dir.x) + CenterPoint;
	TopRight = LWVector2f(TopRight.x*Dir.x - TopRight.y*Dir.y, TopRight.x*Dir.y + TopRight.y*Dir.x) + CenterPoint;
	BtmLeft = LWVector2f(BtmLeft.x*Dir.x - BtmLeft.y*Dir.y, BtmLeft.x*Dir.y + BtmLeft.y*Dir.x) + CenterPoint;
	BtmRight = LWVector2f(BtmRight.x*Dir.x - BtmRight.y*Dir.y, BtmRight.x*Dir.y + BtmRight.y*Dir.x) + CenterPoint;

	V[0] = { LWVector4f(TopLeft, 0.0f, 1.0f), Color };
	V[1] = { LWVector4f(BtmLeft, 0.0f, 1.0f), Color };
	V[2] = { LWVector4f(TopRight, 0.0f, 1.0f), Color };
	V[3] = { LWVector4f(BtmLeft, 0.0f, 1.0f), Color };
	V[4] = { LWVector4f(BtmRight, 0.0f, 1.0f), Color };
	V[5] = { LWVector4f(TopRight, 0.0f, 1.0f), Color };

	return 6;
}

uint32_t LWVertexColor::WriteLine(LWBaseMesh *Mesh, const LWVector2f &PntA, const LWVector2f &PntB, float Thickness, const LWVector4f &Color) {
	if (!Mesh->CanWriteVertices(6)) return 0;
	LWVertexColor *V = ((LWMesh<LWVertexColor>*)Mesh)->GetVertexAt(Mesh->WriteVertices(6));
	LWVector2f Perp = (PntB - PntA).Normalize().Perpindicular()*Thickness;

	V[0] = { LWVector4f(PntA - Perp, 0.0f, 1.0f), Color };
	V[1] = { LWVector4f(PntB - Perp, 0.0f, 1.0f), Color };
	V[2] = { LWVector4f(PntA + Perp, 0.0f, 1.0f), Color };
	V[3] = { LWVector4f(PntA + Perp, 0.0f, 1.0f), Color };
	V[4] = { LWVector4f(PntB - Perp, 0.0f, 1.0f), Color };
	V[5] = { LWVector4f(PntB + Perp, 0.0f, 1.0f), Color };
	return 6;
}


LWMesh<LWVertexColor> *LWVertexColor::MakeMesh(LWAllocator &Allocator, LWVideoBuffer *VertexBuffer, LWVideoBuffer *IndiceBuffer, uint32_t CurrentVertexCount, uint32_t CurrentIndiceCount) {
	return Allocator.Allocate<LWMesh<LWVertexColor>>(VertexBuffer, IndiceBuffer, CurrentVertexCount, CurrentIndiceCount);
}

LWMesh<LWVertexColor> *LWVertexColor::MakeMesh(LWAllocator &Allocator, LWVideoBuffer *VertexBuffer, uint32_t CurrentVertexCount){
	return Allocator.Allocate<LWMesh<LWVertexColor>>(VertexBuffer, CurrentVertexCount);
}

LWVertexColor::LWVertexColor(const LWVector4f &Position, const LWVector4f &Color) : m_Position(Position), m_Color(Color) {}

uint32_t LWVertexTexture::WriteVertex(LWBaseMesh *Mesh, const LWVector4f &Position, const LWVector2f &TexCoord){
	if (!Mesh->CanWriteVertices(1)) return 0;
	LWVertexTexture *V = ((LWMesh<LWVertexTexture>*)Mesh)->GetVertexAt(Mesh->WriteVertices(1));
	*V = { Position, LWVector4f(TexCoord, 0.0f, 0.0f) };
	return 1;
}

uint32_t LWVertexTexture::WriteRectangle(LWBaseMesh *Mesh, const LWVector2f &TopLeftPoint, const LWVector2f &BtmRightPoint, const LWVector2f &TopLeftTexCoord, const LWVector2f &BtmRightTexCoord){
	if (!Mesh->CanWriteVertices(6)) return 0;
	LWVertexColor *V = ((LWMesh<LWVertexColor>*)Mesh)->GetVertexAt(Mesh->WriteVertices(6));
	LWVector4f TopLeftPnt = LWVector4f(TopLeftPoint, 0.0f, 1.0f);
	LWVector4f TopRightPnt = LWVector4f(BtmRightPoint.x, TopLeftPoint.y, 0.0f, 1.0f);
	LWVector4f BtmLeftPnt = LWVector4f(TopLeftPoint.x, BtmRightPoint.y, 0.0f, 1.0f);
	LWVector4f BtmRightPnt = LWVector4f(BtmRightPoint, 0.0f, 1.0f);

	LWVector4f TopLeftTC = LWVector4f(TopLeftTexCoord, 0.0f, 0.0f);
	LWVector4f TopRightTC = LWVector4f(BtmRightTexCoord.x, TopLeftTexCoord.y, 0.0f, 0.0f);
	LWVector4f BtmLeftTC = LWVector4f(TopLeftTexCoord.x, BtmRightTexCoord.y, 0.0f, 0.0f);
	LWVector4f BtmRightTC = LWVector4f(BtmRightTexCoord, 0.0f, 0.0f);
	V[0] = { TopLeftPnt, TopLeftTC };
	V[1] = { BtmLeftPnt, BtmLeftTC };
	V[2] = { BtmRightPnt, BtmRightTC };
	V[3] = { TopLeftPnt, TopLeftTC };
	V[4] = { BtmRightPnt, BtmRightTC };
	V[5] = { TopRightPnt, TopRightTC };
	return 6;
}

LWMesh<LWVertexTexture> *LWVertexTexture::MakeMesh(LWAllocator &Allocator, LWVideoBuffer *VertexBuffer, LWVideoBuffer *IndiceBuffer, uint32_t CurrentVertexCount, uint32_t CurrentIndiceCount) {
	return Allocator.Allocate<LWMesh<LWVertexTexture>>(VertexBuffer, IndiceBuffer, CurrentVertexCount, CurrentIndiceCount);
}
LWMesh<LWVertexTexture> *LWVertexTexture::MakeMesh(LWAllocator &Allocator, LWVideoBuffer *VertexBuffer, uint32_t CurrentVertexCount){
	return Allocator.Allocate<LWMesh<LWVertexTexture>>(VertexBuffer, CurrentVertexCount);
}

LWVertexTexture::LWVertexTexture(const LWVector4f &Position, const LWVector4f &TexCoord) : m_Position(Position), m_TexCoord(TexCoord) {}


LWBaseMesh &LWBaseMesh::Finished(void){
	m_VertexBuffer->SetEditLength(m_TypeSize*m_ActiveVertexCount);
	if (m_IndiceBuffer) m_IndiceBuffer->SetEditLength(m_IndiceTypeSize*m_ActiveIndiceCount);
	return *this;
}

bool LWBaseMesh::isWriteable(void) const{
	return !m_VertexBuffer->isDirty();
}

bool LWBaseMesh::CanWriteVertices(uint32_t VertexCount){
	return (m_ActiveVertexCount + VertexCount <= m_MaxVertexCount);
}

bool LWBaseMesh::CanWriteIndices(uint32_t IndiceCount) {
	return (m_ActiveIndiceCount + IndiceCount <= m_MaxIndiceCount);
}

bool LWBaseMesh::isFinished(void) const{
	return m_VertexBuffer->isDirty();
}

LWBaseMesh &LWBaseMesh::ClearFinished(void){
	if(m_VertexBuffer->isDirty()){
		m_UploadedVertexCount = m_ActiveVertexCount;
		m_UploadedIndiceCount = m_ActiveIndiceCount;
		m_ActiveVertexCount = 0;
		m_ActiveIndiceCount = 0;
	}
	return *this;
}

LWBaseMesh &LWBaseMesh::SetActiveVertexCount(uint32_t ActiveVertexCount){
	m_ActiveVertexCount = ActiveVertexCount;
	return *this;
}

LWBaseMesh &LWBaseMesh::SetActiveIndiceCount(uint32_t ActiveIndiceCount) {
	m_ActiveIndiceCount = ActiveIndiceCount;
	return *this;
}

uint32_t LWBaseMesh::WriteVertices(uint32_t VerticeCount){
	uint32_t v = m_ActiveVertexCount;
	m_ActiveVertexCount += VerticeCount;
	return v;
}

uint32_t LWBaseMesh::WriteIndices(uint32_t IndiceCount) {
	uint32_t v = m_ActiveIndiceCount;
	m_ActiveIndiceCount += IndiceCount;
	return v;
}

uint32_t LWBaseMesh::GetUploadedCount(void) const{
	return m_UploadedVertexCount;
}

uint32_t LWBaseMesh::GetActiveCount(void) const{
	return m_ActiveVertexCount;
}

uint32_t LWBaseMesh::GetMaxCount(void) const{
	return m_MaxVertexCount;
}

uint32_t LWBaseMesh::GetUploadedIndiceCount(void) const {
	return m_UploadedIndiceCount;
}

uint32_t LWBaseMesh::GetActiveIndiceCount(void) const {
	return m_ActiveIndiceCount;
}

uint32_t LWBaseMesh::GetRenderCount(void) const {
	return m_IndiceBuffer ? m_UploadedIndiceCount : m_UploadedVertexCount;
}

uint32_t LWBaseMesh::GetMaxIndiceCount(void) const {
	return m_MaxIndiceCount;
}

uint32_t LWBaseMesh::GetTypeSize(void) const{
	return m_TypeSize;
}

uint32_t LWBaseMesh::GetIndiceTypeSize(void) const {
	return m_IndiceTypeSize;
}

LWVideoBuffer *LWBaseMesh::GetIndiceBuffer(void) const {
	return m_IndiceBuffer;
}

LWVideoBuffer *LWBaseMesh::GetVertexBuffer(void) const{
	return m_VertexBuffer;
}

LWBaseMesh::LWBaseMesh(LWVideoBuffer *VertexBuffer, LWVideoBuffer *IndiceBuffer, uint32_t CurrentVertexCount, uint32_t MaxVertexCount, uint32_t CurrentIndiceCount, uint32_t MaxIndiceCount) : m_VertexBuffer(VertexBuffer), m_IndiceBuffer(IndiceBuffer), m_TypeSize(0), m_IndiceTypeSize(0), m_ActiveIndiceCount(0), m_UploadedIndiceCount(CurrentIndiceCount), m_MaxIndiceCount(MaxIndiceCount), m_ActiveVertexCount(0), m_UploadedVertexCount(CurrentVertexCount), m_MaxVertexCount(MaxVertexCount) {
	m_TypeSize = m_VertexBuffer->GetRawLength() / m_MaxVertexCount;
	m_IndiceTypeSize = IndiceBuffer->GetType() == LWVideoBuffer::Index16 ? 16 : 32;
}

LWBaseMesh::LWBaseMesh(LWVideoBuffer *VertexBuffer, uint32_t CurrentVertexCount, uint32_t MaxVertexCount) : m_VertexBuffer(VertexBuffer), m_IndiceBuffer(nullptr),  m_TypeSize(0), m_IndiceTypeSize(0), m_ActiveIndiceCount(0), m_UploadedIndiceCount(0), m_MaxIndiceCount(0), m_ActiveVertexCount(0), m_UploadedVertexCount(CurrentVertexCount), m_MaxVertexCount(MaxVertexCount){
	m_TypeSize = m_VertexBuffer->GetRawLength() / m_MaxVertexCount;
}
