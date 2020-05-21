#ifndef LWIMAGE_H
#define LWIMAGE_H
#include "LWCore/LWVector.h"
#include "LWCore/LWAllocator.h"
#include "LWPlatform/LWTypes.h"

/*!< \brief the basic image class which supports loading png, tga, ktx2, and dds files.*/
class LWImage{
public:
	enum{
		RGBA8 = 0, /*!< \brief pack red green blue alpha, each component is 8 bits. */
		RGBA8U, /*!< \brief pack red green blue alpha, each component is 8 unsigned bits. */
		RGBA16, /*!< \brief pack red green blue alpha, each component is 16 bits. .*/
		RGBA16U, /*!< \brief pack red green blue alpha, each component is 16 unsigned bits. */
		RGBA32, /*!< \brief pack red green blue alpha, each component is 32 bits. */
		RGBA32U, /*!< \brief pack red green blue alpha, each component is 32 unsigned bits. */
		RGBA32F, /*!< \brief pack red green blue alpha, each component is 32 bit floating point number. */
		RG8, /*!< \brief pack red and green, each component is 8 bits. */
		RG8U, /*!< \brief pack red and green, each component is 8 unsigned bits. */
		RG16, /*!< \brief pack red and green, each component is 16 bits. */
		RG16U, /*!< \brief pack red and green, each component is 16 unsigned bits. */
		RG32, /*!< \brief pack red and green, each component is 32 bits. */
		RG32U, /*!< \brief pack red and green, each component is 32 unsigned bits. */
		RG32F, /*!< \brief pack red and green, each component is 32 bit floating point number. */
		R8, /*!< \brief pack single 8 bit component. */
		R8U, /*!< \brief pack single 8 unsigned bit component. */
		R16, /*!< \brief pack single 16 bit component. */
		R16U, /*!< \brief pack single 16 unsigned bit component. */
		R32, /*!< \brief pack single 32 bit component. */
		R32U, /*!< \brief pack single 32 unsigned bit component. */
		R32F, /*!< \brief pack single 32 bit floating point number. */
		DEPTH16, /*!< \brief pack type is a single 16 bit depth type. */
		DEPTH24, /*!< \brief pack type is a single 24 bit depth type. */
		DEPTH32, /*!< \brief pack type is a single 32 bit depth type. */
		DEPTH24STENCIL8, /*!< \brief pack type is a 32 bit type that makes up 24 bits of depth, and 8 bits of stencil. */
		DXT1, /*!< \brief pack type is a compressed DXT1 type(128 bits per 16 pixels). most image functions will not function with this pack type. */
		DXT2, /*!< \brief pack type is a compressed DXT2 type(128 bits per 16 pixels), most image functions will not function with this pack type. */
		DXT3, /*!< \brief pack type is a compressed DXT3 type(128 bits per 16 pixels), most image functions will not function with this pack type. */
		DXT4, /*!< \brief pack type is a compressed DXT4 type(128 bits per 16 pixels), most image functions will not function with this pack type. */
		DXT5, /*!< \brief pack type is a compressed DXT5 type(128 bits per 16 pixels), most image functions will not function with this pack type. */
		DXT6, /*!< \brief pack type is a compressed DXT6 type(128 bits per 16 pixels). most image functions will not function with this pack type. */
		DXT7, /*!< \brief pack type is a compressed DXT7 type(128 bits per 16 pixels). most image functions will not function with this pack type. */

		PackTypeBits = 0xFF, /*!< \brief pack type part of the flag bits. */

		NearestFilter = 0, /*!< \brief mipmap generation uses the nearest pixel to the original texture. */
		LinearFilter = 1, /*!< \brief mipmap generation uses linear interpolation of weighted average of the four texture elements that are closest of the origin pixel. */

		Image1D=0x0, /*!< \brief image type is a 1D image. */
		Image2D=0x100, /*!< \brief image type is a 2D image. */
		Image3D=0x200, /*!< \brief image type is a 3D image. */
		ImageCubeMap=0x300, /*!< \brief image type is a cube map.  face order is POS_X, NEG_X, POS_Y, NEG_Y, POS_Z, NEG_Z */

		ImageTypeBits = 0xF00 /*!< \brief bits of flag to represent the type of image. */
	};
	static const uint32_t MaxTextureImages = 128;

	/*!< \brief image loading which automatically attempts to load an image from a file source, and deduce it's type based on the associated extension. 
		 \return true on success, or false on failure.
	*/
	static bool LoadImage(LWImage &Image, const LWText &FilePath, LWAllocator &Allocator, LWFileStream *ExistingStream = nullptr);

