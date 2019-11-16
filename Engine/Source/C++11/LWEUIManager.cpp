#include "LWEUIManager.h"
#include <LWVideo/LWFont.h>
#include <LWPlatform/LWWindow.h>
#include <LWPlatform/LWFileStream.h>
#include <LWCore/LWText.h>
#include <LWCore/LWTimer.h>
#include "LWEAsset.h"
#include "LWEXML.h"
#include <map>
#include <cstdarg>
#include <iostream>
#include <algorithm>

LWEUIMaterial::LWEUIMaterial(const LWVector4f &Color) : m_Color(Color), m_Texture(nullptr), m_SubRegion(LWVector4f(0.0f, 0.0f, 1.0f, 1.0f)) {}

LWEUIMaterial::LWEUIMaterial(const LWVector4f &Color, LWTexture *Tex, const LWVector4f &SubRegion) : m_Color(Color), m_Texture(Tex), m_SubRegion(SubRegion) {}

bool LWEUIFrame::SetActiveTexture(LWTexture *Texture, bool FontTexture) {
	if (!m_TextureCount) {
		m_VertexCount[m_TextureCount] = 0;
		m_FontTexture[m_TextureCount] = FontTexture;
		m_Textures[m_TextureCount++] = Texture;
		return true;
	}
	uint32_t Active = m_TextureCount - 1;
	if (m_Textures[Active] == Texture && m_FontTexture[Active]==FontTexture) return true;
	if (m_TextureCount >= MaxTextures) return false;
	m_VertexCount[m_TextureCount] = 0;
	m_FontTexture[m_TextureCount] = FontTexture;
	m_Textures[m_TextureCount++] = Texture;
	return true;
}

bool LWEUIFrame::WriteFontGlyph(LWTexture *Texture, const LWVector2f &Position, const LWVector2f &Size, const LWVector4f &TexCoord, const LWVector4f &Color) {
	if (!SetActiveTexture(Texture, true)) return false;
	if (!m_Mesh->CanWriteVertices(6)) return false;
	LWVertexUI *V = m_Mesh->GetVertexAt(m_Mesh->WriteVertices(6));
	LWVector2f BtmLeft = Position;
	LWVector2f TopLeft = Position + LWVector2f(0.0f, Size.y);
	LWVector2f BtmRight = Position + LWVector2f(Size.x, 0.0f);
	LWVector2f TopRight = Position + Size;

	LWVector2f BtmLeftTC = LWVector2f(TexCoord.x, TexCoord.w);
	LWVector2f TopLeftTC = LWVector2f(TexCoord.x, TexCoord.y);
	LWVector2f BtmRightTC = LWVector2f(TexCoord.z, TexCoord.w);
	LWVector2f TopRightTC = LWVector2f(TexCoord.z, TexCoord.y);

	*(V + 0) = { LWVector4f(BtmLeft, 0.0f, 1.0f), Color, LWVector4f(BtmLeftTC, 0.0f, 0.0f) };
	*(V + 1) = { LWVector4f(TopRight, 0.0f, 1.0f), Color, LWVector4f(TopRightTC, 0.0f, 0.0f) };
	*(V + 2) = { LWVector4f(TopLeft, 0.0f, 1.0f), Color, LWVector4f(TopLeftTC, 0.0f, 0.0f) };
	*(V + 3) = { LWVector4f(BtmLeft, 0.0f, 1.0f), Color, LWVector4f(BtmLeftTC, 0.0f, 0.0f) };
	*(V + 4) = { LWVector4f(BtmRight, 0.0f, 1.0f), Color, LWVector4f(BtmRightTC, 0.0f, 0.0f) };
	*(V + 5) = { LWVector4f(TopRight, 0.0f, 1.0f), Color, LWVector4f(TopRightTC, 0.0f, 0.0f) };
	m_VertexCount[m_TextureCount - 1] += 6;
	return true;
}


uint32_t LWEUIFrame::WriteVertices(uint32_t VertexCount) {
	if (!m_TextureCount) return 0xFFFFFFFF;
	if (!m_Mesh->CanWriteVertices(VertexCount)) return 0xFFFFFFFF;
	uint32_t Active = m_TextureCount - 1;
	m_VertexCount[Active] += VertexCount;

	return m_Mesh->WriteVertices(VertexCount);
}

LWEUIFrame &LWEUIFrame::operator=(LWEUIFrame &&F) {
	m_Mesh = F.m_Mesh;
	m_TextureCount = F.m_TextureCount;
	for (uint32_t i = 0; i < m_TextureCount; i++) {
		m_Textures[i] = F.m_Textures[i];
		m_FontTexture[i] = F.m_FontTexture[i];
		m_VertexCount[i] = F.m_VertexCount[i];
	}
	return *this;
}

LWEUIFrame &LWEUIFrame::operator=(LWEUIFrame &F) {
	m_Mesh = F.m_Mesh;
	m_TextureCount = F.m_TextureCount;
	for (uint32_t i = 0; i < m_TextureCount; i++) {
		m_Textures[i] = F.m_Textures[i];
		m_FontTexture[i] = F.m_FontTexture[i];
		m_VertexCount[i] = F.m_VertexCount[i];
	}
	return *this;
}

LWEUIFrame::LWEUIFrame(LWEUIFrame &&F) : m_Mesh(F.m_Mesh), m_TextureCount(F.m_TextureCount) {
	for (uint32_t i = 0; i < m_TextureCount; i++) {
		m_Textures[i] = F.m_Textures[i];
		m_FontTexture[i] = F.m_FontTexture[i];
		m_VertexCount[i] = F.m_VertexCount[i];
	}
}

LWEUIFrame::LWEUIFrame(LWEUIFrame &F) : m_Mesh(F.m_Mesh), m_TextureCount(F.m_TextureCount) {
	for (uint32_t i = 0; i < m_TextureCount; i++) {
		m_Textures[i] = F.m_Textures[i];
		m_FontTexture[i] = F.m_FontTexture[i];
		m_VertexCount[i] = F.m_VertexCount[i];
	}
}

LWEUIFrame::LWEUIFrame(LWMesh<LWVertexUI> *Mesh) : m_Mesh(Mesh), m_TextureCount(0) {}

LWEUIFrame::LWEUIFrame() : m_Mesh(nullptr), m_TextureCount(0) {}

