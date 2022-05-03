#ifndef LWIMAGE_H
#define LWIMAGE_H
#include "LWCore/LWVector.h"
#include "LWCore/LWAllocator.h"
#include "LWPlatform/LWTypes.h"

/*!< \brief the basic image class which supports loading png, tga, ktx2, and dds files.
	 \param note for older rendering api's, srgb is not directly supported, as such all images will be treated as linear images.
*/
class LWImage{
public:
	enum{
		SRGBA = 0, /*!< \brief pack red green blue alpha, each component is 8 bit.  when processed by the gpu this will automatically convert from srgb to linear color spaces. */
		RGBA8, /*!< \brief pack red green blue alpha, each component is 8 unsigned bits. */
		RGBA8S, /*!< \brief pack red green blue alpha, each component is 8 signed bits. */
		RGBA16, /*!< \brief pack red green blue alpha, each component is 16 unsigned bits. .*/
		RGBA16S, /*!< \brief pack red green blue alpha, each component is 16 signed bits. */
		RGBA32, /*!< \brief pack red green blue alpha, each component is 32 unsigned bits. */
		RGBA32S, /*!< \brief pack red green blue alpha, each component is 32 signed bits. */
		RGBA32F, /*!< \brief pack red green blue alpha, each component is 32 bit floating point number. */
		RG8, /*!< \brief pack red and green, each component is 8 unsigned bits. */
		RG8S, /*!< \brief pack red and green, each component is 8 Signed bits. */
		RG16, /*!< \brief pack red and green, each component is 16 unsigned bits. */
		RG16S, /*!< \brief pack red and green, each component is 16 Signed bits. */
		RG32, /*!< \brief pack red and green, each component is 32 unsigned bits. */
		RG32S, /*!< \brief pack red and green, each component is 32 signed bits. */
		RG32F, /*!< \brief pack red and green, each component is 32 bit floating point number. */
		R8, /*!< \brief pack single 8 bit unsigned component. */
		R8S, /*!< \brief pack single 8 signed bit component. */
		R16, /*!< \brief pack single 16 bit unsigned component. */
		R16S, /*!< \brief pack single 16 signed bit component. */
		R32, /*!< \brief pack single 32 unsigned bit component. */
		R32S, /*!< \brief pack single 32 signed bit component. */
		R32F, /*!< \brief pack single 32 bit floating point number. */
		DEPTH16, /*!< \brief pack type is a single 16 bit unsigned depth type. */
		DEPTH24, /*!< \brief pack type is a single 24 bit unsigned depth type. */
		DEPTH32, /*!< \brief pack type is a single 32 bit unsigned depth type. */
		DEPTH24STENCIL8, /*!< \brief pack type is a 32 bit type that makes up 24 unsigned bits of depth, and 8 bits of stencil. */
		BC1, /*!< \brief pack type is a compressed Block Compression 1 type(64 bits per 16 pixels). this is an RGBA image format(Alpha is a simple 1 bit per pixel), most image functions will not function with this pack type. */
		BC1_SRGB, /*!< \brief pack type is a compressed SRGB DXT1 type(64 bits per 16 pixels). gpu will transform from srgb color to linear space when sampled. */
		BC2, /*!< \brief pack type is a compressed Block Compression 2 type(128 bits per 16 pixels), best for sharp alpha transition images, most image functions will not function with this pack type. */
		BC2_SRGB, /*!< \brief pack type is a compressed SRGBA DXT2 type(128 bits per 16 pixels). gpu will transform from srgb color to linear space when sampled. */ 
		BC3, /*!< \brief pack type is a compressed Block Compression 3 type(128 bits per 16 pixels),  best for image's with smooth alpha's this is an RGBA image format most image functions will not function with this pack type. */
		BC3_SRGB, /*!< \brief pack type is a compressed SRGBA Block Compression 3 type(128 bits per 16 pixels).  gpu will transform from srgb color to linear space when sampled. */
		BC4, /*!< \brief pack type is a compressed Block Compression 4 type(64 bits per 16 pixels), used for grayscale images. most image functions will not function with this pack type. */
		BC5, /*!< \brief pack type is a compressed Block Compression 5 type(128 bits per 16 pixels), RG float component(best for normal/tangent maps). most image functions will not function with this pack type. */
		BC6, /*!< \brief pack type is a compressed Block Compression 6 type(128 bits per 16 pixels). Encodes 16 RGB half float's. most image functions will not function with this pack type. */
		BC7, /*!< \brief pack type is a compressed Block Compression 7 type(128 bits per 16 pixels). Encodes RGBA image with better quality. image functions will not function with this pack type. */
		BC7_SRGB, /*!< \brief pack type is a compressed Block Compression 7 type(128 bits per 16 pixels).  gpu will transform from srgb color to linear space when sampled. */