	/*!< \brief loads a TGA file if it is a supported type. */
	static bool LoadImageTGA(LWImage &Image, const LWText &FilePath, LWAllocator &Allocator, LWFileStream *ExistingStream = nullptr);
	
	/*!< \brief loads a tga file from a memory buffer. */
	static bool LoadImageTGA(LWImage &Image, const uint8_t *Buffer, uint32_t BufferLen, LWAllocator &Allocator);

	/*!< \brief loads a PNG file. */
	static bool LoadImagePNG(LWImage &Image, const LWText &FilePath, LWAllocator &Allocator, LWFileStream *ExistingStream = nullptr);

	/*!< \brief loads a png file from a memory buffer. */
	static bool LoadImagePNG(LWImage &Image, const uint8_t *Buffer, uint32_t BufferLen, LWAllocator &Allocator);

	/*!< \brief loads a dds file. */
	static bool LoadImageDDS(LWImage &Image, const LWText &FilePath, LWAllocator &Allocator, LWFileStream *ExistingStream = nullptr);

	/*!< \brief loads a dds file from a memory buffer. */
	static bool LoadImageDDS(LWImage &Image, const uint8_t *Buffer, uint32_t BufferLen, LWAllocator &Allocator);

	/*! \brief loads a ktx2 file. */
	static bool LoadImageKTX2(LWImage &Image, const LWText &FilePath, LWAllocator &Allocator, LWFileStream *ExistingStream = nullptr);

	/*! \brief loads a ktx2 file from a memory buffer. */
	static bool LoadImageKTX2(LWImage &Image, const uint8_t *Buffer, uint32_t BufferLen, LWAllocator &Allocator);

	/*!< \brief copys n texels into buffer at a certain weight.
		 \param PackType the pack type of the texels.
		 \param TexelCnt the number of texels and weights.
		 \param TexelPtrs the pointer to each texel to be summed together.
		 \param TexelWeights the weights to apply to each texel when being summed together.
		 \param Buffer the buffer to write out to.
		 \return the number of bytes written into buffer(or number of bytes if buffer is null.)
	*/
	static uint32_t CopyWeightedTexels(uint32_t PackType, uint32_t TexelCnt, const uint8_t **TexelPtrs, const float *TexelWeights, uint8_t *Buffer);

	/*!< \brief samples a 1D Image of length and writes the results into buffer(if not null) the nearest sample to u(0-1).
		\param Texels the image data to be sampled from.
		\param Length the number of texels in the 1D image.
		\param PackType the image data's packing type.
		\param u where to sample from in 0-1 units.
		\param Buffer where to store the results(pass null to not write anything.)
		\return the amount of bytes written to buffer(rather buffer is null or not.)
	*/
	static uint32_t SampleNearest1D(const uint8_t *Texels, uint32_t Length, uint32_t PackType, float u, uint8_t *Buffer);

	/*!< \brief samples a 1D Image of length and writes the results into buffer(if not null) the linear sample of near by texels to u(0-1).
		\param Texels the image data to be sampled from.
		\param Length the number of texels in the 1D image.
		\param PackType the image data's packing type.
		\param u where to sample from in 0-1 units.
		\param Buffer where to store the results(pass null to not write anything.)
		\return the amount of bytes written to buffer(rather buffer is null or not.)
	*/
	static uint32_t SampleLinear1D(const uint8_t *Texels, uint32_t Length, uint32_t PackType, float u, uint8_t *Buffer);

	/*!< \brief samples a 2D Image of size and writes the results into buffer(if not null) the nearest sample to u(horizontal 0-1) v(Vertical 0-1).
		\param Texels the image data to be sampled from.
		\param Size the 2d size of texels in the image.
		\param PackType the image data's packing type.
		\param uv where to sample from in 0-1 units.
		\param Buffer where to store the results(pass null to not write anything.)
		\return the amount of bytes written to buffer(rather buffer is null or not.)
	*/
	static uint32_t SampleNearest2D(const uint8_t *Texels, const LWVector2i &Size, uint32_t PackType, const LWVector2f &UV, uint8_t *Buffer);

	/*!< \brief samples a 2D Image of size and writes the results into buffer(if not null) the linear sample of near by texels to u(horizontal 0-1) v(Vertical 0-1).
		\param Texels the image data to be sampled from.
		\param Size the 2d size of texels in the image.
		\param PackType the image data's packing type.
		\param uv where to sample from in 0-1 units.
		\param Buffer where to store the results(pass null to not write anything.)
		\return the amount of bytes written to buffer(rather buffer is null or not.)
	*/
	static uint32_t SampleLinear2D(const uint8_t *Texels, const LWVector2i &Size, uint32_t PackType, const LWVector2f &UV, uint8_t *Buffer);