const char *LWEUIManager::GetTextureShaderSource(void) {
	static const char TextureSource[] = ""\
		"#module Vertex DirectX11_1\n"\
		"cbuffer UIUniform{\n"\
		"	float4x4 Matrix;\n"\
		"};\n"\
		"struct Vertex {\n"\
		"	float4 Position : POSITION;\n"\
		"	float4 Color : COLOR;\n"\
		"	float4 TexCoord : TEXCOORD;\n"\
		"};\n"\
		"struct Pixel {\n"\
		"	float4 Position : SV_POSITION;\n"\
		"	float4 Color : COLOR0;\n"\
		"	float4 TexCoord : TEXCOORD0;\n"\
		"};\n"\
		"Pixel main(Vertex In) {\n"\
		"	Pixel O;\n"\
		"	O.Position = mul(Matrix, In.Position);\n"\
		"	O.Color = In.Color;\n"\
		"	O.TexCoord = In.TexCoord;\n"\
		"	return O;\n"\
		"}\n"\
		"#module Pixel DirectX11_1\n"\
		"struct Pixel {\n"\
		"	float4 Position : SV_POSITION;\n"\
		"	float4 Color : COLOR0;\n"\
		"	float4 TexCoord : TEXCOORD0;\n"\
		"};\n"\
		"Texture2D Tex;\n"\
		"SamplerState TexSampler;\n"\
		"float4 main(Pixel In) : SV_TARGET{\n"\
		"	return In.Color*Tex.Sample(TexSampler, In.TexCoord.xy);\n"\
		"}\n"\
		"#module Vertex OpenGL3_2 OpenGL4_4\n"\
		"#version 330\n"\
		"layout(std140) uniform UIUniform {\n"\
		"	mat4 Matrix;\n"\
		"};\n"\
		"in vec4 Position | 0;\n"\
		"in vec4 Color | 1;\n"\
		"in vec4 TexCoord | 2;\n"\
		"out vec4 pColor;\n"\
		"out vec4 pTexCoord;\n"\
		"void main(void) {\n"\
		"	gl_Position = Matrix*Position;\n"\
		"	pColor = Color;\n"\
		"	pTexCoord = TexCoord;\n"\
		"}\n"\
		"#module Pixel OpenGL3_2 OpenGL4_4\n"\
		"#version 330\n"\
		"uniform sampler2D Tex;\n"\
		"in vec4 pColor;\n"\
		"in vec4 pTexCoord;\n"\
		"out vec4 p_Color | 0 | Output;\n"\
		"void main(void) {\n"\
		"	p_Color = texture(Tex, pTexCoord.xy)*pColor;\n"\
		"}\n"\
		"#module Vertex OpenGL2_1\n"\
		"attribute vec4 Position | 0;\n"\
		"attribute vec4 Color | 1;\n"\
		"attribute vec4 TexCoord | 2;\n"\
		"varying vec4 pColor;\n"\
		"varying vec4 pTexCoord;\n"\
		"#block UIUniform\n"\
		"uniform mat4 Matrix;\n"\
		"void main(void) {\n"\
		"	gl_Position = Matrix*Position;\n"\
		"	pColor = Color;\n"\
		"	pTexCoord = TexCoord;\n"\
		"}\n"\
		"#module Pixel OpenGL2_1\n"\
		"uniform sampler2D Tex;\n"\
		"varying vec4 pColor;\n"\
		"varying vec4 pTexCoord;\n"\
		"void main(void) {\n"\
		"	gl_FragColor = texture2D(Tex, pTexCoord.xy)*pColor;\n"\
		"}\n"\
		"#module Vertex OpenGLES2\n"\
		"attribute highp vec4 Position | 0;\n"\
		"attribute lowp vec4 Color | 1;\n"\
		"attribute lowp vec4 TexCoord | 2;\n"\
		"varying lowp vec4 pColor;\n"\
		"varying lowp vec4 pTexCoord;\n"\
		"#block UIUniform\n"\
		"uniform highp mat4 Matrix;\n"\
		"void main(void) {\n"\
		"	gl_Position = Matrix*Position;\n"\
		"	pColor = Color;\n"\
		"	pTexCoord = TexCoord;\n"\
		"}\n"\
		"#module Pixel OpenGLES2\n"\
		"uniform sampler2D Tex;\n"\
		"varying lowp vec4 pColor;\n"\
		"varying lowp vec4 pTexCoord;\n"\
		"void main(void) {\n"\
		"	gl_FragColor = texture2D(Tex, pTexCoord.xy)*pColor;\n"\
		"}\n";
		return TextureSource;
}