		PackTypeBits = 0xFF, /*!< \brief pack type part of the flag bits. */

		NearestFilter = 0, /*!< \brief mipmap generation uses the nearest pixel to the original texture. */
		LinearFilter = 1, /*!< \brief mipmap generation uses linear interpolation of weighted average of the four texture elements that are closest of the origin pixel. */

		Image1D=0x0, /*!< \brief image type is a 1D image. */
		Image2D=0x100, /*!< \brief image type is a 2D image. */
		Image3D=0x200, /*!< \brief image type is a 3D image. */
		ImageCubeMap=0x300, /*!< \brief image type is a cube map.  face order is POS_X, NEG_X, POS_Y, NEG_Y, POS_Z, NEG_Z */

		ImageTypeBits = 0xF00 /*!< \brief bits of flag to represent the type of image. */
	};
	/*!< \brief the total number of mipmaps*layers*faces the image will store. */
	static const uint32_t MaxTextureImages = 128;

	/*!< \brief image loading which automatically attempts to load an image from a file source, and deduce it's type based on the associated extension. 
		 \return true on success, or false on failure.
	*/
	static bool LoadImage(LWImage &Image, const LWUTF8Iterator &FilePath, LWAllocator &Allocator, LWFileStream *ExistingStream = nullptr);

	/*!< \brief image loading from memory, but type is selected by filepath type.
		 \return true on success, or false on failure.
	*/
	static bool LoadImage(LWImage &Image, const LWUTF8Iterator &FilePath, LWByteBuffer &Buffer, LWAllocator &Allocator);

	/*!< \brief loads a TGA file if it is a supported type. */
	static bool LoadImageTGA(LWImage &Image, const LWUTF8Iterator &FilePath, LWAllocator &Allocator, LWFileStream *ExistingStream = nullptr);
	
	/*!< \brief loads a tga file from a memory buffer. */
	static bool LoadImageTGA(LWImage &Image, LWByteBuffer &Buffer, LWAllocator &Allocator);

	/*!< \brief loads a PNG file. */
	static bool LoadImagePNG(LWImage &Image, const LWUTF8Iterator &FilePath, LWAllocator &Allocator, LWFileStream *ExistingStream = nullptr);

	/*!< \brief loads a png file from a memory buffer. */
	static bool LoadImagePNG(LWImage &Image, LWByteBuffer &Buffer, LWAllocator &Allocator);

	/*!< \brief loads a dds file. */
	static bool LoadImageDDS(LWImage &Image, const LWUTF8Iterator &FilePath, LWAllocator &Allocator, LWFileStream *ExistingStream = nullptr);

	/*!< \brief loads a dds file from a memory buffer. */
	static bool LoadImageDDS(LWImage &Image, LWByteBuffer &Buffer, LWAllocator &Allocator);

	/*! \brief loads a ktx2 file. *WARNING: Unfinished* */
	static bool LoadImageKTX2(LWImage &Image, const LWUTF8Iterator &FilePath, LWAllocator &Allocator, LWFileStream *ExistingStream = nullptr);

