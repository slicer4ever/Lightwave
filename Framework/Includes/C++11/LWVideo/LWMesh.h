#ifndef LWMESH_H
#define LWMESH_H
#include "LWVideo/LWTypes.h"
#include "LWVideo/LWVideoBuffer.h"
#include "LWVideo/LWVideoDriver.h"
#include "LWCore/LWTypes.h"
#include "LWCore/LWVector.h"
#include "LWCore/LWMath.h"

/*!< \brief a ui vertex, used in font, and other ui elements. */
struct LWVertexUI{
	LWVector4f m_Position; /*!< \brief position of the vertex. */
	LWVector4f m_Color; /*!< \brief the color to modulate the texture by. */
	LWVector4f m_TexCoord; /*!< \brief the texture coordinates(only the x, and y coordinates are considered, the last 2 are padding). */

	/*!< \brief writes a single vertex to the buffer for the mesh if possible. returns the number of vertices written. */
	static uint32_t WriteVertex(LWBaseMesh *Mesh, const LWVector2f &Position, const LWVector4f &Color, const LWVector2f &TexCoord);

	/*1< \brief writes a single vertex to the buffer for the mesh if possible. returns the number of vertices written. */
	static uint32_t WriteVertex(LWBaseMesh *Mesh, const LWVector3f &Position, const LWVector4f &Color, const LWVector2f &TexCoord);

	/*!< \brief writes a single vertex to the buffer for the mesh if possible.  returns the number of vertices written. */
	static uint32_t WriteVertex(LWBaseMesh *Mesh, const LWVector4f &Position, const LWVector4f &Color, const LWVector2f &TexCoored);

	/*!< \brief writes a centered rectangle to the buffer for the mesh, if possible. returns the number of vertices written. */
	static uint32_t WriteRect(LWBaseMesh *Mesh, const LWVector2f &CtrPnt, const LWVector2f &Size, const LWVector4f &Color, const LWVector2f &TexCtrPnt, const LWVector2f &TexSize);

	/*!< \brief writes a rotated centered rectangle to the buffer for the mesh, if possible. Theta is in radians. returns the number of vertices written. */
	static uint32_t WriteRect(LWBaseMesh *Mesh, const LWVector2f &CtrPnt, const LWVector2f &Size, float Theta, const LWVector4f &Color, const LWVector2f &TexCtrPnt, const LWVector2f &TexSize);

	/*!< \brief writes a simple rectangle to the buffer for the mesh, if possible. returns the number of vertices written. */
	static uint32_t WriteRectangle(LWBaseMesh *Mesh, const LWVector2f &TopLeftPoint, const LWVector2f &BtmRightPoint, const LWVector4f &Color, const LWVector2f &TopLeftTexCoord, const LWVector2f &BtmRightTexCoord);

	/*!< \brief clips the rectangle with the AABB specified, returns the number of vertices written(can be 0 if the entire rectangle is to be clipped.) AABB is a 4 vec where x = left, y = bottom, z = width, w = height of box. */
	static uint32_t WriteRectangle(LWBaseMesh *Mesh, const LWVector2f &TopLeftPoint, const LWVector2f &BtmRightPoint, const LWVector4f &Color, const LWVector2f &TopLeftTexCoord, const LWVector2f &BtmRightTexCoord, const LWVector4f &AABB);

	/*!< \brief constructs a mesh object with indice for working with. */
	static LWMesh<LWVertexUI> *MakeMesh(LWAllocator &Allocator, LWVideoBuffer *VertexBuffer, LWVideoBuffer *IndiceBuffer, uint32_t CurrentVertexCount, uint32_t CurrentIndiceCount);

	/*!< \brief constructs a mesh object for working with. */
	static LWMesh<LWVertexUI> *MakeMesh(LWAllocator &Allocator, LWVideoBuffer *VertexBuffer, uint32_t CurrentVertexCount);
};