const char *LWEUIManager::GetColorShaderSource(void) {
	static const char ColorSource[] = ""\
		"#module Vertex DirectX11_1\n"\
		"cbuffer UIUniform{\n"\
		"	float4x4 Matrix;\n"\
		"};\n"\
		"struct Vertex {\n"\
		"	float4 Position : POSITION;\n"\
		"	float4 Color : COLOR;\n"\
		"	float4 TexCoord : TEXCOORD;\n"\
		"};\n"\
		"struct Pixel {\n"\
		"	float4 Position : SV_POSITION;\n"\
		"	float4 Color : COLOR0;\n"\
		"	float4 TexCoord : TEXCOORD0;\n"\
		"};\n"\
		"Pixel main(Vertex In) {\n"\
		"	Pixel O;\n"\
		"	O.Position = mul(Matrix, In.Position);\n"\
		"	O.Color = In.Color;\n"\
		"	O.TexCoord = In.TexCoord;\n"\
		"	return O;\n"\
		"}\n"\
		"#module Pixel DirectX11_1\n"\
		"struct Pixel {\n"\
		"	float4 Position : SV_POSITION;\n"\
		"	float4 Color : COLOR0;\n"\
		"	float4 TexCoord : TEXCOORD0;\n"\
		"};\n"\
		"float4 main(Pixel In) : SV_TARGET{\n"\
		"	return In.Color;\n"\
		"}\n"\
		"#module Vertex OpenGL3_2 OpenGL4_4\n"\
		"#version 330\n"\
		"layout(std140) uniform UIUniform {\n"\
		"	mat4 Matrix;\n"\
		"};\n"\
		"in vec4 Position | 0;\n"\
		"in vec4 Color | 1;\n"\
		"in vec4 TexCoord | 2;\n"\
		"out vec4 pColor;\n"\
		"out vec4 pTexCoord;\n"\
		"void main(void) {\n"\
		"	gl_Position = Matrix*Position;\n"\
		"	pColor = Color;\n"\
		"	pTexCoord = TexCoord;\n"\
		"}\n"\
		"#module Pixel OpenGL3_2 OpenGL4_4\n"\
		"#version 330\n"\
		"in vec4 pColor;\n"\
		"in vec4 pTexCoord;\n"\
		"out vec4 p_Color | 0 | Output;\n"\
		"void main(void) {\n"\
		"	p_Color = pColor;\n"\
		"}\n"\
		"#module Vertex OpenGL2_1\n"\
		"attribute vec4 Position | 0;\n"\
		"attribute vec4 Color | 1;\n"\
		"attribute vec4 TexCoord | 2;\n"\
		"varying vec4 pColor;\n"\
		"varying vec4 pTexCoord;\n"\
		"#block UIUniform\n"\
		"uniform mat4 Matrix;\n"\
		"void main(void) {\n"\
		"	gl_Position = Matrix*Position;\n"\
		"	pColor = Color;\n"\
		"	pTexCoord = TexCoord;\n"\
		"}\n"\
		"#module Pixel OpenGL2_1\n"\
		"varying vec4 pColor;\n"\
		"varying vec4 pTexCoord;\n"\
		"void main(void) {\n"\
		"	gl_FragColor = pColor;\n"\
		"}\n"\
		"#module Vertex OpenGLES2\n"\
		"attribute highp vec4 Position | 0;\n"\
		"attribute lowp vec4 Color | 1;\n"\
		"attribute lowp vec4 TexCoord | 2;\n"\
		"varying lowp vec4 pColor;\n"\
		"varying lowp vec4 pTexCoord;\n"\
		"#block UIUniform\n"\
		"uniform highp mat4 Matrix;\n"\
		"void main(void) {\n"\
		"	gl_Position = Matrix*Position;\n"\
		"	pColor = Color;\n"\
		"	pTexCoord = TexCoord;\n"\
		"}\n"\
		"#module Pixel OpenGLES2\n"\
		"varying lowp vec4 pColor;\n"\
		"varying lowp vec4 pTexCoord;\n"\
		"void main(void) {\n"\
		"	gl_FragColor = pColor;\n"\
		"}\n";
	return ColorSource;
}

