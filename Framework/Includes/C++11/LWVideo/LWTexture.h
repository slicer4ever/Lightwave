#ifndef LWTEXTURE_H
#define LWTEXTURE_H
#include "LWCore/LWTypes.h"
#include "LWCore/LWVector.h"
#include "LWVideo/LWTypes.h"

/*!< \brief texture object. */
class LWTexture : public LWVideoResource {
public:
	enum{
		MinNearest              = 0, /*!< \brief min filter is determined by nearest texels. */
		MagNearest              = 0, /*!< \brief mag filter is determined by nearest texels. */
		WrapSClampToEdge        = 0, /*!< \brief clamps texture S coordinates to edge of texture. */
		WrapTClampToEdge        = 0, /*!< \brief clamps texture T coordinates to edge of texture. */
		WrapRClampToEdge        = 0, /*!< \brief clamps texture R coordinates to edge of texture. */
		CompareNone             = 0, /*!< \brief specify the depth compare mode to none. */
		DepthRead               = 0, /*!< \brief flag to specify the depth component should be read from if a depth texture. */
		MinLinear               = 0x1, /*!< \brief min filter is set to linear mode. */
		MinNearestMipmapNearest = 0x2, /*!< \brief min filter is set to nearest, and mipmaps are also set to nearest. */
		MinLinearMipmapNearest  = 0x3, /*!< \brief min filter is set to linear, and mipmaps are set to nearest. */
		MinNearestMipmapLinear  = 0x4, /*!< \brief min filter is set to nearest, and mipmaps are set to linear. */
		MinLinearMipmapLinear   = 0x5, /*!< \brief min filter is set to linear, and mipmaps are also set to linear. */
		MinFilterFlag           = 0x7, /*!< \brief used to find the min filter flag. */
		MinFilterBitOffset      = 0x0, /*!< \brief used for offsetting the min filter flag to the first bit.*/

		MagLinear               = 0x8, /*!< \brief mag filter is set to linear. */
		MagFilterFlag           = 0x8, /*!< \brief used to find the mag filter flag. */
		MagFilterBitOffset      = 0x3, /*!< \brief used for offsetting the mag filter flag to the first bit.*/

		WrapSClampToBorder      = 0x10, /*!< \brief clamps texture S coordinates to the border of the texture .*/
		WrapSMirroredRepeat     = 0x20, /*!< \brief S coordinates are mirrored and repeated indefinitely. */
		WrapSRepeat             = 0x30, /*!< \brief S coordinates are repeated indefinitely. */
		WrapSFilterFlag         = 0x30, /*!< \brief used to find the wrap S filter flag. */
		WrapSFilterBitOffset    = 0x4,  /*!< \brief used for offsetting the wrap s filter flag to the first bit. */

		WrapTClampToBorder      = 0x40, /*!< \brief clamps texture T coordinates to the border of the texture. */
		WrapTMirroredRepeat     = 0x80, /*!< \brief T coordinates are mirrored and repeated indefinitely. */
		WrapTRepeat             = 0xC0, /*!< \brief T coordinates are repeated indefinitely. */
		WrapTFilterFlag         = 0xC0, /*!< \brief used to find the wrap T filter flag. */
		WrapTFilterBitOffset    = 0x6,  /*!< \brief used for offsetting wrap t filter flag to the first bit. */

		WrapRClampToBorder      = 0x100, /*!< \brief clamps texture R coordinates to the border of the texture. */
		WrapRMirroredRepeat     = 0x200, /*!< \brief R coordinates are mirrored and repeated indefinitely. */
		WrapRRepeat             = 0x300, /*!< \brief R coordinates are repeated indefinitely. */
		WrapRFilterFlag         = 0x300, /*!< \brief used to find the wrap R filter flag. */
		WrapRFilterBitOffset    = 0x8,   /*!< \brief used to offsetting wrap r filter flag to the first bit. */

		CompareModeRefTexture = 0x400, /*!< \brief flag used for comparing depth filters for pcf filtering. */
		CompareModeFlag = 0x400, /*!< \brief used to get the comparison function the texture is set to. */
		CompareModeBitOffset = 0xA, /*!< \brief used for offsetting the comparison function to the first bit. */