	/*! \brief loads a ktx2 file from a memory buffer.  */
	static bool LoadImageKTX2(LWImage &Image, LWByteBuffer &Buffer, LWAllocator &Allocator);

	/*! \brief write's to filepath a png file. returns true on success, or false on failure.
		\note currently only RGBA8 image's are supported for png.
	*/
	static bool SaveImagePNG(LWImage &Image, const LWUTF8Iterator &FilePath, LWAllocator &Allocator, LWFileStream *ExistingStream = nullptr);

	/*! \brief write's a png file into buffer, returns the number of bytes. 
		\note currently only RGBA8 image's are supported for png.
	*/
	static uint32_t SaveImagePNG(LWImage &Image, LWByteBuffer &Buffer);

	/*!< \brief write's to filepath a dds file.  returns true on success, or false on failure.
	*/
	static bool SaveImageDDS(LWImage &Image, const LWUTF8Iterator &FilePath, LWAllocator &Allocator, LWFileStream *ExistingStream = nullptr);

	/*!< \brief write's a dds file into buffer, returns the number of bytes.
	*/
	static uint32_t SaveImageDDS(LWImage &Image, LWByteBuffer &Buffer);

	/*!, \brief converts the RGBA8 InTexel's into the specified BC format(BC1-3, + BC7, BC5(only the RG format is encoded, encoding will convert 0-255 values to -1-1 range), BC6(only the RGB format is encoded, encoding will convert 0-255 values to -1-1 range.)). TexelSize(x/y) must be a multiple of 4.
		 \return the number of bytes to store into OutTexels.
	*/
	static uint32_t RGBA8toBC(const LWVector2i &TexelsSize, const uint8_t *InTexels, uint8_t *OutTexels, uint32_t BCFormat);

	/*!< \brief converts the RGBA32F InTexel's into the specified BC format(BC5(only the RG components is encoded), BC6(only the RGB components is encoded, half precision is also lost as BC6 is float16 format). TexelSize must be a multiple of 4. */
	static uint32_t RGBA32FtoBC(const LWVector2i &TexelsSize, const float *InTexels, uint8_t *OutTexels, uint32_t BCFormat);

	/*!< \brief converts the R8 InTexel's into the specified BC format(only BC4 is supported). TexelSize must be a multiple of 4.
	*	 \return the number of bytes to store into OutTexels.
	*/
	static uint32_t R8toBC(const LWVector2i &TexelSize, const uint8_t *InTexels, uint8_t *OutTexels, uint32_t BCFormat);

	/*!< \brief converts the RG8 InTexles into the specified BC format(only BC5 is supported(encoding will convert 0-255 values to -1-1 range) TexelSize must be a multiple of 4. */
	static uint32_t RG8toBC(const LWVector2i &TexelSize, const uint8_t *InTexels, uint8_t *OutTexels, uint32_t BCFormat);

	/*!< \brief converts the RG32F InTexel's into the specified BC format(only BC5 is supported). */
	static uint32_t RG32FtoBC(const LWVector2i &TexelSize, const uint8_t *InTexels, uint8_t *OutTexels, uint32_t BCFormat);

	/*!< \brief converts the specified BC format(BC1-3, BC5(converts from -1-1 range to 0-255 for rg components, b is set to 0), BC6(converts from -1-1 range to 0-255 for rga components) +BC7) InTexel's into RGBA format, alpha is set to 1 for formats that don't retain alpha components.  TexelSize(x/y) must also be a multiple of 4.
	*	 \return the number of bytes to store into OutTexels.
	*/
	static uint32_t BCtoRGBA8(const LWVector2i &TexelsSize, const uint8_t *InTexels, uint8_t *OutTexels, uint32_t BCFormat);

	/*!< \brief converts the specified BC format(only BC5 is supported) to a RG32F format. TexelSize must be a multiple of 4. */
	static uint32_t BCtoRG32F(const LWVector2i &TexelsSize, const uint8_t *InTexels, uint8_t *OutTexels, uint32_t BCFormat);