const char *LWEUIManager::GetYUVTextureShaderSource(void) {
	static const char YUVSource[] = ""\
	"#module Vertex DirectX11_1\n"\
	"	cbuffer UIUniform{\n"\
	"		float4x4 Matrix;\n"\
	"};\n"\
	"struct Vertex {\n"\
	"	float4 Position : POSITION;\n"\
	"	float4 Color : COLOR;\n"\
	"	float4 TexCoord : TEXCOORD;\n"\
	"};\n"\
	"struct Pixel {\n"\
	"	float4 Position : SV_POSITION;\n"\
	"	float4 Color : COLOR0;\n"\
	"	float4 TexCoordA : TEXCOORD0;\n"\
	"	float4 TexCoordB : TEXCOORD1;\n"\
	"};\n"\
	"Pixel main(Vertex In) {\n"\
	"	const float Third = 1.0f / 3.0f;\n"\
	"	Pixel O;\n"\
	"	O.Position = mul(Matrix, In.Position);\n"\
	"	O.Color = In.Color;\n"\
	"	O.TexCoordA.xy = In.TexCoord.xy*float2(1.0f, Third*2.0f);\n"\
	"	O.TexCoordA.zw = In.TexCoord.xy*float2(0.5f, Third) + float2(0.0f, Third*2.0f);\n"\
	"	O.TexCoordB.xy = In.TexCoord.xy*float2(0.5f, Third) + float2(0.5f, Third*2.0f);\n"\
	"	return O;\n"\
	"}\n"\
	"#module Pixel DirectX11_1\n"\
	"	struct Pixel {\n"\
	"	float4 Position : SV_POSITION;\n"\
	"	float4 Color : COLOR0;\n"\
	"	float4 TexCoordA : TEXCOORD0;\n"\
	"	float4 TexCoordB : TEXCOORD1;\n"\
	"};\n"\
	"Texture2D Tex;\n"\
	"SamplerState TexSampler;\n"\
	"float4 DecodeYUV(float2 TexCoordY, float2 TexCoordU, float2 TexCoordV) {\n"\
	"	float Y = Tex.Sample(TexSampler, TexCoordY).r;\n"\
	"	float U = Tex.Sample(TexSampler, TexCoordU).r - 0.5f;\n"\
	"	float V = Tex.Sample(TexSampler, TexCoordV).r - 0.5f;\n"\
	"	float R = Y + 1.402f*V;\n"\
	"	float G = Y - 0.344f*U - 0.714f*V;\n"\
	"	float B = Y + 1.722f*U;\n"\
	"	return float4(R, G, B, 1.0f);\n"\
	"}\n"\
	"float4 main(Pixel In) : SV_TARGET{\n"\
	"	return DecodeYUV(In.TexCoordA.xy, In.TexCoordA.zw, In.TexCoordB.xy)*In.Color;\n"\
	"}\n"\
	"#module Vertex OpenGL3_2 OpenGL4_5\n"\
	"#version 330\n"\
	"layout(std140) uniform UIUniform {\n"\
	"	mat4 Matrix;\n"\
	"};\n"\
	"in vec4 Position;\n"\
	"in vec4 Color;\n"\
	"in vec4 TexCoord;\n"\
	"out vec4 pColor;\n"\
	"out vec4 pTexCoordA;\n"\
	"out vec4 pTexCoordB;\n"\
	"void main(void) {\n"\
	"	const float Third = 1.0f / 3.0f;\n"\
	"	gl_Position = Matrix * Position;\n"\
	"	pColor = Color;\n"\
	"	pTexCoordA.xy = TexCoord.xy*vec2(1.0f, Third*2.0f);\n"\
	"	pTexCoordA.zw = TexCoord.xy*vec2(0.5f, Third) + vec2(0.0f, Third*2.0f);\n"\
	"	pTexCoordB.xy = TexCoord.xy*vec2(0.5f, Third) + vec2(0.5f, Third*2.0f);\n"\
	"}\n"\
	"#module Pixel OpenGL3_2 OpenGL4_5\n"\
	"#version 330\n"\
	"in vec4 pColor;\n"\
	"in vec4 pTexCoordA;\n"\
	"in vec4 pTexCoordB;\n"\
	"uniform sampler2D Tex;\n"\
	"out vec4 p_Color | 0 | Output;\n"\
	"vec4 DecodeYUV(vec2 TexCoordY, vec2 TexCoordU, vec2 TexCoordV) {\n"\
	"	float Y = texture2D(Tex, TexCoordY).r;\n"\
	"	float U = texture2D(Tex, TexCoordU).r - 0.5f;\n"\
	"	float V = texture2D(Tex, TexCoordV).r - 0.5f;\n"\
	"	float R = Y + 1.402f*V;\n"\
	"	float G = Y - 0.344f*U - 0.714f*V;\n"\
	"	float B = Y + 1.722f*U;\n"\
	"	return vec4(R, G, B, 1.0f);\n"\
	"}\n"\
	"void main(void) {\n"\
	"	p_Color = DecodeYUV(pTexCoordA.xy, pTexCoordA.zw, pTexCoordB.xy)*pColor;\n"\
	"}\n"\
	"#module Vertex OpenGL2_1\n"\
	"attribute vec4 Position;\n"\
	"attribute vec4 Color;\n"\
	"attribute vec4 TexCoord;\n"\
	"varying vec4 pColor;\n"\
	"varying vec4 pTexCoordA;\n"\
	"varying vec4 pTexCoordB;\n"\
	"#block UIUniform\n"\
	"uniform mat4 Matrix;\n"\
	"void main(void) {\n"\
	"	const float Third = 1.0f / 3.0f;\n"\
	"	gl_Position = Matrix * Position;\n"\
	"	pColor = Color;\n"\
	"	pTexCoordA.xy = TexCoord.xy*vec2(1.0f, Third*2.0f);\n"\
	"	pTexCoordA.zw = TexCoord.xy*vec2(0.5f, Third) + vec2(0.0f, Third*2.0f);\n"\
	"	pTexCoordB.xy = TexCoord.xy*vec2(0.5f, Third) + vec2(0.5f, Third*2.0f);\n"\
	"}\n"\
	"#module Pixel OpenGL2_1\n"\
	"varying vec4 pColor;\n"\
	"varying vec4 pTexCoordA;\n"\
	"varying vec4 pTexCoordB;\n"\
	"uniform sampler2D Tex;\n"\
	"vec4 DecodeYUV(vec2 TexCoordY, vec2 TexCoordU, vec2 TexCoordV) {\n"\
	"	float Y = texture(Tex, TexCoordY).r;\n"\
	"	float U = texture(Tex, TexCoordU).r - 0.5f;\n"\
	"	float V = texture(Tex, TexCoordV).r - 0.5f;\n"\
	"	float R = Y + 1.402f*V;\n"\
	"	float G = Y - 0.344f*U - 0.714f*V;\n"\
	"	float B = Y + 1.722f*U;\n"\
	"	return vec4(R, G, B, 1.0f);\n"\
	"}\n"\
	"void main(void) {\n"\
	"	gl_FragColor = DecodeYUV(pTexCoordA.xy, pTexCoordA.zw, pTexCoordB.xy)*pColor;\n"\
	"}\n"\
	"#module Vertex OpenGLES2\n"\
	"attribute highp vec4 Position;\n"\
	"attribute lowp vec4 Color;\n"\
	"attribute lowp vec4 TexCoord;\n"\
	"varying lowp vec4 pColor;\n"\
	"varying lowp vec4 pTexCoordA;\n"\
	"varying lowp vec4 pTexCoordB;\n"\
	"#block UIUniform\n"\
	"uniform highp mat4 Matrix;\n"\
	"void main(void) {\n"\
	"	const float Third = 1.0f / 3.0f;\n"\
	"	gl_Position = Matrix * Position;\n"\
	"	pColor = Color;\n"\
	"	pTexCoordA.xy = TexCoord.xy*vec2(1.0f, Third*2.0f);\n"\
	"	pTexCoordA.zw = TexCoord.xy*vec2(0.5f, Third) + vec2(0.0f, Third*2.0f);\n"\
	"	pTexCoordB.xy = TexCoord.xy*vec2(0.5f, Third) + vec2(0.5f, Third*2.0f);\n"\
	"}\n"\
	"#module Pixel OpenGLES2\n"\
	"varying lowp vec4 pColor;\n"\
	"varying lowp vec4 pTexCoordA;\n"\
	"varying lowp vec4 pTexCoordB;\n"\
	"uniform sampler2D Tex;\n"\
	"vec4 DecodeYUV(vec2 TexCoordY, vec2 TexCoordU, vec2 TexCoordV) {\n"\
	"	float Y = texture(Tex, TexCoordY).r;\n"\
	"	float U = texture(Tex, TexCoordU).r - 0.5f;\n"\
	"	float V = texture(Tex, TexCoordV).r - 0.5f;\n"\
	"	float R = Y + 1.402f*V;\n"\
	"	float G = Y - 0.344f*U - 0.714f*V;\n"\
	"	float B = Y + 1.722f*U;\n"\
	"	return vec4(R, G, B, 1.0f);\n"\
	"}\n"\
	"void main(void) {\n"\
	"	gl_FragColor = DecodeYUV(pTexCoordA.xy, pTexCoordA.zw, pTexCoordB.xy)*pColor;\n"\
	"}\n";
	return YUVSource;
}

