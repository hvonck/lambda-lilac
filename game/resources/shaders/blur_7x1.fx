[HORIZONTAL|VERTICAL]
#include "common.fxh"

#if TYPE == HORIZONTAL
static const float2 BLUR_SCALE = float2(1.0f, 0.0f);
#elif TYPE == VERTICAL
static const float2 BLUR_SCALE = float2(0.0f, 1.0f);
#else
static const float BLUR_SCALE = 1.0f;
#endif

struct VSOutput
{
  float2 tex      : TexCoords;
  float4 position : SV_POSITION;
};

VSOutput VS(uint id : SV_VertexID)
{
  VSOutput vOut;
  vOut.tex      = float2((id << 1) & 2, id & 2);
  vOut.position = float4(vOut.tex * float2(2, -2) + float2(-1, 1), 0, 1);

  return vOut;
}

Make_Texture2D(tex_to_blur, 0);

static const float blur_table[4] = {
  1.0f / 64.0f,
  6.0f / 64.0f,
  15.0f / 64.0f,
  20.0f / 64.0f,
};

float4 PS(VSOutput pIn) : SV_TARGET0
{
  float4 colour = 0.0f;

  float2 scale = BLUR_SCALE * inv_texture_size;

  colour += Sample(tex_to_blur, SamLinearClamp, pIn.tex + -3.0f * scale) * blur_table[0];
  colour += Sample(tex_to_blur, SamLinearClamp, pIn.tex + -2.0f * scale) * blur_table[1];
  colour += Sample(tex_to_blur, SamLinearClamp, pIn.tex + -1.0f * scale) * blur_table[2];
  colour += Sample(tex_to_blur, SamLinearClamp, pIn.tex + +0.0f * scale) * blur_table[3];
  colour += Sample(tex_to_blur, SamLinearClamp, pIn.tex + +1.0f * scale) * blur_table[2];
  colour += Sample(tex_to_blur, SamLinearClamp, pIn.tex + +2.0f * scale) * blur_table[1];
  colour += Sample(tex_to_blur, SamLinearClamp, pIn.tex + +3.0f * scale) * blur_table[0];

  return colour;
}