#version 330
#extension GL_ARB_explicit_attrib_location : require

// Uniform constants
uniform vec3 u_specularColor;
uniform vec3 u_specularPower; // Ensure this is defined as float
uniform vec3 u_ambientColor;
uniform float u_enableGammaCorrection;

// Fragment shader inputs
in vec3 v_viewSpaceNormal;
in vec3 v_viewSpaceLightDir;
in vec3 v_viewDirection;
in vec3 v_color;

// Fragment shader outputs
out vec4 frag_color;

void main()
{
    // Normalize input vectors
    vec3 normal = normalize(v_viewSpaceNormal);
    vec3 lightDir = normalize(v_viewSpaceLightDir);
    vec3 viewDir = normalize(v_viewDirection);
    
    // Lambertian (diffuse) reflection
    float diffuseStrength = max(dot(normal, lightDir), 0.0);
    vec3 diffuseColor = diffuseStrength * v_color;

    // Blinn-Phong (specular) reflection
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float specularStrengthR = pow(max(dot(normal, halfwayDir), 0.0), u_specularPower.r);
    float specularStrengthG = pow(max(dot(normal, halfwayDir), 0.0), u_specularPower.g);
    float specularStrengthB = pow(max(dot(normal, halfwayDir), 0.0), u_specularPower.b);
    vec3 specularStrength = vec3(specularStrengthR, specularStrengthG, specularStrengthB);
    vec3 specularColor = u_specularColor * specularStrength;

    // Ambient reflection
    vec3 ambientColor = u_ambientColor * v_color;

    // Combine all components to get final color
    vec3 finalColor = ambientColor + diffuseColor + specularColor;

    // Apply gamma correction if enabled
    
    if (u_enableGammaCorrection >0 ) {
        finalColor = pow(finalColor, vec3(1.0 / 2.2));
    } else {

    }

    // Output final color
    frag_color = vec4(finalColor, 1.0);
}