/*!< \brief simple positional vertex, with a number of of helper functions. */
struct LWVertexPosition{
	LWVector4f m_Position; /*!< \brief position of the vertex. */

	/*!< \brief writes a vertex to the buffer for the mesh, if possible. returns the number of vertices written. */
	static uint32_t WriteVertex(LWBaseMesh *Mesh, const LWVector4f &Position);

	/*!< \brief writes a 2D rectangle to the buffer. */
	static uint32_t WriteRectangle(LWBaseMesh *Mesh, const LWVector2f &TopLeftPoint, const LWVector2f &BtmRightPoint);

	/*!< \brief writes a 2D triangle filled circle to the buffer. 
		 \param Mesh the mesh to write to.
		 \param CenterPnt the center point of the vector.
		 \param Radius the radius of the circle.
		 \param Steps the number of steps to take for a full circle, the higher the count, the more circle like the shape.
	*/
	static uint32_t WriteCircle(LWBaseMesh *Mesh, const LWVector2f &CenterPnt, float Radius, uint32_t Steps);

	/*!< \brief constructs a mesh object with indice for working with. */
	static LWMesh<LWVertexPosition> *MakeMesh(LWAllocator &Allocator, LWVideoBuffer *VertexBuffer, LWVideoBuffer *IndiceBuffer, uint32_t CurrentVertexCount, uint32_t CurrentIndiceCount);

	/*!< \brief constructs a mesh object for working with. */
	static LWMesh<LWVertexPosition> *MakeMesh(LWAllocator &Allocator, LWVideoBuffer *VertexBuffer, uint32_t CurrentVertexCount);
};

/*!< \brief simple positional and color vertex. */
struct LWVertexColor{
	LWVector4f m_Position; /*!< \brief position of the vertex. */
	LWVector4f m_Color; /*!< \brief color of the vertex. */

	/*!< \brief writes a vertex to the buffer for the mesh, if possible. returns the number of vertices written. */
	static uint32_t WriteVertex(LWBaseMesh *Mesh, const LWVector4f &Position, const LWVector4f &Color);

	/*!< \brief writes a 2D rectangle to the buffer. */
	static uint32_t WriteRectangle(LWBaseMesh *Mesh, const LWVector2f &TopLeftPoint, const LWVector2f &BtmRightPoint, const LWVector4f &Color);

	/*!< \brief writes a 2D triangle filled circle to the buffer.
		 \param Mesh the mesh to write to.
		 \param CenterPnt the center point of the circle.
		 \param Radius the radius of the circle.
		 \param Steps the number of steps to take for a full circle, the higher the count, the more circle like the shape.
		 */
	static uint32_t WriteCircle(LWBaseMesh *Mesh, const LWVector2f &CenterPnt, const LWVector4f &Color, float Radius, uint32_t Steps, float MinTheta = 0.0f, float MaxTheta = LW_2PI);

	static uint32_t WriteRectangle(LWBaseMesh *Mesh, const LWVector2f &CenterPoint, const LWVector2f &HalfSize, float Theta, const LWVector4f &Color);

	static uint32_t WriteLine(LWBaseMesh *Mesh, const LWVector2f &PntA, const LWVector2f &PntB, float Thickness, const LWVector4f &Color);

	/*!< \brief constructs a mesh object with indice for working with. */
	static LWMesh<LWVertexColor> *MakeMesh(LWAllocator &Allocator, LWVideoBuffer *VertexBuffer, LWVideoBuffer *IndiceBuffer, uint32_t CurrentVertexCount, uint32_t CurrentIndiceCount);

	/*!< \brief constructs a mesh object for working with. */
	static LWMesh<LWVertexColor> *MakeMesh(LWAllocator &Allocator, LWVideoBuffer *VertexBuffer, uint32_t CurrentVertexCount);
};

