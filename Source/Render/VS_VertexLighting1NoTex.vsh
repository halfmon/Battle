//-----------------------------------------------------------------------------
// Global variables
//-----------------------------------------------------------------------------

// World matrix, combined view / projection matrix and camera position for transformation
// and lighting calculations
float4x3 WorldMatrix    : WORLD;
float4x4 ViewProjMatrix : VIEWPROJECTION;

// Material colour
float3 MaterialColour;

// Ambient light colour
float3 AmbientLight;

// Light position colour and attenuation
float3 LightPosition;
float3 LightColour;
float  LightAtten;


//-----------------------------------------------------------------------------
// Input / output structures
//-----------------------------------------------------------------------------

// Input to Vertex Shader
struct VS_Input
{
	float3 Position  : POSITION;  // The position of the vertex in model space
	float3 Normal    : NORMAL;
};

// Output from Vertex Shader
struct VS_Output
{
	float4 Position      : POSITION;
	float3 DiffuseColour : COLOR0;
};

//-----------------------------------------------------------------------------
// Main function
//-----------------------------------------------------------------------------

// Main vertex shader function. Calculates ambient and diffuse lighting from a single
// light and passes colour along with texture coordinate to pixel shader
void main( in VS_Input i, out VS_Output o ) 
{
    // Transform model vertex position to world space, then to viewport space
    float3 WorldPosition = mul( float4(i.Position, 1.0f), WorldMatrix );         
    o.Position = mul( float4(WorldPosition, 1.0f), ViewProjMatrix );

    // Transform model normal to world space
    float3 Normal = normalize( mul( i.Normal, (float3x3)WorldMatrix ) );
	
    // Calculate light attenuation at this world position
	float3 LightVector = LightPosition - WorldPosition;
	float InvLightDist = 1.0f / length( LightVector );
	float LightIntensity = min( 1.0, (LightAtten * InvLightDist) );
	
	// Get direction vector from pixel to light
	float3 LightDir = LightVector * InvLightDist;
	
	// Calculate diffuse light level from the light
	float DiffuseLevel = max( 0.0, dot( Normal, LightDir ) );
		
	// Combine and output colour
	o.DiffuseColour = MaterialColour * (AmbientLight + LightIntensity * LightColour * DiffuseLevel);
}