	/*!< \brief converts the specified BC format(only BC5 is supported) to a RG8 format, values are converted from -1-1 range to 0-255. TexelSize must be a multiple of 4. */
	static uint32_t BCtoRG8(const LWVector2i &TexelsSize, const uint8_t *InTexels, uint8_t *OutTexels, uint32_t BCformat);

	/*!< \brief converts the specified BC format(only BC6 is supported) to a RGBA32 format, where A is set to 1.  TexelSize must be a multiple of 4. */
	static uint32_t BCtoRGBA32F(const LWVector2i &TexelSize, const uint8_t *InTexels, uint8_t *OutTexels, uint32_t BCFormat);

	/*!< \brief copys n texels into buffer at a certain weight.
		 \param PackType the pack type of the texels.
		 \param TexelCnt the number of texels and weights.
		 \param TexelPtrs the pointer to each texel to be summed together.
		 \param TexelWeights the weights to apply to each texel when being summed together.
		 \param Buffer the buffer to write out to.
		 \return the number of bytes written into buffer(or number of bytes if buffer is null.)
		 \note can not be used with Compressed formats.
	*/
	static uint32_t CopyWeightedTexels(uint32_t PackType, uint32_t TexelCnt, const uint8_t **TexelPtrs, const float *TexelWeights, uint8_t *Buffer);

	/*!< \brief samples a 1D Image of length and writes the results into buffer(if not null) the nearest sample to u(0-1).
		\param Texels the image data to be sampled from.
		\param Length the number of texels in the 1D image.
		\param PackType the image data's packing type.
		\param u where to sample from in 0-1 units.
		\param Buffer where to store the results(pass null to not write anything.)
		\return the amount of bytes written to buffer(rather buffer is null or not.)
	    \note can not be used with Compressed formats.

	*/
	static uint32_t SampleNearest1D(const uint8_t *Texels, uint32_t Length, uint32_t PackType, float u, uint8_t *Buffer);

	/*!< \brief samples a 1D Image of length and writes the results into buffer(if not null) the linear sample of near by texels to u(0-1).
		\param Texels the image data to be sampled from.
		\param Length the number of texels in the 1D image.
		\param PackType the image data's packing type.
		\param u where to sample from in 0-1 units.
		\param Buffer where to store the results(pass null to not write anything.)
		\return the amount of bytes written to buffer(rather buffer is null or not.)
		\note can not be used with Compressed formats.
	*/
	static uint32_t SampleLinear1D(const uint8_t *Texels, uint32_t Length, uint32_t PackType, float u, uint8_t *Buffer);

	/*!< \brief samples a 2D Image of size and writes the results into buffer(if not null) the nearest sample to u(horizontal 0-1) v(Vertical 0-1).
		\param Texels the image data to be sampled from.
		\param Size the 2d size of texels in the image.
		\param PackType the image data's packing type.
		\param uv where to sample from in 0-1 units.
		\param Buffer where to store the results(pass null to not write anything.)
		\return the amount of bytes written to buffer(rather buffer is null or not.)
		\note can not be used with Compressed formats.
	*/
	static uint32_t SampleNearest2D(const uint8_t *Texels, const LWVector2i &Size, uint32_t PackType, const LWVector2f &UV, uint8_t *Buffer);

	/*!< \brief samples a 2D Image of size and writes the results into buffer(if not null) the linear sample of near by texels to u(horizontal 0-1) v(Vertical 0-1).
		\param Texels the image data to be sampled from.
		\param Size the 2d size of texels in the image.
		\param PackType the image data's packing type.
		\param uv where to sample from in 0-1 units.
		\param Buffer where to store the results(pass null to not write anything.)
		\return the amount of bytes written to buffer(rather buffer is null or not.)
		\note can not be used with Compressed formats.
	*/
	static uint32_t SampleLinear2D(const uint8_t *Texels, const LWVector2i &Size, uint32_t PackType, const LWVector2f &UV, uint8_t *Buffer);

