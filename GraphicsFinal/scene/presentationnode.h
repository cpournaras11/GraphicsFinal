//============================================================================
//	Johns Hopkins University Engineering Programs for Professionals
//	605.467 Computer Graphics and 605.767 Applied Computer Graphics
//	Instructor:	David W. Nesbitt
//
//	Author:  	David W. Nesbitt
//	File:     presentationnode.h
//	Purpose:	Scene graph presentation node.
//
//============================================================================

#ifndef __PRESENTATIONNODE_H
#define __PRESENTATIONNODE_H

// DevIL include -just the base image library
#include <IL/il.h>
#include <sstream>
/**
* Presentation node. Holds material properties.
*/
class PresentationNode : public SceneNode {
public:
	/**
	 * Constructor
	 */
	PresentationNode()
    {
		initialize();

        material_shininess = 1.0f;
		// Note: color constructors default rgb to 0 and alpha to 1
	}

	/**
	 * Constructor given material properties.
	 * @param  ma  Material ambient reflection coefficients (color).
	 * @param  md  Material diffuse reflection coefficients (color).
	 * @param  ms  Material specular reflection coefficients (color).
	 * @param  me  Material emission (color).
	 * @param  s   Material shininess.
	 */
	PresentationNode(const Color4& ma, const Color4& md, const Color4& ms, const Color4& me, const float s)
      : material_ambient(ma),
		material_diffuse(md),
		material_specular(ms),
		material_emission(me),
		material_shininess(s)
    {
        initialize();
	}

    /**
     * Initialize the presentation node attributes
     */
    void initialize()
    {
        node_type = SCENE_PRESENTATION;
        reference_count = 0;

        texture_id = 0;       // Default to no texture
        textureScale = 1.0f;  // Default texture scale to normal size
        normalMapID = 0;      // Default to no normal map
    }

	/**
	 * Set material ambient reflection coefficient.
	 * @param  c  Ambient reflection coefficients (color).
	 */
	void SetMaterialAmbient(const Color4& c) {
		material_ambient = c;
	}

	/**
	 * Set material diffuse reflection coefficient.
	 * @param  c  Diffuse reflection coefficients (color).
	 */
	void SetMaterialDiffuse(const Color4& c) {
		material_diffuse = c;
	}

	/**
	 * Set material ambient and diffuse reflection coefficient to the
	 * same value.
	 * @param  c  Ambient/diffuse reflection coefficients (color).
	 */
	void SetMaterialAmbientAndDiffuse(const Color4& c) {
		material_ambient = c;
		material_diffuse = c;
	}

	/**
	 * Set material specular reflection coefficient.
	 * @param  c  Specular reflection coefficients (color).
	 */
	void SetMaterialSpecular(const Color4& c) {
		material_specular = c;
	}

	/**
	 * Set material emission.
	 * @param  c  Emission (color).
	 */
	void SetMaterialEmission(const Color4& c) {
		material_emission = c;
	}

	/**
	 * Set material shininess.
	 * @param  s  Shininess.
	 */
	void SetMaterialShininess(const float s) {
		material_shininess = s;
	}

	/**
	 * Set the texture to used for the material.
	 * @param  fname  Texture image filename
	 * @param  wrap_s  OpenGL wrap option (s)
	 * @param  wrap_t  OpenGL wrap option (t)
	 * @param  min_filter  OpenGL filter to use for minification
	 * @param  mag_filter  OpenGL filter to use for magnification
	 */
	void SetTexture(const std::string& fname, GLuint wrap_s, GLuint wrap_t,
		GLuint min_filter, GLuint mag_filter) {
		// Bind a DevIL image
		ILuint id;
		ilGenImages(1, &id);
		ilBindImage(id);
		ILuint err = ilGetError();
		if (err) {
			printf("Error binding image. %s %d\n", fname, err);
			return;
		}

		// Load image using lower left origin. Convert to RGBA. Tyr loading from 
		// ../textures, if that fails try ../../textures
		ilOriginFunc(IL_ORIGIN_LOWER_LEFT);
		ilEnable(IL_ORIGIN_SET);
		std::string full_path_name = "../textures/" + fname;
		ilLoadImage(full_path_name.c_str());
		err = ilGetError();
		if (err) {
			full_path_name = "../../textures/" + fname;
			ilLoadImage(full_path_name.c_str());
		}
		err = ilGetError();
		if (err) {
			printf("Error loading texture. %s %d\n", fname, err);
			return;
		}
		ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE);
		err = ilGetError();
		if (err) {
			printf("Could not convert texture to RGBA. %s %d\n", fname, err);
			return;
		}

