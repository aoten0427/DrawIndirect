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
    float4 FrustumCorners[8];
    float4 FrustumPlanes[6];
    uint TotalInstanceCount;
    uint IndexCountPerInstance;
    uint2 padding;
};

struct InstanceData
{
    float4x4 World;
    float4 Extents;
};


StructuredBuffer<InstanceData> g_AllInstances : register(t0);
RWBuffer<uint> g_DrawArgs : register(u0);
RWByteAddressBuffer g_VisibleInstances : register(u1);


bool TestAABBAgainstFrustum(float4 center, float4 extents)
{
    // AABB��8�̒��_���v�Z
    float4 aabbCorners[8];
    
    // �e���_�̌v�Z�i�����p�^�[�����g�p�j
    aabbCorners[0] = float4(center.xyz + float3(-extents.x, -extents.y, -extents.z), 1.0f);
    aabbCorners[1] = float4(center.xyz + float3(extents.x, -extents.y, -extents.z), 1.0f);
    aabbCorners[2] = float4(center.xyz + float3(extents.x, extents.y, -extents.z), 1.0f);
    aabbCorners[3] = float4(center.xyz + float3(-extents.x, extents.y, -extents.z), 1.0f);
    aabbCorners[4] = float4(center.xyz + float3(-extents.x, -extents.y, extents.z), 1.0f);
    aabbCorners[5] = float4(center.xyz + float3(extents.x, -extents.y, extents.z), 1.0f);
    aabbCorners[6] = float4(center.xyz + float3(extents.x, extents.y, extents.z), 1.0f);
    aabbCorners[7] = float4(center.xyz + float3(-extents.x, extents.y, extents.z), 1.0f);
    
    // ���ʕϐ��i�f�t�H���g��1=true�j
    float result = 1.0f;
    
    // �e���ʂɑ΂��ăe�X�g
    for (int p = 0; p < 6; p++)
    {
        float4 plane = FrustumPlanes[p]; // xyz:�@��, w:����
        
        // �e���_�ɑ΂��镽�ʃe�X�g���ʂ�ݐ�
        float insideCount = 0.0f;
        
        for (int c = 0; c < 8; c++)
        {
            // ���ʂ̕�����: dot(normal, point) + distance
            float distance = dot(plane.xyz, aabbCorners[c].xyz) + plane.w;
            
            // distance >= 0 �Ȃ�1�A�����łȂ����0
            insideCount += step(0.0f, distance);
        }
        
        // ��̒��_�������ɂȂ���Ί��S�ɊO�i=0�j�A�����łȂ���Γ����������i=1�j
        float planeResult = step(0.1f, insideCount);
        
        // ���ׂĂ̕��ʂɂ���AND���Z�i1�ł��O�Ȃ�result��0�ɂȂ�j
        result *= planeResult;
    }
    
    return result > 0.5f;
}


// �œK���ŁF�t���X�^���e�X�g
bool OptimizedTestAABBAgainstFrustum(float4 center, float4 extents)
{
    for (int p = 0; p < 6; p++)
    {
        // 1�ł����ʂ̊O���ɂ���΁A�O������
        if (!TestAABBAgainstFrustum(center, extents))
        {
            return false; // ���S�ɊO��
        }
    }
    
    return true; // �����܂��͌���
}

[numthreads(64, 1, 1)]
void main(uint3 id : SV_DispatchThreadID)
{
    float4x4 world = g_AllInstances[id.x].World;
    float4 center = float4(world._14, world._24, world._34,0);
    float4 extents = float4(world._11, world._22, world._33,0);
    extents *= g_AllInstances[id.x].Extents;
    
    bool visible = false;
    
    visible = OptimizedTestAABBAgainstFrustum(center, extents);
    
    if(visible)
    {
         // ���C���X�^���X�̃C���f�b�N�X���擾
        uint index;
        InterlockedAdd(g_DrawArgs[1], 1, index); // InstanceCount�𑝂₷
        
        // ���C���X�^���X�̃��[���h�s����i�[
        uint offset = index * 64; // 16 floats * 4 bytes = 64 bytes per matrix
        
        // �s���]�u���Ċi�[�i���_�V�F�[�_�[�Ŏg�p���邽�߁j
        float4x4 transposed = g_AllInstances[id.x].World;
        
        g_VisibleInstances.Store4(offset + 0, asuint(transposed[0]));
        g_VisibleInstances.Store4(offset + 16, asuint(transposed[1]));
        g_VisibleInstances.Store4(offset + 32, asuint(transposed[2]));
        g_VisibleInstances.Store4(offset + 48, asuint(transposed[3]));
    }
}