	/*!< \brief samples a 3D Image of size and writes the results into buffer(if not null) the nearest sample to u(horizontal 0-1) v(Vertical 0-1) r(depth 0-1).
		\param Texels the image data to be sampled from.
		\param Size the 3d size of texels in the image.
		\param PackType the image data's packing type.
		\param uvr where to sample from in 0-1 units.
		\param Buffer where to store the results(pass null to not write anything.)
		\return the amount of bytes written to buffer(rather buffer is null or not.)
		\note can not be used with Compressed formats.
	*/
	static uint32_t SampleNearest3D(const uint8_t *Texels, const LWVector3i &Size, uint32_t PackType, const LWVector3f &UVR, uint8_t *Buffer);

	/*!< \brief samples a 3D Image of size and writes the results into buffer(if not null) the linear sample of near by texels to u(horizontal 0-1) v(Vertical 0-1) r(depth 0-1).
		\param Texels the image data to be sampled from.
		\param Size the 3d size of texels in the image.
		\param PackType the image data's packing type.
		\param uvr where to sample from in 0-1 units.
		\param Buffer where to store the results(pass null to not write anything.)
		\return the amount of bytes written to buffer(rather buffer is null or not.)
		\note can not be used with Compressed formats.
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
		\note can not be used with Compressed formats.
	*/
	static uint32_t MakeMipmapLevel2D(const uint8_t *Texels, const LWVector2i &Size, uint32_t PackType, uint32_t MipmapLevel, uint8_t *Buffer, uint32_t SampleMode = LinearFilter);

	/*!< \brief writes into buffer a new 3D image with the specified mipmap level of the 3D image.
		 \note pass null into buffer to only receive the specified mipmap's expected width, height, and depth.
		 \return the number of bytes in the new mipmap image.
		\note can not be used with Compressed formats.
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
	
	/*!< \brief returns true if the pack type is a depth format type(this and stencil type may both return true if the type is a depth+stencil type). */
	static bool DepthType(uint32_t PackType);

	/*!< \brief returns true if the pack type is a stencil format type(this and depth type may both return true if the type is a depth+stencil type). */
	static bool StencilType(uint32_t PackType);

	/*!< \brief returns true if the pack type is an srgb type, otherwise false. */
	static bool SRGBAType(uint32_t PackType);

	/*!< \brief returns the mipmap size for the specified mipmap level of the source length. */
	static uint32_t MipmapSize1D(uint32_t SrcLen, uint32_t MipmapLevel);

	/*!< \brief returns the mipmap 2D size for the specified mipmap level of the source size. */
	static LWVector2i MipmapSize2D(const LWVector2i &SrcSize, uint32_t MipmapLevel);

	/*!< \brief returns the mipmap 3D size for the specified mipmap level of the source size. */
	static LWVector3i MipmapSize3D(const LWVector3i &SrcSize, uint32_t MipmapLevel);

	/*!< \brief auto generates all mipmap levels for the image(does not work with dxt formats). */
	LWImage &BuildMipmaps(uint32_t SampleMode);

	/*!< \brief compress's RGBA/RG32F/R8 images to the target bc format. */
	LWImage &Compress(uint32_t DXTFormat);

	/*!< \brief decompress's a bc format to a relevant format(bc1-3, +7 = RGBA8, bc4 = r8, bc5 = .  does nothing if not a supported dxt format. */
	LWImage &Decompress(void);

	/*!< \brief converts the current pack type to an SRGBA type(or vice versa if false) if the current type is an applicable type. */
	LWImage &SetSRGBA(bool bIsSRGBA);

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

	/*!< \brief returns the number of layers the image uses(for cubemap images this value is a multiple of 6). */
	uint32_t GetLayers(void) const;

	/*!< \brief returns a pointer to the internal texels, which allows for the application to manipulate the underlying data freely. 
		 \param Index the texel layer to return.
	*/
	uint8_t *GetTexels(uint32_t Index);

