///============================================================================
//	Johns Hopkins University Engineering Programs for Professionals
//	605.467 Computer Graphics and 605.767 Applied Computer Graphics
//	Instructor:	David W. Nesbitt
//
//	Author:  David W. Nesbitt
//	File:		Color4.h
//	Purpose: Color structure with RGBA. Supports adding and blending colors.
//          Includes clamping to [0,1] range.
//============================================================================

#ifndef __COLOR4_H__
#define __COLOR4_H__

#include <math.h>

/**
 * Color structure. Colors are specified as red, green, and blue values
 * with range [0.0-1.0].
 * Note: methods no longer clamp to [0,1] range. It is the responsibility
 * of the app to call Clamp when needed. This is an efficiency consideration
 * to support ray-tracing.
 */
struct Color4 {
	float r, g, b, a;

	/**
   * Constructor.  Values default to 0,0,0.
   */
  Color4(void) 
    : r(0.0f), 
      g(0.0f), 
      b(0.0f) {
  }

  /**
   * Constructor. Set RGB to specified values. Clamps to range [0.0, 1.0]
   * @param	ir		Red intensity
	 * @param	ig		Green intensity
	 * @param	ib		Blue intensity
   */
  Color4(const float ir, const float ig, const float ib, const float ia) 
    : r(ir),
      g(ig),
      b(ib),
      a(ia) {
  }

  /**
   * Constructor with RGB. Sets A to 1.0. Clamps to range [0.0, 1.0]
   * @param	ir		Red intensity
	 * @param	ig		Green intensity
	 * @param	ib		Blue intensity
   */
  Color4(const float ir, const float ig, const float ib)  
    : r(ir),
      g(ig),
      b(ib),
      a(1.0f) {
  }

  /**
   * Constructor from a Color3. Sets A to 1.0f. Should be no need to clamp
   * since Color3 must have been clamped to [0,1] range.
   * @param	c	Color assigned to member.
   */
  Color4(const Color3& c)
    : r(c.r),
      g(c.g),
      b(c.b),
      a(1.0f) {
  }

  /**
   * Copy constructor.
   * @param	c	Color assigned to member.
   */
  Color4(const Color4& c)
    : r(c.r),
      g(c.g),
      b(c.b),
      a(c.a) {
  }

  /**
   * Assignment operator.
   * @param	c	Color to assign to the object.
   * @return	Returns the address of the member data.
   */
  Color4& operator = (const Color4& c) {
    r = c.r;
    g = c.g;
    b = c.b;
    a = c.a;
    return *this;
  }

  /**
   *	Set the color to the specified RGB values.
	 * @param	ir		Red intensity
	 * @param	ig		Green intensity 
	 * @param	ib		Blue intensity
   */
	void Set(const float ir, const float ig, const float ib, const float ia) {
    r = ir;
    g = ig;
    b = ib;
    a = ia;
  }

  /**
   *	Get the color components.
	 * @param	ir		Red intensity (return)
	 * @param	ig		Green intensity (return)
	 * @param	ib		Blue intensity (return)
   */
	void Get(float& ir, float& ig, float& ib, float& ia) const {
    ir = r;
    ig = g;
    ib = b;
    ia = a;
  }

  /**
   * Get the red value in the range 0-255
   * @return  Returns red value as a [0-255] value
   */
  uint8_t GetRed() const {
    return static_cast<uint8_t>(r * 255.0f);
  }

  /**
   * Get the green value in the range 0-255
   * @return  Returns green value as a [0-255] value
   */
  uint8_t GetGreen() const {
    return static_cast<uint8_t>(g * 255.0f);
  }

  /**
   * Get the blue value in the range 0-255
   * @return  Returns blue value as a [0-255] value
   */
  uint8_t GetBlue() const {
    return static_cast<uint8_t>(b * 255.0f);
  }

  /**
   * Multiplication operator: Multiplies the color by another color
   */
  Color4 operator * (const Color4& color) const {
    return Color4(r * color.r, g * color.g, b * color.b, a * color.a);
  }

  /**
   * Multiplication operator: Multiplies the color by another color (RGB only).
   * Ignores alpha.
   * @return  Returns RGB color
   */
  Color3 operator * (const Color3& color) const {
    return Color3(r * color.r, g * color.g, b * color.b);
  }

  /**
   * Scales the color by a scalar factor.
   */
  Color4 operator * (const float factor) {
    return Color4(r * factor, g * factor, b * factor, a * factor);
  }

  /**
   * Adds another color to the current color. Clamps to the valid range.
   */
  Color4& operator += (const Color4& color) {
    r += color.r;
    g += color.g;
    b += color.b;
    a += color.a;
    return *this;
  }

  /**
   * Creates a new color that is the current color plus the
   * specified color.
   * @param   c  Color to add to the current color.
   * @return  Returns the resulting color.
   */
	Color4 operator + (const Color4& c) const {
    return Color4(r + c.r, g + c.g, b + c.b, a + c.a);
	}

  /**
   * Clamps a color to the range [0.0, 1.0].
   */
  void Clamp(void) {
    if (r <= 0.0f)
      r = 0.0f;
    else if (r >= 1.0f)
      r = 1.0f;
    if (g <= 0.0f)
      g = 0.0f;
    else if (g >= 1.0f)
      g = 1.0f;      
    if (b <= 0.0f)
      b = 0.0f;
    else if (b >= 1.0f)
      b = 1.0f;
    if (a <= 0.0f)
      a = 0.0f;
    else if (a >= 1.0f)
      a = 1.0f;
  }
};

#endif