	/*!< \brief samples a 3D Image of size and writes the results into buffer(if not null) the nearest sample to u(horizontal 0-1) v(Vertical 0-1) r(depth 0-1).
		\param Texels the image data to be sampled from.
		\param Size the 3d size of texels in the image.
		\param PackType the image data's packing type.
		\param uvr where to sample from in 0-1 units.
		\param Buffer where to store the results(pass null to not write anything.)
		\return the amount of bytes written to buffer(rather buffer is null or not.)
	*/
	static uint32_t SampleNearest3D(const uint8_t *Texels, const LWVector3i &Size, uint32_t PackType, const LWVector3f &UVR, uint8_t *Buffer);

	/*!< \brief samples a 3D Image of size and writes the results into buffer(if not null) the linear sample of near by texels to u(horizontal 0-1) v(Vertical 0-1) r(depth 0-1).
		\param Texels the image data to be sampled from.
		\param Size the 3d size of texels in the image.
		\param PackType the image data's packing type.
		\param uvr where to sample from in 0-1 units.
		\param Buffer where to store the results(pass null to not write anything.)
		\return the amount of bytes written to buffer(rather buffer is null or not.)
	*/
	static uint32_t SampleLinear3D(const uint8_t *Texels, const LWVector3i &Size, uint32_t PackType, const LWVector3f &UVR, uint8_t *Buffer);

	/*!< \brief writes into buffer a new 1D image with the specified mipmap level of the 1D image. 
		 \note pass null into buffer to only receive the specified mipmap's expected width.
		 \return the number of bytes in the new mipmap image.
	*/
	static uint32_t MakeMipmapLevel1D(const uint8_t *Texels, uint32_t Width, uint32_t PackType, uint32_t MipmapLevel, uint8_t *Buffer, uint32_t SampleMode = LinearFilter);

	/*!< \brief writes into buffer a new 2D image with the specified mipmap level of the 2D image.
		 \note pass null into buffer to only receive the specified mipmap's expected width and height.
		 \return the number of bytes in the new mipmap image.
	*/
	static uint32_t MakeMipmapLevel2D(const uint8_t *Texels, const LWVector2i &Size, uint32_t PackType, uint32_t MipmapLevel, uint8_t *Buffer, uint32_t SampleMode = LinearFilter);

	/*!< \brief writes into buffer a new 3D image with the specified mipmap level of the 3D image.
		 \note pass null into buffer to only receive the specified mipmap's expected width, height, and depth.
		 \return the number of bytes in the new mipmap image.
	*/
	static uint32_t MakeMipmapLevel3D(const uint8_t *Texels, const LWVector3i &Size, uint32_t PackType, uint32_t MipmapLevel, uint8_t *Buffer, uint32_t SampleMode = LinearFilter);

	/*!< \brief calculates the total number of mipmaps for an 1D image of x size: */
	static uint32_t MipmapCount(uint32_t Size);

	/*!< \brief calculates the total number of mipmaps for an 2D image of WidthxHeight size: */
	static uint32_t MipmapCount(const LWVector2i &Size);

	/*!< \brief calculates the total number of mipmaps for an 3D image of WidthxHeightxDepth size: */
	static uint32_t MipmapCount(const LWVector3i &Size);

	/*!< \brief returns the total number of bits per component for a particular pack type.  compressed formats return the block size. */
	static uint32_t GetBitSize(uint32_t PackType);

	/*!< \brief returns the total number of bytes to meet the intended stride rows for an image with size and packing type.  compressed images should multiply this value by (Height+3)/4 to get the total number of bytes to contain an image. */
	static uint32_t GetStride(int32_t Width, uint32_t PackType);

	/*!< \brief returns the total number of bytes to contain a 1D image. */
	static uint32_t GetLength1D(int32_t Width, uint32_t PackType);

	/*!< \brief returns the total number of bytes to contain a 2D image(compressed or uncompressed.) */
	static uint32_t GetLength2D(const LWVector2i &Size, uint32_t PackType);

	/*!< \brief returns the total number of bytes to contain a 3D image. */
	static uint32_t GetLength3D(const LWVector3i &Size, uint32_t PackType);

	/*!< \brief returns true if the pack type is a compressed format type. */
	static bool CompressedType(uint32_t PackType);
	
	/*!< \brief returns true if the pack type is a depth format type. */
	static bool DepthType(uint32_t PackType);

