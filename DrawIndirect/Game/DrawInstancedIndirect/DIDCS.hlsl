cbuffer MeshInfo : register(b0)
{
    uint indexCount;
    uint startIndexLocation;
    uint baseVertexLocation;
    uint padding;
}

struct OutPut
{
    uint VertexCountPerInstance;
    uint InstanceCount;
    uint StartVertexLocation;
    uint StartInstanceLocation;
};

// ComputeShader.hlsl
RWStructuredBuffer<OutPut> IndirectArgs : register(u0);

[numthreads(1, 1, 1)]
void main(uint3 id : SV_DispatchThreadID)
{
    IndirectArgs[id.x].VertexCountPerInstance = indexCount;
    IndirectArgs[id.x].InstanceCount = 1;
    IndirectArgs[id.x].StartVertexLocation = 0;
    IndirectArgs[id.x].StartInstanceLocation = 0;
}