# OBJ Loader

A small header-only Wavefront OBJ loader for OpenGL

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
- Each ObjVertex contains a position, UV coordinate, and normal. Missing UVs default to (0, 0).

## MTL / material instructions

This loader does not parse `.mtl` files or apply material assignments from `mtllib` / `usemtl` statements. If your OBJ references a material library, the geometry will still load, but the material information will be ignored.

Typical OBJ material lines look like this:

```obj
mtllib example.mtl
usemtl Wood
```

When using this loader:
1. Keep the `.obj` and `.mtl` files together in the same directory if you want to keep the asset structure organized.
2. Load the geometry with this loader as usual.
3. Handle materials separately in your own OpenGL code if needed.
4. If you want textures or colors from the MTL file, parse the `.mtl` file with a separate material loader or manually map the material names to texture IDs in your renderer.

This loader only provides mesh geometry; it does not expose material names, diffuse colors, or texture references.

Limitations
Material libraries and textures (mtllib, usemtl) are not loaded. Concave polygons may not triangulate correctly with the triangle-fan method.