/*!< \brief simple position, and texture coordinate vertex. */
struct LWVertexTexture{
	LWVector4f m_Position; /*!< \brief position of the vertex. */
	LWVector4f m_TexCoord; /*!< \brief texture coordinate, only the first two parameters are used, the last two are padding. */

	/*!< \brief writes a vertex to the buffer for the mesh, if possible.  returns the number of vertices written. */
	static uint32_t WriteVertex(LWBaseMesh *Mesh, const LWVector4f &Position, const LWVector2f &TexCoord);

	/*!< \brief writes a 2D rectangle to the buffer. */
	static uint32_t WriteRectangle(LWBaseMesh *Mesh, const LWVector2f &TopLeftPoint, const LWVector2f &BtmRightPoint, const LWVector2f &TopLeftTexCoord, const LWVector2f &BtmRightTexCoord);

	/*!< \brief construsts a mesh object with indice for working with. */
	static LWMesh<LWVertexTexture> *MakeMesh(LWAllocator &Allocator, LWVideoBuffer *VertexBuffer, LWVideoBuffer *IndiceBuffer, uint32_t CurrentVertexCount, uint32_t CurrentIndiceCount);

	/*!< \brief constructs a mesh object for working with. */
	static LWMesh<LWVertexTexture> *MakeMesh(LWAllocator &Allocator, LWVideoBuffer *VertexBuffer, uint32_t CurrentVertexCount);
};

/*!< \brief the base mesh object, which handles all the buffer operations. this object should never be created by itself. */
class LWBaseMesh{
public:
	
	/*!< \brief marks the vertex mesh as dirty. after this function is called, it is not safe to write additional vertices to the vertex buffer. */
	LWBaseMesh &Finished(void);

	/*!< \brief returns rather or not it is safe to write to the underlying buffer. */
	bool isWriteable(void) const;

	/*!< \brief returns if the vertex mesh is dirty. */
	bool isFinished(void) const;

	/*!< \brief set's the uploaded vertices to the active vertices. */
	LWBaseMesh &ClearFinished(void);

	/*!< \brief tests if we have enough room to write the vertices being requested. */
	bool CanWriteVertices(uint32_t VertexCount);

	/*!< \brief tests if we have enough room to write the indices being requested. */
	bool CanWriteIndices(uint32_t IndiceCount);

	/*!< \brief sets the current active vertex pointer. */
	LWBaseMesh &SetActiveVertexCount(uint32_t ActiveVertexCount);

	/*!< \brief sets the current active indice position. */
	LWBaseMesh &SetActiveIndiceCount(uint32_t ActiveIndiceCount);

	/*!< \brief increments the internal vertex count, and returns the previous count where we can write the vertices to. */
	uint32_t WriteVertices(uint32_t VerticeCount);

	/*!< \brief increments the internal indice count, and returns the previous count where we can write the indices to. */
	uint32_t WriteIndices(uint32_t IndiceCount);

	/*!< \brief returns the current uploaded to gpu vertex count. */
	uint32_t GetUploadedCount(void) const;

	/*!< \brief returns the current active writing vertex count. */
	uint32_t GetActiveCount(void) const;

	/*!< \brief returns the max number of vertex that can be written. */
	uint32_t GetMaxCount(void) const;

	/*!< \brief returns the currently uploaded to gpu indice count. */
	uint32_t GetUploadedIndiceCount(void) const;

	/*!< \brief returns the current active writing indice count. */
	uint32_t GetActiveIndiceCount(void) const;

	/*!< \brief returns the max number of indices that can be written. */
	uint32_t GetMaxIndiceCount(void) const;

	/*!< \brief returns the render count(if no indice buffer is attached this is the uploadedVertexCount, if indice buffer is attached this is uploadedIndiceCount). */
	uint32_t GetRenderCount(void) const;

	/*!< \brief returns the type size of each vertex. */
	uint32_t GetTypeSize(void) const;

	/*!< \brief returns the indice type size(16/32). */
	uint32_t GetIndiceTypeSize(void) const;

