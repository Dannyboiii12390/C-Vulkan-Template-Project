# Technical Report: Graphics Project (700106)

## Geometry Representation and Processing

The project employs a layered geometry system built on Vulkan. At the core, the `Mesh` class encapsulates vertex and index buffers, supporting various vertex attributes including position, color, normal, UV coordinates, tangent, and binormal vectors. This comprehensive vertex structure enables advanced rendering techniques like bump mapping and normal mapping.

Geometry is procedurally generated through the `ModelLoader` class, which provides factory methods for primitive shapes (spheres, cubes, toruses, terrain grids) and OBJ file loading. The sphere generation uses parametric equations with configurable latitude/longitude subdivisions, while terrain uses height-map-based vertex displacement. The modular design allows easy extension for new geometry types.

Object-to-world transformation uses the `Transform` class storing position, rotation, and scale as separate components, composed into model matrices using GLM. Push constants deliver per-object model matrices to shaders efficiently, avoiding descriptor set overhead for frequently changing data.

## Shading and Lighting

The lighting system implements both per-vertex and per-pixel shading models. The Uniform Buffer Object (UBO) contains shared rendering parameters including view/projection matrices, camera position, and dual light sources (sun and moon) with positions, colors, and intensities.

**Bump Mapping**: Implemented through tangent-space normal mapping with parallax occlusion mapping. The vertex shader constructs TBN (Tangent-Bitangent-Normal) matrices transforming light and view directions into tangent space. The fragment shader samples height maps for parallax displacement, creating the illusion of surface depth. The implementation uses steep parallax mapping with 8-16 adaptive layers based on view angle, balancing quality and performance.

**Globe Rendering**: The globe features dual-mode rendering based on viewer position (inside vs. outside), controlled via the `inside_globe` UBO flag. Inside the globe, the environment map displays sun and moon as emissive discs using angular masks computed from dot products with light directions. Spherical UV mapping prevents interpolation seams by computing UVs directly from world-space surface directions in the fragment shader rather than interpolating from vertices.

**Lighting Model**: Uses Blinn-Phong shading with ambient, diffuse, and specular components. The fragment shader computes lighting per-pixel for both sun and moon, with configurable specular strength and shininess parameters. Shadow attenuation modulates diffuse and specular contributions.

## Shadow Generation

Shadow mapping is implemented using dual cascaded shadow maps for sun and moon. The `ShadowMapPipeline` renders scene geometry from the light's perspective into 4096x4096 depth-only framebuffers using dynamic rendering (Vulkan 1.4).

**Shadow Map Generation**: The pipeline uses front-face culling to reduce shadow acne and enables dynamic depth bias (set via `vkCmdSetDepthBias`). Model matrices are passed via push constants, and only depth testing is performed without color attachments.

**Shadow Sampling**: Fragment shaders use `sampler2DShadow` for hardware PCF (Percentage Closer Filtering). The implementation performs 3x3 PCF sampling with dynamic bias calculation based on surface-to-light angle (`max(0.005 * (1.0 - NdotL), 0.0005)`). Shadow coordinates are transformed to light space using `lightSpaceMatrix` from the UBO, then projected and offset to [0,1] range for shadow map lookup.

**Optimizations**: Bounds checking prevents sampling outside shadow maps (returning unshadowed when out of bounds). The dual shadow system allows independent day/night shadow casting.

## Application Objects and Graphics Representation

The architecture follows a component-based design where application objects (`Object` class) aggregate rendering components:

- **Mesh**: Vertex/index buffers representing geometry
- **Pipeline**: Shader programs and render state configuration
- **Texture**: Albedo and normal map resources
- **Transform**: Position, rotation, scale data
- **Descriptor Sets**: Per-frame uniform buffer and texture bindings

This separation provides clear advantages: meshes and pipelines are reusable across objects, textures can be shared, and transforms update independently of rendering resources. The disadvantage is increased memory indirection and potential cache misses during rendering.

The `VulkanContext` maintains object collections and orchestrates rendering. Objects are created through `ModelLoader::loadFromConfig()`, parsing a text-based configuration file that specifies object types, shaders, textures, and transforms. This data-driven approach allows scene changes without recompilation.

## Update Propagation System

The update cycle follows this sequence per frame:

1. **Input Processing**: `updateInputHandler()` polls GLFW events
2. **Uniform Update**: `updateUniformBuffer()` computes view/projection matrices, light positions based on time, and populates the UBO
3. **Particle Update**: `particleManager.update(deltaTime)` advances particle simulations
4. **Command Recording**: `recordCommandBuffer()` rebuilds rendering commands

Uniform buffers use double buffering (`MAX_FRAMES_IN_FLIGHT = 2`), allowing CPU updates while GPU processes the previous frame. The UBO is mapped persistently and updated via `memcpy` each frame. Object transforms propagate to shaders via push constants during draw calls.

The `ParticleManager` demonstrates behavior-driven updates: it monitors weather conditions and cactus growth states, spawning/despawning particle systems and scaling objects over time. Updates write directly to particle vertex buffers, which are consumed during particle rendering.

**Configuration-Driven Updates**: The config file supports initial light directions and season parameters (`snow_probability`, `snow_increment`), enabling runtime behavior customization without code changes.

## Potential Extensions and Scalability

**Non-Implemented Features**:
- **Displacement Mapping**: While bump mapping simulates surface detail, true displacement would require tessellation shaders, adding geometric complexity for more realistic terrain deformation
- **Environment Mapping**: Reflections could enhance realism, requiring cube map generation and per-frame updates for dynamic objects
- **Deferred Rendering**: Would improve scalability with many lights but requires Multiple Render Targets (MRT) and increases memory bandwidth
- **HDR and Tone Mapping**: Would provide better dynamic range control, particularly for bright sun/moon against dark environments

**Scalability Issues**:
1. **Draw Calls**: Current forward rendering issues one draw call per object. Instancing could batch similar objects
2. **Shadow Map Resolution**: 4096x4096 depth maps consume significant memory (64MB total for dual shadows). Cascaded shadow maps or virtual shadow maps would improve quality/performance tradeoff
3. **Particle Count**: High-density particle systems (up to 2 million particles for dust effects) can cause vertex buffer exhaustion. GPU-based particle simulation via compute shaders would scale better
4. **Descriptor Updates**: Per-object descriptor sets limit batching. Bindless textures (descriptor indexing) would reduce overhead

**Optimization Opportunities**: Frustum culling is absent—off-screen objects still render. Occlusion culling could skip objects hidden by terrain or globe. Level-of-detail (LOD) systems could reduce geometry complexity at distance.

## Feature Wishlist

**Volumetric Dust Clouds**: Rather than particle-based dust, volumetric rendering using raymarching through 3D noise textures would create more convincing atmospheric effects with natural light scattering. This could simulate sandstorms with proper density falloff and shadowing.

**Procedural Terrain Generation**: Replacing static height maps with GPU-generated Perlin/Simplex noise would enable infinite terrain variation. Combined with tessellation, this would allow dynamic detail levels and runtime modification, creating more engaging environments.

**Temporal Anti-Aliasing (TAA)**: Would significantly improve visual quality by reducing specular aliasing on normal maps and particle flickering, crucial given the high geometric detail from bump mapping.

---

**Word Count**: ~998 words