	/*!< \brief returns a const-pointer to the specified texels. */
	const uint8_t *GetTexels(uint32_t Index) const;

	/*!< \brief returns a pointer to the internal texels, which allows for the application to manipulate the underlying data freely.
	*	 \param Layer the layer to get for multi-layered(of cubemap'd) textures.
	*/
	uint8_t *GetTexels(uint32_t Layer, uint32_t Mipmap);

	/*!< \brief returns a const-pointer to the specified texels. */
	const uint8_t *GetTexels(uint32_t Layer, uint32_t Mipmap) const;

	/*!< \brief returns the underlying texture arrays. */
	uint8_t **GetTexels(void);

	/*!< \brief returns true if the packtype is an SRGBA packtype. */
	bool isSRGBA(void) const;

	/*!< \brief returns true if the PackType is an compressed type. */
	bool isCompressed(void) const;

	/*!< \brief returns true if the PackType is a depth type. */
	bool isDepth(void) const;

	/*!< \brief returns true if the PackType is a stencil type. */
	bool isStencil(void) const;

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
		 \param MipmapCount the number of mipmap texels that are provided for the image.
		 \param Texels the texel data(or null if no data is to be passed in).
		 \param Allocator the allocator used to create the internal texel data.
		 \note Texel order is: Layers*(MipmapCount+1)+Mipmap;
	*/
	LWImage(int32_t Size, uint32_t Layers, uint32_t PackType, uint32_t MipmapCount, uint8_t **Texels, LWAllocator &Allocator);
	
	/*< \brief Constructs a 2D LWImage object.
		\param Size the size of the image.
		\param Layers the number of layers for the image, if the image is a cubemap this number will be multiplied by 6 internally for each cubemap face.
		\param PackType the packing type of the image.  itwise-or ImageCubemap with this parameter if this is to be a cubemap image instead of a regular 2D image.
		\param MipmapCount the number of mipmap texels that are provided for the image.
		\param Texels the texel data(or null if no data is to be passed in, the entire image chain is expected if this paramater is non-null).  the order of texels for a cube map is in the order of POS_X, NEG_X, POS_Y, NEG_Y, POS_Z, NEG_Z, each face is treated as a layer.
		\param Allocator the allocator used to create the internal texel data.
		\note Texel order is: Layers*(MipmapCount+1)+Mipmap;
	*/

	LWImage(const LWVector2i &Size, uint32_t Layers, uint32_t PackType, uint32_t MipmapCount, uint8_t **Texels, LWAllocator &Allocator);

	/*< \brief Constructs a 3D LWImage object.  DXT images are not supported.
		\param Size the size of the image.
		\param Layers the number of 3d image's stacked.
		\param PackType the packing type of the image.
		\param MipmapCount the number of mipmap texels that are provided for the image.
		\param Texels the texel data(or null if no data is to be passed in, if non-null the entire image chain is expected.).
		\param Allocator the allocator used to create the internal texel data.
		\note Texel order is: Layers*(MipmapCount+1)+Mipmap.
	*/
	LWImage(const LWVector3i &Size, uint32_t Layers, uint32_t PackType, uint32_t MipmapCount, uint8_t **Texels, LWAllocator &Allocator);

	/*!< \brief the default constructor. */
	LWImage() = default;

	/*!< \brief Destructor. */
	~LWImage();
	
private:
	/*!< \brief universal constructor for any image type, used internally by other constructors to setup Texels data structure. */
	LWImage(const LWVector3i &Size, uint32_t Layers, uint32_t iType, uint32_t PackType, uint32_t MipmapCount, uint8_t **Texels, LWAllocator & Allocator);

	LWVector4i m_Size = LWVector4i(1,1,1,0); //w = layer count.
	uint32_t m_MipmapCount = 0;
	uint32_t m_Flag = 0;
	uint8_t *m_Texels[MaxTextureImages] = {};
};

#endif