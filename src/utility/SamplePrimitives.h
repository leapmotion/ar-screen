#pragma once

class ImagePrimitive;
class SVGPrimitive;

/// <summary>
/// Creates a "box" svg primitive
/// </summary>
std::shared_ptr<SVGPrimitive> MakeBoxPrimitive(void);

/// <summary>
/// Creates a patterend square texture with the given dimensions
/// </summary>
std::shared_ptr<ImagePrimitive> MakePatternedTexture(size_t cx, size_t cy);