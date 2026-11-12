# OBJ Loader

A small header-only Wavefront OBJ loader for the OpenGL project.

## Usage

Keep the `.obj` file in the program's working directory, then load it:

```cpp
ObjMesh mesh;
std::string error;

if (!loadObj("sphere.obj", mesh, error)) {
    std::cerr << "OBJ load failed: " << error << '\n';
    return -1;
}
```

mesh.vertices contains expanded triangle vertices. Upload them to an OpenGL VBO and draw with:
```cpp
glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(mesh.vertices.size()));
```
Supported
- Vertex positions (v)
- Texture coordinates (vt)
- Normals (vn)
- Faces (f) with separate position/UV/normal indices
- Positive and negative OBJ indices
- Polygon triangulation using a triangle fan
- Generated face normals when the OBJ does not provide normals
Each ObjVertex contains a position, UV coordinate, and normal. Missing UVs default to (0, 0).


Limitations
Material libraries and textures (mtllib, usemtl) are not loaded. Concave polygons may not triangulate correctly with the triangle-fan method.
