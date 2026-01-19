/*
 * Fragment Shader - Runs once per pixel (fragment)
 *
 * This shader determines the final color of each pixel. It receives
 * interpolated values from the vertex shader and computes lighting.
 *
 * We use a simple "flat shading" style for the low-poly aesthetic:
 * - Diffuse lighting based on surface angle to light
 * - Ambient light to prevent completely dark areas
 * - No specular highlights (keeps it flat/matte)
 */

#version 330 core

/* =============================================================================
 * Input from Vertex Shader (interpolated)
 *
 * These values are automatically interpolated across the triangle surface.
 * The GPU calculates weighted averages based on the fragment's barycentric
 * coordinates relative to the three vertices.
 * ============================================================================= */
in vec3 FragPos;     /* World-space position of this fragment */
in vec3 Normal;      /* Surface normal (interpolated, may need re-normalizing) */
in vec3 VertexColor; /* Base color from vertex data */

/* =============================================================================
 * Output
 *
 * FragColor is the final RGBA color written to the framebuffer.
 * Alpha is 1.0 (fully opaque).
 * ============================================================================= */
out vec4 FragColor;

/* =============================================================================
 * Uniforms (lighting parameters set from C++)
 * ============================================================================= */
uniform vec3 lightDir;        /* Direction TO the light source (normalized) */
uniform vec3 lightColor;      /* Color/intensity of the light */
uniform float ambientStrength;/* How much ambient (indirect) light */

/* =============================================================================
 * main() - Calculate final pixel color
 * ============================================================================= */
void main()
{
    /*
     * AMBIENT LIGHTING
     *
     * Ambient light simulates indirect/bounced light that illuminates
     * everything equally. Without it, surfaces facing away from the light
     * would be completely black.
     *
     * ambient = ambientStrength * lightColor
     */
    vec3 ambient = ambientStrength * lightColor;

    /*
     * DIFFUSE LIGHTING (Lambertian reflection)
     *
     * Diffuse light depends on the angle between the surface normal and
     * the light direction. Surfaces facing the light are brightest.
     *
     * The dot product of two unit vectors equals cos(angle between them):
     *   - Facing light (0°): dot = 1.0 (full brightness)
     *   - Perpendicular (90°): dot = 0.0 (no light)
     *   - Facing away (>90°): dot < 0 (we clamp to 0)
     *
     * Formula: diffuse = max(dot(normal, lightDir), 0.0) * lightColor
     */

    /* Re-normalize the normal (interpolation can change length) */
    vec3 norm = normalize(Normal);

    /* Light direction is typically specified as "direction TO light"
     * If stored as "direction FROM light", negate it here */
    vec3 lightDirection = normalize(-lightDir);

    /* Calculate diffuse intensity */
    float diff = max(dot(norm, lightDirection), 0.0);
    vec3 diffuse = diff * lightColor;

    /*
     * FINAL COLOR
     *
     * Combine ambient and diffuse lighting with the surface color.
     *
     * (ambient + diffuse) is the total light intensity at this point.
     * Multiplying by VertexColor applies the object's base color.
     */
    vec3 result = (ambient + diffuse) * VertexColor;

    /* Output with full opacity */
    FragColor = vec4(result, 1.0);
}

/*
 * NOTES ON LOW-POLY STYLE:
 *
 * For a "flat shaded" low-poly look, we could use:
 *   - flat out vec3 Normal; in vertex shader
 *   - flat in vec3 Normal; in fragment shader
 * The "flat" qualifier disables interpolation, so each triangle
 * gets a uniform color based on one vertex's normal.
 *
 * Currently we're using smooth shading. To get flat shading per-face,
 * ensure each face has its own vertices with face normals (which we do).
 */
