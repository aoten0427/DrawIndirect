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
    // AABBの8つの頂点を計算
    float4 aabbCorners[8];
    
    // 各頂点の計算（符号パターンを使用）
    aabbCorners[0] = float4(center.xyz + float3(-extents.x, -extents.y, -extents.z), 1.0f);
    aabbCorners[1] = float4(center.xyz + float3(extents.x, -extents.y, -extents.z), 1.0f);
    aabbCorners[2] = float4(center.xyz + float3(extents.x, extents.y, -extents.z), 1.0f);
    aabbCorners[3] = float4(center.xyz + float3(-extents.x, extents.y, -extents.z), 1.0f);
    aabbCorners[4] = float4(center.xyz + float3(-extents.x, -extents.y, extents.z), 1.0f);
    aabbCorners[5] = float4(center.xyz + float3(extents.x, -extents.y, extents.z), 1.0f);
    aabbCorners[6] = float4(center.xyz + float3(extents.x, extents.y, extents.z), 1.0f);
    aabbCorners[7] = float4(center.xyz + float3(-extents.x, extents.y, extents.z), 1.0f);
    
    // 結果変数（デフォルトで1=true）
    float result = 1.0f;
    
    // 各平面に対してテスト
    for (int p = 0; p < 6; p++)
    {
        float4 plane = FrustumPlanes[p]; // xyz:法線, w:距離
        
        // 各頂点に対する平面テスト結果を累積
        float insideCount = 0.0f;
        
        for (int c = 0; c < 8; c++)
        {
            // 平面の方程式: dot(normal, point) + distance
            float distance = dot(plane.xyz, aabbCorners[c].xyz) + plane.w;
            
            // distance >= 0 なら1、そうでなければ0
            insideCount += step(0.0f, distance);
        }
        
        // 一つの頂点も内側になければ完全に外（=0）、そうでなければ内側か交差（=1）
        float planeResult = step(0.1f, insideCount);
        
        // すべての平面についてAND演算（1つでも外ならresultは0になる）
        result *= planeResult;
    }
    
    return result > 0.5f;
}


// 最適化版：フラスタムテスト
bool OptimizedTestAABBAgainstFrustum(float4 center, float4 extents)
{
    for (int p = 0; p < 6; p++)
    {
        // 1つでも平面の外側にあれば、外側判定
        if (!TestAABBAgainstFrustum(center, extents))
        {
            return false; // 完全に外側
        }
    }
    
    return true; // 内側または交差
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
         // 可視インスタンスのインデックスを取得
        uint index;
        InterlockedAdd(g_DrawArgs[1], 1, index); // InstanceCountを増やす
        
        // 可視インスタンスのワールド行列を格納
        uint offset = index * 64; // 16 floats * 4 bytes = 64 bytes per matrix
        
        // 行列を転置して格納（頂点シェーダーで使用するため）
        float4x4 transposed = g_AllInstances[id.x].World;
        
        g_VisibleInstances.Store4(offset + 0, asuint(transposed[0]));
        g_VisibleInstances.Store4(offset + 16, asuint(transposed[1]));
        g_VisibleInstances.Store4(offset + 32, asuint(transposed[2]));
        g_VisibleInstances.Store4(offset + 48, asuint(transposed[3]));
    }
}