	/*!< \brief returns the mipmap size for the specified mipmap level of the source length. */
	static uint32_t MipmapSize1D(uint32_t SrcLen, uint32_t MipmapLevel);

	/*!< \brief returns the mipmap 2D size for the specified mipmap level of the source size. */
	static LWVector2i MipmapSize2D(const LWVector2i &SrcSize, uint32_t MipmapLevel);

	/*!< \brief returns the mipmap 3D size for the specified mipmap level of the source size. */
	static LWVector3i MipmapSize3D(const LWVector3i &SrcSize, uint32_t MipmapLevel);

	/*!< \brief auto generates all mipmap levels for the image. */
	LWImage &BuildMipmaps(uint32_t SampleMode);

	/*!< \brief returns the packing order of the image. */
	uint32_t GetPackType(void) const;

	/*!< \brief returns the type of image. */
	uint32_t GetType(void) const;

	/*!< \brief returns the width of a 1D image. */
	int32_t GetSize1D(void) const;

	/*!< \brief returns the dimensions of a 2D image. */
	LWVector2i GetSize2D(void) const;

	/*!< \brief returns the dimensions of a 3D image. */
	LWVector3i GetSize3D(void) const;

	/*!< \brief returns the mipmap size of the 1d image at the specified mipmap level. */
	int32_t GetMipmapSize1D(uint32_t MipLevel) const;

	/*!< \brief returns the mipmap size of the 2d image at the specified mipmap level. */
	LWVector2i GetMipmapSize2D(uint32_t MipLevel) const;

	/*!< \brief returns the mipmap size of the 3d image at the specified mipmap level. */
	LWVector3i GetMipmapSize3D(uint32_t MipLevel) const;

	/*!< \brief returns the number of mipmaps in the image. */
	uint32_t GetMipmapCount(void) const;

	/*!< \brief returns a non-const pointer to the internal texels, which allows for the application to manipulate the underlying data freely. 
		 \param Index the 3D-index of textures to modify.
	*/
	uint8_t *GetTexels(uint32_t Index);

	/*!< \brief returns the underlying texture arrays. */
	uint8_t **GetTexels(void);

	/*!< \brief move operator. */
	LWImage &operator = (LWImage &&Image);

	/*!< \brief copy operator. */
	LWImage &operator = (const LWImage &Image);

	/*!< \brief move construcutor. */
	LWImage(LWImage &&Image);

	/*!< \brief copy constructor. */
	LWImage(const LWImage &Image);

	/*!< \brief Constructs a 1D LWImage object. DXT images are not supported.
		 \param Size the size of the image.
		 \param PackType the packing type of the image.
		 \param Texels the texel data(or null if no data is to be passed in).
		 \param MipmapCount the number of mipmap texels that are provided for the image.
		 \param Allocator the allocator used to create the internal texel data.
	*/
	LWImage(int32_t Size, uint32_t PackType, uint8_t **Texels, uint32_t MipmapCount, LWAllocator &Allocator);
	
	/*< \brief Constructs a 2D LWImage object.
		\param Size the size of the image.
		\param PackType the packing type of the image.  or ImageCubemap with this parameter if this is to be a cubemap image instead of a regular 2D image.
		\param Texels the texel data(or null if no data is to be passed in).  the order of texels for a cube map is expected to be the face + mipmaps then the next face, etc. in the order of POS_X, NEG_X, POS_Y, NEG_Y, POS_Z, NEG_Z.
		\param MipmapCount the number of mipmap texels that are provided for the image.
		\param Allocator the allocator used to create the internal texel data. */
	LWImage(const LWVector2i &Size, uint32_t PackType, uint8_t **Texels, uint32_t MipmapCount, LWAllocator &Allocator);

	/*< \brief Constructs a 3D LWImage object.  DXT images are not supported.
		\param Size the size of the image.
		\param PackType the packing type of the image.
		\param Texels the texel data(or null if no data is to be passed in).
		\param MipmapCount the number of mipmap texels that are provided for the image.
		\param Allocator the allocator used to create the internal texel data. 
	*/
	LWImage(const LWVector3i &Size, uint32_t PackType, uint8_t **Texels, uint32_t MipmapCount, LWAllocator &Allocator);

	/*!< \brief the default constructor. */
	LWImage();

	/*!< \brief Destructor. */
	~LWImage();
	
private:
	LWAllocator *m_Allocator;
	LWVector3i m_Size;
	uint32_t m_MipmapCount;
	uint32_t m_Flag;
	uint8_t *m_Texels[MaxTextureImages];
};

#endif