		CompareNever        = 0x0, /*!< \brief flag used to indicate comparison mode is set to never pass. */
		CompareAlways       = 0x800, /*!< \brief flag used to indicate comparison mode is set to always pass. */
		CompareLess         = 0x1000, /*!< \brief flag used to indicate comparison mode is set to less to pass. */
		CompareEqual        = 0x1800, /*< \brief flag used to indicate comparison mode is set to equal to pass. */
		CompareLessEqual    = 0x2000, /*< \brief flag used to indicate comparison mode is set to less than or equal to pass. */
		CompareGreater      = 0x2800, /*!< \brief flag used to indicate comparison mode is set to greater to pass. */
		CompareGreaterEqual = 0x3000, /*!< \brief flag used to indicate comparison mode is set to greater or equal to pass. */
		CompareNotEqual     = 0x3800, /*!< \brief flag used to indicate comparison mode is set to not equal to pass. */
		CompareFuncFlag = 0x3800, /*!< \brief used to find the comparison mode the texture is set to. */
		CompareFuncBitOffset = 0xB, /*!< \brief used for offsetting the comparison mode to the first bit. */

		StencilRead = 0x4000, /*!< \brief flag to indicate the texture should read from the stencil component if a depth-stencil texture. */
		DepthReadFlag = 0x4000, /*!< \brief used to find the depth/stencil reading mode for the texture that it is set to. This flag is only used in openGL context's, directX allows reading from both. */
		DepthReadBitOffset = 0xE, /*!< \brief used for offsetting the depth/stencil reading mode to the first bit. */

		Anisotropy_None = 0x0, /*!< \brief default flag for Anisotropy multiplier, if non zero Anisotropy flag is set then sampler set's relevant Anisotropy filtering(which can override mag/min settings depending on api). */
		Anisotropy_2x = 0x8000, /*!< \brief set's Anisotropy filtering to 2x(depending on api this will override min/mag filter settings) */
		Anisotropy_4x = 0x10000, /*!< \brief set's Anisotropy filtering to 4x(depending on api this will override min/mag filter settings). */
		Anisotropy_8x = 0x18000, /*!< \brief set's Anisotropy filtering to 8x(depending on api this will override min/mag filter settings). */
		Anisotropy_16x = 0x20000, /*!< \brief set's Anisotropy filtering to 16x(depending on api this will override min/mag filter settings). */
		AnisotropyFlag = 0x38000, /*!< \brief used to isolate the Anisotropy flags. */
		AnisotropyBitOffset = 0xF, /*!< \brief used for offsetting the Anisotropy flags to 0 indexed array. */

		RenderTarget            = 0x08000000, /*!< \brief the texture is going to be bound to a framebuffer. */
		RenderBuffer            = 0x10000000, /*!< \brief the texture is going to be bound to a framebuffer, and only used for rendering, not used as a texture(note: d3d11 doesn't respect this flag, rendertarget is more consistent across different api's). */
		MakeMipmaps             = 0x20000000, /*!< \brief tells the driver to construct the mipmaps for the textures. */
		Dirty                   = 0x80000000, /*!< \brief marks the texture as dirty, and requires updating. */

		Texture1D = 0, /*!< \brief texture type is a 1D texture.*/
		Texture2D = 1, /*!< \brief texture type is a 2D texture. */
		Texture3D = 2, /*!< \brief texture type is a 3D texture. */
		TextureCubeMap = 3, /*!< \brief texture type is a cube map texture. */
		Texture1DArray = 4, /*!< \brief texture type is a 1D array texture. */
		Texture2DArray = 5, /*!< \brief texture type is a 2D array texture. */
		TextureCubeMapArray = 6, /*<! \brief texture type is a cubemap array of textures. */
		Texture2DMS = 7, /*! \brief texture type is a 2D multisampled Texture. */
		Texture2DMSArray = 8, /*! \brief texture type is a 2D multisampled array Texture. */

