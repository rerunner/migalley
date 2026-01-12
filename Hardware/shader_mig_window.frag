#version 450

// ──────────────────────────────────────────────────────────────────────────────
// Descriptor bindings
layout(set = 0, binding = 0) uniform sampler2D screenTex;
layout(location = 0) out vec4 outColor;

// ──────────────────────────────────────────────────────────────────────────────
// Constants
const float PI = 3.14159265359;

// ──────────────────────────────────────────────────────────────────────────────
// Specialization constants (tunable defaults)
layout(constant_id = 0) const float curvature     = 0.01; // 0.02 → 0.08
layout(constant_id = 1) const float scanline      = 0.28;  // 0.15 → 0.40
layout(constant_id = 2) const float scanline_fade = 0.45;  // vertical bleed
layout(constant_id = 3) const float phosphor_mask = 0.18;  // RGB mask strength
layout(constant_id = 4) const float vignette      = 0.22;  // corner darkening
layout(constant_id = 5) const float gamma_in      = 2.2;
layout(constant_id = 6) const float gamma_out     = 2.4;
// ──────────────────────────────────────────────────────────────────────────────

vec2 curve(vec2 uv, float k)
{
    uv = uv * 2.0 - 1.0;
    uv *= 1.0 + k * dot(uv, uv);
    return uv * 0.5 + 0.5;
}

// Cheap but nice shadow mask
vec3 shadowMask(vec2 pos)
{
    pos *= vec2(1.0, 0.5);
    vec2 p = floor(pos + 0.5);
    float v = mod(p.x + p.y, 3.0);

    vec3 mask = vec3(1.0);
    mask.r = 1.0 - phosphor_mask * step(0.0, v - 0.5);
    mask.g = 1.0 - phosphor_mask * step(1.0, v) * step(v, 2.0);
    mask.b = 1.0 - phosphor_mask * step(2.0, v);

    return mask;
}

void main()
{
    ivec2 ts = textureSize(screenTex, 0);
    vec2 uv = gl_FragCoord.xy / vec2(ts);
    float aspect = float(ts.x) / float(ts.y);

    // 1. Screen curvature
    vec2 curvedUV = curve(uv, curvature);

    // Outside screen → black
    if (any(lessThan(curvedUV, vec2(0.0))) ||
        any(greaterThan(curvedUV, vec2(1.0))))
    {
        outColor = vec4(0.0, 0.0, 0.0, 1.0);
        return;
    }

    // 2. Sample + input gamma
    vec3 color = texture(screenTex, curvedUV).rgb;
    color = pow(max(color, 0.0), vec3(gamma_in));

    // 3. Phosphor shadow mask
    vec3 mask = shadowMask(gl_FragCoord.xy);
    color *= mix(vec3(1.0), mask, phosphor_mask);

    // 4. Scanlines
    float y = gl_FragCoord.y;
    float scan = sin(y * PI * 2.0 + 0.5) * scanline_fade
               + (1.0 - scanline_fade);
    scan = mix(1.0, scan, scanline);

    // Vertical bloom (cheap)
    float bloom = 0.0;
    bloom += texture(screenTex, curvedUV + vec2(0.0,  1.0 / ts.y)).r * 0.12;
    bloom += texture(screenTex, curvedUV + vec2(0.0,  2.0 / ts.y)).g * 0.08;
    bloom += texture(screenTex, curvedUV + vec2(0.0, -1.0 / ts.y)).b * 0.10;
    color += bloom * 0.25 * scanline;

    color *= scan;

    // 5. Vignette
    vec2 vigUV = uv * 2.0 - 1.0;
    vigUV.x *= aspect;
    float vig = 1.0 - dot(vigUV, vigUV) * vignette;
    vig = pow(max(vig, 0.0), 1.4);
    color *= vig;

    // 6. Output gamma + slight contrast
    color = pow(color, vec3(1.0 / gamma_out));
    color = color * 1.05 - 0.025;

    outColor = vec4(color, 1.0);
}
