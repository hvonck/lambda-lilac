#include "common.fx"

struct VSInput
{
  float3 position : Positions;
  float2 tex      : TexCoords;
};

struct VSOutput
{
  float4 position  : SV_POSITION0;
  float4 hPosition : HPOSITION;
  float2 tex       : TEX_COORD;
};

#if !defined(VIOLET_DIRECTIONAL) && !defined(VIOLET_POINT) && !defined(VIOLET_SPOT)
#define VIOLET_DIRECTIONAL
#endif

cbuffer cbPerMesh
{
  float4x4 model_matrix;
  float4x4 light_view_projection_matrix;
}

cbuffer cbPostProcess
{
#if defined(VIOLET_POINT) || defined(VIOLET_SPOT)
  float3 light_camera_position;
#elif defined(VIOLET_DIRECTIONAL)
  float light_far = 10.0f;
#endif
}

VSOutput VS(VSInput vIn)
{
  VSOutput vOut;
  vOut.hPosition   = mul(model_matrix, float4(vIn.position, 1.0f));
  vOut.position    = mul(light_view_projection_matrix, vOut.hPosition);
  vOut.tex         = vIn.tex;
  return vOut;
}

Texture2D tex_albedo : register(t0);

// TODO (Hilze): Support alpha.
float4 PS(VSOutput pIn) : SV_Target0
{
  if (tex_albedo.Sample(SamLinearWarp, pIn.tex).a < 0.5f)
    discard;

#if defined(VIOLET_POINT) || defined(VIOLET_SPOT)
  float4 position = pIn.hPosition;
  float depth = length(position - light_camera_position);
#elif defined(VIOLET_DIRECTIONAL)
  float depth = pIn.position.z * 0.5f + 0.5f;
#endif

  float dx = ddx(depth);
  float dy = ddy(depth);
  float moment2 = depth * depth + 0.25f * (dx * dx + dy * dy);
  
  return float4(depth, moment2, depth, 1.0f);
}