bool LWEUIManager::XMLParser(LWEXMLNode *Node, void *UserData, LWEXML *X) {
	std::map<uint32_t, LWEXMLNode*> StyleMap;
	std::map<uint32_t, LWEXMLNode*> ComponentMap;
	uint32_t StyleCount = 0;
	LWEUIManager *Manager = (LWEUIManager*)UserData;

	auto ParseMaterial = [](LWEXMLNode *Node, LWEUIManager *Man)->bool {
		LWXMLAttribute *NameAttr = Node->FindAttribute("Name");
		LWXMLAttribute *ColorAttr = Node->FindAttribute("Color");
		LWXMLAttribute *TexAttr = Node->FindAttribute("Texture");
		LWXMLAttribute *SubRegionAttr = Node->FindAttribute("SubRegion");
		if (!NameAttr) return false;
		LWVector4f Color = LWVector4f(1.0f);
		LWTexture *Tex = nullptr;
		LWVector4f SubRegion = LWVector4f(0.0f, 0.0f, 1.0f, 1.0f);
		if (ColorAttr) {
			const char *C = LWText::NextWord(ColorAttr->m_Value, true);
			
			if (*C == '#') {
				uint32_t Val = 0;
				sscanf(C, "#%x", &Val);
				Color.x = (float)((Val >> 24)&0xFF) / 255.0f;
				Color.y = (float)((Val >> 16)&0xFF) / 255.0f;
				Color.z = (float)((Val >> 8) & 0xFF) / 255.0f;
				Color.w = (float)((Val) & 0xFF) / 255.0f;
			}else sscanf(ColorAttr->m_Value, "%f|%f|%f|%f", &Color.x, &Color.y, &Color.z, &Color.w);
		}
		if (TexAttr) {
			LWEAsset *A = Man->GetAssetManager()->GetAsset(TexAttr->m_Value);
			if (A && A->GetType() == LWEAsset::Texture) Tex = A->AsTexture();
		}
		if (SubRegionAttr && Tex) {
			LWVector2i TexSize = Tex->Get2DSize();
			LWVector4i Region = LWVector4i(0, 0, TexSize);
			sscanf(SubRegionAttr->m_Value, "%d|%d|%d|%d", &Region.x, &Region.y, &Region.z, &Region.w);
			SubRegion.x = ((float)Region.x + 0.5f) / (float)TexSize.x;
			SubRegion.y = ((float)Region.y + 0.5f) / (float)TexSize.y;
			SubRegion.z = ((float)(Region.x +Region.z) - 0.5f) / (float)TexSize.x;
			SubRegion.w = ((float)(Region.y + Region.w) - 0.5f) / (float)TexSize.y;
		}
		return Man->InsertMaterial(NameAttr->m_Value, Color, Tex, SubRegion) != nullptr;
	};

	auto ParseStyle = [](LWEXMLNode *Node, LWEUIManager *Man, std::map<uint32_t, LWEXMLNode*> &StyleMap) {
		LWXMLAttribute *NameAttr = Node->FindAttribute("Name");
		if (!NameAttr) return;
		StyleMap.insert(std::pair<uint32_t, LWEXMLNode*>(LWText::MakeHash(NameAttr->m_Value), Node));
		return;
	};

	auto ParseUIDPIScale = [](LWEXMLNode *Node, LWEUIManager *Man) {
		LWXMLAttribute *DPIAttr = Node->FindAttribute("DPI");
		LWXMLAttribute *ScaleAttr = Node->FindAttribute("Scale");
		if (!DPIAttr || !ScaleAttr) return;
		uint32_t DPI = (uint32_t)atoi(DPIAttr->m_Value);
		float s = (float)atof(ScaleAttr->m_Value);
		Man->PushDPIScale(DPI, s);
		return;
	};

	auto ParseUIResScale = [](LWEXMLNode *Node, LWEUIManager *Man) {

		LWXMLAttribute *WidthAttr = Node->FindAttribute("Width");
		LWXMLAttribute *HeightAttr = Node->FindAttribute("Height");
		LWXMLAttribute *ScaleAttr = Node->FindAttribute("Scale");
		if (!WidthAttr || !HeightAttr || !ScaleAttr) return;
		int32_t w = atoi(WidthAttr->m_Value);
		int32_t h = atoi(HeightAttr->m_Value);
		float s = (float)atof(ScaleAttr->m_Value);
		Man->PushScreenScale(LWVector2i(w, h), s);
		return;
	};

	auto ParseComponent = [](LWEXMLNode *Node, LWEUIManager *Man, std::map<uint32_t, LWEXMLNode*> &ComponentMap) {
		LWXMLAttribute *NameAttr = Node->FindAttribute("Name");
		if (!NameAttr) return;
		ComponentMap.insert(std::pair<uint32_t, LWEXMLNode*>(LWText::MakeHash(NameAttr->m_Value), Node));
		return;
	};

	auto ParseExternal = [](LWEXMLNode *Node, LWEXMLNode *Parent, LWEUIManager *Man, LWEXML *X) {
		char XMLBuffer[64 * 1024]; //max of 64 kb to parse external files.
		LWXMLAttribute *SrcAttr = Node->FindAttribute("Src");
		if (!SrcAttr) return;
		LWFileStream Stream;
		if (!LWFileStream::OpenStream(Stream, SrcAttr->m_Value, LWFileStream::ReadMode | LWFileStream::BinaryMode, *Man->GetAllocator())) {
			std::cout << "Error opening external: '" << SrcAttr->m_Value << "'" << std::endl;
			return;
		}
		Stream.ReadText(XMLBuffer, sizeof(XMLBuffer));
		if (!LWEXML::ParseBuffer(*X, *Man->GetAllocator(), XMLBuffer, true, Parent, Node)) {
			std::cout << "Error parsing: '" << SrcAttr->m_Value << "'" << std::endl;
			return;
		}
		return;
	};


	for (LWEXMLNode *C = X->NextNode(nullptr, Node); C; C = X->NextNode(C, Node, true)) {
		uint32_t Idx = LWText::CompareMultiple(C->m_Name, 6, "Material", "Style", "UIScale", "DPIScale", "Component", "External");
		
		if (Idx == 0) ParseMaterial(C, Manager);
		else if (Idx == 1) ParseStyle(C, Manager, StyleMap);
		else if (Idx == 2) ParseUIResScale(C, Manager);
		else if (Idx == 3) ParseUIDPIScale(C, Manager);
		else if (Idx == 4) ParseComponent(C, Manager, ComponentMap);
		else if (Idx == 5) ParseExternal(C, Node, Manager, X);
		else{
			LWEUI::XMLParseSubNodes(nullptr, C, X, Manager, "", nullptr, nullptr, StyleMap, ComponentMap);
		}
	}
	return true;
}

