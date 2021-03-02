/* Copyright 2013 Simon Opelt and Clemens Birklbauer. All rights reserved.
 * For license and copyright information see license.txt of LFC2013.
 */
#pragma once

#include <string>
#include <vector>
#include <glm/glm.hpp>


class Lightfield
{
private:
	std::vector<std::string> names;
	std::vector<glm::mat4> poses;
	std::vector<unsigned int> ogl_textures;
public:
	Lightfield(const int preallocate = 0)
	{
		if (preallocate > 0)
		{
			names.reserve(preallocate);
			poses.reserve(preallocate);
			ogl_textures.reserve(preallocate);
		}
	}
	void allocate(const int allocate = 0)
	{
		if (allocate > 0)
		{
			names.reserve(allocate);
			poses.reserve(allocate);
			ogl_textures.reserve(allocate);
		}
		
	}
	const std::string& GetName(const unsigned int index) { return names[index];  }
	const glm::mat4& GetPose(const unsigned int index) { return poses[index]; }
	const glm::vec3 GetPosition(const unsigned int index) { return glm::vec3(glm::inverse(poses[index])[3]); }
	const glm::vec3 GetUp(const unsigned int index) { return glm::vec3(glm::inverse(poses[index])[1]); }
	const glm::vec3 GetForward(const unsigned int index) { return glm::vec3(glm::inverse(poses[index])[2]); }
	const unsigned int& GetOglTexture(const unsigned int index) { return ogl_textures[index]; }
	const unsigned int GetSize() { return poses.size(); }


friend class LFGenerator;
};



class LFGenerator // Universal & Unstructured LF Generator: takes care of loading the light field
{
private:

public:
	LFGenerator(void);
	virtual ~LFGenerator(void);
	Lightfield *Generate( const std::string &jsonPoseFile, const std::string &imgFilePath = "" );
};


