#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <fstream>
#include <sstream>
#include <string>
#include <stdexcept>

struct ObjVertex {
	glm::vec3 position{ 0.0f };
	glm::vec2 texCoord{ 0.0f }; 
	glm::vec3 normal{ 0.0f };
};

struct ObjMesh {
	std::vector<glm::vec3> positions;
	std::vector<glm::vec2> texCoords;
	std::vector<glm::vec3> normals;
	std::vector<ObjVertex> vertices;
};

inline bool loadObjAttributes(const std::string& path, ObjMesh& mesh, std::string& error) {
	std::ifstream file(path);
	if (!file) {
		error = "Could not open OBJ file: " + path;
		return false;
	}
	std::vector<glm::vec3> positions;
	std::vector<glm::vec2> texCoords;
	std::vector<glm::vec3> normals;
	std::string line;
	std::size_t lineNumber = 0;

	while (std::getline(file, line)) {
		++lineNumber;
		
		std::istringstream row(line);
		std::string type;

		if (!(row >> type) || type[0] == '#') continue;

		if (type == "v") {
			glm::vec3 position;
			if (!(row >> position.x >> position.y >> position.z)) {
				error = "Malformed vertex at line " + std::to_string(lineNumber);
				return false;
			}
			positions.push_back(position);
		}
		else if (type == "vt") {
			glm::vec2 texCoord;
			if (!(row >> texCoord.x >> texCoord.y)) {
				error = "Malformed texture coordinate at line " + std::to_string(lineNumber);
				return false;
			}
			texCoords.push_back(texCoord);
		}
		else if (type == "vn") {
			glm::vec3 normal;
			if (!(row >> normal.x >> normal.y >> normal.z)) {
				error = "Malformed normal at line " + std::to_string(lineNumber);
				return false;
			}
			normals.push_back(normal);
		}
	}

	if (positions.empty()) {
		error = "No vertex positions were found in OBJ file";
		return false;
	}

	mesh.positions.swap(positions);
	mesh.texCoords.swap(texCoords);
	mesh.normals.swap(normals);
	mesh.vertices.clear();
	error.clear();
	return true;
}

inline bool resolveObjIndex(const std::string& text, std::size_t count, std::size_t& index) {
	if (text.empty())
		return false;

	try {
		std::size_t parsedChars = 0;
		const int objIndex = std::stoi(text, &parsedChars);
		if (parsedChars != text.size() || objIndex == 0)
			return false;

		const long long resolved = objIndex > 0
			? static_cast<long long>(objIndex) - 1
			: static_cast<long long>(count) + objIndex;
		if (resolved < 0 || resolved >= static_cast<long long>(count))
			return false;

		index = static_cast<std::size_t>(resolved);
		return true;
	}
	catch (const std::exception&) {
		return false;
	}
}

inline bool parseObjFaceVertex(
	const std::string& token,
	std::size_t positionCount,
	std::size_t texCoordCount,
	std::size_t normalCount,
	const ObjMesh& mesh,
	ObjVertex& vertex)
{
	const std::size_t firstSlash = token.find('/');
	const std::string positionText = token.substr(0, firstSlash);
	std::size_t index = 0;

	if (!resolveObjIndex(positionText, positionCount, index))
		return false;
	vertex.position = mesh.positions[index];

	if (firstSlash == std::string::npos)
		return true;

	const std::size_t secondSlash = token.find('/', firstSlash + 1);
	const std::string texCoordText = token.substr(
		firstSlash + 1,
		secondSlash == std::string::npos ? std::string::npos : secondSlash - firstSlash - 1);
	if (!texCoordText.empty()) {
		if (!resolveObjIndex(texCoordText, texCoordCount, index))
			return false;
		vertex.texCoord = mesh.texCoords[index];
	}

	if (secondSlash == std::string::npos)
		return true;
	if (token.find('/', secondSlash + 1) != std::string::npos)
		return false;

	const std::string normalText = token.substr(secondSlash + 1);
	if (!normalText.empty()) {
		if (!resolveObjIndex(normalText, normalCount, index))
			return false;
		vertex.normal = mesh.normals[index];
	}

	return true;
}


inline bool loadObj(
    const std::string& path,
    ObjMesh& mesh,
    std::string& error)
{
    ObjMesh parsed;
    if (!loadObjAttributes(path, parsed, error))
        return false;

    std::ifstream file(path);
    if (!file) {
        error = "Could not reopen OBJ file: " + path;
        return false;
    }

    std::string line;
    std::size_t lineNumber = 0;
    std::size_t positionsSeen = 0;
    std::size_t texCoordsSeen = 0;
    std::size_t normalsSeen = 0;

    while (std::getline(file, line)) {
        ++lineNumber;

        std::istringstream row(line);
        std::string type;
        if (!(row >> type) || type[0] == '#')
            continue;

        if (type == "v") {
            ++positionsSeen;
            continue;
        }
        if (type == "vt") {
            ++texCoordsSeen;
            continue;
        }
        if (type == "vn") {
            ++normalsSeen;
            continue;
        }
        if (type != "f")
            continue;

        std::vector<ObjVertex> face;
        std::string token;

        while (row >> token) {
            if (token[0] == '#')
                break;

            ObjVertex vertex;
            if (!parseObjFaceVertex(
                    token, positionsSeen, texCoordsSeen, normalsSeen, parsed, vertex)) {
                error = "Invalid face index at line "
                    + std::to_string(lineNumber);
                return false;
            }
            face.push_back(vertex);
        }

        if (face.size() < 3) {
            error = "Face has fewer than 3 vertices at line "
                + std::to_string(lineNumber);
            return false;
        }

        // Turn a polygon into triangles: (0,1,2), (0,2,3), ...
        for (std::size_t i = 1; i + 1 < face.size(); ++i) {
            ObjVertex a = face[0];
            ObjVertex b = face[i];
            ObjVertex c = face[i + 1];

            glm::vec3 faceNormal = glm::cross(b.position - a.position, c.position - a.position);
            if (glm::dot(faceNormal, faceNormal) > 0.000001f)
                faceNormal = glm::normalize(faceNormal);
            else
                faceNormal = glm::vec3(0.0f, 0.0f, 1.0f);

            if (glm::dot(a.normal, a.normal) < 0.000001f) a.normal = faceNormal;
            if (glm::dot(b.normal, b.normal) < 0.000001f) b.normal = faceNormal;
            if (glm::dot(c.normal, c.normal) < 0.000001f) c.normal = faceNormal;

            parsed.vertices.push_back(a);
            parsed.vertices.push_back(b);
            parsed.vertices.push_back(c);
        }
    }

    if (parsed.vertices.empty()) {
        error = "No faces found in OBJ file.";
        return false;
    }

    mesh.positions.swap(parsed.positions);
    mesh.texCoords.swap(parsed.texCoords);
    mesh.normals.swap(parsed.normals);
    mesh.vertices.swap(parsed.vertices);
    error.clear();
    return true;
}
