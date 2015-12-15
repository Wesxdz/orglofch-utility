#include "mesh.hpp"

#include <iostream>
#include <set>
#include <vector>

#include "algebra.hpp"
#include "util.hpp"

using namespace std;

/*// TODO(orglofch): Possibly remove
inline
bool MeshIntersect(const Mesh &mesh, const Ray &ray, Intersection *intersection) {
intersection->t = std::numeric_limits<double>::max();
bool has_intersection = false;
for (const Face &tri : mesh.faces) {
assert(tri.size() == 3);
Point3 vertex_0 = mesh.vertices[tri[0]];

Vector3 edge_1 = mesh.vertices[tri[1]] - vertex_0;
Vector3 edge_2 = mesh.vertices[tri[2]] - vertex_0;

Vector3 P = ray.dir.cross(edge_2);
double det = edge_1.dot(P);
if (det > -EPSILON && det < EPSILON) {
continue;
}
double inv_det = 1.0f / det;

Vector3 T = ray.origin - vertex_0;

double u = T.dot(P) * inv_det;
if (u < 0.0f || u > 1.0f) {
continue;
}

Vector3 Q = T.cross(edge_1);

double v = ray.dir.dot(Q) * inv_det;
if (v < 0.0f || u + v > 1.0f) {
continue;
}

double t = edge_2.dot(Q) * inv_det;
if (t > 0 && t < intersection->t) {
Point3 point = RayProjection(ray, t);
Vector3 normal = edge_1.cross(edge_2);
intersection->pos = point;
intersection->normal = normal.dot(ray.dir) < 0 ? normal : -1 * normal;
intersection->normal.normalize();
intersection->t = t;
intersection->material = mesh.material;
has_intersection = true;
}
}
return has_intersection;
}*/

struct VertexIndex {
	int pos;
	int texture;
	int normal;
};

struct Face {
	std::vector<VertexIndex> vertices;
};

struct TriIndex {
	unsigned int i1, i2, i3;
};

struct MeshVertex {
	Point3 pos;
	Point2 texture;
	Vector3 normal;

	bool operator < (const MeshVertex &other) const {
		return false; // TODO(orglofch):
	}
};

void ExtractUniqueVertices(const std::vector<Point3> &positions,
		const std::vector<Point2> &textures,
		const std::vector<Vector3> &normals,
		const std::vector<Face> &faces,
		std::vector<MeshVertex> *vertices,
		std::vector<int> *face_indices) {
	std::set<MeshVertex> unique_vertices;
	for (const Face &face : faces) {
		for (const VertexIndex &index : face.vertices) {
			/*MeshVertex mesh_vertex;
			mesh_vertex.pos = positions.at(index.pos);
			mesh_vertex.texture = textures.at(index.texture);
			mesh_vertex.normal = normals.at(index.normal);
			unique_vertices.insert(mesh_vertex);*/
		}
	}
}

Mesh LoadMesh(const std::vector<Point3> &positions,
		const std::vector<Point2> &textures,
		const std::vector<Vector3> &normals,
		const std::vector<Face> &faces) {
	Mesh mesh;
	mesh.face_count = faces.size();

	unsigned int face_count = faces.size();
	TriIndex *indices = new TriIndex[face_count];
	for (unsigned int i = 0; i < face_count; ++i) {
		const Face &face = faces.at(i);

		indices[i].i1 = face.vertices.at(0).pos;
		indices[i].i2 = face.vertices.at(1).pos;
		indices[i].i3 = face.vertices.at(2).pos;
	}

	glGenBuffers(1, &mesh.vertexVBO);
	glBindBuffer(GL_ARRAY_BUFFER, mesh.vertexVBO);
	glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(Point3), &positions[0], GL_STATIC_DRAW);

	glGenBuffers(1, &mesh.indexVBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.indexVBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, face_count*sizeof(TriIndex), indices, GL_STATIC_DRAW);

	return mesh;
}

// TODO(orglofch): Possibly remove or optimize instead of connecting last vertex to all other vertices
void TriangulateFaces(const std::vector<Face> &faces, std::vector<Face> *triangulated_faces) {
	/*for (const Face &face : faces) {
		int vertices = face.size();
		if (vertices > 3) {
			int last_vertex = face.at(vertices - 1);
			for (int i = 0; i < vertices - 2; ++i) {
				Face triangle;

				triangle.push_back(face[i]);
				triangle.push_back(face[i + 1]);
				triangle.push_back(last_vertex);

				triangulated_faces->push_back(triangle);
			}
		}
	}*/
}

Face ReadFace(std::istringstream &iss) {
	Face face;
	int pos, texture, normal;
	while (iss >> pos) {
		VertexIndex index;
		index.pos = pos - 1;
		if (iss.peek() == '/') {
			iss >> texture;
			index.texture = texture - 1;
			if (iss.peek() == '/') {
				iss >> normal;
				index.normal = normal - 1;
			}
		}
		face.vertices.push_back(index);
	}
	return face;
}

Mesh LoadOBJ(const std::string &filename) {
	std::vector<Point3> positions;
	std::vector<Point2> textures;
	std::vector<Vector3> normals;
	std::vector<Face> faces;

	std::ifstream ifs(filename);
	if (!ifs.is_open()) {
		LOG("Failed to open %s\n", filename.c_str());
		return Mesh();
	}

	std::string lineHeader;
	while (ifs >> lineHeader) {
		if (lineHeader.compare("v") == 0) { // Vertex
			Point3 vertex;
			ifs >> vertex.x >> vertex.y >> vertex.z;
			positions.push_back(vertex);
		} else if (lineHeader.compare("vt") == 0) { // Texture
			Point2 texture;
			ifs >> texture.x >> texture.y;
			textures.push_back(texture);
		} else if (lineHeader.compare("vn") == 0) { // Normal
			Vector3 normal;
			ifs >> normal.x >> normal.y >> normal.z;
			normals.push_back(normal);
		} else if (lineHeader.compare("f") == 0) { // Face
			char line[256];
			ifs.getline(line, 256);
			std::istringstream iss(line);
			faces.push_back(ReadFace(iss));
		}
	}
	return LoadMesh(positions, textures, normals, faces);
}

void RenderMesh(const Mesh &mesh) {
	glEnableClientState(GL_VERTEX_ARRAY);
	glBindBuffer(GL_ARRAY_BUFFER, mesh.vertexVBO);
	glVertexPointer(3, GL_DOUBLE, sizeof(Point3), (char*)NULL + 0);

	glEnableClientState(GL_NORMAL_ARRAY);
	glNormalPointer(GL_FLOAT, sizeof(Point3), (char*)NULL + 12);
	glClientActiveTexture(GL_TEXTURE0);

	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, sizeof(Point3), (char*)NULL + 24);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.indexVBO);
	glDrawElements(GL_TRIANGLES, 3 * mesh.face_count, GL_UNSIGNED_INT, (char*)NULL + 0);

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);

	glBindTexture(GL_TEXTURE_2D, 0);
}