LWEUIManager &LWEUIManager::Update(const LWVector2f &Position, const LWVector2f &Size, float Scale, uint64_t lCurrentTime) {
	m_VisiblePosition = Position;
	m_VisibleSize = Size;
	memset(m_TempCount, 0, sizeof(m_TempCount));
	m_LastScale = Scale;
	bool OnlyFocusedTIBox = false;
	LWKeyboard *Keyboard = m_Window->GetKeyboardDevice();

	if (m_FocusedUI) {
		LWEUITextInput *TI = dynamic_cast<LWEUITextInput*>(m_FocusedUI);
		if (TI && m_Window->GetFlag()&LWWindow::KeyboardPresent) {
			LWVector4f KeyboardDim = m_Window->GetKeyboardLayout();
			LWVector2f RemainSize = LWVector2f((float)m_Window->GetSize().x, (float)m_Window->GetSize().y) - LWVector2f(KeyboardDim.z, KeyboardDim.w);
			LWVector2f TextBoxPos;
			TextBoxPos.x = KeyboardDim.x + RemainSize.x*0.5f - TI->GetVisibleSize().x*0.5f;
			TextBoxPos.x = std::max<float>(TextBoxPos.x, KeyboardDim.x + RemainSize.x*0.1f);
			TextBoxPos.y = KeyboardDim.y + KeyboardDim.w + RemainSize.y*0.5f - TI->GetVisibleSize().y*0.5f;
			TextBoxPos.y = std::min<float>(TextBoxPos.y + TI->GetVisibleSize().y - RemainSize.y*0.1f, TextBoxPos.y + TI->GetVisibleSize().y);
			TI->SetVisiblePosition(TextBoxPos);
			TI->UpdateSelf(this, Scale, lCurrentTime);
			OnlyFocusedTIBox = true;
		}
	}

	if (m_FirstUI && !OnlyFocusedTIBox) {
		m_FirstUI->Update(this, Scale, lCurrentTime);
		if (Keyboard) {
			LWEUI *N = m_FocusedUI;
			if (Keyboard->ButtonPressed(LWKey::Tab)) {
				if (Keyboard->ButtonDown(LWKey::LShift)) {
					for (LWEUI *C = GetNext(nullptr); C; C = GetNext(C, (C->GetFlag()&LWEUI::Invisible) != 0)) {
						if (C->GetFlag()&LWEUI::Invisible) continue;
						if (C == m_FocusedUI) break;
						if (C->GetFlag()&LWEUI::TabAble) N = C;
					}
				} else {
					for (N = GetNext(N); N; N = GetNext(N, (N->GetFlag()&LWEUI::Invisible) != 0)) {
						if (N->GetFlag()&LWEUI::Invisible) continue;
						if (N->GetFlag()&LWEUI::TabAble) break;
					}
				}
			}
			if (N != m_FocusedUI) SetFocused(N);
		}
	}
	memcpy(m_OverCount, m_TempCount, sizeof(m_OverCount));
	return *this;
}

LWEUIManager &LWEUIManager::Update(float Scale, uint64_t lCurrentTime) {
	LWVector2i WndSize = m_Window->GetSize();
	return Update(LWVector2f(0.0f), LWVector2f((float)WndSize.x, (float)WndSize.y), Scale, lCurrentTime);
}

LWEUIManager &LWEUIManager::Update(uint64_t lCurrentTime) {
	LWVector2i WndSize = m_Window->GetSize();
	float S = FindScaleForSize(WndSize);
	return Update(LWVector2f(0.0f), LWVector2f((float)WndSize.x, (float)WndSize.y), S, lCurrentTime);
}

LWEUIManager &LWEUIManager::Draw(LWEUIFrame *Frame, float Scale, uint64_t lCurrentTime) {

	bool OnlyFocusedTIBox = false;
	if (m_FocusedUI) {
		LWEUITextInput *TI = dynamic_cast<LWEUITextInput*>(m_FocusedUI);
		if (TI && m_Window->GetFlag()&LWWindow::KeyboardPresent) {
			TI->DrawSelf(this, Frame, Scale, lCurrentTime);
			OnlyFocusedTIBox = true;
		}
	}
	if (m_FirstUI && !OnlyFocusedTIBox) m_FirstUI->Draw(this, Frame, Scale, lCurrentTime);
	return *this;
}

LWEUIManager &LWEUIManager::Draw(LWEUIFrame *Frame, uint64_t lCurrentTime) {
	return Draw(Frame, m_LastScale, lCurrentTime);
}

LWEUI *LWEUIManager::GetNext(LWEUI *Current, bool SkipChildren) {
	if (!Current) return m_FirstUI;
	if (Current->GetFirstChild() && !SkipChildren) return Current->GetFirstChild();
	if (!Current->GetNext()) {
		if (Current->GetParent()) return GetNext(Current->GetParent(), true);
	}
	return Current->GetNext();
}

LWEUIManager &LWEUIManager::InsertUIAfter(LWEUI *UI, LWEUI *Parent, LWEUI *Prev) {
	UI->SetParent(Parent);
	if (!Prev) {
		if (!Parent) {
			UI->SetNext(m_FirstUI);
			m_FirstUI = UI;
			if (!m_LastUI) m_LastUI = m_FirstUI;
		} else {
			UI->SetNext(Parent->GetFirstChild());
			Parent->SetFirstChild(UI);
			if (!Parent->GetLastChild()) Parent->SetLastChild(UI);
		}
	} else {
		UI->SetParent(Prev->GetParent()).SetNext(Prev->GetNext());
		Prev->SetNext(UI);
		if (Prev->GetParent()) {
			if (Parent->GetLastChild() == Prev) Parent->SetLastChild(UI);
		} else {
			if (m_LastUI == Prev) m_LastUI = UI;
		}
	}
	return *this;
}