	/*!< \brief returns the underlying vertex video buffer associated with the mesh. */
	LWVideoBuffer *GetVertexBuffer(void) const;

	/*!< \brief returns the underlying indice video buffer associated with the mesh. */
	LWVideoBuffer *GetIndiceBuffer(void) const;

	/*!< \brief consturcts a vertex+indice mesh object. */
	LWBaseMesh(LWVideoBuffer *VertexBuffer, LWVideoBuffer *IndiceBuffer, uint32_t CurrentVertexCount, uint32_t MaxVertexCount, uint32_t CurrentIndiceCount, uint32_t MaxIndiceCount);

	/*!< \brief constructs a vertex only mesh object. */
	LWBaseMesh(LWVideoBuffer *VertexBuffer, uint32_t CurrentVertexCount, uint32_t MaxVertexCount);
protected:
	LWVideoBuffer *m_VertexBuffer; //Vertex buffer.
	LWVideoBuffer *m_IndiceBuffer; //Indice buffer.
	uint32_t m_TypeSize; //Vertice type size. 
	uint32_t m_IndiceTypeSize; //Indice type size

	uint32_t m_ActiveIndiceCount; //Currently written to indices.
	uint32_t m_UploadedIndiceCount; //Currently uploaded indices.
	uint32_t m_MaxIndiceCount; //Max total indices.

	uint32_t m_ActiveVertexCount; //Currently written to Vertices.
	uint32_t m_UploadedVertexCount; //Currently uploaded vertices.
	uint32_t m_MaxVertexCount; //Max total vertices.
};

/*!< \brief the actual mesh objects which contains the actual vertex container. */
template<class VertexType>
class LWMesh : public LWBaseMesh{
public:
	/*!< \brief returns the vertex from the vertex video buffer at the specified position. */
	VertexType *GetVertexAt(uint32_t Position){
		return ((VertexType*)m_VertexBuffer->GetLocalBuffer()) + Position;
	}

	/*!< \brief returns the 32 bit indice from the indice video buffer at the specified position. */
	uint32_t *GetIndice32At(uint32_t Position) {
		return ((uint32_t*)m_IndiceBuffer->GetLocalBuffer()) + Position;
	}

	/*!< \brief returns the 16 bit indice from the indice video buffer at the specified position. */
	uint16_t *GetIndice16At(uint32_t Position) {
		return ((uint16_t*)m_IndiceBuffer->GetLocalBuffer()) + Position;
	}

	/*!< \brief automatically destroys this mesh object and the underlying vertice buffers unless specified not to. */
	void Destroy(LWVideoDriver *Driver, bool DestroyVertexBuffer = true, bool DestroyIndiceBuffer = true) {
		if (DestroyIndiceBuffer && m_IndiceBuffer) Driver->DestroyVideoBuffer(m_IndiceBuffer);
		if (DestroyVertexBuffer && m_VertexBuffer) Driver->DestroyVideoBuffer(m_VertexBuffer);
		LWAllocator::Destroy(this);
	}

	/*!< \brief constructs the mesh object with an indice buffer. */
	LWMesh(LWVideoBuffer *VertexBuffer, LWVideoBuffer *IndiceBuffer, uint32_t CurrentVertexCount, uint32_t CurrentIndiceCount) : LWBaseMesh(VertexBuffer, IndiceBuffer, CurrentIndiceCount, VertexBuffer->GetRawLength() / sizeof(VertexType), CurrentIndiceCount, IndiceBuffer->GetRawLength() / (IndiceBuffer->GetType() == LWVideoBuffer::Index16 ? 16 : 32)) {}

	/*!< \brief constructs the mesh object. */
	LWMesh(LWVideoBuffer *VertexBuffer, uint32_t CurrentVertexCount) : LWBaseMesh(VertexBuffer, CurrentVertexCount, VertexBuffer->GetRawLength()/sizeof(VertexType)){}
protected:
};

#endif