		Face_Negative_X = 0, /*!< \brief texture cube map face -x. */
		Face_Positive_X, /*!< \brief texture cube map face +x. */
		Face_Negative_Y, /*!< \brief texture cube map face -y. */
		Face_Positive_Y, /*!< \brief texture cube map face +y. */
		Face_Negative_Z, /*!< \brief texture cube map face -z. */
		Face_Positive_Z /*!< \brief texture cube map face +z. */
	};

	/*!< \brief changes the texture state, and marks it dirty. */
	LWTexture &SetTextureState(uint32_t TextureState);

	/*! \brief constructs mipmap chain from base image upto the number of mipmaps specified on texture creation. */
	LWTexture &GenerateMipmaps(void);

	/*!< \brief returns true if the dirty flag is set. */
	bool isDirty(void) const;

	/*!< \brief clears the dirty flag from the texture state flags. */
	LWTexture &ClearDirty(void);

	/*!< \brief returns the current texture state flags. */
	uint32_t GetTextureState(void) const;

	/*! \brief returns true if the type is one of the two multisampled texture types. */
	bool isMultiSampled(void) const;

	/*!< \brief returns the number of mipmaps the texture has.
		 \return 0 if no mipmaps, otherwise the number of mipmap layers(this number does not include the base texture).
	*/
	uint32_t GetMipmapCount(void) const;

	/*! \brief returns the number of samples the texture has. */
	uint32_t GetSamples(void) const;

	/*!< \brief returns the underlying type this texture is. */
	uint32_t GetType(void) const;

	/*!< \brief returns the packing type of the texture(see LWImage for a list of supported packing types. */
	uint32_t GetPackType(void) const;

	/*!< \brief returns the size of the texture as if it were a 1D texture.  */
	uint32_t Get1DSize(void) const;

	/*!< \brief returns the size of the texture as if it were a 1D texture as a float. */
	float Get1DSizef(void) const;

	/*!< \brief returns the size of the texture as if it were a 2D texture, also useful for cubemap textures to get each faces size. */
	LWVector2i Get2DSize(void) const;

	/*!< \brief returns the size of the texture as if it were a 2D texture as a float. */
	LWVector2f Get2DSizef(void) const;

	/*!< \brief returns the size of the texture as if it were a 3D texture. */
	LWVector3i Get3DSize(void) const;

	/*!< \brief returns the size of the texture as if it were a 3D texture as a float. */
	LWVector3f Get3DSizef(void) const;

	/*!< \brief returns the layers for a 1D texture array. */
	uint32_t Get1DLayers(void) const;

	/*!< \brief returns the layers for a 2D texture array or cubemap. */
	uint32_t Get2DLayers(void) const;
	
	/*!< \brief constructor for the texture object. 
		 \param TextureState a combined flag of possible texture states.
		 \param PackType the underlying packing type of the texels.
		 \param MipmapCount the number of mipmaps for the texture.
		 \param Size the size of the texture, some of the components may be undefined depending on the TexType.
		 \param TexType the texture type(1D, 2D, 3D, CubeMap).
	*/
	LWTexture(uint32_t TextureState, uint32_t PackType, uint32_t MipmapCount, const LWVector3i &Size, uint32_t TexType);


protected:
	uint32_t m_TextureState;
	uint32_t m_PackType;
	uint32_t m_MipmapCount;
	uint32_t m_Type;
	LWVector3i m_Size;
};

/*!< \cond */
template<class Type>
class LWTextureCon : public LWTexture {
public:

	LWTextureCon<Type> &SetContext(Type &Context) {
		m_Context = Context;
		return *this;
	}
	
	Type &GetContext(void) {
		return m_Context;
	}

	Type &GetContext(void) const {
		return m_Context;
	}

	LWTextureCon(Type Context, uint32_t TextureState, uint32_t PackType, uint32_t MipmapCount, const LWVector3i &Size, uint32_t TexType) : LWTexture(TextureState, PackType, MipmapCount, Size, TexType), m_Context(Context) {}
private:
	Type m_Context;
};


/*!< \endcond */
#endif	