LWEUIManager &LWEUIManager::RemoveUI(LWEUI *UI, bool Destroy) {
	LWEUI *PrevUI = nullptr;
	LWEUI *Parent = UI->GetParent();

	for (LWEUI *U = Parent?Parent->GetFirstChild():m_FirstUI; U != UI; PrevUI = U, U = U->GetNext()) {}
	if (Parent) {
		if (!PrevUI) Parent->SetFirstChild(UI->GetNext());
		else PrevUI->SetNext(UI->GetNext());
		
		if (Parent->GetLastChild() == UI) Parent->SetLastChild(PrevUI);
	} else {
		if (!PrevUI) m_FirstUI = UI->GetNext();
		else PrevUI->SetNext(UI->GetNext());

		if (m_LastUI == UI) m_LastUI = PrevUI;
	}

	if (Destroy) {

		if (dynamic_cast<LWEUIRect*>(UI)) LWAllocator::Destroy((LWEUIRect*)UI);
		else if (dynamic_cast<LWEUIButton*>(UI)) LWAllocator::Destroy((LWEUIButton*)UI);
		else if (dynamic_cast<LWEUILabel*>(UI)) LWAllocator::Destroy((LWEUILabel*)UI);
		else if (dynamic_cast<LWEUITextInput*>(UI)) LWAllocator::Destroy((LWEUITextInput*)UI);
		else if (dynamic_cast<LWEUIScrollBar*>(UI)) LWAllocator::Destroy((LWEUIScrollBar*)UI);
		else if (dynamic_cast<LWEUIListBox*>(UI)) LWAllocator::Destroy((LWEUIListBox*)UI);
		else if (dynamic_cast<LWEUIComponent*>(UI)) LWAllocator::Destroy((LWEUIComponent*)UI);
		else if (dynamic_cast<LWEUIAdvLabel*>(UI)) LWAllocator::Destroy((LWEUIAdvLabel*)UI);
	}
	return *this;
}

bool LWEUIManager::RegisterEvent(LWEUI *UI, uint32_t EventCode, std::function<void(LWEUI*, uint32_t, void*)> Callback, void *UserData) {
	if (!UI) return false;
	return UI->RegisterEvent(EventCode, Callback, UserData);
}

bool LWEUIManager::RegisterEvent(const LWText &UIName, uint32_t EventCode, std::function<void(LWEUI *, uint32_t, void *)> Callback, void *UserData) {
	LWEUI *UI = GetNamedUI(UIName);
	if (!UI) return false;
	return RegisterEvent(UI, EventCode, Callback, UserData);
}

bool LWEUIManager::UnregisterEvent(LWEUI *UI, uint32_t EventCode) {
	if (!UI) return false;
	return UI->UnregisterEvent(EventCode);
}

bool LWEUIManager::UnregisterEvent(const LWText &UIName, uint32_t EventCode) {
	LWEUI *UI = GetNamedUI(UIName);
	if (!UI) return false;
	return UnregisterEvent(UI, EventCode);
}

LWEUIManager &LWEUIManager::DispatchEvent(LWEUI *Dispatchee, uint32_t EventCode) {
	uint32_t ECode = EventCode&LWEUI::Event_Flags;
	uint32_t TouchIdx = (EventCode&LWEUI::Event_OverIdx) >> LWEUI::Event_OverOffset;
	if (ECode == LWEUI::Event_TempOverInc && (Dispatchee->GetFlag()&LWEUI::IgnoreOverCounter)==0) {
		m_TempCount[TouchIdx]++;
		return *this;
	}
	Dispatchee->DispatchEvent(ECode);
	return *this;
}

LWEUIManager &LWEUIManager::DispatchEvent(const char *DispatcheeName, uint32_t EventCode) {
	auto Iter = m_NameMap.find(LWText::MakeHash(DispatcheeName));
	if (Iter == m_NameMap.end()) return *this;
	return DispatchEvent(Iter->second, EventCode);
}

LWEUIManager &LWEUIManager::DispatchEventf(const char *DispathceeNameFmt, uint32_t EventCode, ...) {
	char Buffer[256];
	va_list lst;
	va_start(lst, EventCode);
	vsnprintf(Buffer, sizeof(Buffer), DispathceeNameFmt, lst);
	va_end(lst);
	return DispatchEvent(Buffer, EventCode);
}

LWEUIManager &LWEUIManager::SetFocused(LWEUI *UI) {
	LWEUI *PrevFoc = m_FocusedUI;
	m_FocusedUI = UI;
	if (PrevFoc) {
		DispatchEvent(PrevFoc, LWEUI::Event_LostFocus);
		if (dynamic_cast<LWEUITextInput*>(PrevFoc)) m_Window->CloseKeyboard();
		
	}
	if (m_FocusedUI) {
		DispatchEvent(m_FocusedUI, LWEUI::Event_Focused);
		if (dynamic_cast<LWEUITextInput*>(m_FocusedUI)) {
			if(m_FocusedUI->GetFlag()&LWEUI::TouchEnabled) m_Window->OpenKeyboard();
		}
	}
	return *this;
}

bool LWEUIManager::PushScreenScale(const LWVector2i &Resolution, float Scale) {
	if (m_ResScaleCount >= MaxScreenScales) return false;
	uint32_t Area = Resolution.x*Resolution.y;
	uint32_t i = 0;
	for (; i < m_ResScaleCount; i++) {
		if (m_ResScaleMap[i].m_ScreenArea >= Area) break;
	}
	std::copy_backward(m_ResScaleMap + i, m_ResScaleMap + m_ResScaleCount, m_ResScaleMap + (m_ResScaleCount - 1));
	m_ResScaleMap[i].m_ScreenArea = Area;
	m_ResScaleMap[i].m_Scale = Scale;
	m_ResScaleCount++;
	return true;
}

bool LWEUIManager::PushDPIScale(uint32_t DPI, float Scale) {
	if (m_DPIScaleCount >= MaxDPIScales) return false;
	uint32_t i = 0;
	for (; i < m_DPIScaleCount; i++) {
		if (m_DPIScaleMap[i].m_ScreenDPI >= DPI) break;
	}
	std::copy_backward(m_DPIScaleMap + i, m_DPIScaleMap + m_DPIScaleCount, m_DPIScaleMap + (m_DPIScaleCount - 1));
	m_DPIScaleMap[i].m_ScreenDPI = DPI;
	m_DPIScaleMap[i].m_Scale = Scale;
	m_DPIScaleCount++;
	return true;
}