		// Get image dimensions and data
		int w = ilGetInteger(IL_IMAGE_WIDTH);
		int h = ilGetInteger(IL_IMAGE_HEIGHT);

		unsigned char* data = ilGetData();
		if (ilGetError() != IL_NO_ERROR) {
			std::cout << "Error getting image data" << std::endl;
			return;
		}

		// Generate an OpenGL textureID, bind it
		glGenTextures(1, &texture_id);
		glBindTexture(GL_TEXTURE_2D, texture_id);

		// Load image data and generate mipmaps
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h,
			0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		// Set wrapping mode
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap_s);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap_t);

		// Set texture filters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_filter);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filter);

		// Bind null texture
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	/**
	* Set the texture to used for the material.
	* @param  basefname  Texture image filename base
	* @param  wrap_s  OpenGL wrap option (s)
	* @param  wrap_t  OpenGL wrap option (t)
	* @param  min_filter  OpenGL filter to use for minification
	* @param  mag_filter  OpenGL filter to use for magnification
	*/
	void SetAnimatedTexture(const std::string& basefname, GLuint wrap_s, GLuint wrap_t,
		GLuint min_filter, GLuint mag_filter, int frames, const std::string& ext) {
		this->texture_ids = std::vector<GLuint>();
		this->frames = frames;
		for (int i = 1; i <= frames + 1; i++)
		{
			// Bind a DevIL image
			ILuint id;
			ilGenImages(1, &id);
			ilBindImage(id);
			std::string fname = basefname;
			std::stringstream ss;
			ss << i;
			if (i < 10)
			{
				fname += "0" + ss.str() + ext;
			}
			else
			{
				fname += ss.str() + ext;
			}

			ILuint err = ilGetError();
			if (err) {
				printf("Error binding image. %s %d\n", fname, err);
				return;
			}

			// Load image using lower left origin. Convert to RGBA. Tyr loading from 
			// ../textures, if that fails try ../../textures
			ilOriginFunc(IL_ORIGIN_LOWER_LEFT);
			ilEnable(IL_ORIGIN_SET);
			std::string full_path_name = "../textures/" + fname;
			ilLoadImage(full_path_name.c_str());
			err = ilGetError();
			if (err) {
				full_path_name = "../../textures/" + fname;
				ilLoadImage(full_path_name.c_str());
			}
			err = ilGetError();
			if (err) {
				printf("Error loading texture. %s %d\n", fname, err);
				return;
			}
			ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE);
			err = ilGetError();
			if (err) {
				printf("Could not convert texture to RGBA. %s %d\n", fname, err);
				return;
			}

			// Get image dimensions and data
			int w = ilGetInteger(IL_IMAGE_WIDTH);
			int h = ilGetInteger(IL_IMAGE_HEIGHT);

			unsigned char* data = ilGetData();
			if (ilGetError() != IL_NO_ERROR) {
				std::cout << "Error getting image data" << std::endl;
				return;
			}

			// Generate an OpenGL textureID, bind it
			glGenTextures(1, &texture_id);
			glBindTexture(GL_TEXTURE_2D, texture_id);
			this->texture_ids.push_back(texture_id);
			// Load image data and generate mipmaps
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h,
				0, GL_RGBA, GL_UNSIGNED_BYTE, data);
			glGenerateMipmap(GL_TEXTURE_2D);

			// Set wrapping mode
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap_s);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap_t);

			// Set texture filters
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_filter);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filter);

			// Bind null texture
			glBindTexture(GL_TEXTURE_2D, 0);
		}
		texture_id = this->texture_ids[0];
		texture_index = 0;
	}

	/**
	* Set the normal map to use for the material.
	* @param  fname  Texture image filename
	* @param  wrap_s  OpenGL wrap option (s)
	* @param  wrap_t  OpenGL wrap option (t)
	* @param  min_filter  OpenGL filter to use for minification
	* @param  mag_filter  OpenGL filter to use for magnification
	*/
	void setNormalMap(const std::string& fname, GLuint wrap_s, GLuint wrap_t, GLuint min_filter, GLuint mag_filter)
	{
		// Bind a DevIL image
		ILuint id;
		ilGenImages(1, &id);
		ilBindImage(id);
		ILuint err = ilGetError();
		if (err) {
			printf("Error binding image. %s %d\n", fname, err);
			return;
		}

		// Load image using lower left origin. Convert to RGB. Tyr loading from 
		// ../textures, if that fails try ../../textures
		ilOriginFunc(IL_ORIGIN_LOWER_LEFT);
		ilEnable(IL_ORIGIN_SET);
		std::string full_path_name = "../textures/" + fname;
		ilLoadImage(full_path_name.c_str());
		err = ilGetError();
		if (err) {
			full_path_name = "../../textures/" + fname;
			ilLoadImage(full_path_name.c_str());
		}
		err = ilGetError();
		if (err) {
			printf("Error loading texture. %s %d\n", fname, err);
			return;
		}
		ilConvertImage(IL_RGB, IL_UNSIGNED_BYTE);
		err = ilGetError();
		if (err) {
			printf("Could not convert texture to RGB. %s %d\n", fname, err);
			return;
		}

		// Get image dimensions and data
		int w = ilGetInteger(IL_IMAGE_WIDTH);
		int h = ilGetInteger(IL_IMAGE_HEIGHT);

		unsigned char* data = ilGetData();
		if (ilGetError() != IL_NO_ERROR) {
			std::cout << "Error getting image data" << std::endl;
			return;
		}

		// Generate an OpenGL textureID, bind it
		glGenTextures(1, &normalMapID);
		glBindTexture(GL_TEXTURE_2D, normalMapID);

		// Load image data and generate mipmaps
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		// Set wrapping mode
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap_s);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap_t);

		// Set texture filters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_filter);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filter);

		// Bind null texture
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	/**
	 * Update texture filtering for this material
	 * @param  min_filter  OpenGL filter to use for minification
	 * @param  mag_filter  OpenGL filter to use for magnification
	 */
	void UpdateTextureFilters(GLuint min_filter, GLuint mag_filter) {
		if (texture_id) {
			glBindTexture(GL_TEXTURE_2D, texture_id);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_filter);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filter);
		}

		if (normalMapID)
		{
			glBindTexture(GL_TEXTURE_2D, normalMapID);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_filter);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filter);
		}
	}

    /**
     * Scales the texture by the specified factor
     * @param  scale  The factor to scale the texture coordinates by
     */
    void setTextureScale(float scale)
    {
        textureScale = scale;
    }

	/**
	 * Draw. Sets the material properties.
	 * @param  scene_state  Scene state (holds material uniform locations)
	 */
	void Draw(SceneState& scene_state) {
		// Set the material uniform values
		glUniform4fv(scene_state.materialambient_loc, 1, &material_ambient.r);
		glUniform4fv(scene_state.materialdiffuse_loc, 1, &material_diffuse.r);
		glUniform4fv(scene_state.materialspecular_loc, 1, &material_specular.r);
		glUniform4fv(scene_state.materialemission_loc, 1, &material_emission.r);
		glUniform1f(scene_state.materialshininess_loc, material_shininess);

		// Enable texture mapping and bind the texture
		if (texture_id) {
            glUniform1i(scene_state.usetexture_loc, 1);   // Tell shader we are using textures
            glUniform1f(scene_state.texturescale_loc, textureScale);

			glUniform1i(scene_state.textureunit_loc, 0);  // Texture unit 0
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, texture_id);
		}
		else {
			// Set a special value to tell the shader we are not using textures
			glUniform1i(scene_state.usetexture_loc, 0);
		}

		// Enable normal mapping and bind the texture
		if (normalMapID)
		{
			glUniform1i(scene_state.usenormalmap_loc, 1);   // Tell shader we are using normal maps
			glUniform1i(scene_state.normalmap_loc, 1);      // Texture unit 1
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, normalMapID);
		}

		// Draw children of this node
		SceneNode::Draw(scene_state);

		// Turn off texture mapping for any nodes not descended from this presentation node
		glUniform1i(scene_state.usetexture_loc, 0);
		glUniform1i(scene_state.usenormalmap_loc, 0);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	/**
	 * Updates the image to be used during an animation
	 */
	void UpdateFrame()
	{
		texture_index++;

		if (texture_index == frames)
		{
			texture_index = 0;
		}

		texture_id = this->texture_ids[texture_index];
	}

	/**
	 * Return whether the screen is powered on
	 */
	bool GetPoweredOn()
	{
		return powered_on;
	}

	/**
	 * Toggle the tv power
	 */
	void TogglePower()
	{
		powered_on = !powered_on;

		if (!powered_on)
		{
			texture_id = texture_ids[frames];
		}
	}

protected:
	Color4  material_ambient;
	Color4  material_diffuse;
	Color4  material_specular;
	Color4  material_emission;
	GLfloat material_shininess;

	std::vector<GLuint> texture_ids;
	GLuint texture_id;
	GLuint normalMapID;
    float textureScale;

	int texture_index;
	int frames;
	bool powered_on = true;
};

#endif
