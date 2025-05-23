struct DrawArgs
{
    uint IndexCountPerInstance;
    uint InstanceCount;
    uint StartIndexLocation;
    int BaseVertexLocation;
    uint StartInstanceLocation;
};

cbuffer ComputeConstants : register(b0)
{
    uint TotalInstanceCount;
    uint IndexCountPerInstance;
    uint2 padding;
};

RWBuffer<uint> g_DrawArgs : register(u0);

[numthreads(1, 1, 1)]
void main(uint3 id : SV_DispatchThreadID)
{
   // •`‰æˆø”‚ğŒÂ•Ê‚Éİ’è
    g_DrawArgs[0] = IndexCountPerInstance; // IndexCountPerInstance
    g_DrawArgs[1] = TotalInstanceCount; // InstanceCount
    g_DrawArgs[2] = 0; // StartIndexLocation
    g_DrawArgs[3] = 0; // BaseVertexLocation (intŒ^‚¾‚ªuint‚Æ‚µ‚Äˆµ‚¤)
    g_DrawArgs[4] = 0; // StartInstanceLocation
}