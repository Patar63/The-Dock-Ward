#include "Texture2D.h"
#include <stb_image.h>
#include <Logging.h>
#include "GLM/glm.hpp"

/// <summary>
/// Get the number of mipmap levels required for a texture of the given size
/// </summary>
/// <param name="width">The width of the texture in pixels</param>
/// <param name="height">The height of the texture in pixels</param>
/// <returns>Number of mip levels required for the texture</returns>
inline int CalcRequiredMipLevels(int width, int height) {
	return (1 + floor(log2(glm::max(width, height))));
}

Texture2D::Texture2D(const Texture2DDescription& description) : ITexture(TextureType::_2D) {
	_description = description;
	_SetTextureParams();
	if (!description.Filename.empty()) {
		_LoadDataFromFile();
	}
}

Texture2D::Texture2D(const std::string& filePath) : ITexture(TextureType::_2D) {
	_description.Filename = filePath;
	_SetTextureParams();
	_LoadDataFromFile();
}

void Texture2D::SetMinFilter(MinFilter value) {
	_description.MinificationFilter = value;
	glTextureParameteri(_handle, GL_TEXTURE_MIN_FILTER, *_description.MinificationFilter);
}

void Texture2D::SetMagFilter(MagFilter value) {
	_description.MagnificationFilter = value;
	glTextureParameteri(_handle, GL_TEXTURE_MAG_FILTER, *_description.MagnificationFilter);
}

void Texture2D::SetAnisoLevel(float value) {
	if (value != _description.MaxAnisotropic) {
		_description.MaxAnisotropic = glm::clamp(value, 1.0f, ITexture::GetLimits().MAX_ANISOTROPY);
		glTextureParameterf(_handle, GL_TEXTURE_MAX_ANISOTROPY, _description.MaxAnisotropic);

		if (_description.GenerateMipMaps) {
			glGenerateTextureMipmap(_handle);
		}
	}
}

void Texture2D::LoadData(uint32_t width, uint32_t height, PixelFormat format, PixelType type, void* data, uint32_t offsetX, uint32_t offsetY) {
	// Ensure the rectangle we're setting is within the bounds of the image
	LOG_ASSERT((width + offsetX) <= _description.Width, "Pixel bounds are outside of the X extents of the image!");
	LOG_ASSERT((height + offsetY) <= _description.Height, "Pixel bounds are outside of the Y extents of the image!");

	// Align the data store to the size of a single component to ensure we don't get weirdness with images that aren't RGBA
	// See https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glPixelStore.xhtml
	int componentSize = (GLint)GetTexelComponentSize(type);
	glPixelStorei(GL_PACK_ALIGNMENT, componentSize);

	// Upload our data to our image
	glTextureSubImage2D(_handle, 0, offsetX, offsetY, width, height, (GLenum)format, (GLenum)type, data);

	// If requested, generate mip-maps for our texture
	if (_description.GenerateMipMaps) {
		glGenerateTextureMipmap(_handle);
	}
}

void Texture2D::_LoadDataFromFile() {
	LOG_ASSERT(_description.Width + _description.Height == 0, "This texture has already been configured with a size! Cannot re-allocate memory!");

	if (!_description.Filename.empty()) {
		// Variables that will store properties about our image
		int width, height, numChannels;
		const int targetChannels = GetTexelComponentCount(_description.FormatHint);

		// Use STBI to load the image
		stbi_set_flip_vertically_on_load(true);
		uint8_t* data = stbi_load(_description.Filename.c_str(), &width, &height, &numChannels, targetChannels);

		// If we could not load any data, warn and return null
		if (data == nullptr) {
			LOG_WARN("STBI Failed to load image from \"{}\"", _description.Filename);
			return ;
		}

		// We should estimate a good format for our data

		// numChannels will store the number of channels in the image on disk, if we overrode that we should use the override value
		if (targetChannels != 0)
			numChannels = targetChannels;

		// We'll determine a recommended format for the image based on number of channels
		// We hinted that we wanted a certain number of channels, but we're not guaranteed
		// that all those channels exist (ex: loading an RGB image but requesting RGBA)
		InternalFormat internal_format = GetInternalFormatForChannels8(numChannels);
		PixelFormat    image_format = GetPixelFormatForChannels(numChannels);

		// This is one of those poorly documented things in OpenGL
		if ((numChannels * width) % 4 != 0) {
			LOG_WARN("The alignment of a horizontal line is not a multiple of 4, this will require a call to glPixelStorei(GL_PACK_ALIGNMENT)");
		}

		// Update our description to match what we loaded
		_description.Format = internal_format;
		_description.Width = width;
		_description.Height = height;

		// Allocates our memory
		_SetTextureParams();

		// Upload data to our texture
		LoadData(width, height, image_format, PixelType::UByte, data);

		// We now have data in the image, we can clear the STBI data
		stbi_image_free(data);
	}
}

void Texture2D::_SetTextureParams() {
	// If the anisotropy is negative, we assume that we want max anisotropy
	if (_description.MaxAnisotropic < 0.0f) {
		_description.MaxAnisotropic = ITexture::GetLimits().MAX_ANISOTROPY;
	}

	// Make sure the size is greater than zero and that we have a format specified before trying to set parameters
	if ((_description.Width * _description.Height > 0) && _description.Format != InternalFormat::Unknown) {
		// Calculate how many layers of storage to allocate based on whether mipmaps are enabled or not
		int layers = _description.GenerateMipMaps ? CalcRequiredMipLevels(_description.Width, _description.Height) : 1;
		// Allocates the memory for our texture
		glTextureStorage2D(_handle, layers, (GLenum)_description.Format, _description.Width, _description.Height);

		glTextureParameteri(_handle, GL_TEXTURE_WRAP_S, (GLenum)_description.HorizontalWrap);
		glTextureParameteri(_handle, GL_TEXTURE_WRAP_T, (GLenum)_description.VerticalWrap);
		glTextureParameteri(_handle, GL_TEXTURE_MIN_FILTER, (GLenum)_description.MinificationFilter);
		glTextureParameteri(_handle, GL_TEXTURE_MAG_FILTER, (GLenum)_description.MagnificationFilter);
		glTextureParameterf(_handle, GL_TEXTURE_MAX_ANISOTROPY, _description.MaxAnisotropic);
	}
}

Texture2D::Sptr Texture2D::LoadFromFile(const std::string& path, const Texture2DDescription& description, bool forceRgba) {
	// Create a copy of the description and change filename to the path
	Texture2DDescription desc = description;
	desc.Filename = path;

	// Create a texture from the description (it'll load the file)
	Texture2D::Sptr result = std::make_shared<Texture2D>(desc);

	return result;
}