float LWEUIManager::FindScaleForSize(const LWVector2i &Size) {
	float ScreenScale = m_ResScaleCount ? m_ResScaleMap[m_ResScaleCount-1].m_Scale : 1.0f;
	float DPIScale = m_CachedDPIScale;
	uint32_t Area = Size.x*Size.y;
	for (uint32_t i = 1; i < m_ResScaleCount; i++) {
		if (Area < m_ResScaleMap[i].m_ScreenArea) {
			ScreenScale = m_ResScaleMap[i - 1].m_Scale;
			break;
		}
	}
	if (m_CachedDPIScale == 0.0f) {
		DPIScale = m_DPIScaleCount ? m_DPIScaleMap[m_DPIScaleCount-1].m_Scale : 1.0f;
		for (uint32_t i = 1; i < m_DPIScaleCount; i++) {
			if (m_DPIScaleMap[i].m_ScreenDPI > m_ScreenDPI) {
				uint32_t DPILen = m_DPIScaleMap[i].m_ScreenDPI - m_DPIScaleMap[i - 1].m_ScreenDPI;
				if (!DPILen) {
					DPIScale = m_DPIScaleMap[i].m_Scale;
				} else {
					float v = (float)(m_DPIScaleMap[i].m_ScreenDPI - m_ScreenDPI) / (float)DPILen;
					DPIScale = m_DPIScaleMap[i - 1].m_Scale + v * (m_DPIScaleMap[i].m_Scale - m_DPIScaleMap[i - 1].m_Scale);
				}
				break;
			}
		}
		m_CachedDPIScale = DPIScale;
	}
	return ScreenScale*DPIScale;
}

bool LWEUIManager::InsertNamedUI(const LWText &Name, LWEUI *UI) {
	auto Res = m_NameMap.insert(std::pair<uint32_t, LWEUI*>(Name.GetHash(), UI));
	return Res.second;
}

bool LWEUIManager::isTextInputFocused(void) {
	return dynamic_cast<LWEUITextInput*>(m_FocusedUI) != nullptr;
}

LWEUI *LWEUIManager::GetNamedUI(const LWText &Name) {
	auto Iter = m_NameMap.find(Name.GetHash());
	if (Iter == m_NameMap.end()) std::cout << "Could not find ui: '" << Name.GetCharacters() << "'" << std::endl;
	return Iter == m_NameMap.end() ? nullptr : Iter->second;
}

LWEUI *LWEUIManager::GetNamedUIf(const char *Format, ...) {
	char Buffer[256];
	va_list lst;
	va_start(lst, Format);
	vsnprintf(Buffer, sizeof(Buffer), Format, lst);
	va_end(lst);
	return GetNamedUI(Buffer);
}

LWEUIMaterial *LWEUIManager::InsertMaterial(const LWText &Name, const LWVector4f &Color, LWTexture *Texture, const LWVector4f &SubRegion) {
	if (m_MaterialCount >= MaxMaterials) return nullptr;
	LWEUIMaterial *Mat = m_MaterialTable + m_MaterialCount;
	Mat->m_Color = Color;
	Mat->m_Texture = Texture;
	Mat->m_SubRegion = SubRegion;
	auto Res = m_MatTable.insert(std::pair<uint32_t, LWEUIMaterial*>(Name.GetHash(), Mat));
	if (!Res.second) return nullptr;
	m_MaterialCount++;
	return Mat;
}

LWEUIMaterial *LWEUIManager::GetMaterial(const LWText &Name) {
	auto Iter = m_MatTable.find(Name.GetHash());
	return Iter == m_MatTable.end() ? nullptr : Iter->second;
}

LWVector2f LWEUIManager::GetVisibleSize(void) const {
	return m_VisibleSize;
}

LWVector2f LWEUIManager::GetVisiblePosition(void) const {
	return m_VisiblePosition;
}

LWELocalization *LWEUIManager::GetLocalization(void) {
	return m_Localization;
}

LWEAssetManager *LWEUIManager::GetAssetManager(void) {
	return m_AssetManager;
}

LWAllocator *LWEUIManager::GetAllocator(void) {
	return m_Allocator;
}

LWWindow *LWEUIManager::GetWindow(void) {
	return m_Window;
}

LWEUI *LWEUIManager::GetFirstUI(void) {
	return m_FirstUI;
}

LWEUI *LWEUIManager::GetLastUI(void) {
	return m_LastUI;
}

LWEUI *LWEUIManager::GetFocusedUI(void) {
	return m_FocusedUI;
}

uint32_t LWEUIManager::GetOverCount(uint32_t PointerIdx) const{
	return m_OverCount[PointerIdx];
}

float LWEUIManager::GetLastScale(void) const {
	return m_LastScale;
}

uint32_t LWEUIManager::GetScreenDPI(void) const{
	return m_ScreenDPI;
}

LWEUIManager::LWEUIManager(LWWindow *Window, uint32_t ScreenDPI, LWAllocator *Allocator, LWELocalization *Localization, LWEAssetManager *AssetManager) : m_Window(Window), m_AssetManager(AssetManager), m_Allocator(Allocator), m_Localization(Localization), m_FirstUI(nullptr), m_LastUI(nullptr), m_FocusedUI(nullptr), m_LastScale(1.0f), m_MaterialCount(0), m_EventCount(0), m_ScreenDPI(ScreenDPI), m_ResScaleCount(0), m_DPIScaleCount(0), m_CachedDPIScale(0.0f) {
	memset(m_OverCount, 0, sizeof(m_OverCount));
}

LWEUIManager::~LWEUIManager() {
	std::function<void(LWEUI *U)> RecursiveDestroy;
	RecursiveDestroy = [&RecursiveDestroy](LWEUI *N) {
		for (LWEUI *C = N->GetFirstChild(), *K = C ? C->GetNext() : C; C; C = K, K = K ? K->GetNext() : K) {
			RecursiveDestroy(C);
		}
		if (dynamic_cast<LWEUIRect*>(N)) LWAllocator::Destroy((LWEUIRect*)N);
		else if (dynamic_cast<LWEUIButton*>(N)) LWAllocator::Destroy((LWEUIButton*)N);
		else if (dynamic_cast<LWEUILabel*>(N)) LWAllocator::Destroy((LWEUILabel*)N);
		else if (dynamic_cast<LWEUITextInput*>(N)) LWAllocator::Destroy((LWEUITextInput*)N);
		else if (dynamic_cast<LWEUIScrollBar*>(N)) LWAllocator::Destroy((LWEUIScrollBar*)N);
		else if (dynamic_cast<LWEUIListBox*>(N)) LWAllocator::Destroy((LWEUIListBox*)N);
		else if (dynamic_cast<LWEUIComponent*>(N)) LWAllocator::Destroy((LWEUIComponent*)N);
		else if (dynamic_cast<LWEUIAdvLabel*>(N)) LWAllocator::Destroy((LWEUIAdvLabel*)N);
	};
	for (LWEUI *C = m_FirstUI, *K = C ? C->GetNext() : C; C; C = K, K = K ? K->GetNext() : K) {
		RecursiveDestroy(C);
	}
}



